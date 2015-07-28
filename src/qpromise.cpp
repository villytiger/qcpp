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

#include <qpromise/qpromise.h>

namespace qpromise {

QPromise QDeferredPromise::then(const std::function<QVariant(const QVariant&)>& fulfilled) {
	auto deferred = Q::defer();

	mQueue.push_back(
	    [deferred, fulfilled](const QVariant& value) mutable { deferred.resolve(fulfilled(value)); });

	return deferred.promise();
}

void QPromiseBase::resolve(QPromiseBase& promise) {
	promise.then(sharedFromThis());
}

/*void QDeferredPromise::then(const QSharedPointer<QPromiseBase>& promise) {
	mQueue.push_back([promise](const QVariant& value) mutable { promise->resolve(value); });
}

void QPromiseBase::resolve(const QVariant& value) {
	Q::nextTick([this, value]() {
		for (const auto& f : mQueue) f(value);
	});
	}*/

QFulfilledPromise::QFulfilledPromise(const QVariant& value)
	: mValue(value) {
}

QPromise QFulfilledPromise::then(const std::function<QVariant(const QVariant&)>& fulfilled) override {
	auto deferred = Q::defer();

	Q::nextTick([thisPtr = sharedFromThis(), deferred, fulfilled]() mutable {
		deferred.resolve(fulfilled(thisPtr->mValue));
	});

	return deferred.promise();
}

void QFulfilledPromise::resolve(QPromiseBase& promise) {
	qWarning("Resolving already resolved promise");
	return;
}

void QDeferred::resolve(const QVariant& value) {
	auto resolvedPromise = QSharedPointer<QFulfilledPromise>::create(value);
	mPromise->resolve(resolvedPromise);
	mPromise = resolvedPromise;
}

void QDeferred::resolve(const QPromise& promise) {
	mPromise->resolve(promise);
	mPromise = promise;
}

}
