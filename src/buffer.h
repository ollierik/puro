#pragma once

#include <array>

template <typename F, int MAXCHS=1>
struct Buffer
{
    int chs;
    int n;
    std::array<F*, MAXCHS> data;
};


template <typename F, int MAXCHS>
Buffer<F, MAXCHS> crop_buffer(const Buffer<F, MAXCHS> buffer, int offset, int remaining)
{
    if (offset >= buffer.n)
    {
        return Buffer<F, MAXCHS>(0, 0, nullptr);
    }

    // local copy for possible mutating
    Buffer<F, MAXCHS> cropped = buffer;

    if (offset > 0)
        trim_begin(cropped, offset);

    // restrict range if the sound object should terminate this block
    if (remaining < buffer.n)
        trim_length(buffer, remaining);

    return cropped;
}
