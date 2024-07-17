#pragma once

template<typename T>
class LeakyIntegrator
{
    T lastSample;
    T alpha;
    T initialValue;
public:
    LeakyIntegrator(T alpha, T initialValue) : lastSample(0), alpha(alpha), initialValue(initialValue){};

    T step(T sample)
    {
        lastSample = alpha * lastSample + (1 - alpha) * sample;
        return lastSample;
    }
};
