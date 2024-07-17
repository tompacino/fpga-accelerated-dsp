#pragma once

#include <array>
#include <algorithm>

// TODO:
/*
    - Loop unrolling
    - Custom bounds for step size
*/

namespace LMS
{
    template<typename T>
    struct Output
    {
        T errorSignal;
        T currentStepSize;
    };

    template<typename T, std::size_t Taps>
    class VSS
    {

        T initialStepSize;
        T stepSize;
        T alpha;
        T gamma;

        T minStepSize;
        T maxStepSize;

        std::size_t idxSample;

        std::array<T, Taps> x_hat;
        std::array<T, Taps> h_hat;

    public:
        VSS(T initialStepSize, T alpha, T gamma) : initialStepSize(initialStepSize), stepSize(initialStepSize), alpha(alpha), gamma(gamma)
        {
            minStepSize = initialStepSize / 10;
            maxStepSize = initialStepSize * 10;
            x_hat.fill(0);
            h_hat.fill(0);
        };

        LMS::Output<T> step(T xNext, T dNext)
        {

            // Compute next sample estimate, error, and power
            T errorSignal = dNext;
            T power = 0;

            for (auto idx = 0; idx < Taps; idx++)
            {
                errorSignal -= x_hat[idx] * h_hat[idx];
                power += x_hat[idx] * x_hat[idx];
            }

            // Update filter taps based on error
            T estimator = stepSize * errorSignal;
            for (auto idx = (Taps - 1); idx > 0; idx--)
            {
                T tapDelta = estimator * x_hat[idx];
                tapDelta /= power;
                if (tapDelta != tapDelta)
                        tapDelta = 0;
                h_hat[idx] = h_hat[idx] + tapDelta;
                x_hat[idx] = x_hat[idx - 1];
            }
            T tapDelta = estimator * x_hat[0];
            tapDelta /= power;
            if (tapDelta != tapDelta)
                tapDelta = 0;
            h_hat[0] = h_hat[0] + tapDelta;

            // Store latest samples
            x_hat[0] = xNext;

            // Update step size
            stepSize = alpha * stepSize + gamma * errorSignal;
            stepSize = std::max(stepSize, minStepSize);
            stepSize = std::min(stepSize, maxStepSize);

            return Output<T>{errorSignal, stepSize};
        };

        void resetStepSize()
        {
            stepSize = initialStepSize;
        };

    };
}
