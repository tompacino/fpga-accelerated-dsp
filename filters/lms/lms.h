#pragma once

#include <array>
#include <algorithm>
#include <ranges>

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

        T last() const
        {
            return err;
        }

        T getStepSize() const
        {
            return stepSize;
        }

        void serialiseX_hat(std::ostream &os) const
        {
            os << "x_hat: [";
            std::ranges::for_each(x_hat, [&](const auto &tap)
                                { os << tap << ", "; });
            os << " ]";
        }

        void serialiseH_hat(std::ostream &os) const
        {
            os << "h_hat: [";
            std::ranges::for_each(h_hat, [&](const auto &tap)
                                { os << tap << ", "; });
            os << "]";
        }

        template<typename TT, std::size_t TTaps, bool TNormalised>
        friend std::ostream& operator<<(std::ostream &os, const FSS<TT, TTaps, TNormalised> &fss);

    protected:
        // Compute next sample estimate, error, and return power
        T computeNext(T xNxt, T dNxt)
        {

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
                if constexpr (Normalised)
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
            T alphaStepSize = alpha * stepSize;
            T gammaStepSize = gamma * err * xNxt;
            stepSize = alphaStepSize + gammaStepSize;
            stepSize = std::max(stepSize, minStep);
            stepSize = std::min(stepSize, maxStep);

            return err;
        };

        T resetStepSize()
        {
            stepSize = intStep;
            return stepSize;
        };

        template<typename TT, std::size_t TTaps, bool TNormalised>
        friend std::ostream& operator<<(std::ostream &os, const VSS<TT, TTaps, TNormalised> &vss);

    };

    template<typename T, std::size_t Taps, bool Normalised>
    std::ostream& operator<<(std::ostream &os, const FSS<T, Taps, Normalised> &fss)
    {
        os << "stepSize :\t" << fss.stepSize << "\n";
        os << "epsilon  :\t" << fss.epsilon << "\n";
        os << "err      :\t" << fss.err;
        return os;
    };

    template<typename T, std::size_t Taps, bool Normalised>
    std::ostream& operator<<(std::ostream &os, const VSS<T, Taps, Normalised> &vss)
    {
        os << static_cast<const FSS<T, Taps, Normalised>&>(vss) << "\n";
        os << "alpha  :\t" << vss.alpha << "\n";
        os << "gamma  :\t" << vss.gamma << "\n";
        os << "minstep:\t" << vss.minStep << "\n";
        os << "maxStep:\t" << vss.maxStep;
        return os;
    };
}
