#pragma once

#include <chrono>
#include <list>
#include <map>

#include "purolib.hpp"
#include "plot.hpp"

#define PURO_USE_TRACE 0
#define PURO_USE_PROFILE 1

namespace puro {
    
template <int NumChannels, typename T>
buffer<NumChannels, T> make_buffer (std::vector<T>& vector)
{
    buffer<NumChannels, T> buf ((int)vector.size() / NumChannels);
    
    for (int ch=0; ch<NumChannels; ++ch)
        buf.ptrs[ch] = &vector[vector.size()/NumChannels * ch];
    
    return buf;
}
    
} // namespace puro



#if PURO_USE_TRACE == 1

struct TraceEntry
{
    typedef std::chrono::steady_clock clock;
    typedef std::chrono::time_point<std::chrono::steady_clock> time_point;
    typedef const char* (*name_function_type)();
    
    TraceEntry() : name_function (nullptr) {}
    
    TraceEntry (name_function_type name_func) : name_function (name_func), time (clock::now())
    {
    }
    
    name_function_type name_function;
    time_point time;
};


struct TraceFrame
{
    static TraceFrame* current_frame;
    static std::list<TraceFrame> frame_list;
    
    TraceEntry frame_start;
    std::vector<TraceEntry> entries;
    int entry_index;

    TraceFrame() : entry_index (0)
    {
        entries.resize(256);
    }

    static void begin()
    {
        frame_list.push_back(TraceFrame());
        current_frame = &frame_list.back();
        current_frame->frame_start = TraceEntry (nullptr);
    }
    
    static void done()
    {
        // for each frame
            // for each entry
                // if entrylist doesnt exist
                    // create entry list
                // add entry to entrylist
                    // pad with zeros if we skipped frames
        
        struct TraceList // list of values of a trace point in successive frames
        {
            std::vector<float> frames;
            
            TraceEntry::name_function_type name_function = nullptr;
            float average = 0;
        };

        std::vector<TraceList> tracelists; // all trace lists, to be sorted in chronological order
        
        int frame_index = 0;

        // populate all tracelists
        
        for (const auto& f : frame_list) // iterate frames
        {
            const auto frame_start_time = f.frame_start.time;
            
            for (int i=0; i<f.entry_index; ++i) // iterate entries per frame
            {
                const TraceEntry& te = f.entries[i];
                auto name = te.name_function;
                
                // find correct tracelist for entry
                TraceList* tracelist = nullptr;
                for (auto& tl : tracelists)
                {
                    if (name == tl.name_function)
                    {
                        tracelist = &tl;
                        break;
                    }
                }
                // if such tracelist doesn't exist, create it
                if (tracelist == nullptr)
                {
                    tracelists.push_back ({});
                    tracelist = &tracelists.back();
                    tracelist->name_function = name;
                    for (int i=0; i<frame_index; ++i)
                        tracelist->frames.push_back(0);
                    // TODO don't fill with zeros, find the previous entry, this needs to be done later on
                }

                float delta = std::chrono::duration<float, std::micro> (te.time - frame_start_time).count();
                tracelist->frames.push_back (delta);
            }
        }
        
        // calculate statistics
        for (auto& tl : tracelists)
        {
            float sum = 0;
            for (int i=0; i<tl.frames.size(); ++i)
                sum += tl.frames[i];
            tl.average = sum / (float)tl.frames.size();
        }
        
        // sort based on statistics

        std::sort (tracelists.begin(), tracelists.end(), [](const TraceList& a, const TraceList& b) {
            return a.average > b.average;
        });
        
        // plot
        
        auto plt = Figure();

        for (auto& tl : tracelists)
            plt.fill_between(puro::make_buffer<1>(tl.frames), tl.name_function());
        //plt.plot(puro::make_buffer<1>(tl.frames) , "-", "%", tl.name_function());

        plt.show();
    }

    static TraceFrame& get_frame()
    {
        return *current_frame;
    }
    
    static void add_entry (const TraceEntry& e)
    {
        TraceFrame& f = get_frame();
        f.entries[f.entry_index] = e;
        f.entry_index += 1;
    }
};

TraceFrame* TraceFrame::current_frame = nullptr;
std::list<TraceFrame> TraceFrame::frame_list;

#define TRACE_FRAME TraceFrame::begin()
#define TRACE_DONE TraceFrame::done()

