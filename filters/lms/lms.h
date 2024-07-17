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
    template<typename DT>
    struct Output
    {
        DT errorSignal;
        DT currentStepSize;
    };

    template<typename DT, std::size_t Taps>
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
            minStepSize = initialStepSize / 10;
            maxStepSize = initialStepSize * 10;
            resetTaps(0);
            resetHistory();
        };

        LMS::Output<DT> step(DT xNext, DT dNext)
        {

            // Compute next sample estimate, error, and power
            DT errorSignal = d_hat[0];
            DT power = 0;

            for (auto idx = 0; idx < Taps; idx++)
            {
                errorSignal -= x_hat[idx] * h_hat[idx];
                power += x_hat[idx] * x_hat[idx];
            }

            // std::cout << "errorSignal " << errorSignal << "\n";
            // std::cout << "power " << power << "\n";

            // Update filter taps based on error
            for (auto idx = (Taps - 1); idx > 0; idx--)
            {
                DT estimator = stepSize * errorSignal * x_hat[idx - 1];
                estimator /= power;
                if (estimator != estimator)
                        estimator = 0;
                h_hat[idx] = h_hat[idx - 1] + estimator;
                x_hat[idx] = x_hat[idx - 1];
                d_hat[idx] = d_hat[idx - 1];
            }
            DT estimator = stepSize * errorSignal * x_hat[0];
            estimator /= power;
            if (estimator != estimator)
                estimator = 0;
            h_hat[0] = h_hat[0] + estimator;
            x_hat[0] = xNext;
            d_hat[0] = dNext;

            // std::cout << "estimator " << estimator << "\n";
            // std::cout << "power " << power << "\n";

            // std::cout << "filterTaps:\n";
            // std::for_each(h_hat.begin(), h_hat.end(), [](auto val)
            //               { std::cout << val << " "; });

            // std::cout << "\nxhat:\n";
            // std::for_each(x_hat.begin(), x_hat.end(), [](auto val)
            //               { std::cout << val << " "; });

            // std::cout << "\n";

            // Update step size
            stepSize = alpha * stepSize + gamma * errorSignal;
            stepSize = std::max(stepSize, minStepSize);
            stepSize = std::min(stepSize, maxStepSize);

            return Output<DT>{errorSignal, stepSize};
        };

        void resetStepSize()
        {
            stepSize = initialStepSize;
        };

        void resetTaps(DT tapValue)
        {
            h_hat.fill(0);
        };

        void resetHistory()
        {
            x_hat.fill(0);
            d_hat.fill(0);
        }
    };
}
