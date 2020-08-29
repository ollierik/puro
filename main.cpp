//#include "../examples/01_simple_example.hpp"
//#include "../examples/02_granular_example.hpp"
//#include "../examples/03_threadsafe_example.hpp"
//#include "../tests/nodestack_tests.h"

//#include "../benchmark/rvo_benchmark.hpp"

#include "src/puro.hpp"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

void do_stuff()
{
    puro::stack_block<2, 64> sb;
    puro::fixed_buffer<2, 64> temp0 (sb);
    puro::linspace_fill(temp0, 0, 64);

    puro::heap_block<float, puro::math::allocator<float>> hb1;
    puro::fixed_buffer<2, 64> temp1 (hb1);
    puro::linspace_fill(temp1, 100, 164);

    puro::heap_block<float, puro::math::allocator<float>> hb2;
    puro::buffer<2> temp2 (64, hb2);
    puro::linspace_fill(temp2, 200, 264);

    puro::print_buffer_table("stack", temp0, "heap1", temp1, "heap2", temp2);

    puro::heap_block_pool<float, puro::math::allocator<float>> pool;

    const puro::fixed_buffer<2, 60> temp3 (pool);
    const puro::buffer<2> temp4 (61, pool);
    const puro::buffer<1> temp5 (62, pool);

    puro::linspace_fill(temp3, 300, 360);
    puro::linspace_fill(temp4, 400, 461);
    puro::linspace_fill(temp5, 500, 562);

    puro::print_buffer_table("pool1", temp3, "pool2", temp4, "pool3", temp5);
}

int main()
{
    do_stuff();

    _CrtDumpMemoryLeaks();

    return 0;
}
