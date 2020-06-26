#pragma once

namespace engine
{
    struct Ranges
    {
        Ranges(int offset, int length) : offset(offset), remaining(length) {}
        int offset;
        int remaining;
    };


    void ranges_advance(Ranges& ranges, int numSamplesRequested)
    {
        if (ranges.offset >= numSamplesRequested)
        {
            ranges.offset -= numSamplesRequested;
            return;
        }
        else if (ranges.offset < numSamplesRequested && ranges.offset > 0)
        {
            ranges.remaining -= numSamplesRequested - ranges.offset;
            ranges.offset = 0;
            return;
        }
        
        ranges.remaining -= numSamplesRequested;
    }

    template <typename BufferType>
    BufferType ranges_crop_buffer(const engine::Ranges& ranges, const BufferType& buffer)
    {
        // no operations needed for this block
        if (ranges.offset >= buffer.size())
        {
            return BufferType();
        }

        BufferType cropped = buffer;

        if (ranges.offset > 0)
            cropped.trimBegin(ranges.offset);

        // restrict range if the sound object should terminate this block
        if (ranges.remaining < cropped.size())
            cropped.trimLength(ranges.remaining);

        return cropped;
    }


}
