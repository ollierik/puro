#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

///////////////////////////////////////////
// puro.hpp
///////////////////////////////////////////


///////////////////////////////////////////
// defs.hpp
///////////////////////////////////////////

#if defined (_MSC_VER)
    #define PURO_MSVC 1
#elif defined(__GNUC__)
    #define PURO_GCC 1
#endif

#ifndef PURO_DEBUG
    #define PURO_DEBUG 1
#endif

#if PURO_MSVC
    #define NOMINMAX
#endif

// TODO currently works with Windows only

#if PURO_MSVC
    #include <Windows.h>
    #include <intrin.h> // MSVS breakpoint
    #define breakpoint __debugbreak()

#elif PURO_GCC 
    #define breakpoint raise(SIGABRT)
#endif

#if PURO_DEBUG 
    #define errorif(condition, msg) if ((condition)) { std::cout << msg << std::endl; breakpoint; }
#else
    #define errorif(condition, msg) ((void)0)
#endif


#ifndef PURO_BUFFER_WRAP_VECTOR_RESIZING
    #define PURO_BUFFER_WRAP_VECTOR_RESIZING 1
#endif


 



///////////////////////////////////////////
// math.hpp
///////////////////////////////////////////

/** Maths routines, mostly for buffers. Used to allow flexibility later on by implementing vector math libs such as IPP */
namespace math
{
    template <typename FloatType>
    FloatType pi()
    {
        return static_cast<FloatType> (3.14159265358979323846);
    }

    template <typename ValueType>
    ValueType min(ValueType f1, ValueType f2)
    {
        return f1 < f2 ? f1 : f2;
    }

    template <typename ValueType>
    ValueType max(ValueType f1, ValueType f2)
    {
        return f1 > f2 ? f1 : f2;
    }

    template <typename ValueType>
    ValueType clip(ValueType val, ValueType minValue, ValueType maxValue)
    {
        return min(max(val, minValue), maxValue);
    }

    template <typename FloatType, typename ValueType=int>
    ValueType round(FloatType value)
    {
        return static_cast<ValueType> (value + (FloatType)0.5);
    }

    template <typename FloatType>
    void multiply_inplace(FloatType* dst, const FloatType* src, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] *= src[i];
    };

    template <typename FloatType>
    void multiply_add(FloatType* dst, const FloatType* src1, const FloatType* src2, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] += src1[i] * src2[i];
    };

    template <typename FloatType>
    void multiply_add(FloatType* dst, const FloatType* src, const FloatType value, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] += src[i] * value;
    };

    /** Multiply src buffer with value and set to dst */
    template <typename FloatType>
    void multiply_set(FloatType* dst, const FloatType* src, const FloatType value, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] = src[i] * value;
    }

    /** In-place sin */
    template <typename FloatType>
    void sin(FloatType* buf, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] = std::sin(buf[i]);
    }

    /** In-place cosine */
    template <typename FloatType>
    void cos(FloatType* buf, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] = std::cos(buf[i]);
    }

    /** Copy from source to destination */
    template <typename FloatType>
    void copy(FloatType* dst, FloatType* src, const int n)
    {
        for (int i=0; i<n; ++i)
            dst[i] = src[i];
    }

    /** Add from source to destination */
    template <typename FloatType>
    void add(FloatType* dst, FloatType* src, const int n)
    {
        for (int i=0; i<n; ++i)
            dst[i] += src[i];
    }

    /** Add constant */
    template <typename FloatType>
    void add(FloatType* buf, FloatType value, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] += value;
    }

    /** Set to constant */
    template <typename FloatType>
    void set(FloatType* buf, FloatType value, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] = value;
    }

};

///////////////////////////////////////////
// pools.hpp
///////////////////////////////////////////

/** Memory aligned pool based on a vector. To avoid cache misses, elements are stored in a consecutive order,
    and moved around upon popping. Works best for Elements with small size and trivial copy operation.
*/

