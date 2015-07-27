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

#include "private/traits.h"

#include <functional>
#include <type_traits>

#include <QException>
#include <QMetaMethod>
#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QTimer>
#include <QVariant>
#include <QVector>

namespace qpromise {

class QPromise;

class QPromiseBase : public QEnableSharedFromThis<QPromiseBase> {
	QVector<std::function<void(const QVariant&)>> mQueue;

public:
	QPromise then(const std::function<QVariant(const QVariant&)>& fulfilled);

	void then(const QSharedPointer<QPromiseBase>& promise);

	void resolve(const QVariant& value);

	void resolve(QPromiseBase& promise);
};

template <typename T> T qvariantCast(const QVariant& v);
template <> inline int qvariantCast<int>(const QVariant& v) { return v.toInt(); }

class QPromise {
	friend class QDeferred;

	template <typename F> struct FulfillTraits: public priv::FunctionTraits<F> {};

	QSharedPointer<QPromiseBase> mPromise;

	QPromise(const QSharedPointer<QPromiseBase>& promise) : mPromise(promise) {}

public:
	template <typename F = std::nullptr_t, typename E = std::nullptr_t,
	          std::enable_if_t<
	              !std::is_void<typename FulfillTraits<F>::ReturnType>::value &&
	              std::is_same<QVariant, typename FulfillTraits<F>::template Arg<0>::DecayType>::value>* = nullptr>
	QPromise then(F&& fulfilled = nullptr, E&& = nullptr) {
		return mPromise->then(std::forward<F>(fulfilled));
	}

	template <
	    typename F, typename E = std::nullptr_t,
	    std::enable_if_t<
	        !std::is_same<std::nullptr_t, F>::value && std::is_void<typename FulfillTraits<F>::ReturnType>::value &&
	        std::is_same<QVariant, typename FulfillTraits<F>::template Arg<0>::DecayType>::value>* = nullptr>
	QPromise then(F&& fulfilled = nullptr, E&& = nullptr) {
		return mPromise->then([fulfilled](const QVariant& v) {
			fulfilled(v);
			return QVariant();
		});
	}

	template <typename F, typename E = std::nullptr_t,
	          typename T = std::enable_if_t<
	              !std::is_same<std::nullptr_t, F>::value &&
	                  !std::is_same<QVariant, typename FulfillTraits<F>::template Arg<0>::DecayType>::value,
	              typename FulfillTraits<F>::template Arg<0>::DecayType>>
	QPromise then(F&& fulfilled = nullptr, E&& = nullptr) {
		return then([fulfilled](const QVariant& v) { return fulfilled(qvariantCast<T>(v)); });
	}
};

template <> struct QPromise::FulfillTraits<std::nullptr_t> : public priv::FunctionTraits<QVariant(const QVariant&)> {};

class QDeferred {
	Q_GADGET

	QSharedPointer<QPromiseBase> mPromise;

public:
	QDeferred() : mPromise(new QPromiseBase()) {}

	Q_PROPERTY(QPromise promise READ promise);

	Q_INVOKABLE QPromise promise() const { return QPromise(mPromise); }

	Q_INVOKABLE void resolve(const QVariant& value);

	Q_INVOKABLE void resolve(const QPromise& promise);
};

class Q {
public:
	template <typename F> static void nextTick(F&& f) { QTimer::singleShot(0, f); }

        static QDeferred defer() { return QDeferred(); }

	template <typename T, std::enable_if_t<std::is_same<QPromise, typename std::decay<T>::type>::value>* = nullptr>
	static constexpr auto resolve(T&& v) {
		return std::forward<T>(v);
	}

	template <typename T, std::enable_if_t<!std::is_same<QPromise, typename std::decay<T>::type>::value>* = nullptr>
	static auto resolve(T&& v) {
		fulfill(v);
	}

	template <typename T> static auto fulfill(T&& v) {
		QDeferred d;
		d.resolve(std::forward<T>(v));
		return d.promise();
	}

	template <typename T, typename F> static auto when(T&& value, F&& fulfilled) {
		return resolve(std::forward<T>(value)).then(std::forward<F>(fulfilled));
	}
};
}

#include "private/qpromiseimpl.h"

#endif
