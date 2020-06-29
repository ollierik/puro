#pragma once

namespace engine
{
    struct Ranges
    {
        Ranges(int offset, int length) : offset(offset), remaining(length) {}
        int offset;
        int remaining;
    };

    Ranges ranges_advance(Ranges ranges, int numSamplesRequested)
    {
        if (ranges.offset >= numSamplesRequested)
        {
            ranges.offset -= numSamplesRequested;
        }
        else if (ranges.offset < numSamplesRequested && ranges.offset > 0)
        {
            ranges.remaining -= numSamplesRequested - ranges.offset;
            ranges.offset = 0;
        }
        else
        {
            ranges.remaining -= numSamplesRequested;
        }
        
        return ranges;
    }

    template <typename BufferType>
    BufferType ranges_crop_buffer(const Ranges ranges, const BufferType& buffer)
    {
        // no operations needed for this block
        if (ranges.offset >= buffer.size())
        {
            return BufferType();
        }

        BufferType cropped = buffer;

        if (ranges.offset > 0)
            cropped = puro::trimmed_begin(cropped, ranges.offset);

        // restrict range if the sound object should terminate this block
        if (ranges.remaining < cropped.size())
            cropped = puro::trimmed_length(cropped, ranges.remaining);

        return cropped;
    }
}
