#include <iostream>

#include "filters/lms/lms.h"
#include "filters/leakyIntegrator.h"
#include "AudioFile/AudioFile.h"

int main()
{
    float myScalingFactor = 10;

    float initialStepSize = 0.005F;
    float alpha = 0.99F;
    float gamma = 0.01F;

    LeakyIntegrator leakyRef = LeakyIntegrator<float>(0.99F, 0.0F);
    LeakyIntegrator leakyOpt = LeakyIntegrator<float>(0.99F, 0.0F);

    LMS::VSS myFilter = LMS::VSS<float, 1U>(initialStepSize, alpha, gamma);

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

    // Normalise


    err.load("temp/error_temp_ds_100.wav");
    stp.load("temp/step_temp_ds_100.wav");
    anc.load("temp/anc_temp_ds_100.wav");

    ref.printSummary();
    opt.printSummary();
    err.printSummary();
    stp.printSummary();
    anc.printSummary();

    int channel = 0;
    int numSamples = ref.getNumSamplesPerChannel();

    for (int i = 0; i < numSamples; i++)
    {
        auto refSample = myScalingFactor * ref.samples[channel][i];
        auto optSample = myScalingFactor * ref.samples[channel][i];

        refSample = refSample - leakyRef.step(refSample);
        optSample = optSample - leakyOpt.step(optSample);

        auto [errorSignal, currentStepSize] = myFilter.step(refSample, optSample);

        auto ancSample = optSample - errorSignal;
        anc.samples[channel][i] = ancSample / myScalingFactor;
        err.samples[channel][i] = errorSignal / myScalingFactor;
        stp.samples[channel][i] = currentStepSize / myScalingFactor;

    }

    err.save("temp/error_out_ds_100.wav");
    stp.save("temp/step_out_ds_100.wav");
    anc.save("temp/anc_out_ds_100.wav");

    return 0;
}

