#pragma once

#include <iostream>
#include <type_traits>
#include <vector>

namespace math {
    
template <typename T>
void multiply(T* dst, T* src, const int n)
{
    for (auto i=0; i<n; ++i)
        dst[i] *= src[i];
}
    
template <typename T>
void multiply(T* dst, T* src1, T* src2, const int n)
{
    for (auto i=0; i<n; ++i)
        dst[i] = src1[i] * src2[i];
}

template <typename T>
void add(T* dst, T* src, const int n)
{
    for (auto i=0; i<n; ++i)
        dst[i] += src[i];
}
    
template <typename T>
void add(T* dst, T* src1, T* src2, const int n)
{
    for (auto i=0; i<n; ++i)
        dst[i] = src1[i] + src2[i];
}

} // namespace math

template <typename BufferType>
void multiply (BufferType& dst, BufferType& src)
{
    for (auto ch=0; ch < dst.num_channels(); ++ch)
    {
        math::multiply(dst.channel(ch), src.channel(ch), dst.length());
    }
}

template <typename BufferType>
void multiply (BufferType& dst, BufferType& src1, BufferType& src2)
{
    for (auto ch=0; ch < dst.num_channels(); ++ch)
    {
        math::multiply(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.length());
    }
}

template <typename BufferType>
void add (BufferType& dst, BufferType& src)
{
    for (auto ch=0; ch < dst.num_channels(); ++ch)
    {
        math::add(dst.channel(ch), src.channel(ch), dst.length());
    }
}

template <typename BufferType>
void add (BufferType& dst, BufferType& src1, BufferType& src2)
{
    for (auto ch=0; ch < dst.num_channels(); ++ch)
    {
        math::add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.length());
    }
}


struct is_operation {};

template <typename A, typename B>
struct product_op : is_operation
{
    product_op(A& a, B& b) : a(a), b(b) {}
    
    template <typename BufferType>
    void perform(BufferType& buffer)
    {
        constexpr bool a_is_op = std::is_base_of<is_operation, A>::value;
        constexpr bool b_is_op = std::is_base_of<is_operation, B>::value;
        
        /// Cannot operate between two operations, i.e. for example (a * b) * (c * d)
        /// Commutativity not implemented, operations should be broken up to get around this.
        static_assert(!(a_is_op && b_is_op), "Cannot operate between two operations.");

        if constexpr (a_is_op)
        {
            a.perform(buffer);
            multiply(buffer, b);
            return;
        }
        else if constexpr (b_is_op)
        {
            b.perform(buffer);
            multiply(buffer, a);
            return;
        }
        else
        {
            multiply(buffer, a, b);
        }
    }
    
    A& a;
    B& b;
};

template <typename A, typename B>
struct plus_op : is_operation
{
    plus_op(A& a, B& b) : a(a), b(b) {}
    
    template <typename BufferType>
    void perform(BufferType& buffer)
    {
        constexpr bool a_is_op = std::is_base_of<is_operation, A>::value;
        constexpr bool b_is_op = std::is_base_of<is_operation, B>::value;
        
        static_assert(!(a_is_op && b_is_op), "error");
        
        if constexpr (a_is_op)
        {
            a.perform(buffer);
            add(buffer, b);
            //*ptr += b.x; // todo
            return;
        }
        else if constexpr (b_is_op)
        {
            b.perform(buffer);
            //*ptr += a.x; // todo
            add(buffer, a);
            return;
        }
        else
        {
            //*ptr = a.x + b.x; // todo
            add(buffer, a, b);
        }
    }
    
    A& a;
    B& b;
};


template <int NumChannels = 1, typename T = float>
struct buffer
{
    T* ptrs [NumChannels];
    int num_samples = 0;
    
    constexpr static int num_channels() { return NumChannels; }
    
    buffer() {}
    buffer(std::vector<T>& vec) : num_samples((int)vec.size())
    {
        ptrs[0] = vec.data();
    }

    template<typename A, typename B, template<class, class> typename Op>
    buffer& operator= (Op<A, B> op)
    {
        op.perform(*this);
        return *this;
    }
    
    T* channel(int ch)
    {
        return ptrs[ch];
    }

    int length() const { return num_samples; }
};

template <typename A, typename B>
product_op<A, B> operator* (A&& a, B&& b)
{
    return product_op<A, B> (a, b);
}

template <typename A, typename B>
plus_op<A, B> operator+ (A&& a, B&& b)
{
    return plus_op<A, B> (a, b);
}



int main()
{
    std::vector<float> va (4, 1);
    std::vector<float> vb (4, 2);
    std::vector<float> vc (4, 3);
    std::vector<float> vd (4, 4);
    std::vector<float> vr (4, 0);
    
    buffer<> a (va);
    buffer<> b (vb);
    buffer<> c (vc);
    buffer<> d (vd);
    buffer<> r (vr);
    
    r = a * b + c + d;

    //auto y = (a * b * c) + d + e;
    
    std::cout << r.channel(0)[0] << std::endl;

    return 0;
}



