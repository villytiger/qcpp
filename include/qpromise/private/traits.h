// Copyright (C) 2015 Ilya Lyubimov
//
// This file is part of qpromise.
//
//  Qpromise is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Qpromise is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with qpromise.  If not, see <http://www.gnu.org/licenses/>.

#ifndef PRIVATE_TRAITS_H
#define PRIVATE_TRAITS_H

#include <functional>
#include <tuple>

namespace qpromise {
namespace priv {

template <typename T> struct FunctionTraits : public FunctionTraits<decltype(&T::operator())> {};

template <typename R, typename... Args> struct FunctionTraits<R(Args...)> {
	enum { arity = sizeof...(Args) };

	typedef R ReturnType;

	template <size_t i> struct Arg {
		typedef typename std::tuple_element<i, std::tuple<Args...>>::type Type;
		typedef typename std::decay<Type>::type DecayType;
	};
};

template <typename R, typename... Args> struct FunctionTraits<R (*)(Args...)> : public FunctionTraits<R(Args...)> {};

template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...)> : public FunctionTraits<R(Args...)> {
	typedef C& OwnerType;
};

template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...) const> : public FunctionTraits<R(Args...)> {
	typedef const C& OwnerType;
};

template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...) volatile> : public FunctionTraits<R(Args...)> {
	typedef volatile C& OwnerType;
};

template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...) const volatile> : public FunctionTraits<R(Args...)> {
	typedef const volatile C& OwnerType;
};

template <typename FunctionType>
struct FunctionTraits<std::function<FunctionType>> : public FunctionTraits<FunctionType> {};

template <typename T> struct FunctionTraits<T&> : public FunctionTraits<T> {};
template <typename T> struct FunctionTraits<const T&> : public FunctionTraits<T> {};
template <typename T> struct FunctionTraits<volatile T&> : public FunctionTraits<T> {};
template <typename T> struct FunctionTraits<const volatile T&> : public FunctionTraits<T> {};
template <typename T> struct FunctionTraits<T&&> : public FunctionTraits<T> {};
template <typename T> struct FunctionTraits<const T&&> : public FunctionTraits<T> {};
template <typename T> struct FunctionTraits<volatile T&&> : public FunctionTraits<T> {};
template <typename T> struct FunctionTraits<const volatile T&&> : public FunctionTraits<T> {};
}
}

#endif
