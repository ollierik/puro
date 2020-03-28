#pragma once

//#include <vector>
//#include "AudioSource.hpp"
//#include "Envelope.hpp"
#include <algorithm>

template <class FloatType>
class GrainTemplate
{
public:

    using Buffer = std::vector<FloatType>;

    GrainTemplate(int offset, int lengthInSamples)
        : length(lengthInSamples)
        , offset(offset)
        , index(0)
    {
        std::cout << "*** Create grain *** offset: " << offset << std::endl;
    }

    void addNextOutput(FloatType* vec, int n)
    {
        // Grain created
        const int start = offset;
        const int num = start + std::min(n, length - index);

        for (int i=start; i<num; ++i)
        {
            vec[i] += 1;
        }

        index += num;
        offset = 0;
    }

    bool terminated()
    {
        return index >= length;
    }

private:
    
    const int length;
    int offset;
    int index;
};
