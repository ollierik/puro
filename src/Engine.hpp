#pragma once

#include "structs.hpp"
#include "pool.hpp"
#include "audiosource.hpp"

template <typename FloatType, class AudioObjectType>
class Engine
{
public:

    Engine(PlaybackInfo& info)
        : info(info)
        , pool(32)
    {
    }

    /** Adds n samples of output from all Playheads of the engine to the provided buffer */
    void addNextOutput(FloatType* vec)
    {
        for (auto i=0; i<pool.size();  ++i)
        {
            pool[i].addNextOutput(vec);
        }
    }

    bool addGrain(int offsetFromBlockStart, std::unique_ptr<AudioObjectType> grain)
    {
        return pool.add(std::move(grain));
    }

private:

    const PlaybackInfo& info;
    Pool<AudioObjectType> pool;
};
