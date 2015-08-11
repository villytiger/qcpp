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

#include "qpromiseglobal.h"
#include "qpromisetraits.h"

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

QPROMISE_BEGIN_NAMESPACE

QVariant propagateValue(const QVariant& value);

class QDeferred;
class QPromise;

class QPromiseException {};

class QPromiseBase : public QEnableSharedFromThis<QPromiseBase> {
public:

	virtual ~QPromiseBase() = default;

	virtual void doThen(const std::function<void(const QVariant&)>& fulfilled,
	                    const std::function<void(const QPromiseException&)>& rejected) = 0;

	virtual QPromise then(const std::function<QVariant(const QVariant&)>& fulfilled,
	                      const std::function<QVariant(const QPromiseException&)>& rejected) = 0;

	virtual void resolve(const QSharedPointer<QPromiseBase>& promise) = 0;
};

class QDeferredPromise : public QPromiseBase {
	QVector<QPair<std::function<void(const QVariant&)>, std::function<void(const QPromiseException&)>>> mQueue;
	QSharedPointer<QPromiseBase> mResolvedPromise;
public:
	void doThen(const std::function<void(const QVariant&)>& fulfilled,
	            const std::function<void(const QPromiseException&)>& rejected) override;

	QPromise then(const std::function<QVariant(const QVariant&)>& fulfilled,
	              const std::function<QVariant(const QPromiseException&)>& rejected) override;

	void resolve(const QSharedPointer<QPromiseBase>& promise) override;
};

class QFulfilledPromise : public QPromiseBase {
	QVariant mValue;

public:
	QFulfilledPromise(const QVariant& value);

	void doThen(const std::function<void(const QVariant&)>& fulfilled,
	            const std::function<void(const QPromiseException&)>& rejected) override;

	QPromise then(const std::function<QVariant(const QVariant&)>& fulfilled,
	              const std::function<QVariant(const QPromiseException&)>& rejected) override;

	void resolve(const QSharedPointer<QPromiseBase>& promise) override;
};

class QRejectedPromise : public QPromiseBase {
	QPromiseException mReason;

public:
	QRejectedPromise(const QPromiseException& reason);

	void doThen(const std::function<void(const QVariant&)>& fulfilled,
	            const std::function<void(const QPromiseException&)>& rejected) override;

	QPromise then(const std::function<QVariant(const QVariant&)>& fulfilled,
	              const std::function<QVariant(const QPromiseException&)>& rejected) override;

	void resolve(const QSharedPointer<QPromiseBase>& promise) override;
};

template <typename T> T qvariantCast(const QVariant& v);
template <> inline int qvariantCast<int>(const QVariant& v) { return v.toInt(); }

class QPromise {
	friend class QDeferred;

	template <typename F> struct FulfillTraits: public FunctionTraits<F> {};

	QSharedPointer<QPromiseBase> mPromise;

	QPromise(const QSharedPointer<QPromiseBase>& promise) : mPromise(promise) {}

public:
	QPromise(const QVariant& value) : mPromise(new QFulfilledPromise(value)) {}

	QPromise(const QPromiseException& reason) : mPromise(new QRejectedPromise(reason)) {}

	QPromise then(const std::function<QVariant(const QVariant&)>& fulfilled,
	              const std::function<QVariant(const QPromiseException&)>& rejected = nullptr);

	QPromise then(const std::function<void(const QVariant&)>& fulfilled,
	              const std::function<QVariant(const QPromiseException&)>& rejected = nullptr);

	QPromise then(const std::function<QVariant(const QVariant&)>& fulfilled,
	              const std::function<void(const QPromiseException&)>& rejected);

	QPromise then(const std::function<void(const QVariant&)>& fulfilled,
	              const std::function<void(const QPromiseException&)>& rejected);

	/*	template <typename F = std::nullptr_t, typename E = std::nullptr_t,
	                  std::enable_if_t<
	                      !std::is_void<typename FulfillTraits<F>::ReturnType>::value &&
	                      std::is_same<QVariant, typename FulfillTraits<F>::template Arg<0>::DecayType>::value>* =
	   nullptr>
	        QPromise then(F&& fulfilled = nullptr, E&& = nullptr) {
	                return mPromise->then(std::forward<F>(fulfilled));
	        }

	        template <
	            typename F, typename E = std::nullptr_t,
	            std::enable_if_t<
	                !std::is_same<std::nullptr_t, F>::value && std::is_void<typename
	   FulfillTraits<F>::ReturnType>::value &&
	                std::is_same<QVariant, typename FulfillTraits<F>::template Arg<0>::DecayType>::value>* =
	   nullptr>
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
	        }*/
};

/*constexpr auto wrapFulfilled(nullptr_t) noexcept { return propagateValue; }
template <typename F> constexpr auto wrapFulfilled(F&& fulfilled) noexcept { return std::forward<F>(fulfilled); }
template <typename F, typename R> QPromise QPromiseBase::then(F&& fulfilled, R&& rejected) {
        return doThen(wrapFulfilled(fulfilled), rejected);
}

template <> struct QPromise::FulfillTraits<std::nullptr_t> : public FunctionTraits<QVariant(const QVariant&)> {};*/

class Q {
public:
	template <typename F> static void nextTick(F&& f) { QTimer::singleShot(0, f); }

	static QDeferred defer();

	template <typename T, std::enable_if_t<std::is_same<QPromise, typename std::decay<T>::type>::value>* = nullptr>
	static constexpr auto resolve(T&& v) {
		return std::forward<T>(v);
	}

	template <typename T, std::enable_if_t<!std::is_same<QPromise, typename std::decay<T>::type>::value>* = nullptr>
	static auto resolve(T&& v) {
		return fulfill(v);
	}

	template <typename T> static auto fulfill(T&& v) {
		return QPromise(std::forward<T>(v));
	}

	template <typename T> static auto reject(T&& v) {
		return QPromise(std::forward<T>(v));
	}

	template <typename T, typename F> static auto when(T&& value, F&& fulfilled) {
		return resolve(std::forward<T>(value)).then(std::forward<F>(fulfilled));
	}
};

QPROMISE_END_NAMESPACE

#endif