namespace puro {

template <typename T>
struct AlignedPool
{
    struct Iterator
    {
        Iterator(AlignedPool<T>& p, int i) : pool(p), index(i) {}
        int index;
        AlignedPool<T>& pool;

        T& get() { return pool.elements[index]; }

        bool isValid() { return index >= 0; }

        bool operator!= (const Iterator& other) { return index != other.index; }
        Iterator& operator*() { return *this; }
        T* operator->() { return &pool[index]; }
        Iterator& operator++() { --index; return *this; }
    };


    Iterator push(T&& element)
    {
        if (elements.size() < elements.capacity())
        {
            elements.push_back(element);
            return Iterator(*this, (int)size() - 1);
        }

        return Iterator(*this, -1);
    }

    void pop(const Iterator& it)
    {
        if ((int)size() > 1 && it.index < (int)size() - 1)
        {
            memcpy(&elements[it.index], &elements[size() - 1], sizeof(T));
        }

        elements.pop_back();
    }

    Iterator begin() { return Iterator(*this, (int)size() - 1); }
    Iterator end() { return Iterator(*this, -1); }

    size_t size() { return elements.size(); }

    std::vector<T> elements;
};

}

///////////////////////////////////////////
// node_stacks.hpp
///////////////////////////////////////////

namespace puro {

/** Singly-linked node that is contained in a Stack */
template <typename T>
class Node
{
public:
    Node() : next(nullptr) {}

    /** Not guaranteed to be constructed */
    T& getElement()
    {
        return *reinterpret_cast<T*> (element);
    }

    Node* next;

private:

    // workaround to skip the need for a default ctor
    void* element[sizeof(T)];
};


/** Non-threadsafe stack which contains Nodes. More flexibility than with the thread-safe version. */
template <typename T>
class NodeStack
{
public:

    /** Iterator to access NodeStack contents with range-based loop */
    class Iterator
    {
    public:
        Iterator(NodeStack& s, Node<T>* n)
            : stack(s)
            , prev(nullptr)
            , node(n)
            , next((node != nullptr ? node->next : nullptr))
        {
        }

        T& get() { return node->getElement(); }

        bool operator!= (const Iterator& other) { return node != other.node; }
        Iterator& operator*() { return *this; }
        Iterator& operator++()
        {
            if (node == nullptr)
                return *this;

            prev = node;
            node = next;
            next = (node == nullptr ? nullptr : node->next);

            return *this;
        }

        Node<T>* prev; // only the iterator is doubly linked
        Node<T>* node;
        Node<T>* next;

        NodeStack& stack;
    };

    ///=========================================================

    NodeStack() : head(nullptr) {}

    /** Check if the list is empty */
    bool empty() const { return head == nullptr; }

    /** Get the first element of the list */
    Node<T>* first() { return head; }

    /** Push to the front of the list */
    void push_front(Node<T>* node)
    {
        if (node == nullptr)
            return;

        node->next = head;
        head = node;
    }

    /** Push a forward-linked set of Nodes into the stack */
    void push_multiple(Node<T>* node)
    {
        if (node == nullptr)
            return;

        // traverse to find the last element
        Node<T>* tail = node;
        while (tail->next != nullptr)
            tail = tail->next;

        tail->next = head;
        head = node;
    }

    /** Pop from the front of the list */
    Node<T>* pop_front()
    {
        if (head == nullptr)
            return nullptr;

        Node<T>* popped = head;
        head = head->next;
        popped->next = nullptr;

        return popped;
    }

    Node<T>* pop(Iterator& it)
    {
        Node<T>* popped = it.node;

        // if iterator was at end
        if (popped == nullptr)
        {
            return nullptr;
        }
        // if it is the first element
        else if (popped == head)
        {
            head = head->next;
            it.node = head;
            it.next = it.node;
            popped->next = nullptr;
            return popped;
        }

        // if element is not the first one
        it.prev->next = popped->next;
        it.next = popped->next;
        it.node = it.prev;
        popped->next = nullptr;

        return popped;
    }

