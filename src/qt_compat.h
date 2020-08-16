#ifndef QT_COMPAT_H
#define QT_COMPAT_H

#include <qglobal.h>

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
// Add qOverload from Qt's qglobal.h
template <typename... Args>
struct QNonConstOverload
{
    template <typename R, typename T>
    constexpr auto operator()(R (T::*ptr)(Args...)) const noexcept -> decltype(ptr)
    { return ptr; }
    template <typename R, typename T>
    static constexpr auto of(R (T::*ptr)(Args...)) noexcept -> decltype(ptr)
    { return ptr; }
};
template <typename... Args>
struct QConstOverload
{
    template <typename R, typename T>
    constexpr auto operator()(R (T::*ptr)(Args...) const) const noexcept -> decltype(ptr)
    { return ptr; }
    template <typename R, typename T>
    static constexpr auto of(R (T::*ptr)(Args...) const) noexcept -> decltype(ptr)
    { return ptr; }
};
template <typename... Args>
struct QOverload : QConstOverload<Args...>, QNonConstOverload<Args...>
{
    using QConstOverload<Args...>::of;
    using QConstOverload<Args...>::operator();
    using QNonConstOverload<Args...>::of;
    using QNonConstOverload<Args...>::operator();
    template <typename R>
    constexpr auto operator()(R (*ptr)(Args...)) const noexcept -> decltype(ptr)
    { return ptr; }
    template <typename R>
    static constexpr auto of(R (*ptr)(Args...)) noexcept -> decltype(ptr)
    { return ptr; }
};
template <typename... Args> constexpr QOverload<Args...> qOverload = {};
template <typename... Args> constexpr QConstOverload<Args...> qConstOverload = {};
template <typename... Args> constexpr QNonConstOverload<Args...> qNonConstOverload = {};
#endif

#endif
