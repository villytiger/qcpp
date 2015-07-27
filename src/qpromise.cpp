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

QPromise QPromiseBase::then(const std::function<QVariant(const QVariant&)>& fulfilled) {
	auto deferred = Q::defer();

	mQueue.push_back(
	    [deferred, fulfilled](const QVariant& value) mutable { return deferred.resolve(fulfilled(value)); });

	return deferred.promise();
}

void QPromiseBase::then(const QSharedPointer<QPromiseBase>& promise) {
	mQueue.push_back([promise](const QVariant& value) mutable { promise->resolve(value); });
}

void QPromiseBase::resolve(const QVariant& value) {
	Q::nextTick([this, value]() {
		for (const auto& f : mQueue) f(value);
	});
}

void QPromiseBase::resolve(const QPromiseBase& promise) { promise.then(sharedFromThis()); }

void QDeferred::resolve(const QVariant& value) { mPromise->resolve(value); }

void QDeferred::resolve(const QPromise& promise) { mPromise->resolve(*(promise.mPromise)); }
}
