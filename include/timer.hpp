// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2019-2021] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

//TODO:
// whitepaper
// typeless time input must be in seconds!
// new timer type: checkpoint - like timeout but resets itself after returning true

#ifndef SKLIB_INCLUDED_TIMER_HPP
#define SKLIB_INCLUDED_TIMER_HPP

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#include <thread>
#include <chrono>
#include <ratio>
#endif

#include "helpers.hpp"     // this also loads <type_traits>

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


// 3. For completeness, time conversion as explicit functions

// special: template active for integer or floating type T, makes up type LGT=unsigned long long or long double for respective T, return type is constexpr auto
#define SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_FROM_UNITS(funkname,opconv)                                           \
    template<class T, class LGT = std::conditional_t<SKLIB_TYPES_IS_INTEGER(T), unsigned long long, long double>,    \
                      std::enable_if_t<SKLIB_TYPES_IS_INTEGER(T) || SKLIB_TYPES_IS_FLOATING_POINT(T), bool> = true>  \
    constexpr auto funkname (T t) { return operator""opconv (LGT(t)); }    // NB: no spaces around ""

    SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_FROM_UNITS( time_seconds,      _s_sklib  );
    SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_FROM_UNITS( time_milliseconds, _ms_sklib );
    SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_FROM_UNITS( time_microseconds, _us_sklib );
    SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_FROM_UNITS( time_nanoseconds,  _ns_sklib );

/*
    SKLIB_INTERNAL_TEMPLATE_CE_A_IF_INT_OR_FLOAT_T_SELECT_LGT time_seconds(T t)      { return operator""_s_sklib(LGT(t)); }
    SKLIB_INTERNAL_TEMPLATE_CE_A_IF_INT_OR_FLOAT_T_SELECT_LGT time_milliseconds(T t) { return operator""_ms_sklib(LGT(t)); }
    SKLIB_INTERNAL_TEMPLATE_CE_A_IF_INT_OR_FLOAT_T_SELECT_LGT time_microseconds(T t) { return operator""_us_sklib(LGT(t)); }
    SKLIB_INTERNAL_TEMPLATE_CE_A_IF_INT_OR_FLOAT_T_SELECT_LGT time_nanoseconds(T t)  { return operator""_ns_sklib(LGT(t)); }
*/
/*
    template<class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    constexpr auto time_seconds(T t)
    { return operator""_s_sklib((unsigned long long)t);  }

    template<class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    constexpr auto time_milliseconds(T t)
    { return operator""_ms_sklib((unsigned long long)t); }

    template<class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    constexpr auto time_microseconds(T t)
    { return operator""_us_sklib((unsigned long long)t); }

    template<class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    constexpr auto time_nanoseconds(T t)
    { return operator""_ns_sklib((unsigned long long)t); }

    template<class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr auto time_seconds(T t)
    { return operator""_s_sklib((long double)t);         }

    template<class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr auto time_milliseconds(T t)
    { return operator""_ms_sklib((long double)t);        }

    template<class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr auto time_microseconds(T t)
    { return operator""_us_sklib((long double)t);        }

    template<class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr auto time_nanoseconds(T t)
    { return operator""_ns_sklib((long double)t);        }
*/


// 4. Conversion from intervals in time units to number of seconds, milliseconds, minutes, etc
//    Working with timers (class timer_stopwatch_type), see section 7

// template active for integer or floating return type TT, gets duration<tRep, tPeriod> param,
// makes up type LGT=unsigned long long or long double for respective T, return type is constexpr auto
#define SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_TO_UNITS(funkname,unitref)                                \
    template<class TT, class tRep, class tPeriod = std::ratio<1>,                                        \
        class LGT = std::conditional_t<SKLIB_TYPES_IS_INTEGER(TT), unsigned long long, long double>,     \
        std::enable_if_t<SKLIB_TYPES_IS_INTEGER(TT) || SKLIB_TYPES_IS_FLOATING_POINT(TT), bool> = true>  \
    constexpr TT funkname (const std::chrono::duration<tRep, tPeriod>& t)                                \
    { return TT((std::chrono::duration_cast<std::chrono::duration<LGT, unitref>>(t)).count()); }

    SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_TO_UNITS( time_to_nanoseconds,  std::nano     );
    SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_TO_UNITS( time_to_microseconds, std::micro    );
    SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_TO_UNITS( time_to_milliseconds, std::milli    );
    SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_TO_UNITS( time_to_seconds,      std::ratio<1> );

