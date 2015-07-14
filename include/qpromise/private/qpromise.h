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

#ifndef QPRIVATE_PROMISE_H
#define QPRIVATE_PROMISE_H

#include "exception.h"
#include "traits.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>


namespace qpromise {

template<typename R = void, typename U = void>
class QPromise;

template<typename R = void, typename U = void>
class QDeferred;

namespace priv {

template<typename R>
constexpr auto makePropagation() noexcept {
	return [](const R& r) {return r;};
}

template<> inline
auto makePropagation<void>() noexcept {
	return [](){};
}

template<typename F, typename>
struct FulfillFunction {
	typedef F Type;
	typedef typename FunctionTraits<Type>::ReturnType ReturnType;

	template<typename T>
	static constexpr auto wrap(T&& v) noexcept {
		return std::forward<T>(v);
	}
};

template<typename R>
struct FulfillFunction<std::nullptr_t, R> {
	typedef decltype(makePropagation<R>()) Type;
	typedef typename FunctionTraits<Type>::ReturnType ReturnType;

	static constexpr auto wrap(std::nullptr_t&) noexcept {
		return makePropagation<R>();
	}
};

template<typename F, typename>
struct RejectFunction {
	template<typename T>
	static constexpr auto wrap(T&& v) noexcept {
		return std::forward<T>(v);
	}
};

template<typename R>
struct RejectFunction<std::nullptr_t, R> {
	static constexpr auto wrap(std::nullptr_t&) noexcept {
		return [](const std::exception& e) -> R {
			throw e;
		};
	}
};

/*template<>
struct FulfillFunction<std::nullptr_t, void> {
	typedef std::nullptr_t Type;
	typedef void ReturnType;

	static constexpr auto cast(std::nullptr_t) noexcept {
		return nullptr;
	}
};*/

template<typename F, typename R>
struct FulfillFunction<F&, R>: public FulfillFunction<F, R> {};

template<typename F, typename... R>
struct ResultOf {
	typedef decltype(std::declval<F>()(std::declval<R>()...)) Type;
};



/*template<typename F>
struct ResultOf<F, void> {
	typedef decltype(std::declval<F>()()) Type;
};

template<typename... R>
struct ResultOf<std::nullptr_t, R...> {
	typedef void Type;
};

template<>
struct ResultOf<std::nullptr_t, void> {
	typedef void Type;
};*/

/*template<typename... R>
struct ResultOf<std::nullptr_t&, R...> {
	typedef void Type;
	};*/

template<typename R, typename T>
struct CallbackFunc {
	typedef std::function<R (const T&)> Type;
};

template<typename R>
struct CallbackFunc<R, void> {
	typedef std::function<R ()> Type;
};

template<typename... R>
struct ArgTuple {
	typedef std::tuple<R...> Type;
};

template<>
struct ArgTuple<void> {
	typedef std::tuple<> Type;
};

// std::tuple<> for void, std::tuple<R...> for other types
/*template<typename F, typename... R>
struct ArgsOf {
	typedef typename std::conditional<std::is_void<typename ResultOf<F, R...>::Type>::value, std::tuple<>, std::tuple<R...>>::type Type;
	};*/

template<typename R, typename U>
class ChildChain;

template<typename R>
class ChainResolveBase {
public:
	virtual void resolve(const R&) = 0;
};

template<>
class ChainResolveBase<void> {
public:
	virtual void resolve() = 0;
};

template<typename U>
class ChainNotifyBase {
public:
	virtual void notify(const U&) = 0;
};

template<>
class ChainNotifyBase<void> {
public:
	virtual void notify() = 0;
};

template<typename R, typename U>
class ChainBase: public ChainResolveBase<R>, public ChainNotifyBase<U> {
	ChainBase(const ChainBase&) = delete;
	ChainBase& operator=(const ChainBase&) = delete;

public:
	std::unique_ptr<ChainBase<R, U>> mNext;

	ChainBase() {}
	virtual ~ChainBase() {}
	virtual void reject(const ExceptionPtr&) = 0;
};

template<typename R>
class DetailReasonHolder {
protected:
	std::unique_ptr<R> mReason;
};

template<>
class DetailReasonHolder<void> {
};

template<typename R, typename U = void>
class Detail: public DetailReasonHolder<R> {
	friend class QPromise<R, U>;
	friend class QDeferred<R, U>;
	friend class ChildChain<R, U>;
	Detail(const Detail&) = delete;
	Detail& operator=(const Detail&) = delete;

	struct {
		bool isFulfilled : 1;
		bool isRejected : 1;
		bool hasDeferred : 1;
		bool hasPromise : 1;
		bool hasParent : 1;
	} mState;

