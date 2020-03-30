
#if 0
template <int N>
class ConstParam
{
    constexpr int value = N;
};

template <int N>
class MutableParam
{
    MutableParam() = default;
    int value = N;
};

template <class Param>
class Buffer
{
// if is_same<Param, ConstParam<N>>
// using Buffer = std::array<float, ConstParam>;
// else 
// using Buffer = std::vector<float>;

    Buffer buffer;
};
#endif

#include <array>
#include <vector>

template <typename ValueType, ValueType Value>
struct Parameter
{
    ValueType value = N;
};

template <typename ValueType, ValueType Value>
struct ConstParam : public Parameter<const ValueType>
{

    ValueType value = Value;
};

template <int N>
struct ConstIntParam : ConstParam<int, Value>
{
};

struct MutableParam
{
};

template <class Param, typename FloatType = float>
struct Buffer
{
};

template <typename FloatType, int N>
struct Buffer <FloatType, ConstIntParam<N>>
{
    std::array<FloatType, N> buffer;
};

template <typename FloatType>
struct Buffer <FloatType, MutableParam>
{
    std::vector<FloatType> buffer;
};

int main()
{
    using FloatType = float;
    Buffer<FloatType, ConstIntParam<32>> immutableBuffer;
    Buffer<FloatType, MutableParam> mutableBuffer;

    immutableBuffer.buffer[31] = 1.0f;
    mutableBuffer.buffer.push_back(1.0f);
}