#undef SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_FROM_UNITS
#undef SKLIB_INTERNAL_TIMER_DECLARE_CONVERTER_TO_UNITS

/*/
    SKLIB_INTERNAL_TEMPLATE_CE_IF_INT_OR_FLOAT_TT_DURATN_SELECT_LGT TT time_to_nanoseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return TT((std::chrono::duration_cast<std::chrono::duration<LGT, std::nano>(t)).count()); }

    SKLIB_INTERNAL_TEMPLATE_CE_IF_INT_OR_FLOAT_TT_DURATN_SELECT_LGT TT time_to_microseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return TT((std::chrono::duration_cast<std::chrono::duration<LGT, std::micro>(t)).count()); }

    SKLIB_INTERNAL_TEMPLATE_CE_IF_INT_OR_FLOAT_TT_DURATN_SELECT_LGT TT time_to_milliseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return TT((std::chrono::duration_cast<std::chrono::duration<LGT, std::milli>(t)).count()); }

    SKLIB_INTERNAL_TEMPLATE_CE_IF_INT_OR_FLOAT_TT_DURATN_SELECT_LGT TT time_to_seconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return TT((std::chrono::duration_cast<std::chrono::duration<LGT>(t)).count()); }
*/
/*
    template<class tTarget, class tRep, class tPeriod = std::ratio<1>, std::enable_if_t<std::is_integral_v<tTarget>, bool> = true>
    constexpr tTarget time_to_nanoseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return tTarget((std::chrono::duration_cast<std::chrono::nanoseconds>(t)).count()); }

    template<class tTarget, class tRep, class tPeriod = std::ratio<1>, std::enable_if_t<std::is_integral_v<tTarget>, bool> = true>
    constexpr tTarget time_to_microseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return tTarget((std::chrono::duration_cast<std::chrono::microseconds>(t)).count()); }

    template<class tTarget, class tRep, class tPeriod = std::ratio<1>, std::enable_if_t<std::is_integral_v<tTarget>, bool> = true>
    constexpr tTarget time_to_milliseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return tTarget((std::chrono::duration_cast<std::chrono::milliseconds>(t)).count()); }

    template<class tTarget, class tRep, class tPeriod = std::ratio<1>, std::enable_if_t<std::is_integral_v<tTarget>, bool> = true>
    constexpr tTarget time_to_seconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return tTarget((std::chrono::duration_cast<std::chrono::seconds>(t)).count()); }

    template<class tTarget, class tRep, class tPeriod = std::ratio<1>, std::enable_if_t<std::is_floating_point_v<tTarget>, bool> = true>
    constexpr tTarget time_to_nanoseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return (std::chrono::duration_cast<std::chrono::duration<tTarget, std::nano>>(t)).count(); }

    template<class tTarget, class tRep, class tPeriod = std::ratio<1>, std::enable_if_t<std::is_floating_point_v<tTarget>, bool> = true>
    constexpr tTarget time_to_microseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return (std::chrono::duration_cast<std::chrono::duration<tTarget, std::micro>>(t)).count(); }

    template<class tTarget, class tRep, class tPeriod = std::ratio<1>, std::enable_if_t<std::is_floating_point_v<tTarget>, bool> = true>
    constexpr tTarget time_to_milliseconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return (std::chrono::duration_cast<std::chrono::duration<tTarget, std::milli>>(t)).count(); }

    template<class tTarget, class tRep, class tPeriod = std::ratio<1>, std::enable_if_t<std::is_floating_point_v<tTarget>, bool> = true>
    constexpr tTarget time_to_seconds(const std::chrono::duration<tRep, tPeriod>& t)
    { return (std::chrono::duration_cast<std::chrono::duration<tTarget>>(t)).count(); }
*/


