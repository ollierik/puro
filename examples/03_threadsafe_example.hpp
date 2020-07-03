#include "../src/puro.hpp"

struct Grain
{
    Grain(int offset, int length)
        : bounds(offset, length)
    {}

    puro::Bounds bounds;
};

struct Context
{
    std::vector<float> vec1;
    std::vector<float> vec2;
};

template <typename BufferType, typename ElementType, typename ContextType>
bool process_grain(BufferType dst, ElementType& grain, ContextType& context)
{
	std::tie(dst, grain.bounds) = puro::crop_buffer_and_advance_bounds(dst, grain.bounds);
	
    BufferType tmp1 = puro::wrap_vector<BufferType>(context.vec1, dst.length());
    puro::noise_fill(tmp1);

    BufferType tmp2 = puro::wrap_vector<BufferType>(context.vec2, tmp1.length());
    puro::constant_fill(tmp2, 1.0f);

    BufferType output = puro::trimmed_length(dst, tmp2.length());

    puro::multiply_add(output, tmp1, tmp2);

    return (grain.bounds.remaining <= 0) || (output.length() != dst.length());
}

int main()
{
    using BufferType = puro::Buffer<float, 1>;
    std::vector<float> vec;
    BufferType output = puro::wrap_vector<BufferType>(vec, 256);

    Context context;

    puro::Timer<int> timer (5);

    const int blockSize = 32;

    puro::SafeStack<Grain> inactives;
    puro::NodeStack<Grain> actives;
    puro::SafeStack<Grain> added;
    puro::StackMemoryAllocator<Grain> stackMemory;

    stackMemory.allocateChunk(8, inactives);

    for (int i=0; i<output.length(); i+=blockSize)
    {
        ////////////////////////////////////////////////////////////////
        // HELPER THREAD
        // Add new grains. This could be run from any thread at any time.
        int n = blockSize;
        while (n = timer.advance(n))
        {
            auto* node = inactives.pop_front();
            if (node != nullptr)
            {
                // create grain to the free node, push to the added stack
                auto& e = node->getElement();
                e = Grain(blockSize - n, 4);
                added.push_front(node);
            }
        }
        // End of code that could be run from another thread
        ////////////////////////////////////////////////////////////////



        ////////////////////////////////////////////////////////////////
        // AUDIO THREAD
        // The actual processing that would take place in the high-priority audio processing thread
        puro::Buffer<float, 1> buffer (blockSize, &vec[i]);

        // pop all newly added grains from the atomic stack to the iterable fast stack
        actives.push_multiple(added.pop_all());

        // temporary holder for depleted grains,
        // this is done to reduce the number of atomic CAS operations to two at maximum per block
        puro::NodeStack<Grain> removed;

        for (auto&& it : actives)
        {
            if (process_grain(buffer, it.get(), context))
            {
                // push to temporary NodeStack for speed
                removed.push_front(actives.pop(it));
            }
        }

        // push all removed Grains to inactives in swift swoop
        inactives.push_multiple(removed.pop_all());

        // End of audio processing thread
        ////////////////////////////////////////////////////////////////
    }

    for (auto i=0; i<output.length(); i++)
    {
        std::cout << i << ": " << output(0, i) << std::endl;
    }

    return 0;
}
