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

class QInvokable {
protected:
	virtual const QMetaObject* metaObject() const = 0;

public:
	virtual ~QInvokable() = default;
	// virtual QInvokable* clone() const = 0;

	template <typename... T> QVariant invoke(const char* member, T... args) {
		QVarLengthArray<char, 512> sig;
		int len = qstrlen(member);
		if (len <= 0) return QVariant();
		sig.append(member, len);
		sig.append('(');

		const char* typeNames[] = {args.name()...};

		int paramCount;
		for (paramCount = 0; paramCount != sizeof...(args); ++paramCount) {
			len = qstrlen(typeNames[paramCount]);
			sig.append(typeNames[paramCount], len);
			sig.append(',');
		}
		if (paramCount == 0)
			sig.append(')'); // no parameters
		else
			sig[sig.size() - 1] = ')';
		sig.append('\0');

		const QMetaObject* meta = metaObject();
		int idx = meta->indexOfMethod(sig.constData());
		if (idx < 0) {
			QByteArray norm = QMetaObject::normalizedSignature(sig.constData());
			idx = meta->indexOfMethod(norm.constData());
		}

		if (idx < 0 || idx >= meta->methodCount()) {
			return QVariant();
		}
		QMetaMethod method = meta->method(idx);

		QVariant r;
		method.invokeOnGadget(this, Q_RETURN_ARG(QVariant, r), args...);
		return r;
	}
};

class QPromiseBase : public QEnableSharedFromThis<QPromiseBase> {
public:
	enum State { PendingState, ResolvedState, RejectedState };

public:
	virtual ~QPromiseBase() = default;

	virtual void doPromiseDispatch(const std::function<void(const QVariant&)>& f, const char* op,
	                               const QVector<QGenericArgument>& args) = 0;

	template<typename... T>
	void promiseDispatch(const std::function<void(const QVariant&)>& f, const char* op, T... args) {
		doPromiseDispatch(f, op, QVector<QGenericArgument>{args...});
	}

	QPromise then(const std::function<QVariant(const QVariant&)>& fulfilled,
	              const std::function<QVariant(const QException&)>& rejected);
};

class QResolvedPromise : public QPromiseBase {
	QScopedPointer<QInvokable> mDescriptor;
	State mState;

public:
	QResolvedPromise(QResolvedPromise&& o) : mState(o.mState) { mDescriptor.swap(o.mDescriptor); }

	template <typename T>
	QResolvedPromise(T&& descriptor, State state)
	    : mDescriptor(new typename std::remove_reference<T>::type(std::forward<T>(descriptor))), mState(state) {}

protected:
	void doPromiseDispatch(const std::function<void(const QVariant&)>& f, const char* op,
	                       const QVector<QGenericArgument>& args) override;
};

class QDeferredPromise : public QPromiseBase {
	friend class QDeferred;

	QSharedPointer<QPromiseBase> mResolvedPromise;

	void become(const QSharedPointer<QPromiseBase>& resolvedPromise);

protected:
	void doPromiseDispatch(const std::function<void(const QVariant&)>& f, const char* op,
	                       const QVector<QGenericArgument>& args) override;
};

template <typename T> T qvariantCast(const QVariant& v);
template <> inline int qvariantCast<int>(const QVariant& v) { return v.toInt(); }

class QPromise {
	friend class QDeferred;

	template <typename F> struct FulfillTraits: public priv::FunctionTraits<F> {};

	QSharedPointer<QPromiseBase> mPromise;

	QPromise(const QSharedPointer<QPromiseBase>& promise) : mPromise(promise) {}

public:
	template <typename T>
	QPromise(T&& descriptor, QPromiseBase::State state)
	    : mPromise(new QResolvedPromise(std::forward<T>(descriptor), state)) {}

	template <typename F = std::nullptr_t, typename E = std::nullptr_t,
		std::enable_if_t<!std::is_void<typename FulfillTraits<F>::ReturnType>::value &&
		std::is_same<QVariant, typename FulfillTraits<F>::template Arg<0>::DecayType>::value>* = nullptr>
	QPromise then(F&& fulfilled = nullptr, E&& rejected = nullptr) {
		return mPromise->then(std::forward<F>(fulfilled), std::forward<E>(rejected));
	}

