#pragma once

namespace puro 
{
    struct Range 
    {
        Range(int offset, int length) : offset(offset), remaining(length) {}
        int offset;
        int remaining;
    };

    Range range_advance(Range range, int numSamplesRequested)
    {
        if (range.offset >= numSamplesRequested)
        {
            range.offset -= numSamplesRequested;
        }
        else if (range.offset < numSamplesRequested && range.offset > 0)
        {
            range.remaining -= numSamplesRequested - range.offset;
            range.offset = 0;
        }
        else
        {
            range.remaining -= numSamplesRequested;
        }
        
        return range;
    }

    template <typename BufferType>
    BufferType range_crop_buffer(Range range, BufferType buffer)
    {
        // no operations needed for this block
        if (range.offset >= buffer.size())
        {
            return BufferType();
        }

        if (range.offset > 0)
            buffer = puro::trimmed_begin(buffer, range.offset);

        // restrict range if the sound object should terminate this block
        if (range.remaining < buffer.size())
            buffer = puro::trimmed_length(buffer, range.remaining);

        return buffer;
    }
}
