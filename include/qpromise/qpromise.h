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

#ifndef QPROMISE_H
#define QPROMISE_H

#include "private/qpromise.h"


namespace qpromise {

template<typename R, typename U>
class QDeferred;

template<typename R, typename U>
class QPromise {
	friend class priv::Detail<R, U>;
	friend class QDeferred<R, U>;
	QPromise(const QPromise&) = delete;
	QPromise& operator=(const QPromise&) = delete;

	std::unique_ptr<priv::Detail<R, U>> d;

public:
	explicit QPromise(priv::Detail<R, U>* p);
	QPromise(QPromise&&);
	~QPromise();

	template<typename F1 = std::nullptr_t,
		 typename F2 = std::nullptr_t, typename F3 = std::nullptr_t>
	QPromise<typename priv::FulfillFunction<F1, R>::ReturnType>
	then(F1&& onFulfilled = nullptr,
	     F2&& onRejected = nullptr, F3&& onProgress = nullptr);

	template<typename F>
	QPromise<R> fail(F&& onRejected);
};

template<typename R, typename U>
class QDeferred {
	QDeferred(const QDeferred&) = delete;
	QDeferred& operator=(const QDeferred&) = delete;

	std::unique_ptr<priv::Detail<R, U>> d;

public:
	QDeferred();
	QDeferred(QDeferred&&);
	~QDeferred();

	QPromise<R, U> promise();

	void resolve();

	template<typename T>
	typename std::enable_if<!std::is_void<T>::value && std::is_same<R, T>::value, void>::type
	resolve(const T&);

	template<typename T>
	typename std::enable_if<std::is_same<QPromise<R, U>, T>::value, void>::type
	resolve(T&&);

	void reject(const std::exception&);

	void notify();

	template<typename T>
	typename std::enable_if<!std::is_void<T>::value && std::is_same<U, T>::value, void>::type
	notify(const T&);
};

QPromise<> fulfill();

template<typename T>
QPromise<T> fulfill(T&&);

template<typename R = void, typename U = void, typename E>
QPromise<R, U> reject(E&&);

}

#include "private/qpromiseimpl.h"

#endif
