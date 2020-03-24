#pragma once

#include <vector>

#include "audiosource.hpp"
#include "envelope.hpp"
#include "structs.hpp"

template <class FloatType>
class AudioObject
{
public:

    virtual void addNextOutput(FloatType* vec) = 0;

protected:

    const PlaybackInfo& info;
};
