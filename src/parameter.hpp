#pragma once

#include <random>
#include <type_traits>

template <typename FloatType, typename ValueType=FloatType>
class GaussianParameter
{
public:
    GaussianParameter(FloatType mean, FloatType deviation)
        : generator(std::random_device()()), mean(mean), dev(deviation)
    {
        initialiseDistribution();
    }

    ValueType get()
    {
        if (std::is_integral<ValueType>::value)
        {
            return static_cast<ValueType>(round(distribution(generator)));
        }
        return static_cast<ValueType>(distribution(generator));
    }

    void setMean(FloatType value) { mean = value; }
    void setDeviation(FloatType value) { dev = value; }

    std::mt19937 generator;
    std::normal_distribution<FloatType> distribution;
    
private:

    void initialiseDistribution()
    {
        distribution = std::normal_distribution<FloatType> {mean, dev};
    }

    FloatType mean;
    FloatType dev;
};