    /** Pop all of the elements from the stack.
        The returned element points to the first element of the stack, and should be pushed to another stack with push_multiple() */
    Node<T>* pop_all()
    {
        Node<T>* node = head;
        head = nullptr;

        return node;
    }

    Iterator begin() { return Iterator(*this, head); }
    Iterator end() { return Iterator(*this, nullptr); }

    Node<T>* head;
};




/** Atomic stack containing Nodes. All operations should go through push_front and pop_front.
    This container can't be iterated on. To iterate, the contents should be first transferred to
    an instance of NodeStack, for example via pop_all(). */
template <typename T>
class SafeStack
{
public:
    SafeStack() : head(nullptr)
    {
    }

    /** Check if the list is empty */
    bool empty() const { return head.load() == nullptr; }

    /** Get the first element of the list */
    Node<T>* first() { return head.load(); }

    /** Atomically push to the front of the list */
    void push_front(Node<T>* node)
    {
        if (node == nullptr)
            return;

        // put the current value of head into the new node->next
        node->next = head.load(std::memory_order_relaxed);

        // try to replace head with new node
        // if head != node, i.e., if some other thread got here first:
        // load head into node->next and try again
        while (!head.compare_exchange_weak(node->next, node,
            std::memory_order_release,
            std::memory_order_relaxed))
        {
        }; // empty body
    }

    /** Atomically push a forward-linked set of Nodes into the stack */
    void push_multiple(Node<T>* node)
    {
        if (node == nullptr)
            return;

        // traverse to find the last element
        Node<T>* tail = node;
        while (tail->next != nullptr)
            tail = tail->next;

        // put the current value of head into the tail->next
        tail->next = head.load(std::memory_order_relaxed);

        while (!head.compare_exchange_weak(tail->next, node,
            std::memory_order_release,
            std::memory_order_relaxed))
            ; // empty body
    }

    /** Atomically pop from the front of the stack */
    Node<T>* pop_front()
    {
        Node<T>* node = head.load();

        if (node == nullptr)
            return nullptr;

        while (!head.compare_exchange_weak(node, node->next,
            std::memory_order_release,
            std::memory_order_relaxed))
            ; // empty body

        if (node != nullptr)
            node->next = nullptr;

        return node;
    }

    /** Atomically pop all of the elements from the stack.
        The returned element points to the first element of the stack, and should be pushed to another stack with push_multiple() */
    Node<T>* pop_all()
    {
        Node<T>* node = head.load(std::memory_order_relaxed);

        while (!head.compare_exchange_weak(node, nullptr,
            std::memory_order_release,
            std::memory_order_relaxed))
            ; // empty body

        return node;
    }

    std::atomic<Node<T>*> head;
};


/** A helper class to own the memory the Stacks use. */
template <typename T>
class StackMemoryAllocator
{
    struct Chunk
    {
        Chunk(int size)
        {
            memory.resize(size);
        }

        std::vector<Node<T>> memory;
        std::unique_ptr<Chunk> next;
    };

public:
    StackMemoryAllocator() : head(nullptr) {}

    template <typename StackType>
    void allocateChunk(int numElements, StackType& pushToStack)
    {
        Chunk* newChunk = new Chunk(numElements);
        newChunk->next.reset(head.release());

        head.reset(newChunk);

        for (Node<T>& node : newChunk->memory)
        {
            pushToStack.push_front(&node);
        }
    }

private:
    std::unique_ptr<Chunk> head;
};

}

///////////////////////////////////////////
// buffers.hpp
///////////////////////////////////////////

namespace puro {

/** A Wrapper around audio buffer data with helper functions for accessing and debug checks. Does not own the data. */
template <class FloatType, int numberOfChannels>
struct Buffer
{
    // member fields
    int numSamples;
    std::array<FloatType*, numberOfChannels> channelPtrs;

