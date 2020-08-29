//#include "../examples/01_simple_example.hpp"
//#include "../examples/02_granular_example.hpp"
//#include "../examples/03_threadsafe_example.hpp"
//#include "../tests/nodestack_tests.h"

//#include "../benchmark/rvo_benchmark.hpp"

#include "src/puro.hpp"

int main()
{
    puro::stack_block<2, 64> sb;
    puro::fixed_buffer<2, 64> temp0 (sb);
    puro::linspace_fill(temp0, 0, 64);

    puro::heap_block<float, puro::math::allocator<float>> hb1;
    puro::fixed_buffer<2, 64> temp1 (hb1);
    puro::linspace_fill(temp1, 100, 164);

    puro::heap_block<float, puro::math::allocator<float>> hb2;
    puro::buffer<2> temp2 (64, hb2);
    puro::linspace_fill(temp2, 100, 164);

    puro::print_buffer_table("stack", temp0, "heap1", temp1, "heap2", temp2);
    //puro::print_buffer_table("heap1", temp1, "heap2", temp2);
    //puro::print_buffer_table("stack", temp0, "heap2", temp2);

    /*
    const int num_channels = 4;
    const int num_samples = 8;

    float** ptrs;

    ptrs = new float* [num_channels];

    for (auto ch = 0; ch < num_channels; ++ch)
    {
        ptrs[ch] = new float [num_samples];

        for (int i=0; i<num_samples; ++i)
        {
            ptrs[ch][i] = i + 10.0f;
        }
    }

    for (auto ch = 0; ch < num_channels; ++ch)
    {
        for (int i=0; i<num_samples; ++i)
        {
            std::cout << ptrs[ch][i] << std::endl;
        }
    }
    */


    return 0;
}
