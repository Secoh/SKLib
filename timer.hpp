// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2019-2021] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s).
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

#pragma once

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#include <thread>
#include <chrono>
#endif

// 1. Convert time in certain real-world units to C++ time representation

// reminder: to use the time units conversions directly, use the standard statement:
//     using namespace std::chrono_literals;

// global time conversion operators with "_sklib" suffix
// provides functionality without invoking std::chrono_literals
//
#pragma warning(push)
#pragma warning(disable:4455)   // known deviation from C++ standard: STL provides literals without underscore

constexpr auto operator "" _s_sklib (unsigned long long t)       { using namespace std::chrono_literals; return operator""s(t);   }
constexpr auto operator "" _ms_sklib(unsigned long long t)       { using namespace std::chrono_literals; return operator""ms(t);  }
constexpr auto operator "" _us_sklib(unsigned long long t)       { using namespace std::chrono_literals; return operator""us(t);  }
constexpr auto operator "" _ns_sklib(unsigned long long t)       { using namespace std::chrono_literals; return operator""ns(t);  }

constexpr auto operator "" _seconds_sklib(unsigned long long t)  { using namespace std::chrono_literals; return operator""s(t);   }
constexpr auto operator "" _minutes_sklib(unsigned long long t)  { using namespace std::chrono_literals; return operator""min(t); }
constexpr auto operator "" _hours_sklib  (unsigned long long t)  { using namespace std::chrono_literals; return operator""h(t);   }

constexpr auto operator "" _s_sklib (long double t)              { using namespace std::chrono_literals; return operator""s(t);   }
constexpr auto operator "" _ms_sklib(long double t)              { using namespace std::chrono_literals; return operator""ms(t);  }
constexpr auto operator "" _us_sklib(long double t)              { using namespace std::chrono_literals; return operator""us(t);  }
constexpr auto operator "" _ns_sklib(long double t)              { using namespace std::chrono_literals; return operator""ns(t);  }

constexpr auto operator "" _seconds_sklib(long double t)         { using namespace std::chrono_literals; return operator""s(t);   }
constexpr auto operator "" _minutes_sklib(long double t)         { using namespace std::chrono_literals; return operator""min(t); }
constexpr auto operator "" _hours_sklib  (long double t)         { using namespace std::chrono_literals; return operator""h(t);   }

#pragma warning(pop)

// 2. For better readability, time conversion analogs with underscores

namespace sklib
{
    namespace chrono_literals
    {
        constexpr auto operator "" _s (unsigned long long t)      { return operator""_s_sklib(t);       }
        constexpr auto operator "" _ms(unsigned long long t)      { return operator""_ms_sklib(t);      }
        constexpr auto operator "" _us(unsigned long long t)      { return operator""_us_sklib(t);      }
        constexpr auto operator "" _ns(unsigned long long t)      { return operator""_ns_sklib(t);      }

        constexpr auto operator "" _seconds(unsigned long long t) { return operator""_s_sklib(t);       }
        constexpr auto operator "" _minutes(unsigned long long t) { return operator""_minutes_sklib(t); }
        constexpr auto operator "" _hours  (unsigned long long t) { return operator""_hours_sklib(t);   }

        constexpr auto operator "" _s (long double t)             { return operator""_s_sklib(t);       }
        constexpr auto operator "" _ms(long double t)             { return operator""_ms_sklib(t);      }
        constexpr auto operator "" _us(long double t)             { return operator""_us_sklib(t);      }
        constexpr auto operator "" _ns(long double t)             { return operator""_ns_sklib(t);      }

        constexpr auto operator "" _seconds(long double t)        { return operator""_s_sklib(t);       }
        constexpr auto operator "" _minutes(long double t)        { return operator""_minutes_sklib(t); }
        constexpr auto operator "" _hours  (long double t)        { return operator""_hours_sklib(t);   }
    };

// 3. Platform-independent Sleep() analog

    template<class tRep, class tPeriod = std::ratio<1>>
    inline void delay(const std::chrono::duration<tRep, tPeriod>& t)
    {
        std::this_thread::sleep_for(t);
    }

// 4. If requested by caller, SkLib provides Sleep() analogs with classical calling parameters (either milliseconds or seconds)
//    one appropriate #define, but not both, shall appear in compilation unit before #include

#if defined(SKLIB_TIMER_DEFAULT_UNIT_SECONDS)
 #if defined(SKLIB_TIMER_DEFAULT_UNIT_MILLISECONDS)
 #error SkLib Error -- Both macros SKLIB_TIMER_DEFAULT_UNIT_SECONDS and SKLIB_TIMER_DEFAULT_UNIT_MILLISECONDS cannot be defined in the same time
 #else
    inline void delay(double t_sec)
    {
        std::this_thread::sleep_for(std::chrono::duration<double>(t_sec));
    }
 #endif
#elif defined(SKLIB_TIMER_DEFAULT_UNIT_MILLISECONDS)
    inline void delay(size_t t_msec)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(t_msec));
    }
#endif

// 5. Timeout and/or counter timer class

