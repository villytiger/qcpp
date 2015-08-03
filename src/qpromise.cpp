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

#include "qdeferred.h"
#include "qpromise.h"

QPROMISE_BEGIN_NAMESPACE

QVariant propagateValue(const QVariant& value) { return value; }

QPromise QDeferredPromise::doThen(const std::function<QVariant(const QVariant&)>& fulfilled) {
	if (mResolvedPromise) {
		return mResolvedPromise->then(fulfilled);
	} else {
		auto deferred = Q::defer();

		mQueue.push_back(
		    [deferred, fulfilled](const QVariant& value) mutable { deferred.resolve(fulfilled(value)); });

		return deferred.promise();
	}
}

void QDeferredPromise::thenAction(const std::function<void(const QVariant&)>& action) {
	if (mResolvedPromise) {
		mResolvedPromise->thenAction(action);
	} else {
		mQueue.push_back(action);
	}
}

void QDeferredPromise::resolve(const QSharedPointer<QPromiseBase>& promise) {
	if (mResolvedPromise) {
		qWarning("Resolving already resolved promise");
		return;
	}
	// TODO: check if promise is this

	mResolvedPromise = promise;
	promise->thenAction([thisBase = sharedFromThis()](const QVariant& value) {
		auto thisPtr = static_cast<QDeferredPromise*>(thisBase.data());
		for (const auto& f : thisPtr->mQueue) f(value);
	});
}

QFulfilledPromise::QFulfilledPromise(const QVariant& value)
	: mValue(value) {
}

QPromise QFulfilledPromise::doThen(const std::function<QVariant(const QVariant&)>& fulfilled) {
	auto deferred = Q::defer();

	Q::nextTick([ thisBase = sharedFromThis(), deferred, fulfilled ]() mutable {
		auto thisPtr = static_cast<QFulfilledPromise*>(thisBase.data());
		deferred.resolve(fulfilled(thisPtr->mValue));
	});

	return deferred.promise();
}

void QFulfilledPromise::thenAction(const std::function<void(const QVariant&)>& action) {
	Q::nextTick([ thisBase = sharedFromThis(), action ]() mutable {
		auto thisPtr = static_cast<QFulfilledPromise*>(thisBase.data());
		action(thisPtr->mValue);
	});
}

void QFulfilledPromise::resolve(const QSharedPointer<QPromiseBase>&) {
	qWarning("Resolving already resolved promise");
	return;
}

QRejectedPromise::QRejectedPromise(const QPromiseException& reason) : mReason(reason) {}

QPromise QRejectedPromise::doThen(const std::function<QVariant(const QVariant&)>&) {
	auto deferred = Q::defer();

	/*Q::nextTick([ thisBase = sharedFromThis(), deferred, fulfilled ]() mutable {
	        auto thisPtr = static_cast<QRejectedPromise*>(thisBase.data());
	        deferred.resolve(fulfilled(thisPtr->mValue));
	});*/

	return deferred.promise();
}

void QRejectedPromise::thenAction(const std::function<void(const QVariant&)>&) {

	/*	Q::nextTick([ thisBase = sharedFromThis(), action ]() mutable {
	                auto thisPtr = static_cast<QRejectedPromise*>(thisBase.data());
	                action(thisPtr->mValue);
	        });*/
}

void QRejectedPromise::resolve(const QSharedPointer<QPromiseBase>&) {
	qWarning("Resolving already resolved promise");
	return;
}

QDeferred::QDeferred() : mPromise(new QDeferredPromise()) {}

QPromise QDeferred::promise() const { return QPromise(mPromise); }

void QDeferred::resolve(const QVariant& value) { mPromise->resolve(QSharedPointer<QFulfilledPromise>::create(value)); }

void QDeferred::resolve(const QPromise& promise) { mPromise->resolve(promise.mPromise); }

void QDeferred::reject(const QPromiseException& reason) {
	mPromise->resolve(QSharedPointer<QRejectedPromise>::create(reason));
}

QDeferred Q::defer() { return QDeferred(); }

QPROMISE_END_NAMESPACE
