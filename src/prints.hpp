#pragma once

namespace puro {
namespace details_table {
    
/////////////////////////////////////// LENGTH ///////////////////////////////////////

template <class N, class B>
int print_buffer_table_find_length(int n, const N& name, const B& buffer)
{
    n = buffer.length() > n ? buffer.length() : n;
    return n;
}

template <class N, class B, class... Ts>
int print_buffer_table_find_length(int n, const N& name, const B& buffer, const Ts&... rest)
{
    n = buffer.length() > n ? buffer.length() : n;
    return print_buffer_table_find_length(n, rest...);
}
    
/////////////////////////////////////// ROW ///////////////////////////////////////

template <class N, class B>
void print_buffer_table_row(int i, const N&, const B& buffer)
{
    if (i < buffer.length())
        for (auto ch=0; ch < buffer.num_channels(); ++ch)
            std::cout << std::fixed << std::setw(12) << std::setprecision(5) << buffer.channel(ch)[i] << " | ";
    else
        for (auto ch=0; ch < buffer.num_channels(); ++ch)
            std::cout << std::fixed << std::setw(12) << " " << " | ";
    
    std::cout << "\n";
}

template <class N, class B, class... Ts>
void print_buffer_table_row(int i, const N&, const B& buffer, const Ts&... rest)
{
    if (i < buffer.length())
        for (auto ch=0; ch < buffer.num_channels(); ++ch)
            std::cout << std::fixed << std::setw(12) << std::setprecision(5) << buffer.channel(ch)[i] << " | ";
    else
        for (auto ch=0; ch < buffer.num_channels(); ++ch)
            std::cout << std::fixed << std::setw(12) << " " << " | ";

    print_buffer_table_row(i, rest...);
}

/////////////////////////////////////// CH NUMBERS ///////////////////////////////////////

template <class N, class B>
void print_buffer_table_channel_numbers(const N& name, const B& buffer)
{
    for (auto ch=0; ch<buffer.num_channels(); ++ch)
    {
        if (ch == buffer.num_channels()-1)
            std::cout << "ch " << std::setw(9) << ch << " | ";
        else
            std::cout << "ch " << std::setw(9) << ch << "   ";
    }
    std::cout << "\n";
}

template <class N, class B, class... Ts>
void print_buffer_table_channel_numbers(const N& name, const B& buffer, const Ts&... rest)
{
    for (auto ch=0; ch<buffer.num_channels(); ++ch)
    {
        if (ch == buffer.num_channels()-1)
            std::cout << "ch " << std::setw(9) << ch << " | ";
        else
            std::cout << "ch " << std::setw(9) << ch << "   ";
    }
    
    print_buffer_table_channel_numbers(rest...);
}
    
/////////////////////////////////////// NAMES ///////////////////////////////////////

template <class N, class B>
void print_buffer_table_names(const N& name, const B& buffer)
{
    //std::cout << std::fixed << std::setw(12) << name << " | ";
    
    for (auto ch=0; ch<buffer.num_channels(); ++ch)
    {
        if (ch == buffer.num_channels()-1)
            std::cout << std::fixed << std::setw(12) << name << " | ";
        else
            std::cout << std::fixed << std::setw(12) << " " << "   ";
    }

    std::cout << "\n";
}

template <class N, class B, class... Ts>
void print_buffer_table_names(const N& name, const B& buffer, const Ts&... rest)
{
    for (auto ch=0; ch<buffer.num_channels(); ++ch)
    {
        if (ch == buffer.num_channels()-1)
            std::cout << std::fixed << std::setw(12) << name << " | ";
        else
            std::cout << std::fixed << std::setw(12) << " " << "   ";
    }

    print_buffer_table_names(rest...);
}
    
/////////////////////////////////////// HR ///////////////////////////////////////

template <class N, class B>
void print_buffer_table_hr(const N& name, const B& buffer)
{
    for (auto ch=0; ch<buffer.num_channels(); ++ch)
    {
        if (ch == buffer.num_channels() - 1)
            std::cout << "------------" << " | ";
        else
            std::cout << "------------" << "---";
    }
    std::cout << "\n";
}

template <class N, class B, class... Ts>
void print_buffer_table_hr(const N& name, const B& buffer, const Ts&... rest)
{
    for (auto ch=0; ch<buffer.num_channels(); ++ch)
    {
        if (ch == buffer.num_channels() - 1)
            std::cout << "------------" << " | ";
        else
            std::cout << "------------" << "---";
    }
    
    print_buffer_table_hr(rest...);
}
    
/////////////////////////////////////// HEADING ///////////////////////////////////////

template <class... Ts>
void print_buffer_table_heading(const Ts&... buffersAndNames)
{
    std::cout << "      ";
    print_buffer_table_names(buffersAndNames...);
    std::cout << "      ";
    print_buffer_table_channel_numbers(buffersAndNames...);
    std::cout << "      ";
    print_buffer_table_hr(buffersAndNames...);
}
    
} // namespace details_table

/////////////////////////////////////// MAIN ///////////////////////////////////////

/**
 Prints a table of buffer contents across all channels. Buffers should be inserted as
 an interleaved list of names and buffers:
 
 print_buffer_table("name0", buf0, "name1", buf1, "name2", buf2);
 */
template <class... Ts>
void print_buffer_table(const Ts&... buffersAndNames)
{
    const int n = details_table::print_buffer_table_find_length(0, buffersAndNames...);
    
    details_table::print_buffer_table_heading(buffersAndNames...);
    
    for (int i=0; i<n; ++i)
    {
        std::cout << std::setw(5) << i << ":";
        details_table::print_buffer_table_row(i, buffersAndNames...);
    }
    
    std::cout << "\n\n";
}
    
} // namespace puro
