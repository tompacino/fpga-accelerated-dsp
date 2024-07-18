#include <iostream>

#include "cnl/include/cnl/all.h"
#include "filters/lms/lms.h"
#include "filters/leakyIntegrator.h"
#include "AudioFile/AudioFile.h"

using cnl::power;
using cnl::saturated_overflow_tag;
using cnl::scaled_integer;
using cnl::static_integer;
using cnl::neg_inf_rounding_tag;

// using T_VNLMS = scaled_integer<int16_t, power<-14>>;
// using T_LEAKY = scaled_integer<int32_t, power<-30>>;

// using T_VNLMS = static_integer<16, neg_inf_rounding_tag, saturated_overflow_tag, int16_t>;
// using T_LEAKY = static_integer<24, neg_inf_rounding_tag, saturated_overflow_tag, int32_t>;

using T_VNLMS = float;
using T_LEAKY = float;

int main()
{
    // Account for missing bit depth
    float myScalingFactor = 1.0F;

    auto initialStepSize = T_VNLMS{0.000001F};
    auto alpha = T_VNLMS{0.5F};
    auto gamma = T_VNLMS{0.5F};

    auto alphaLeaky = T_LEAKY{0.99F};
    auto minusalphaLeaky = T_LEAKY{0.01F};
    auto initLeaky = T_LEAKY{0.0F};

    LeakyIntegrator leakyRef = LeakyIntegrator<T_LEAKY>(alphaLeaky, minusalphaLeaky, initLeaky);
    LeakyIntegrator leakyOpt = LeakyIntegrator<T_LEAKY>(alphaLeaky, minusalphaLeaky, initLeaky);

    LMS::VSS myFilter = LMS::VSS<T_VNLMS, 1U, true>(initialStepSize, alpha, gamma);

    AudioFile<float> ref;
    AudioFile<float> opt;
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

    err.load("temp/err.wav");
    stp.load("temp/stp.wav");
    anc.load("temp/anc.wav");

    ref.printSummary();
    opt.printSummary();
    err.printSummary();
    stp.printSummary();
    anc.printSummary();

    int channel = 0;
    int numSamples = ref.getNumSamplesPerChannel();

    for (int i = 0; i < numSamples; i++)
    {
        auto refFlt = ref.samples[channel][i] * myScalingFactor;
        auto optFlt = opt.samples[channel][i] * myScalingFactor;

        // Convert to fixed point (32 fraction bits (s1:31))
        auto ref24 = T_LEAKY{refFlt};
        auto opt24 = T_LEAKY{optFlt};
        auto ref24new = leakyRef.step(ref24);
        auto opt24new = leakyOpt.step(opt24);
        ref24 = ref24 - ref24new;
        opt24 = opt24 - opt24new;

        // Convert to 16 bit fixed point for VLMS filter. Adjust scale factor
        refFlt = static_cast<float>(ref24);
        optFlt = static_cast<float>(opt24);
        auto ref16 = T_VNLMS{refFlt};
        auto opt16 = T_VNLMS{optFlt};
        auto err16 = myFilter.step(opt16, ref16);
        auto css16 = myFilter.getStepSize();
        auto anc16 = opt16 - err16;

        // Convert back to float to store in WAV
        anc.samples[channel][i] = float{anc16} / myScalingFactor;
        err.samples[channel][i] = float{err16} / myScalingFactor;
        stp.samples[channel][i] = float{css16} / myScalingFactor;

    }

    err.save("temp/err.wav");
    stp.save("temp/stp.wav");
    anc.save("temp/anc.wav");

    return 0;
}

