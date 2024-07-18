#pragma once

#include <array>
#include <algorithm>

namespace LMS
{
    template<typename T, std::size_t Taps, bool Normalised>
    class FSS
    {
    protected:
        T stepSize;
        T epsilon;
        T err;

        std::array<T, Taps> x_hat;
        std::array<T, Taps> h_hat;

    public:

        FSS(T stepSize) : FSS(stepSize, stepSize / 100){};

        FSS(T stepSize, T epsilon) : stepSize(stepSize), epsilon(epsilon)
        {
            x_hat.fill(0);
            h_hat.fill(0);
        }

        virtual T step(T xNxt, T dNxt)
        {
            T pow = computeNext(xNxt, dNxt);
            updateFilter(pow);
            return err;
        };

        T last()
        {
            return err;
        }

    protected:
        T computeNext(T xNxt, T dNxt)
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
            return pow;
        }

        void updateFilter(T pow)
        {
            // Update filter taps based on error
            T estimator = stepSize * err;
            for (std::size_t idx = 0; idx < Taps; idx++)
            {
                T tapDelta = estimator * x_hat[idx];
                if (Normalised)
                    tapDelta = normalise(tapDelta, pow);
                h_hat[idx] += tapDelta;
            }
        }

        T normalise(T tapDelta, T pow)
        {
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
    };

    template<typename T, std::size_t Taps, bool Normalised>
    class VSS : public FSS<T, Taps, Normalised>
    {
    protected:
        using FSS<T, Taps, Normalised>::stepSize;
        using FSS<T, Taps, Normalised>::err;
        using FSS<T, Taps, Normalised>::x_hat;

        T alpha;
        T gamma;

        T minStep;
        T maxStep;

        T intStep;

    public:
        VSS(T stepSize, T alpha, T gamma) :
            VSS(stepSize, alpha, gamma, stepSize / 100, stepSize / 100, stepSize * 100){};

        VSS(T stepSize, T alpha, T gamma, T epsilon, T minStep, T maxStep) :
            FSS<T, Taps, Normalised>(stepSize, epsilon), alpha(alpha), gamma(gamma), minStep(minStep), maxStep(maxStep), intStep(stepSize){};

        T step(T xNxt, T dNxt)
        {
            T pow = this->computeNext(xNxt, dNxt);
            this->updateFilter(pow);

            // Update step size
            stepSize = alpha * stepSize + gamma * err * xNxt;
            stepSize = std::max(stepSize, minStep);
            stepSize = std::min(stepSize, maxStep);

            return err;
        };

        T getStepSize()
        {
            return stepSize;
        }

        T resetStepSize()
        {
            stepSize = intStep;
            return stepSize;
        };

    };
}