// 5. Platform-independent Sleep() analog

    template<class tRep, class tPeriod = std::ratio<1>>
    inline void time_wait(const std::chrono::duration<tRep, tPeriod>& t)
    { std::this_thread::sleep_for(t); }

//    Measure Sleep() time, or equivalent, in milliseconds is the long standing tradition in computing.
//    If different unit is required, and using time literals not possible, use conversion functions:
//    seconds, microseconds, etc, see above

    inline void time_wait(size_t t_msec)
    { std::this_thread::sleep_for(std::chrono::milliseconds(t_msec)); }


// 6. Timeout and/or counter timer class implemented around std::chrono::steady_clock

//sk TODO replace description

// Interface:
//      timer_stopwatch_type  Create timer, either stopped or running. Can specify timeout
//      reset           Reset Stopwatch and Timeout timer. Start or stop according to the initial state. Optional: specify new timeout
//      start           Start Stopwatch, reset timeout start time (if wasn't running). Optional: specify new timeout
//      stop            Stop Stopwatch, returns the most recent interval duration
//      read            Returns Stopwatch time (i.e. counting only when running)
//      is_running      Returns True if the timer is running
//      is_timeout      Returns True if time elapsed since the timeout start time is greater than the set time (Timeout event).
//                      Optional: specify alternative timeout period
//
// Use case: typical timeout monitor
//
//    timer_stopwatch_type timeout(delay_seconds);
//    while (...condition...)
//    {
//        if (timeout) { ...handle timeout event... }   // explicit bool() in the name!
//        ...keep processing and/or waiting...
//    }
//
// Use case: typical elapsed time monitor
//
//    timer_stopwatch_type elapsed;
//    while (...condition...) { ...do something... }
//    std::chrono::steady_clock::duration measured_time = elapsed.read();            // also, has explicit duration() cast in the name
//

    // Timeout and Stopwatch timers implemented around std::chrono::steady_clock
    // Intended for use either in timeout, or stopwatch mode, however mixed operation is possible in certain cases
    // Strobe is special version of timeout when it resets itself after returning "timeout=true" - parameter in is_timeout()
    // Timer declaration:  stopwatch_t [<false>] variable [(timeout_interval)] ;
    // Timeout start time is captured at the time of creation (or can be set by start() member function)
    // Timer is started at creation by default, or created stopped if <false> is specified
    // All time intervals are floating point values in seconds
    //
    class timer_stopwatch_type
    {
    public:
        enum class create_mode_type { running, idle };
        static constexpr create_mode_type create_mode_idle    = create_mode_type::idle;
        static constexpr create_mode_type create_mode_running = create_mode_type::running;

        class duration_helper_type
        {
            friend timer_stopwatch_type;

        private:
            std::chrono::steady_clock::duration value;

        public:
            template<class tRep, class tPeriod = std::ratio<1>>
            duration_helper_type(const std::chrono::duration<tRep, tPeriod>& input) : value(std::chrono::duration_cast<std::chrono::steady_clock::duration>(input)) {}

            template<class T, std::enable_if_t<(std::is_integral_v<T> || std::is_floating_point_v<T>), bool> = true>
            duration_helper_type(const T& input) : value(std::chrono::duration_cast<std::chrono::steady_clock::duration>(::sklib::time_milliseconds(input))) {}
        };

    private:
        std::chrono::steady_clock::duration time_accumulator;  // all variables will be initialized at construction
        std::chrono::steady_clock::duration time_delay;
        std::chrono::steady_clock::time_point start_time;
        bool timer_active;
        bool created_active;

        void configure(bool initialize, bool start, bool update_timeout = false, const duration_helper_type& timeout = 0)
        {
            if (initialize) time_accumulator = std::chrono::steady_clock::duration(0);
            if (initialize || (start && !timer_active)) start_time = std::chrono::steady_clock::now();
            if (update_timeout) time_delay = timeout.value;
            timer_active = start;
            // 'created_active' is separately set in construction time
        }

        std::chrono::steady_clock::duration get_dt() const
        { return std::chrono::steady_clock::now() - start_time; }

        void create(const duration_helper_type& timeout, create_mode_type mode)
        {
            created_active = (mode == create_mode_running);
            reset(timeout, mode);
        }

    public:
        // Create Timeout / Stopwatch timer
        // Timer is running by default or stopped if stopwatch_t<false> is used
        // Timeout start time is captured at the time of creation
        // Timeout interval in seconds is set by the parameter
        // See the description of the class for more details
        //

        // timer() - running, 0 timeout
        // timer(time) - running, set timeout
        // timer(time, mode)
        // timer(mode) - create with mode, 0 timeout

        timer_stopwatch_type(const duration_helper_type& timeout, create_mode_type mode = create_mode_running)
        { create(timeout, mode); }

        timer_stopwatch_type(create_mode_type mode = create_mode_running)
        { create(0, mode); }

        // reset() - restart timer with creation params
        // reset(time) - restart and change only 
        // restart(time, mode)
        // restart(mode)

        void reset()
        { configure(true, created_active); }

        void reset(const duration_helper_type& timeout, create_mode_type mode = create_mode_running)
        { configure(true, (mode == create_mode_running), true, timeout); }

        void reset(create_mode_type mode)
        { configure(true, (mode == create_mode_running)); }

        //

        // If timer is stopped, start stopwatch and reset (refresh) timeout start time
        // If timer is running, do nothing
        // Timeout interval is not modified
        //
        void start()
        { configure(false, true); }

        // If timer is stopped, start stopwatch and reset (refresh) timeout start time
        // If timer is running, timeout start time does NOT change
        // Timeout interval is updated to the new value
        //
        void start(const duration_helper_type& update_timeout)
        { configure(false, true, true, update_timeout); }

        // Returns true if timeout has occured, i.e.
        // more than previously specified timeout duration has elapsed since timer creation or last update
        // When enabled, bool() operator also returns is_timeout()
        //
        bool is_timeout(bool automatic_reset = false)
        { return is_timeout(time_delay, automatic_reset); }

        // Returns true if timeout has occured, i.e.
        // more than timeout_s seconds have elapsed since timer creation or last update
        //
        bool is_timeout(const duration_helper_type& timeout, bool automatic_reset = false)
        {
            if (read() <= timeout.value) return false;
            if (automatic_reset) reset();
            return true;
        }

        // synonym for is_timeout()
        explicit operator bool() { return is_timeout(); }
        bool operator() (bool automatic_reset = false) { return is_timeout(automatic_reset); }
        // Returns true if the timer is running
        //
        bool is_running() const { return timer_active; }

        // If timer was running, stop it and remember the time since the last start
        // Returns the duration of the most recent running interval in seconds, or 0 if timer wasn't running
        //
        std::chrono::steady_clock::duration stop()
        {
            if (!timer_active) return std::chrono::steady_clock::duration(0);
            timer_active = false;

            std::chrono::steady_clock::duration current_interval = get_dt();
            time_accumulator += current_interval;

            return current_interval;
        }

        // Returns elapsed Stopwatch time in seconds
        // Only time between start() and stop() is counted
        // (If timer is running, time since the last start is included, too)
        // When enabled, double() operator also returns read()
        //
        std::chrono::steady_clock::duration read() const
        {
            return time_accumulator + (timer_active ? get_dt() : std::chrono::steady_clock::duration(0));
        }

        // synonym for read()
        explicit operator std::chrono::steady_clock::duration() const { return read(); }
    };

// 7. Conversions directly from class timer_stopwatch_type type

    template<class tTarget>
    constexpr tTarget time_to_nanoseconds(const timer_stopwatch_type& t)
    { return time_to_nanoseconds<tTarget>(t.read()); }

    template<class tTarget>
    constexpr tTarget time_to_microseconds(const timer_stopwatch_type& t)
    { return time_to_microseconds<tTarget>(t.read()); }

    template<class tTarget>
    constexpr tTarget time_to_milliseconds(const timer_stopwatch_type& t)
    { return time_to_milliseconds<tTarget>(t.read()); }

    template<class tTarget>
    constexpr tTarget time_to_seconds(const timer_stopwatch_type& t)
    { return time_to_seconds<tTarget>(t.read()); }

};

#endif // SKLIB_INCLUDED_TIMER_HPP