#define CONCAT(a, b) a ## b
#define _TRACE(class_name, pretty_name) \
    struct class_name { constexpr static const char* name() { return #pretty_name; } }; \
TraceFrame::add_entry(class_name::name);

#define TRACE(class_name) _TRACE(CONCAT(class_name, _TraceEntry), class_name)

#else // NO TRACE

#define TRACE_FRAME
#define TRACE_DONE
#define TRACE(a)

#endif

////////////////////////////////////////////////////////////////////


#if PURO_USE_PROFILE == 1

struct ProfileEntry
{
    typedef std::chrono::steady_clock clock;
    typedef std::chrono::time_point<std::chrono::steady_clock> time_point;
    
    float get_duration() const
    {
        return std::chrono::duration<float, std::micro> (end_time - start_time).count();
    }

    time_point start_time;
    time_point end_time;
};

struct ProfileFrame
{
    static ProfileFrame* current_frame;
    static std::list<ProfileFrame> frame_list;
    
    ProfileEntry frame_span;
    std::vector<ProfileEntry> entries;
    int entry_index;
    
    ProfileFrame() : entry_index (0)
    {
        entries.resize(256);
    }
    
    static void begin()
    {
        frame_list.push_back(ProfileFrame());
        current_frame = &frame_list.back();
        current_frame->frame_span.start_time = ProfileEntry::clock::now();
    }
    
    static void end()
    {
        get_frame().frame_span.end_time = ProfileEntry::clock::now();
    }

    static ProfileFrame& get_frame()
    {
        return *current_frame;
    }
    
    static void add_entry (const ProfileEntry& e)
    {
        ProfileFrame& f = get_frame();
        f.entries[f.entry_index] = e;
        f.entry_index += 1;
    }
    
    struct Stats
    {
        float average;
        float variance;
        float deviation;
        float minimum;
    };
    
    static void done()
    {
        // for each frame
            // calculate frame time
            // calculate time spent in function
        
        std::vector<float> frame_measurements;
        std::vector<float> func_measurements;
        
        // populate measurement vectors
        for (const auto& f : frame_list) // iterate frames
        {
            //float delta = std::chrono::duration<float, std::micro> (f.frame_span.end_time - f.frame_span.start_time).count();
            float delta = f.frame_span.get_duration();
            frame_measurements.push_back(delta);

            for (int i=0; i<f.entry_index; ++i)
            {
                auto& e = f.entries[i];
                float dur = e.get_duration();
                func_measurements.push_back(dur);
            }
        }
        
        // calculate average frame time
        auto frame_stats = calculate_stats(frame_measurements);
        auto func_stats = calculate_stats(func_measurements);

        std::cout << "Frame time average:  " << frame_stats.average << "\n";
        std::cout << "Frame time deviance: " << frame_stats.deviation << "\n";
        std::cout << "Frame time minimum: " << frame_stats.minimum << "\n";
        std::cout << "Function time average:  " << func_stats.average << "\n";
        std::cout << "Function time deviance: " << func_stats.deviation << "\n";
        std::cout << "Function time minimum: " << func_stats.minimum << "\n";
    }
    
    static Stats calculate_stats(std::vector<float>& vec)
    {
        float average = 0;
        float minimum = 1e30;
        for (int i=0; i<vec.size(); ++i)
        {
            average += vec[i];
            minimum = vec[i] < minimum ? vec[i] : minimum;
        }
        
        average /= vec.size();
        
        float variance = 0;
        for (int i=0; i<vec.size(); ++i)
        {
            float diff = vec[i] - average;
            variance += diff * diff;
        }
        variance /= vec.size()-1;
        
        return { average, variance, std::sqrt(variance), minimum };
    }
};

struct ScopedProfileEntry
{
    typedef std::chrono::steady_clock clock;
    typedef std::chrono::time_point<std::chrono::steady_clock> time_point;
    
    ScopedProfileEntry() : start_time(clock::now())
    {
    }
    
    ~ScopedProfileEntry()
    {
        ProfileFrame::get_frame().add_entry({start_time, clock::now()});
    }
    
    time_point start_time;
};



ProfileFrame* ProfileFrame::current_frame = nullptr;
std::list<ProfileFrame> ProfileFrame::frame_list;

#define PROFILE_FRAME_BEGIN ProfileFrame::begin()
#define PROFILE_FRAME_END ProfileFrame::end()
#define PROFILE_DONE ProfileFrame::done()

#define CONCAT(a, b) a ## b

#else // NO PROFILE

#define PROFILE_FRAME_BEGIN
#define PROFILE_FRAME_END
#define PROFILE_DONE
#define PROFILE(a)

#endif


