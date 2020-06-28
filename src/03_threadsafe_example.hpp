#include "puro.hpp"
#include "safepool.hpp"

struct Scheduler
{
    int tick(int numSamples)
    {
        counter += numSamples;

        if (counter <= interval)
            return 0;

        numSamples = counter - interval;
        counter = 0;

        return numSamples;
    }

    int interval = 20;
    int counter = 0;
};

struct Grain
{
    Grain(int offset, int length)
        : ranges(offset, length)
        //, envelope(length, false)
        , envelope()
        , source()
    {}

    engine::Ranges ranges;
    //HannEnvelope<float> envelope;
    ConstSource<float> envelope;
    ConstSource<float> source;
};

struct Context
{
    std::vector<float> temp1;
    std::vector<float> temp2;
};

template <typename BufferType, typename ElementType, typename ContextType>
bool process_grain(const BufferType& buffer, ElementType& grain, ContextType& context)
{
	BufferType output = engine::ranges_crop_buffer(grain.ranges, buffer);
	
    BufferType temp1 = bops::fit_vector<BufferType>(context.temp1, output.size());
    bops::filled_from_source(temp1, grain.source);

    BufferType temp2 = bops::fit_vector<BufferType>(context.temp2, temp1.size());
    bops::filled_from_source(temp2, grain.envelope);

    output = bops::trimmed_length(output, temp2.size());

    bops::multiply_add(output, temp1, temp2);

    grain.ranges = engine::ranges_advance(grain.ranges, buffer.size());

    return (grain.ranges.remaining <= 0);
}

int main()
{
    using BufferType = Buffer<float, 1>;
    std::vector<float> vec;
    BufferType output = bops::fit_vector<BufferType>(vec, 256);

    Context context;

    Scheduler scheduler;

    const int blockSize = 32;

    SafeStack<Grain> inactive;
    NodeStack<Grain> active;
    SafeStack<Grain> added;

    for (int i=0; i<4; i++)
    {
        inactive.push_front(new Node<Grain>());
    }

    for (int i=0; i<output.size(); i+=blockSize)
    {
        ////////////////////////////////////////////////////////////////
        // Add new grains. This could be run from any thread at any time.
        int n = blockSize;
        while (n = scheduler.tick(n))
        {
            auto* node = inactive.pop_front();
            if (node != nullptr)
            {
                // create grain to the free node, push to the added stack
                auto& e = node->getElement();
                e = Grain(blockSize - n, 10);
                added.push_front(node);
            }
        }
        // End of code that could be in other threads
        ////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////
        // The actual processing that would take place in the high-priority audio processing thread
        Buffer<float, 1> buffer (blockSize, &vec[i]);

        // pop all newly added grains from the atomic stack to the fast stack
        active.push_multiple(added.pop_all());

        // temporary holder for depleted grains,
        // this is done to reduce the number of atomic CAS operations to two at maximum per block
        NodeStack<Grain> removed;

        for (auto&& it : active)
        {
            if (process_grain(buffer, it.get(), context))
            {
                // push to temporary NodeStack for speed
                removed.push_front(active.pop(it));
            }
        }

        // push all removed Grains to inactives in one go
        inactive.push_multiple(removed.pop_all());

        // End of audio processing thread
        ////////////////////////////////////////////////////////////////
    }

    for (int i=0; i<vec.size(); i++)
    {
        std::cout << i << ": " << vec[i] << std::endl;
    }

    return 0;
}
