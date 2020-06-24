#include <vector>
#include "buffer.h"
#include "pool.h"
#include <iostream>

struct Grain
{
    int x;
};

struct Context
{
    std::vector<float> temp1;
    std::vector<float> temp2;
};


template <typename F, typename E, typename C>
bool process_grain(const Buffer<F>& buffer, E& grain, C& context)
{
    std::cout << grain.x << std::endl;

    /*
	Buffer local = crop_buffer(buffer, grain.offset, grain.remaining);
	
	Buffer temp1 (context.temp1, local.chs, local.n);    
	fill_interpolated(temp1, grain.source, grain.rate);

	Buffer temp2 = Buffer(context.temp2, temp1.chs, temp1.n);
	fill(temp2, grain.envelope);

	trim_buffer(local, temp2.n);

	multiply_add(buffer, temp1, temp2);

	advance_offset_and_remaining(grain);
	return (buffer.n != local.n);
    */
    return grain.x == 4;
}

int main()
{
    std::vector<float> vec(64, 0.0f);
    Buffer<float> buffer {1, 32, vec.data()};

    Context context;

    const int blockSize = 32;

    Pool<Grain> pool;
    for (int i=0; i<8; i++)
    {
        pool.push(Grain{i});
    }

    for (auto&& it : pool)
    {
        if (process_grain(buffer, it.get(), context))
        {
            pool.pop(it);
        }
    }

    Grain* added = nullptr;
    int n = blockSize;
    while (scheduler.tick(n))
    {
        auto& g = pool.push(Grain{10});
        process_grain(buffer, g, context);
    }

    return 0;
}