    // template arg broadcasts
    typedef FloatType value_type;
    static constexpr int num_channels = numberOfChannels;

    // getters
    bool isInvalid() const { return numSamples <= 0; }
    int length() const { return numSamples; };
    constexpr int getNumChannels() const { return num_channels; } // some more advanced class may want to redefine this

    FloatType& operator() (int ch, int i)
    {
        errorif(ch < 0 || ch >= num_channels, "channel out of range");
        errorif(i < 0 || i >= numSamples, "sample index out of range");
        return channelPtrs[ch][i];
    }

    FloatType* channel(int ch) const
    {
        errorif(ch < 0 || ch >= num_channels, "channel out of range");
        return channelPtrs[ch];
    }

    // constructors

    Buffer() : numSamples(0) {} // invalid Buffer

    Buffer (int numSamples)
        : numSamples(numSamples)
    {}

    /** Buffer from raw allocated memory.
        Provided data is expected to be able to hold (numSamples * numChannels) of data */
    Buffer (int numSamples, FloatType* data)
        : numSamples(numSamples)
    {
        for (int ch = 0; ch < num_channels; ++ch)
            channelPtrs[ch] = &data[ch * numSamples];
    }

    /** Buffer from raw allocated memory. Provided array should contain the per-channel pointers. */
    Buffer (int numSamples, std::array<FloatType, numberOfChannels> data)
        : numSamples(numSamples)
    {
        for (int ch = 0; ch < data.size(); ++ch)
            channelPtrs[ch] = &data[ch];
    }
};


#ifndef PURO_DYNAMIC_BUFFER_MAX_CHANNELS
#define PURO_DYNAMIC_BUFFER_MAX_CHANNELS 8
#endif PURO_DYNAMIC_BUFFER_MAX_CHANNELS

/** Dynamic wrapper around audio buffer data with resizeable channel count. Does not own the data. */
template <class FloatType, int maxNumberOfChannels = PURO_DYNAMIC_BUFFER_MAX_CHANNELS>
struct DynamicBuffer
{
    // member fields
    int numSamples;
    int numChannels;
    std::array<FloatType*, maxNumberOfChannels> channelPtrs;

    // template arg broadcasts
    typedef FloatType value_type;

    // getters
    bool isInvalid() const { return numSamples <= 0 || numChannels <= 0; }
    int length() const { return numSamples; };
    int getNumChannels() const { return numChannels; } // some more advanced class may want to redefine this

    FloatType& operator() (int ch, int i)
    {
        errorif(ch < 0 || ch >= numChannels, "channel out of range");
        errorif(i < 0 || i >= numSamples, "sample index out of range");
        return channelPtrs[ch][i];
    }

    FloatType* channel(int ch) const
    {
        errorif(ch < 0 || ch >= numChannels, "channel out of range");
        return channelPtrs[ch];
    }

    // constructors

    DynamicBuffer() : numChannels(0), numSamples(0) {} // invalid Buffer

    DynamicBuffer (int numChannels, int numSamples)
        : numChannels(numChannels), numSamples(numSamples)
    {}