	std::unique_ptr<ChainBase<R, U>> mChain;
	ExceptionPtr mExceptionPtr;

	void addChild(Detail<R, U>*);

	template<typename F>
	typename std::enable_if<std::is_same<typename FulfillFunction<F, R>::ReturnType, void>::value
	                        && std::is_same<R, void>::value,
                                QPromise<typename FulfillFunction<F, R>::ReturnType>>::type
	fulfill(F&&);

	template<typename F>
	typename std::enable_if<!std::is_same<typename FulfillFunction<F, R>::ReturnType, void>::value
	                        && std::is_same<R, void>::value,
                                QPromise<typename FulfillFunction<F, R>::ReturnType>>::type
	fulfill(F&&);

	template<typename F>
	typename std::enable_if<std::is_same<typename FulfillFunction<F, R>::ReturnType, void>::value
	                        && !std::is_same<R, void>::value,
                                QPromise<typename FulfillFunction<F, R>::ReturnType>>::type
	fulfill(F&&);

	template<typename F>
	typename std::enable_if<!std::is_same<typename FulfillFunction<F, R>::ReturnType, void>::value
	                        && !std::is_same<R, void>::value,
	                        QPromise<typename FulfillFunction<F, R>::ReturnType>>::type
	fulfill(F&&);

public:
	Detail();
	Detail(Detail&&);

	template<typename F1, typename F2, typename F3>
	QPromise<typename FulfillFunction<F1, R>::ReturnType>
	then(F1&& onFulfilled, F2&& onRejected, F3&& onProgress);

	QPromise<R, U> promise();

	void resolve();

	template<typename T>
	typename std::enable_if<!std::is_void<T>::value && std::is_same<R, T>::value, void>::type
	resolve(const T&);

	template<typename T>
	typename std::enable_if<std::is_same<QPromise<R, U>, T>::value, void>::type
	resolve(T&&);

	template<typename E>
	void reject(E&&);

	void notify();

	template<typename T>
	typename std::enable_if<!std::is_void<T>::value && std::is_same<U, T>::value, void>::type
	notify(const T&);
};

template<typename R, typename U, typename R2>
class ChainImpl: public ChainBase<R, U> {
protected:
	Detail<R2> mDetail;
	typename CallbackFunc<R2, R>::Type mOnFulfilled;
	std::function<void (const std::exception&)> mOnRejected;
	typename CallbackFunc<void, U>::Type mOnProgress;
};

template<typename R, typename U, typename R2>
class ChainResolve: public ChainImpl<R, U, R2> {
public:
	void resolve(const R&);
};

template<typename U, typename R2>
class ChainResolve<void, U, R2>: public ChainImpl<void, U, R2> {
public:
	void resolve();
};

template<typename R, typename U>
class ChainResolve<R, U, void>: public ChainImpl<R, U, void> {
public:
	void resolve(const R&);
};

template<typename U>
class ChainResolve<void, U, void>: public ChainImpl<void, U, void> {
public:
	void resolve();
};

template<typename R, typename U, typename R2>
class ChainNotify: public ChainResolve<R, U, R2> {
public:
	void notify(const U&);
};

template<typename R, typename R2>
class ChainNotify<R, void, R2>: public ChainResolve<R, void, R2> {
public:
	void notify();
};

template<typename R, typename U, typename R2>
class Chain: public ChainNotify<R, U, R2> {
public:
	template<typename F1, typename F2, typename F3>
	Chain(F1&& onFulfilled, F2&& onRejected, F3&& onProgress);

	QPromise<R2> promise();

	void reject(const ExceptionPtr&);
};

template<typename R, typename U>
class ChildChainImpl: public ChainBase<R, U> {
protected:
	Detail<R, U>* mChild;
};

template<typename R, typename U>
class ChildResolveChain: public ChildChainImpl<R, U> {
public:
	void resolve(const R&);
};

template<typename U>
class ChildResolveChain<void, U>: public ChildChainImpl<void, U> {
public:
	void resolve();
};

template<typename R, typename U>
class ChildNotifyChain: public ChildResolveChain<R, U> {
public:
	void notify(const U&);
};

template<typename R>
class ChildNotifyChain<R, void>: public ChildResolveChain<R, void> {
public:
	void notify();
};

template<typename R, typename U>
class ChildChain: public ChildNotifyChain<R, U> {
public:
	ChildChain(Detail<R, U>*);
	~ChildChain();

	void reject(const ExceptionPtr&);
};

template<typename T>
T propagate(const T& v) {
	return v;
}

}

}

#endif