// Timeout and Stopwatch timers implemented around std::chrono::steady_clock
//
// Copyright [2019-2020] Secoh
//
// Licensed under the Apache License, Version 2.0 (the "License") - see http://www.apache.org/licenses/LICENSE-2.0
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//
// Interface:
//      TimerWatch      Create timer, either stopped or running. Optional: specify timeout
//      reset           Reset Stopwatch and Timeout timer. Start or stop according to the initial state. Optional: specify new timeout
//      start           Start Stopwatch, reset timeout start time (if wasn't running). Optional: specify new timeout
//      stop            Stop Stopwatch, returns the most recent interval duration
//      read            Returns Stopwatch time (i.e. counting only when running)
//      is_running      Returns True if the timer is running
//      is_timeout      Returns True if time elapsed since the timeout start time is greater than the set time (Timeout event).
//                      Optional: specify alternative timeout period
//
// Define SKLIB_TIMER_STOPWATCH_DISABLE_EXPLICIT_CAST to *disable* direct conversions from class instance to query functions
//
// Define _SKLIB_ENABLE_DEMO to enable example_TimerWatch() function with demo and test code
//
// Use case: typical timeout monitor
//
//    TimerWatch check(delay_seconds);
//    while (...condition...)
//    {
//        if (check.is_timeout()) { ...handle timeout event... }
//        ...keep processing and/or waiting...
//    }
//
// Use case: typical elapsed time monitor
//
//    TimerWatch elapsed;
//    while (...condition...) { ...do something... }
//    double time_interval = elapsed.read();
//

// ??? #if ((defined(_MSVC_LANG) && _MSVC_LANG > 201700L) || __cplusplus > 201700L)

    // Timeout and Stopwatch timers implemented around std::chrono::steady_clock
    // Intended for use either in timeout, or stopwatch mode, however mixed operation is possible in certain cases
    // Timer declaration:  TimerWatch [<false>] variable [(timeout_interval)] ;
    // Timeout start time is captured at the time of creation (or can be set by start() member function)
    // Timer is started at creation by default, or created stopped if <false> is specified
    // All time intervals are floating point values in seconds
    // Define _SKLIB_TIMEWATCH_ALLOW_EXPLICIT_CAST to enable shortcuts for is_timeout() and read() functions
    // Requires C++17 or later to compile
    //
    template<bool create_running = true>
    class stopwatch
    {
    private:
        bool timer_active = false;
        std::chrono::duration<double> time_accumulator = 0;     // base unit is 1 second
        std::chrono::duration<double> time_delay = 0;
        std::chrono::steady_clock::time_point start_time;

        void configure(bool initialize, bool start, bool update_timeout, std::chrono::duration<double> timeout_s = 0)
        {
            if (initialize) time_accumulator = 0;
            if (initialize || (start && !timer_active)) start_time = std::chrono::steady_clock::now();
            if (update_timeout) time_delay = timeout_s;
            timer_active = start;
        }

        double get_dt() const
        {
            auto current_time = std::chrono::steady_clock::now();
            return std::chrono::duration<double>(current_time - start_time).count();
        }

    public:
        // Create Timeout / Stopwatch timer
        // Timer is running by default or stopped if TimerWatch<false> is used
        // Timeout start time is captured at the time of creation
        // Timeout interval in seconds is set by the parameter
        // See the description of the class for more details
        //
        stopwatch(double timeout_s = 0) { reset(timeout_s); }

        //
        void reset() { configure(true, create_running, false); }

        //
        void reset(double timeout_s) { configure(true, create_running, true, timeout_s); }

        // If timer is stopped, start stopwatch and reset (refresh) timeout start time
        // If timer is running, do nothing
        // Timeout interval is not modified
        //
        void start() { configure(false, true, false, 0); }

        // If timer is stopped, start stopwatch and reset (refresh) timeout start time
        // If timer is running, timeout start time does NOT change
        // Timeout interval is updated to the new value
        //
        void start(double update_timeout_s) { configure(false, true, true, update_timeout_s); }

        // Returns true if timeout has occured, i.e.
        // more than previously specified timeout duration has elapsed since timer creation or last update
        // When enabled, bool() operator also returns is_timeout()
        //
        bool is_timeout() const { return is_timeout(time_delay); }

        // Returns true if timeout has occured, i.e.
        // more than timeout_s seconds have elapsed since timer creation or last update
        //
        bool is_timeout(double timeout_s) const { return (get_dt() > timeout_s); }

#ifndef SKLIB_TIMER_STOPWATCH_DISABLE_EXPLICIT_CAST
        // synonym for is_timeout()
        explicit operator bool() const { return is_timeout(); }
        // synonym for read()
        explicit operator double() const { return read(); }
#endif

        // Returns true if the timer is running
        //
        bool is_running() const { return timer_active; }

        // If timer was running, stop it and remember the time since the last start
        // Returns the duration of the most recent running interval in seconds, or 0 if timer wasn't running
        //
        double stop()
        {
            if (!timer_active) return 0;
            timer_active = false;

            double current_interval = get_dt();
            time_accumulator += current_interval;

            return current_interval;
        }

        // Returns elapsed Stopwatch time in seconds
        // Only time between start() and stop() is counted
        // (If timer is running, time since the last start is included, too)
        // When enabled, double() operator also returns read()
        //
        double read() const
        {
            return time_accumulator + (timer_active ? get_dt() : 0);
        }
    };

};
