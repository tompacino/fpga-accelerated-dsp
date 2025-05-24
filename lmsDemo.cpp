#include <iostream>
#include <ranges>
#include <cstdint>
#include <numeric>

#include "AudioFile.h"
#include "cnl/all.h"
#include "gcem.hpp"
#include "gsl/gsl_statistics.h"

#include "filters/lms.h"
#include "filters/leakyIntegrator.h"

using cnl::neg_inf_rounding_tag;
using cnl::power;
using cnl::saturated_overflow_tag;
using cnl::scaled_integer;
using cnl::static_integer;

using T_VNLMS = float;
using T_LEAKY = float;

// using T_VNLMS = scaled_integer<int16_t, power<-14>>;
// using T_LEAKY = scaled_integer<int32_t, power<-30>>;

// using T_VNLMS = static_integer<16, neg_inf_rounding_tag, saturated_overflow_tag, int16_t>;
// using T_LEAKY = static_integer<24, neg_inf_rounding_tag, saturated_overflow_tag, int32_t>;

int main()
{
    // Reference channel lookahead?
    constexpr std::size_t filterTaps = 1U;
    constexpr std::size_t lookahead = (filterTaps == 1U) ? 0U : filterTaps / 2;

    // Account for external gain control on LRB (normalise close to (1, -1)
    constexpr float myScalingFactor = 1.0F;

    // Leaky integrators
    constexpr float alphaLeakyF = 0.999F;
    constexpr float minusalphaLeakyF = 0.001F;
    constexpr float initLeakyF = 0.0F;

    constexpr auto alphaLeaky = T_LEAKY{alphaLeakyF};
    constexpr auto minusalphaLeaky = T_LEAKY{minusalphaLeakyF};
    constexpr auto initLeaky = T_LEAKY{initLeakyF};

    // Fixed point converion values
    constexpr auto alphaLeakyFF = float{alphaLeaky};
    constexpr auto minusalphaLeakyFF = float{minusalphaLeaky};
    constexpr auto initLeakyFF = float{initLeaky};

    LeakyIntegrator leakyRef = LeakyIntegrator<T_LEAKY>(alphaLeaky, minusalphaLeaky, initLeaky);
    LeakyIntegrator leakyOpt = LeakyIntegrator<T_LEAKY>(alphaLeaky, minusalphaLeaky, initLeaky);

    std::cout << "leaky integrator\n"
              << leakyRef << "\n";

    // VSS NLMS
    constexpr float initialStepSizeF = 0.0005F;
    constexpr float epsilonF = static_cast<float>(std::numeric_limits<T_VNLMS>::min());
    constexpr float minStepSizeF = initialStepSizeF / 100.0F;
    constexpr float maxStepSizeF = initialStepSizeF * 100.0F;
    constexpr float alphaF = 0.9F;
    constexpr float gammaF = 0.1F;

    constexpr auto initialStepSize = T_VNLMS{initialStepSizeF};
    constexpr auto epsilon = T_VNLMS{epsilonF};
    constexpr auto minStepSize = T_VNLMS{minStepSizeF};
    constexpr auto maxStepSize = T_VNLMS{maxStepSizeF};
    constexpr auto alpha = T_VNLMS{alphaF};
    constexpr auto gamma = T_VNLMS{gammaF};

    constexpr auto initialStepSizeFF = float{initialStepSize};
    constexpr auto epsilonFF = float{epsilon};
    constexpr auto minStepSizeFF = float{minStepSize};
    constexpr auto maxStepSizeFF = float{maxStepSize};
    constexpr auto alphaFF = float{alpha};
    constexpr auto gammaFF = float{gamma};

    LMS::VSS myFilter = LMS::VSS<T_VNLMS, filterTaps, true>(initialStepSize, alpha, gamma, epsilon, minStepSize, maxStepSize);

    std::cout << "lms filter\n"
              << myFilter << "\n";

    AudioFile<float> ref;
    AudioFile<float> opt;

    AudioFile<float> refL;
    AudioFile<float> optL;

    AudioFile<float> err;
    AudioFile<float> stp;
    AudioFile<float> anc;

    // ref.load("temp/i311_ref_data_dt_32.wav");
    // opt.load("temp/i311_opt_data_dt_32.wav");
    // ref.load("temp/i311_ref_data_ds_100.wav");
    // opt.load("temp/i311_opt_data_ds_100.wav");

    // Downsample
    ref.load("temp/i311_ref_data_ds_100.wav");
    opt.load("temp/i311_opt_data_ds_100.wav");

    // err.load("temp/error_temp_ds_100.wav");
    // stp.load("temp/step_temp_ds_100.wav");
    // anc.load("temp/anc_temp_ds_100.wav");

    // Generated
    // ref.load("temp/i311_ref_data.wav");
    // opt.load("temp/i311_opt_data.wav");

    // Big Dataset
    // ref.load("temp/ADC_CONCAT_1.wav");
    // opt.load("temp/ADC_CONCAT_2.wav");

    refL.load("temp/refL.wav");
    optL.load("temp/optL.wav");

    err.load("temp/err.wav");
    stp.load("temp/stp.wav");
    anc.load("temp/anc.wav");

    ref.printSummary();

    int channel = 0;
    int numSamples = ref.getNumSamplesPerChannel() - lookahead;

    double *optCor = new double[numSamples];
    double *refCor = new double[numSamples];
    double *ancCor = new double[numSamples];

    for (std::size_t idxOpt = 0; idxOpt < numSamples; idxOpt++)
    {
        std::size_t idxRef = idxOpt + lookahead;

        auto refFlt = ref.samples[channel][idxRef] * myScalingFactor;
        auto optFlt = opt.samples[channel][idxOpt] * myScalingFactor;

        refCor[idxRef] = static_cast<double>(refFlt);
        optCor[idxOpt] = static_cast<double>(optFlt);

        // Convert to fixed point (32 fraction bits (s1:31))
        T_LEAKY ref24 = static_cast<T_LEAKY>(refFlt);
        T_LEAKY opt24 = static_cast<T_LEAKY>(optFlt);
        T_LEAKY ref24new = leakyRef.step(ref24);
        T_LEAKY opt24new = leakyOpt.step(opt24);

        refL.samples[channel][idxRef] = float{ref24new} / myScalingFactor;
        optL.samples[channel][idxOpt] = float{opt24new} / myScalingFactor;

        // Subtract average
        ref24 = ref24 - ref24new;
        opt24 = opt24 - opt24new;
        // Skip integration
        // ref24 = ref24;
        // opt24 = opt24;

        // Convert to 16 bit fixed point for VLMS filter. Adjust scale factor
        ref24 = ref24 * myScalingFactor;
        opt24 = opt24 * myScalingFactor;
        refFlt = float{ref24};
        optFlt = float{opt24};
        T_VNLMS ref16 = static_cast<T_VNLMS>(refFlt);
        T_VNLMS opt16 = static_cast<T_VNLMS>(optFlt);
        T_VNLMS err16 = myFilter.step(opt16, ref16);
        T_VNLMS css16 = myFilter.getStepSize();
        T_VNLMS anc16 = opt16 - err16;

        // Convert back to float to store in WAV
        anc.samples[channel][idxOpt] = float{anc16} / myScalingFactor;
        err.samples[channel][idxOpt] = float{err16} / myScalingFactor;
        stp.samples[channel][idxOpt] = float{css16} / myScalingFactor;

        ancCor[idxOpt] = static_cast<double>(float{anc16});
    }

    refL.save("temp/refL.wav");
    optL.save("temp/optL.wav");

    err.save("temp/err.wav");
    stp.save("temp/stp.wav");
    anc.save("temp/anc.wav");

    std::cout << "Pearson correlation OPT & ANC:\n"
              << gsl_stats_correlation(optCor, 1, ancCor, 1, numSamples) << "\n";

    std::cout << "Pearson correlation REF & ANC:\n"
              << gsl_stats_correlation(refCor, 1, ancCor, 1, numSamples) << "\n";

    std::cout << "Pearson correlation OPT & REF:\n"
              << gsl_stats_correlation(optCor, 1, refCor, 1, numSamples) << "\n";

    delete[] optCor;
    delete[] refCor;
    delete[] ancCor;

    return 0;
}
