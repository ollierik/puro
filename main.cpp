//#include "../examples/01_simple_example.hpp"
//#include "../examples/02_granular_example.hpp"
//#include "../examples/03_threadsafe_example.hpp"
//#include "../tests/nodestack_tests.h"

//#include "../benchmark/rvo_benchmark.hpp"

#include "src/puro.hpp"
#include "include/pffft.h"

#include <iomanip>

int fft_test()
{
    int N = 64;

    std::vector<float, puro::math::Allocator<float>> input (N, 0);
    std::vector<float, puro::math::Allocator<float>> output (N, 0);
    std::vector<float, puro::math::Allocator<float>> reco (N, 0);

    input[0] = 1;
    input[1] = -1;
    
    puro::math::FFT fft(N);
    
    for (int i=0; i < N; ++i)
    {
        std::cout << i << ":\t" << input[i] << std::endl;
    }
    
    std::cout << "----------------------------------\n";

    fft.rfft(output.data(), input.data());
    
    std::cout << std::setprecision(4) << std::fixed;

    std::cout << 0 << ":\t" << std::abs(output[0]) << std::endl;

    for (int i=2; i < N; i+=2)
    {
        std::cout << i/2 << ":\t" << std::hypot(output[i], output[i+1]) << std::endl;
    }
    
    std::cout << N/2 << ":\t" << std::abs(output[1]) << std::endl;
    
    std::cout << "----------------------------------\n";
    
    fft.irfft(reco.data(), output.data());
    
    for (int i=0; i < N; ++i)
    {
        std::cout << i << ":\t" << reco[i] << std::endl;
    }
    
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    
    N *= 2;
    puro::math::FFT fft2 (N);

    input.resize(N, 0);
    output.resize (N, 0);
    reco.resize (N, 0);
    
    input[0] = 1;
    input[1] = -1;

    for (int i=0; i < N; ++i)
    {
        std::cout << i << ":\t" << input[i] << std::endl;
    }
    
    std::cout << "----------------------------------\n";
    
    fft2.rfft(output.data(), input.data());
    
    std::cout << std::setprecision(4) << std::fixed;
    
    std::cout << 0 << ":\t" << std::abs(output[0]) << std::endl;
    
    for (int i=2; i < N; i+=2)
    {
        std::cout << i/2 << ":\t" << std::hypot(output[i], output[i+1]) << std::endl;
    }
    
    std::cout << N/2 << ":\t" << std::abs(output[1]) << std::endl;
    
    std::cout << "----------------------------------\n";
    
    fft2.irfft(reco.data(), output.data());
    
    for (int i=0; i < N; ++i)
    {
        std::cout << i << ":\t" << reco[i] << std::endl;
    }
    
    return 0;
}




int main()
{
    std::vector<float> v0;
    std::vector<float> v1;
    std::vector<float> v2;
    
    auto b0 = puro::buffer_wrap_vector<puro::Buffer<float, 2>> (v0, 10);
    auto b1 = puro::buffer_wrap_vector<puro::Buffer<float, 1>> (v1, 20);
    auto b2 = puro::buffer_wrap_vector<puro::Buffer<float, 2>> (v2, 5);
    
    puro::linspace_fill(b0, 0.123f, 1.0f);
    puro::linspace_fill(b1, 100.4567f, 1.0f);
    puro::linspace_fill(b2, 1000.1f, 1.0f);

    puro::print_buffer_table("main", b0, "side", b1, "aux", b2);
}
