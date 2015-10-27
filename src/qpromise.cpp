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

void QDeferredPromise::doThen(const std::function<void(const QVariant&)>& fulfilled,
                              const std::function<void(const QPromiseException&)>& rejected) {
	if (mResolvedPromise) {
		mResolvedPromise->doThen(fulfilled, rejected);
	} else {
		mQueue.push_back(qMakePair(fulfilled, rejected));
	}
}

QPromise QDeferredPromise::then(const std::function<QVariant(const QVariant&)>& fulfilled,
                                const std::function<QVariant(const QPromiseException&)>& rejected) {
	if (mResolvedPromise) {
		return mResolvedPromise->then(fulfilled, rejected);
	} else {
		auto deferred = Q::defer();

		mQueue.push_back(qMakePair(
		    [deferred, fulfilled](const QVariant& value) mutable { deferred.resolve(fulfilled(value)); },
		    [deferred, rejected](const QPromiseException& reason) mutable {
			    deferred.resolve(rejected(reason));
		    }));

		return deferred.promise();
	}
}

void QDeferredPromise::resolve(const QSharedPointer<QPromiseBase>& promise) {
	if (mResolvedPromise) {
		qWarning("Resolving already resolved promise");
		return;
	}
	// TODO: check if promise is this

	mResolvedPromise = promise;
	promise->doThen(
	    [thisBase = sharedFromThis()](const QVariant& value) {
		    auto thisPtr = static_cast<QDeferredPromise*>(thisBase.data());
		    for (const auto& p : thisPtr->mQueue) p.first(value);
	    },
	    [thisBase = sharedFromThis()](const QPromiseException& reason) {
		    auto thisPtr = static_cast<QDeferredPromise*>(thisBase.data());
		    for (const auto& p : thisPtr->mQueue) p.second(reason);
	    });
}

QFulfilledPromise::QFulfilledPromise(const QVariant& value) : mValue(value) {}

void QFulfilledPromise::doThen(const std::function<void(const QVariant&)>& fulfilled,
                               const std::function<void(const QPromiseException&)>&) {
	Q::nextTick([ thisBase = sharedFromThis(), fulfilled ]() mutable {
		auto thisPtr = static_cast<QFulfilledPromise*>(thisBase.data());
		fulfilled(thisPtr->mValue);
	});
}

QPromise QFulfilledPromise::then(const std::function<QVariant(const QVariant&)>& fulfilled,
                                 const std::function<QVariant(const QPromiseException&)>&) {
	auto deferred = Q::defer();

	Q::nextTick([ thisBase = sharedFromThis(), deferred, fulfilled ]() mutable {
		auto thisPtr = static_cast<QFulfilledPromise*>(thisBase.data());
		deferred.resolve(fulfilled(thisPtr->mValue));
	});

	return deferred.promise();
}

void QFulfilledPromise::resolve(const QSharedPointer<QPromiseBase>&) {
	qWarning("Resolving already resolved promise");
	return;
}

QRejectedPromise::QRejectedPromise(const QPromiseException& reason) : mReason(reason) {}

void QRejectedPromise::doThen(const std::function<void(const QVariant&)>&,
                              const std::function<void(const QPromiseException&)>& rejected) {

	Q::nextTick([ thisBase = sharedFromThis(), rejected ]() mutable {
		auto thisPtr = static_cast<QRejectedPromise*>(thisBase.data());
		rejected(thisPtr->mReason);
	});
}

QPromise QRejectedPromise::then(const std::function<QVariant(const QVariant&)>&,
                                const std::function<QVariant(const QPromiseException&)>& rejected) {
	auto deferred = Q::defer();

	Q::nextTick([ thisBase = sharedFromThis(), deferred, rejected ]() mutable {
	        auto thisPtr = static_cast<QRejectedPromise*>(thisBase.data());
	        deferred.resolve(rejected(thisPtr->mReason));
	});

	return deferred.promise();
}

void QRejectedPromise::resolve(const QSharedPointer<QPromiseBase>&) {
	qWarning("Resolving already resolved promise");
	return;
}

QPromise QPromise::then(const std::function<QVariant(const QVariant&)>& fulfilled,
                        const std::function<QVariant(const QPromiseException&)>& rejected) {
	return mPromise->then(fulfilled, rejected);
}

/*QPromise QPromise::then(const std::function<void(const QVariant&)>& fulfilled,
                        const std::function<QVariant(const QPromiseException&)>& rejected) {
	return mPromise->then([fulfilled](const QVariant& value) {
		fulfilled(value);
		return QVariant();
	}, rejected);
}

QPromise QPromise::then(const std::function<QVariant(const QVariant&)>& fulfilled,
                        const std::function<void(const QPromiseException&)>& rejected) {
	return mPromise->then(fulfilled, [rejected](const QPromiseException& reason) {
		rejected(reason);
		return QVariant();
	});
}

QPromise QPromise::then(const std::function<void(const QVariant&)>& fulfilled,
                        const std::function<void(const QPromiseException&)>& rejected) {
	return mPromise->then(
	    [fulfilled](const QVariant& value) -> QVariant {
		    fulfilled(value);
		    return QVariant();
	    },
	    [rejected](const QPromiseException& reason) -> QVariant {
		    rejected(reason);
		    return QVariant();
	    });
	    }*/

QDeferred::QDeferred() : mPromise(new QDeferredPromise()) {}

QPromise QDeferred::promise() const { return QPromise(mPromise); }

void QDeferred::resolve(const QVariant& value) { mPromise->resolve(QSharedPointer<QFulfilledPromise>::create(value)); }

void QDeferred::resolve(const QPromise& promise) { mPromise->resolve(promise.mPromise); }

void QDeferred::reject(const QPromiseException& reason) {
	mPromise->resolve(QSharedPointer<QRejectedPromise>::create(reason));
}

QDeferred Q::defer() { return QDeferred(); }

QPROMISE_END_NAMESPACE
