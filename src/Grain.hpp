#pragma once

//#include <vector>
//#include "AudioSource.hpp"
//#include "Envelope.hpp"

template <class FloatType>
class GrainTemplate
{
public:

    using Buffer = std::vector<FloatType>;

    GrainTemplate(int offset, int lengthInSamples)
        : offset(offset)
        , lengthInSamples(lengthInSamples)
    {
        std::cout << "*** Create grain *** offset: " << offset << std::endl;
    }

    void addNextOutput(FloatType* vec, int n)
    {
        // Grain created
        if (offset > 0)
        {
            for (int i=offset; i<n; i++)
            {
                vec[i] += 1;
            }
            offset = 0;
        }
        else
        {
            for (int i=0; i < n && index < lengthInSamples; i++, index++)
            {
                vec[i] += 1;
            }
        
        }
    }

    bool terminated()
    {
        return index >= lengthInSamples;
    }


private:
    
    int offset;
    int index = 0;
    const int lengthInSamples;
};