	template <typename F, typename E = std::nullptr_t,
		std::enable_if_t<!std::is_same<std::nullptr_t, F>::value &&
		                 std::is_void<typename FulfillTraits<F>::ReturnType>::value &&
	                         std::is_same<QVariant, typename FulfillTraits<F>::template Arg<0>::DecayType>::value>* = nullptr>
	QPromise then(F&& fulfilled = nullptr, E&& rejected = nullptr) {
		(void)rejected;
		return mPromise->then([fulfilled](const QVariant& v) {
			fulfilled(v);
			return QVariant();
		}, std::forward<E>(rejected));
	}

	template <typename F, typename E = std::nullptr_t,
	          typename T = std::enable_if_t<!std::is_same<std::nullptr_t, F>::value &&
	                                        !std::is_same<QVariant, typename FulfillTraits<F>::template Arg<0>::DecayType>::value,
		typename FulfillTraits<F>::template Arg<0>::DecayType>>
	QPromise then(F&& fulfilled = nullptr, E&& rejected = nullptr) {
		(void)rejected;
		return then([fulfilled](const QVariant& v) { return fulfilled(qvariantCast<T>(v)); },
		            std::forward<E>(rejected));
	}
};

template <>
struct QPromise::FulfillTraits<std::nullptr_t>: public priv::FunctionTraits<QVariant(const QVariant&)> {};

class QFulfilledDescriptor : public QInvokable {
	Q_GADGET

	QVariant mValue;

protected:
	const QMetaObject* metaObject() const override { return &staticMetaObject; }

public:
	QFulfilledDescriptor(const QFulfilledDescriptor&) = default;
	QFulfilledDescriptor(QFulfilledDescriptor&&) = default;

	explicit QFulfilledDescriptor(const QVariant& v) : mValue(v) {}

	Q_INVOKABLE QVariant when(const std::function<QVariant(QException&)>&) { return mValue; }
};

template <typename T>
class QWrappedException : public QException {
	T mException;

public:
	QWrappedException() = default;

	QWrappedException(T&& exception) : mException(std::forward<T>(exception)) {}

	const char* what() const noexcept override {
		return mException.what();
	}

	constexpr const T& wrapped() const noexcept {
		return mException;
	}
};

template <typename T>
constexpr auto wrapException(T&& exception) {
	return QWrappedException<T>(std::forward<T>(exception));
}

class QRejectedDescriptor : public QInvokable {
	Q_GADGET

	QScopedPointer<QException> mReason;

protected:
	const QMetaObject* metaObject() const override { return &staticMetaObject; }

public:
	QRejectedDescriptor(QRejectedDescriptor&&) = default;

	QRejectedDescriptor(const QRejectedDescriptor& o) : mReason(o.mReason->clone()) {}

	explicit QRejectedDescriptor(const QException& reason) : mReason(reason.clone()) {}

	Q_INVOKABLE QVariant when(const std::function<QVariant(QException&)>& rejected) { return rejected(*mReason); }
};

class QDeferred {
	Q_GADGET

	QSharedPointer<QDeferredPromise> mPromise;

	void become(const QPromise& v);

public:
	QDeferred() : mPromise(new QDeferredPromise()) {}

	Q_PROPERTY(QPromise promise READ promise);

	Q_INVOKABLE QPromise promise() const { return QPromise(mPromise); }

	Q_INVOKABLE void resolve(const QVariant& value);

	Q_INVOKABLE void resolve(const QPromise& promise);

	Q_INVOKABLE void reject(const QException& reason);
};

class Q {
public:
	template <typename F> static void nextTick(F&& f) { QTimer::singleShot(0, f); }

        static QDeferred defer() { return QDeferred(); }

        template <typename T, std::enable_if_t<std::is_same<QPromise, typename std::decay<T>::type>::value>* = nullptr>
        static constexpr auto resolve(T&& v) { return std::forward<T>(v); }

        template <typename T, std::enable_if_t<!std::is_same<QPromise, typename std::decay<T>::type>::value>* = nullptr>
	static auto resolve(T&& v) { return fulfill(std::forward<T>(v)); }

	template <typename T> static auto fulfill(T&& v) {
		return QPromise(QFulfilledDescriptor(std::forward<T>(v)), QPromiseBase::ResolvedState);
	}

	template <typename T> static auto reject(T&& v) {
		return QPromise(QRejectedDescriptor(wrapException(std::forward<T>(v))), QPromiseBase::RejectedState);
	}

	template <typename T, typename F>
	static auto when(T&& value, F&& fulfilled) {
		return resolve(std::forward<T>(value)).then(std::forward<F>(fulfilled));
	}
};
}

#include "private/qpromiseimpl.h"

#endif
