#pragma once

#include <array>

// TODO:
/*
    - Loop unrolling
    - Custom bounds for step size
*/

namespace LMS
{
    template<typename DT>
    struct Debug
    {
        DT errorSignal;
        DT currentStepSize;
    };

    template<typename DT, std::size_t Taps, bool OutputDebug>
    class VSS
    {
        DT initialStepSize;
        DT stepSize;
        DT alpha;
        DT gamma;

        DT minStepSize;
        DT maxStepSize;

        std::size_t idxSample;

        std::array<DT, Taps> x_hat;
        std::array<DT, Taps> d_hat;
        std::array<DT, Taps> h_hat;

    public:
        VSS(DT initialStepSize, DT alpha, DT gamma) : initialStepSize(initialStepSize), stepSize(initialStepSize), alpha(alpha), gamma(gamma)
        {
            minStepSize = stepSize / 100;
            maxStepSize = stepSize * 100;
            resetTaps(0);
            resetHistory();
        };

        DT step(DT xNext, DT dNext)
        {

            std::size_t idxStop = idxSample + 1;
            if (idxStop >= Taps)
            {
                idxStop = 0;
            }

            // Compute next sample estimate, error, and power
            DT errorSignal = d_hat(0);
            DT power = 0;

            for (auto idx = 0; idx < Taps; idx++)
            {
                errorSignal -= x_hat(idx) * h_hat(idx);
                power += x_hat(idx) * x_hat(idx);
            }

            // Update filter taps based on error
            for (auto idx = Taps - 1; idx > 0; idx++)
            {
                DT estimator = stepSize * errorSignal * x_hat(idx);
                estimator /= power;
                h_hat(idx) = h_hat(idx) + estimator;
                x_hat(idx) = x_hat(idx - 1);
                d_hat(idx) = d_hat(idx - 1);
            }
            DT estimator = stepSize * errorSignal * x_hat(0);
            estimator /= power;
            h_hat(0) = h_hat(0) + estimator;
            x_hat(0) = xNext;
            d_hat(0) = dNext;

            // Update step size
            stepSize = alpha * stepSize + gamma * errorSignal;

            if (OutputDebug)
            {
                return Debug<DT>(errorSignal, stepSize);
            }
            else
            {
                return errorSignal;
            }
        };

        void resetStepSize()
        {
            stepSize = initialStepSize;
        };

        void resetTaps(DT tapValue)
        {
            h_hat.fill(tapValue);
        };

        void resetHistory()
        {
            x_hat.fill(0);
            d_hat.fill(0);
        }
    };
}
