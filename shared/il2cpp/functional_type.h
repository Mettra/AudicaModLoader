#pragma once
//From https://stackoverflow.com/questions/27822277/finding-out-the-return-type-of-a-function-lambda-or-function/, since c++ has a hard time converting a lambda into a stl function

template <typename F>
struct functional_type_impl;

template <typename R, typename... Args>
struct functional_type_impl<R(Args...)> { using type = std::function<R(Args...)>; };

template <typename R, typename... Args>
struct functional_type_impl<R(Args..., ...)> { using type = std::function<R(Args...)>; };

template <typename R, typename... Args>
struct functional_type_impl<R(*)(Args...)> { using type = std::function<R(Args...)>; };

template <typename R, typename... Args>
struct functional_type_impl<R(*)(Args..., ...)> { using type = std::function<R(Args...)>; };

template <typename R, typename... Args>
struct functional_type_impl<R(&)(Args...)> { using type = std::function<R(Args...)>; };

template <typename R, typename... Args>
struct functional_type_impl<R(&)(Args..., ...)> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...)> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...)> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) &> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) &> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) && > { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) && > { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) const> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) const> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) const&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) const&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) const&&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) const&&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) volatile> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) volatile> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) volatile&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) volatile&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) volatile&&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) volatile&&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) const volatile> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) const volatile> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) const volatile&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) const volatile&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args...) const volatile&&> { using type = std::function<R(Args...)>; };

template <typename R, typename C, typename... Args>
struct functional_type_impl<R(C::*)(Args..., ...) const volatile&&> { using type = std::function<R(Args...)>; };

template <typename T, typename = void>
struct functional_type
	: functional_type_impl<T> {};

template <typename T>
struct functional_type<T, decltype(void(&T::operator()))>
	: functional_type_impl<decltype(&T::operator())> {};

template <typename T>
using functional_type_t = typename functional_type<T>::type;