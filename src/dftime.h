/*
Dwarf Therapist
Copyright (c) 2020 Clement Vuchener

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef DFTIME_H
#define DFTIME_H

#include <array>
#include <chrono>
#include <iostream>
#include <tuple>

using df_time = std::chrono::duration<int64_t>;
using df_tick = std::chrono::duration<int32_t>;
using df_day = std::chrono::duration<int32_t, std::ratio<1200>>;
using df_week = std::chrono::duration<int32_t, std::ratio<1200*7>>;
using df_month = std::chrono::duration<int32_t, std::ratio<1200*28>>;
using df_season = std::chrono::duration<int32_t, std::ratio<1200*28*3>>;
using df_year = std::chrono::duration<int32_t, std::ratio<1200*28*12>>;

extern const std::array<const char *, 4> DFSeasons;
extern const std::array<const char *, 12> DFMonths;

// Use df_date<Durations...> convert single duration to a tuple of duration
// Durations must be sorted from largest unit to smallest
// e.g. df_date<df_year, df_month, df_day>::make_date
template <typename... Durations>
struct df_date;

template <typename D, typename... Ds>
struct df_date<D, Ds...>
{
    template <typename Rep, typename Period>
    static std::tuple<D, Ds...> make_date(std::chrono::duration<Rep, Period> d)
    {
        return std::tuple_cat(
                std::make_tuple(std::chrono::duration_cast<D>(d)),
                df_date<Ds...>::make_date(d%D{1}));
    }
};

template <>
struct df_date<>
{
    template <typename Rep, typename Period>
    static std::tuple<> make_date(std::chrono::duration<Rep, Period>)
    {
        return {};
    }
};

// df_date_convert<Duration> converts a tuple of duration to a single one
template <std::size_t I, typename Duration>
struct df_date_convert_impl
{
    template <typename... Ts>
    static Duration convert(const std::tuple<Ts...> &t)
    {
        return std::chrono::duration_cast<Duration>(std::get<I>(t)) + df_date_convert_impl<I-1, Duration>::convert(t);
    }
};

template <typename Duration>
struct df_date_convert_impl<0, Duration>
{
    template <typename... Ts>
    static Duration convert(const std::tuple<Ts...> &t)
    {
        return std::chrono::duration_cast<Duration>(std::get<0>(t));
    }
};

template <typename Duration, typename... Ts>
Duration df_date_convert(const std::tuple<Ts...> &t)
{
    return df_date_convert_impl<sizeof...(Ts)-1, Duration>::convert(t);
}

// Return st/nd/rd/th depending on n (in 1-30 range)
const char *day_suffix(int n);

#endif
