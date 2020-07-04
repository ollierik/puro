#pragma once

namespace puro 
{
    struct RelativeAlignment
    {
        RelativeAlignment(int offset, int length) : offset(offset), remaining(length) {}
        int offset;
        int remaining;
    };

    template <typename BufferType, typename AlignmentType>
    std::tuple<BufferType, AlignmentType> alignment_advance_and_crop_buffer(BufferType buffer, AlignmentType alignment) noexcept
    {
        // no operations needed for this block
        if (alignment.offset >= buffer.length())
        {
            alignment.offset -= buffer.length();
            return std::make_tuple(BufferType(), std::move(alignment));
        }

        // grain should begin this block
        if (alignment.offset > 0)
        {
            buffer = puro::buffer_trim_begin(buffer, alignment.offset);
            alignment.offset = 0;
        }

        // restrict range if the sound object should terminate this block
        if (alignment.remaining < buffer.length())
        {
            buffer = puro::buffer_trim_length(buffer, alignment.remaining);
        }

        alignment.remaining -= buffer.length();

        return std::make_tuple(std::move(buffer), std::move(alignment));
    }
}
