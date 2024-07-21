#pragma once

template<typename T>
class LeakyIntegrator
{
    T alpha;
    T minusAlpha;
    T lastSample;
public:
    LeakyIntegrator(T alpha, T minusAlpha, T initialValue) : alpha(alpha), minusAlpha(minusAlpha), lastSample(initialValue) {};

    T step(T sample)
    {
        T alphaTerm = alpha * lastSample;
        T minusTerm = minusAlpha * sample;
        lastSample = alphaTerm + minusTerm;

        return lastSample;
    }

    T stepV(T sample)
    {
        T alphaTerm = alpha * lastSample;
        T minusTerm = minusAlpha * sample;
        lastSample = alphaTerm + minusTerm;

        std::cout << "sample    : " << sample << "\n";
        std::cout << "alphaTerm : " << alphaTerm << "\n";
        std::cout << "minusTerm : " << minusTerm << "\n";
        std::cout << "lastSample: " << lastSample << "\n";

        return lastSample;
    }

    T last() const
    {
        return lastSample;
    }

    template<typename TT>
    friend std::ostream& operator<<(std::ostream& os, const LeakyIntegrator<TT>& li);

};

template<typename T>
std::ostream& operator<<(std::ostream& os, const LeakyIntegrator<T>& li)
{
    os << "alpha        :\t" << li.alpha << "\n";
    os << "minusAlpha   :\t" << li.minusAlpha << "\n";
    os << "lastSample   :\t" << li.lastSample;
    return os;
}
