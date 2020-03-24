#include <iostream>
#include <vector>
#include "engine.hpp"
#include "grain.hpp"


int main ()
{
    std::vector<float> buffer (128, 0);

    PlaybackInfo info (44100, 32);

    Engine<float, Grain<float>> engine (info);

    for (int i=0; i < (int)buffer.size(); i += info.bs)
    {
        engine.addNextOutput(&buffer[i]);
    }

    double sum = 0;
    for (int i=0; i < (int)buffer.size(); ++i)
    {
        std::cout << buffer[i] << std::endl;;
        sum += buffer[i];
    }
    std::cout << "sum: " << sum << std::endl;
}
