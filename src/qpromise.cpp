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

QPromise QPromiseBase::then(const std::function<QVariant(const QVariant&)>& fulfilled,
			    const std::function<QVariant(const QException&)>& rejected) {
	auto deferred = Q::defer();

	auto onFulfilled = [fulfilled](const QVariant& value) {
		if (fulfilled) {
			return fulfilled(value);
		} else {
			return value;
		}
	};

	auto onRejected = [rejected](const QException& exception) {
		return rejected(exception);
	};

	Q::nextTick([ thisPtr = sharedFromThis(), deferred, onFulfilled, onRejected ]() mutable {
		std::function<QVariant(QException&)> arg0 = [deferred, onRejected](const QException& exception) mutable {
			deferred.resolve(onRejected(exception));
		};
		thisPtr->promiseDispatch([deferred, onFulfilled](const QVariant& value) mutable {
			deferred.resolve(onFulfilled(value));
			}, "when", Q_ARG(decltype(arg0), arg0));
	});

	return deferred.promise();
}

void QResolvedPromise::doPromiseDispatch(const std::function<void(const QVariant&)>& f, const char* op,
					 const QVector<QGenericArgument>& args) {
	QVariant r;

	switch (args.size()) {
	case 0: r = mDescriptor->invoke(op); break;
	case 1: r = mDescriptor->invoke(op, args[0]); break;
	default: Q_ASSERT(!"ololo");
	}

	f(r);
}

void QDeferredPromise::become(const QSharedPointer<QPromiseBase>& resolvedPromise) {
	mResolvedPromise = resolvedPromise;
}

void QDeferredPromise::doPromiseDispatch(const std::function<void(const QVariant&)>& f, const char* op,
					 const QVector<QGenericArgument>& args) {
	mResolvedPromise->doPromiseDispatch(f, op, args);
}

void QDeferred::become(const QPromise& v) { mPromise->become(v.mPromise); }

void QDeferred::resolve(const QVariant& value) { become(Q::resolve(value)); }

void QDeferred::resolve(const QPromise& promise) { become(promise); }

void QDeferred::reject(const QException& reason) { become(Q::reject(reason)); }

}
