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

#ifndef QPROMISEIMPL_H
#define QPROMISEIMPL_H


namespace qpromise {

template<typename R, typename U>
QPromise<R, U>::QPromise(priv::Detail<R, U>* p)
	: d(p) {
	d->mState.hasPromise = true;
}

template<typename R, typename U>
QPromise<R, U>::QPromise(QPromise&& o)
	: d(std::move(o.d)) {
}

template<typename R, typename U>
QPromise<R, U>::~QPromise() {
	if (d) {
		d->mState.hasPromise = false;
		if (d->mState.hasDeferred || d->mState.hasParent) d.release();
	}
}

template<typename R, typename U>
template<typename F1, typename F2, typename F3>
QPromise<typename priv::FulfillFunction<F1, R>::ReturnType>
QPromise<R, U>::then(F1&& onFulfilled, F2&& onRejected, F3&& onProgress) {
	return d->then(priv::FulfillFunction<F1, R>::cast(onFulfilled),
		       std::forward<F2>(onRejected), std::forward<F3>(onProgress));
}

template<typename R, typename U>
template<typename F>
QPromise<R> QPromise<R, U>::fail(F&& onRejected) {
	return then(priv::makePropagation<R>(), onRejected);
}

template<typename R, typename U>
QDeferred<R, U>::QDeferred()
	: d(new priv::Detail<R, U>()) {
}

template<typename R, typename U>
QDeferred<R, U>::QDeferred(QDeferred&& o)
	: d(std::move(o.d)) {
}

template<typename R, typename U>
QDeferred<R, U>::~QDeferred() {
	if (d) {
		d->mState.hasDeferred = false;
		if (d->mState.hasParent || d->mState.hasPromise) d.release();
	}
}

template<typename R, typename U>
QPromise<R, U> QDeferred<R, U>::promise() {
	return d->promise();
}

template<typename R, typename U>
void QDeferred<R, U>::resolve() {
	d->resolve();
}

template<typename R, typename U>
template<typename T>
typename std::enable_if<!std::is_void<T>::value && std::is_same<R, T>::value, void>::type
QDeferred<R, U>::resolve(const T& v) {
	d->resolve(v);
}

template<typename R, typename U>
template<typename T>
typename std::enable_if<std::is_same<QPromise<R, U>, T>::value, void>::type
QDeferred<R, U>::resolve(T&& v) {
	d->resolve(std::forward<T>(v));
}

template<typename R, typename U>
void QDeferred<R, U>::reject(const std::exception& e) {
	d->reject(e);
}

template<typename R, typename U>
void QDeferred<R, U>::notify() {
	d->notify();
}

template<typename R, typename U>
template<typename T>
typename std::enable_if<!std::is_void<T>::value && std::is_same<U, T>::value, void>::type
QDeferred<R, U>::notify(const T& v) {
	d->notify(v);
}

inline
QPromise<> fulfill() {
	QDeferred<> d;
	return d.promise();
}

template<typename T> inline
QPromise<T> fulfill(T&& v) {
	QDeferred<T> d;
	d.resolve(std::forward<T>(v));
	return d.promise();
}

template<typename R, typename U, typename E> inline
QPromise<R, U> reject(E&& e) {
	QDeferred<R, U> d;
	d.reject(std::forward<E>(e));
	return d.promise();
}

namespace priv {

template<typename R, typename U>
Detail<R, U>::Detail()
	: mState{false, false, true, false, false} {
}

template<typename R, typename U>
Detail<R, U>::Detail(Detail&& o)
	: DetailReasonHolder<R>(std::move(o))
	, mState(std::move(o.mState))
	, mChain(std::move(o.mChain))
	, mException(std::move(o.mException)) {
}

template<typename R, typename U>
void Detail<R, U>::addChild(Detail<R, U>* child) {
	auto c = new ChildChain<R, U>(child);
	if (mChain) mChain->mNext.reset(c);
	else mChain.reset(c);
}

template<typename R, typename U>
template<typename F>
typename std::enable_if<std::is_same<typename FulfillFunction<F, R>::ReturnType, void>::value
                        && std::is_same<R, void>::value,
QPromise<typename FulfillFunction<F, R>::ReturnType>>::type
Detail<R, U>::fulfill(F&& f) {
	f();
	return ::qpromise::fulfill();
}

template<typename R, typename U>
template<typename F>
typename std::enable_if<!std::is_same<typename FulfillFunction<F, R>::ReturnType, void>::value
                        && std::is_same<R, void>::value,
QPromise<typename FulfillFunction<F, R>::ReturnType>>::type
Detail<R, U>::fulfill(F&& f) {
	return ::qpromise::fulfill(f());
}

template<typename R, typename U>
template<typename F>
typename std::enable_if<std::is_same<typename FulfillFunction<F, R>::ReturnType, void>::value
                        && !std::is_same<R, void>::value,
QPromise<typename FulfillFunction<F, R>::ReturnType>>::type
Detail<R, U>::fulfill(F&& f) {
	f(*(this->mReason));
	return ::qpromise::fulfill();
}

template<typename R, typename U>
template<typename F>
typename std::enable_if<!std::is_same<typename FulfillFunction<F, R>::ReturnType, void>::value
                        && !std::is_same<R, void>::value,
QPromise<typename FulfillFunction<F, R>::ReturnType>>::type
Detail<R, U>::fulfill(F&& f) {
	return ::qpromise::fulfill(f(*(this->mReason)));
}

template<typename R, typename U>
template<typename F1, typename F2, typename F3>
QPromise<typename FulfillFunction<F1, R>::ReturnType>
Detail<R, U>::then(F1&& onFulfilled, F2&& onRejected, F3&& onProgress) {
	if (mState.isFulfilled) {
		return fulfill(onFulfilled);
	} else if (mState.isRejected) {
		return ::qpromise::fulfill(onRejected(*mException));
	} else {
		auto c = new Chain<R, U, typename FulfillFunction<F1, R>::ReturnType>(std::forward<F1>(onFulfilled),
										      std::forward<F2>(onRejected),
										      std::forward<F3>(onProgress));
		if (mChain) mChain->mNext.reset(c);
		else mChain.reset(c);
		return c->promise();
	}
}

template<typename R, typename U>
QPromise<R, U> Detail<R, U>::promise() {
	return QPromise<R, U>(this);
}

template<typename R, typename U>
void Detail<R, U>::resolve() {
	mState.isFulfilled = true;
	for (auto c = mChain.get(); c != nullptr; c = c->mNext.get()) c->resolve();
}

template<typename R, typename U>
template<typename T>
typename std::enable_if<!std::is_void<T>::value && std::is_same<R, T>::value, void>::type
Detail<R, U>::resolve(const T& v) {
	mState.isFulfilled = true;
	this->mReason.reset(new T(v));
	for (auto c = mChain.get(); c != nullptr; c = c->mNext.get()) c->resolve(v);
}

template<typename R, typename U>
template<typename T>
typename std::enable_if<std::is_same<QPromise<R, U>, T>::value, void>::type
Detail<R, U>::resolve(T&& v) {
	v.d->addChild(this);
	mState.hasParent = true;
}

template<typename R, typename U>
void Detail<R, U>::reject(const std::exception& e) {
	mState.isRejected = true;
	this->mException.reset(new std::exception(e));
	if (mChain) mChain->reject(e);
}

template<typename R, typename U>
void Detail<R, U>::notify() {
	if (mChain) mChain->notify();
}

template<typename R, typename U>
template<typename T>
typename std::enable_if<!std::is_void<T>::value && std::is_same<U, T>::value, void>::type
Detail<R, U>::notify(const T& v) {
	if (mChain) mChain->notify(v);
}

template<typename R, typename U, typename R2>
template<typename F1, typename F2, typename F3>
Chain<R, U, R2>::Chain(F1&& onFulfilled, F2&& onRejected, F3&& onProgress) {
	this->mOnFulfilled = std::forward<typename CallbackFunc<R2, R>::Type>(onFulfilled);
	this->mOnRejected = std::forward<std::function<void (const std::exception&)>>(onRejected);
	this->mOnProgress = std::forward<typename CallbackFunc<void, U>::Type>(onProgress);
}

template<typename R, typename U, typename R2>
QPromise<R2> Chain<R, U, R2>::promise() {
	return this->mDetail.promise();
}

template<typename R, typename U, typename R2>
void Chain<R, U, R2>::reject(const std::exception& e) {
	if (this->mOnRejected) {
		try {
			this->mOnRejected(e);
		} catch (const std::exception& e2) {
			this->mDetail.reject(e2);
		}
	} else {
		this->mDetail.reject(e);
	}
}

template<typename R, typename U, typename R2>
void ChainResolve<R, U, R2>::resolve(const R& r) {
	try {
		this->mDetail.resolve(this->mOnFulfilled(r));
	} catch (const std::exception& e) {
		this->mDetail.reject(e);
	}
}

template<typename U, typename R2>
void ChainResolve<void, U, R2>::resolve() {
	try {
		this->mDetail.resolve(this->mOnFulfilled());
	}  catch (const std::exception& e) {
		this->mDetail.reject(e);
	}
}

template<typename R, typename U>
void ChainResolve<R, U, void>::resolve(const R& r) {
	try {
		this->mOnFulfilled(r);
		this->mDetail.resolve();
	}  catch (const std::exception& e) {
		this->mDetail.reject(e);
	}
}

template<typename U>
void ChainResolve<void, U, void>::resolve() {
	try {
		if (this->mOnFulfilled) this->mOnFulfilled();
		this->mDetail.resolve();
	}  catch (const std::exception& e) {
		this->mDetail.reject(e);
	}
}

template<typename R, typename U, typename R2>
void ChainNotify<R, U, R2>::notify(const U& u) {
	this->mOnProgress(u);
}

template<typename R, typename R2>
void ChainNotify<R, void, R2>::notify() {
	if (this->mOnProgress) this->mOnProgress();
}

template<typename R, typename U>
ChildChain<R, U>::ChildChain(Detail<R, U>* child) {
	this->mChild = child;
}

template<typename R, typename U>
ChildChain<R, U>::~ChildChain() {
	this->mChild->mState.hasParent = false;
	if (!this->mChild->mState.hasDeferred && !this->mChild->mState.hasPromise) {
		delete this->mChild;
	}
}

template<typename R, typename U>
void ChildResolveChain<R, U>::resolve(const R& r) {
	this->mChild->resolve(r);
}

template<typename U>
void ChildResolveChain<void, U>::resolve() {
	this->mChild->resolve();
}

template<typename R, typename U>
void ChildChain<R, U>::reject(const std::exception& e) {
	this->mChild->reject(e);
}

template<typename R, typename U>
void ChildNotifyChain<R, U>::notify(const U& u) {
	this->mChild->notify(u);
}

template<typename R>
void ChildNotifyChain<R, void>::notify() {
	this->mChild->notify();
}

}

}

#endif