    /** Buffer from raw allocated memory.
        Provided data is expected to be able to hold (numSamples * numChannels) of data */
    DynamicBuffer (int numChannels, int numSamples, FloatType* data)
        : numChannels(numChannels), numSamples(numSamples)
    {
        for (int ch = 0; ch < numChannels; ++ch)
            channelPtrs[ch] = &data[ch * numSamples];
    }
};


////////////////////////////////
// Buffer operations
////////////////////////////////

template <typename BufferType>
BufferType buffer_trim_begin(BufferType buffer, int offset)
{
    errorif(offset < 0 || offset > buffer.numSamples, "offset out of bounds");

    buffer.numSamples -= offset;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

    return buffer;
}


template <typename BufferType>
BufferType buffer_trim_length(BufferType buffer, int newLength)
{
    errorif(newLength > buffer.numSamples, "new length out of bounds");

    buffer.numSamples = math::max(newLength, 0);
    return buffer;
}

template <typename BufferType>
BufferType buffer_slice(BufferType buffer, int offset, int length)
{
    errorif(offset > buffer.numSamples, "slice offset greater than number of samples available");
    errorif(length < 0 || length > (offset + buffer.numSamples), "slice length out of bounds");

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

    buffer.numSamples = length;
    return buffer;
}

/** Create a Buffer with the data laid out into the provided vector.
    The vector may be resized if needed depending on template arg. Number of channels is deducted from the template args. */
template <typename BufferType, typename FloatType, bool resizeIfNeeded = PURO_BUFFER_WRAP_VECTOR_RESIZING>
BufferType buffer_wrap_vector(std::vector<FloatType>& vector, int numSamples)
{
    if (resizeIfNeeded)
    {
        const int totLength = BufferType::num_channels * numSamples;

        if ((int)vector.size() < totLength)
            vector.resize(totLength);
    }

    return BufferType(numSamples, vector.data());
}

template <typename ToBufferType, typename FromBufferType>
ToBufferType buffer_convert_to_type(FromBufferType src)
{
    ToBufferType dst (src.length());
    for (int ch=0; ch < dst.getNumChannels(); ++ch)
    {
        errorif (ch >= src.getNumChannels(), "trying to convert from less channels to a larger one");
        dst.channelPtrs[ch] = src.channelPtrs[ch];
    }
    return dst;
}

template <typename BufferType, typename FloatType>
BufferType fit_vector_into_dynamic_buffer(std::vector<FloatType>& vector, int numChannels, int numSamples)
{
    const int totLength = numChannels * numSamples;

    // resize if needed
    if ((int)vector.size() < totLength)
        vector.resize(totLength);

    return BufferType(numChannels, numSamples, vector.data());
}

template <typename BufferType, typename MultBufferType>
BufferType multiply_add(BufferType dst, const BufferType src1, const MultBufferType src2)
{
    errorif(!(dst.length() == src1.length()), "dst and src1 buffer lengths don't match");
    errorif(!(dst.length() == src2.length()), "dst and src2 buffer lengths don't match");

    // identical channel configs
    if (src1.getNumChannels() == src2.getNumChannels())
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.length());
        }
    }
    // src2 is a mono buffer
    else if (src1.getNumChannels() > 1 && src2.getNumChannels() == 1)
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(0), dst.length());
        }
    }
    else
    {
        errorif(true, "channel config not implemented");
    }
       
    return dst;
}

template <typename BufferType>
void buffer_clear(BufferType buffer)
{
    for (int ch=0; ch<buffer.getNumChannels(); ++ch)
    {
        math::set<typename BufferType::value_type>(buffer.channel(ch), 0, buffer.length());
    }
}

} // namespace puro

///////////////////////////////////////////
// sequences.hpp
///////////////////////////////////////////

namespace puro {

template <int increment = 1>
struct IndexSequence
{
    IndexSequence(int val) : value(val) {}

    static constexpr int increment = increment;

    IndexSequence operator++() // prefix
    {
        value += increment;
        return value;
    };

    IndexSequence operator++(int) // postfix
    {
        IndexSequence temp (value, increment);
        value += increment;
        return temp;
    };

    IndexSequence& operator+=(const int rhs)
    {
        value += rhs;
        return *this;
    }

    operator int() { return value; }
    int value;
};



template <typename T>
struct Sequence
{
    Sequence(T val, T inc) : value(val), increment(inc) {}

    Sequence operator++() // prefix
    {
        value += increment;
        return value;
    };

    Sequence operator++(int) // postfix
    {
        Sequence temp (value, increment);
        value += increment;
        return temp;
    };

    operator T() { return value; }

    T value;
    T increment;
};

} // namespace puro

///////////////////////////////////////////
// panning.hpp
///////////////////////////////////////////

