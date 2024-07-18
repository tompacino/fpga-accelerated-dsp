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
        T err;
        T currentStepSize;
    };

    template<typename T, std::size_t Taps, bool Normalised>
    class VSS
    {

        T stepSize;
        T alpha;
        T gamma;

        T epsilon;

        T minStep;
        T maxStep;

        T curStep;

        std::array<T, Taps> x_hat;
        std::array<T, Taps> h_hat;

        T err;

    public:
        VSS(T stepSize, T alpha, T gamma) : VSS(stepSize, alpha, gamma, stepSize / 100, stepSize / 100, stepSize * 100){};

        VSS(T stepSize, T alpha, T gamma, T epsilon, T minStep, T maxStep) :
            stepSize(stepSize), alpha(alpha), gamma(gamma), epsilon(epsilon), minStep(minStep), maxStep(maxStep), curStep(stepSize) {
            x_hat.fill(0);
            h_hat.fill(0);
        }

        LMS::Output<T> step(T xNxt, T dNxt)
        {
            // Compute next sample estimate, error, and power
            T est = 0;
            T pow = epsilon; // To avoid divide by zero
            for (std::size_t idx = (Taps - 1); idx > 0; idx--)
            {
                x_hat[idx] = x_hat[idx - 1];
                est += h_hat[idx] * x_hat[idx];
                pow += x_hat[idx] * x_hat[idx];
            }
            x_hat[0] = xNxt;
            est += h_hat[0] * x_hat[0];
            pow += x_hat[0] * x_hat[0];
            err = dNxt - est;

            // Update filter taps based on error
            T estimator = curStep * err;
            for (std::size_t idx = 0; idx < Taps; idx++)
            {
                T tapDelta = estimator * x_hat[idx];
                if (Normalised)
                    tapDelta = normalise(tapDelta, pow);
                h_hat[idx] += tapDelta;
            }

            // Update step size
            curStep = alpha * curStep + gamma * err * x_hat[0];
            curStep = std::max(curStep, minStep);
            curStep = std::min(curStep, maxStep);

            return Output<T>{err, curStep};
        };

        LMS::Output<T> last()
        {
            return Output<T>{err, curStep};
        }

        T normalise(T tapDelta, T pow) {
            if (pow == 0)
            {
                return tapDelta;
            }

            if constexpr (std::is_same<T, float>::value)
            {
                if (tapDelta != tapDelta)  // Check for float NaN
                    return 0;
            }

            tapDelta /= pow;

            return tapDelta;
        };

        void resetStepSize()
        {
            curStep = stepSize;
        };

    };
}
