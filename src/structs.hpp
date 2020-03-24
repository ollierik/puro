#pragma once

#include <memory>

class PlaybackInfo
{
public:

    PlaybackInfo(int sampleRate, int blockSize)
        : sr(sampleRate)
        , bs(blockSize)
    {}

    int getSampleRate() const { return sr; }
    int getBlockSize() const { return bs; }

    //PlaybackInfo(PlaybackInfo const &) = delete;

    int sr;
    int bs;
};
