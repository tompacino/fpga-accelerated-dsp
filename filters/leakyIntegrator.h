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
        lastSample = alpha * lastSample + minusAlpha * sample;
        return lastSample;
    }
};