namespace puro {


/** Represents the coefficients used for panning.
    Multiplication coefficients are stored in an array, source channel first.
    So for stereo channel:
        [ leftToLeft, leftToRight, rightToLeft, rightToRight ]
*/
template <typename FloatType, int numChannels>
struct PanCoeffs
{
    constexpr int getNumChannels() { return numChannels; };
    std::array<FloatType, numChannels*numChannels> coeffs;

    FloatType operator() (int fromCh, int toCh)
    {
        return coeffs[numChannels * fromCh + toCh];
    }
};

/** Pan range is [-1, 1], where -1 is hard left and 1 is hard right. */
    
template <typename FloatType>
PanCoeffs<FloatType, 2> pan_create_stereo(FloatType pan)
{
    // [ leftToLeft, leftToRight, rightToLeft, rightToRight ]
    if (pan <= 0)
    {
        const FloatType ltol = 1;
        const FloatType ltor = 0;
        const FloatType rtol = -pan;
        const FloatType rtor = 1 + pan;
        return PanCoeffs<FloatType, 2> { ltol, ltor, rtol, rtor };
    }
    else
    {
        const FloatType ltol = 1 - pan;
        const FloatType ltor = pan;
        const FloatType rtol = 0;
        const FloatType rtor = 1;
        return PanCoeffs<FloatType, 2> { ltol, ltor, rtol, rtor };
    }
}

template <typename BufferType, typename PanType>
void content_pan_apply(BufferType dst, BufferType src, PanType coeffs)
{
    errorif(src.getNumChannels() != coeffs.getNumChannels(), "channel configs between src and coeffs don't match");
    errorif(dst.getNumChannels() != coeffs.getNumChannels(), "channel configs between dst and coeffs don't match");

    const auto numChannels = coeffs.getNumChannels();
    using FloatType = typename BufferType::value_type;

    puro::buffer_clear(dst);

    // TODO optimise for special cases coef == 0 and coef == 1

    for (int fromCh = 0; fromCh < numChannels; ++fromCh)
    {
        for (int toCh = 0; toCh < numChannels; ++toCh)
        {
            const auto coef = coeffs(fromCh, toCh);
            math::multiply_add(dst.channel(toCh), src.channel(fromCh), coef, dst.length());
        }
    }
}

template <typename BufferType, typename SourceBufferType, typename PanType>
void content_pan_apply_and_add(BufferType dst, SourceBufferType src, PanType coeffs)
{
    using FloatType = typename BufferType::value_type;

    const int numChannels = coeffs.getNumChannels();

    if (dst.getNumChannels() == src.getNumChannels())
    {
        for (int fromCh = 0; fromCh < numChannels; ++fromCh)
        {
            for (int toCh = 0; toCh < numChannels; ++toCh)
            {
                const auto coef = coeffs(fromCh, toCh);
                math::multiply_add(dst.channel(toCh), src.channel(fromCh), coef, dst.length());
            }
        }
    }
    // mono source to multichannel
    else if (dst.getNumChannels() != src.getNumChannels() && src.getNumChannels() == 1)
    {
        for (int toCh = 0; toCh < numChannels; ++toCh)
        {
            FloatType coef = 0;
            for (int fromCh = 0; fromCh < coeffs.getNumChannels(); ++fromCh)
            {
                coef += coeffs(fromCh, toCh);
            }
            coef /= numChannels;

            math::multiply_add(dst.channel(toCh), src.channel(0), coef, dst.length());
        }
    }
}

} // namespace puro

///////////////////////////////////////////
// alignment.hpp
///////////////////////////////////////////

namespace puro 
{

    struct RelativeAlignment
    {
        RelativeAlignment(int offset, int length) : offset(offset), remaining(length) {}
        int offset;
        int remaining;
    };

    template <typename BufferType, typename AlignmentType>
    std::tuple<BufferType, AlignmentType> alignment_advance_and_crop_buffer(BufferType buffer, AlignmentType alignment)
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

///////////////////////////////////////////
// utils.hpp
///////////////////////////////////////////

namespace puro {

template <typename ValueType>
struct Timer
{
    Timer(ValueType interval) : interval(interval), counter(0) {}

