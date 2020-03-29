#pragma once

#include <cstdlib>
#include <math.h>
#include <vector>

class Grain
{
public:
    Grain()
        : index(std::rand())
        , offset(std::rand() % 20)
    {
    }

    void addOutput(float* vec, int n)
    {
        for (int i=offset; i<n; ++i, --index)
        {
            if (terminated())
                break;
            vec[i] = 1.0f;
        }

        offset = 0;
    }


    void addOutputExp(float* vec, int n)
    {
        const int i0 = offset;
        const int i1 = (offset + index > n) ? n : offset + index;

        for (int i=i0 ; i<i1; ++i)
        {
            vec[i] = 1.0f;
        }
        
        index -= (n-offset);
        offset = 0;
    }

    bool terminated()
    {
        return (index <= 0);
    }

    int index;
    int offset;
};

int main()
{
    const int n = 1024;

    std::vector<float> output (n, 0.0f);

    int bs = 32;

    Grain grain;

    for (int i=0; i<n; i+=bs)
    {
        grain.addOutputExp(&output[i], bs);
    }

    return 0;
}