    ValueType advance(ValueType units)
    {
        counter += units;

        if (counter <= interval)
            return 0;

        units = counter - interval;
        counter = 0;

        return units;
    }

    ValueType interval;
    ValueType counter;
};


template <typename ValueType, bool useRelativeDeviation, typename FloatType=float>
struct Parameter
{
public:
    Parameter(ValueType min, ValueType max)
        : generator(std::random_device()()), centre(0), deviation(0), minimum(min), maximum(max)
    {}

    Parameter(FloatType centre, FloatType deviation, ValueType min, ValueType max)
        : generator(std::random_device()()), centre(centre), deviation(deviation), minimum(min), maximum(max)
    {}

    ValueType get()
    {
        FloatType f;
        if (useRelativeDeviation)
            f = random_bspline2_fast(generator, centre, centre * deviation);
        else
            f = random_bspline2_fast(generator, centre, deviation);

        if (std::is_integral<ValueType>::value)
        {
            const ValueType i = static_cast<ValueType> (math::round<FloatType, ValueType>(f));
            return math::clip<ValueType> (i, minimum, maximum);
        }
        return math::clip<ValueType> (static_cast<ValueType>(f), minimum, maximum);
    }

    std::mt19937 generator;

    FloatType centre;
    FloatType deviation;
    ValueType minimum;
    ValueType maximum;
};


template <typename FloatType>
FloatType random_bspline2_fast(std::mt19937& gen, FloatType mean, FloatType dev)
{
    constexpr int smin = std::numeric_limits<short>::min();
    constexpr int smax = std::numeric_limits<short>::max();
    constexpr FloatType fdiv = static_cast<FloatType>(1.0 / (3.0 * static_cast<FloatType>(smax)));

    const std::uniform_int_distribution<short> dist (smin, smax);

    const FloatType r = fdiv * (static_cast<float>(dist(gen)) + static_cast<float>(dist(gen)) + static_cast<float>(dist(gen)));

    return r * dev + mean;
}



}

///////////////////////////////////////////
// operations.hpp
///////////////////////////////////////////

namespace puro {

template <typename BufferType, typename MultBufferType>
BufferType content_multiply_inplace(BufferType dst, const MultBufferType src)
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.getNumChannels() == src.getNumChannels())
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply_inplace(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.getNumChannels() != src.getNumChannels() && src.getNumChannels() == 1)
    {
        for (int ch = 0; ch < dst.getNumChannels(); ++ch)
        {
            math::multiply_inplace(dst.channel(ch), src.channel(0), dst.length());
        }
    }
       
    return dst;
}

template <typename BufferType>
void constant_fill(BufferType buffer, typename BufferType::value_type value)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        math::set(buffer.channel(ch), value, buffer.length());
    }
}


template <typename BufferType>
void noise_fill(BufferType buffer)
{
    using FloatType = typename BufferType::value_type;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        for (int i = 0; i < buffer.numSamples; ++i)
        {
            const FloatType coef = static_cast<FloatType> (1) / static_cast<FloatType> (RAND_MAX / 2);
            const FloatType r = static_cast<FloatType> (std::rand()) * coef - 1;

            buffer(ch, i) = r;
        }
    }
}

template <typename BufferType, typename ValueType>
void linspace_fill(BufferType buffer, ValueType start, const ValueType increment)
{
    for (int i=0; i<buffer.length(); ++i)
    {
        buffer(0, i) = start;
        start += increment;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.length());
    }
}

/////////////////////////////////////////////
// Envelope sources
/////////////////////////////////////////////

template <typename FloatType>
Sequence<FloatType> content_envelope_halfcos_create_seq(int lengthInSamples)
{
    const FloatType val = math::pi<FloatType>() / static_cast<FloatType>(lengthInSamples + 1);
    return { val, val };
}

template <typename BufferType, typename SeqType>
SeqType content_envelope_halfcos_fill(BufferType buffer, SeqType seq)
{
    for (int i = 0; i < buffer.length(); ++i)
    {
        buffer(0, i) = seq++;
    }

    math::sin(&buffer(0, 0), buffer.length());

    // copy to other channels
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), buffer.channel(0), buffer.length());
    }

    return seq;
}

template <typename FloatType>
Sequence<FloatType> envl_hann_create_seq(int lengthInSamples, bool symmetric = true)
{
    const FloatType div = static_cast<FloatType>(lengthInSamples) + (symmetric ? 1 : 0);
    const FloatType val = 2 * math::pi<FloatType>() / div;

    return { val, val };
}

template <typename BufferType, typename SeqType>
SeqType envl_hann_fill(BufferType buffer, SeqType seq)
{
    for (int i = 0; i < buffer.length(); ++i)
    {
        const auto sample = (1 - cos(seq++)) / 2;
        buffer(0, i) = sample;
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
    {
        math::copy(buffer.channel(ch), &buffer.channel(ch), buffer.length());
    }

    return seq;
}


template <typename BufferType, typename SrcBufferType, typename SeqType>
std::tuple <BufferType, SeqType> buffer_fill(BufferType buffer, SrcBufferType source, SeqType seq)
{
    using FloatType = typename BufferType::value_type;

    // source buffer will run out, trim the destination buffer
    if (source.length() < (seq + buffer.length()))
    {
        buffer = trimmed_length(buffer, source.length() - seq);
    }

    // identical channel config
    if (buffer.getNumChannels() == source.getNumChannels())
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy<FloatType>(buffer.channel(ch), source.channel(ch), buffer.length());
        }
        seq += buffer.length();
    }
    // mono source, use for all channels
    else if (source.getNumChannels() == 1)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            math::copy(buffer.channel(ch), source.channel(0), buffer.length());
        }
        seq += buffer.length();
    }
    else
    {
        errorif(true, "channel config combination not implemented");
    }

    return std::make_tuple(buffer, seq);
}


template <typename FloatType>
int content_interpolation_num_samples_available(int length, FloatType position, const FloatType rate, const int interp_order)
{
    return (int)std::ceil((static_cast<FloatType>(length-interp_order) - position)/rate);
}

template <typename BufferType, typename SeqType>
BufferType content_interpolation_crop_buffer(BufferType buffer, int samplesAvailable, SeqType seq, const int interpOrder)
{
    const int numAvailable = content_interpolation_num_samples_available(samplesAvailable, seq.value, seq.increment, interpOrder);

    if (numAvailable < buffer.length())
        buffer = puro::buffer_trim_length(buffer, numAvailable);

    return buffer;
}

/** Assumes that the source buffer can provide all the required samples, i.e. doesn't do bound checking.
    Buffer should be cropped for example with buffer_crop_for_interp before-hand. */
template <typename BufferType, typename SrcBufferType, typename SeqType>
std::tuple <BufferType, SeqType> content_interpolation1_fill(BufferType buffer, SrcBufferType source, SeqType seq)
{
    using FloatType = typename BufferType::value_type;

    errorif((buffer.getNumChannels() != source.getNumChannels())
            && (source.getNumChannels() != 1),
            "channel configuration not impltemented");

    // identical channel config
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        SeqType chSeq = seq;
        for (int i = 0; i < buffer.length(); ++i)
        {
            const FloatType pos = chSeq++;
            const int index = static_cast<int> (pos);
            const FloatType fract = pos - index;

            buffer(ch, i) = source(ch, index) * (1 - fract) + source(ch, index + 1) * fract;
        }

        if (ch == buffer.getNumChannels()- 1)
            seq = chSeq;
    }

    return std::make_tuple(std::move(buffer), std::move(seq));
}


} // namespace puro


