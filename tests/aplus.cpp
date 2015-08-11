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

#include <memory>

#include <QCoreApplication>

#include <gtest/gtest.h>

#include <QPromise/QDeferred>

namespace AplusplusTests {

namespace Adapter {

template <typename F> void setTimeout(F&& f, int msec) { QTimer::singleShot(msec, f); }

QVariant value("dummy");
QPromiseException reason;

typedef const decltype(value)& Value;
typedef const decltype(reason)& Reason;

QDeferred deferred() noexcept { return QDeferred(); }
QPromise resolved(const QVariant& v) noexcept { return Q::resolve(v); }
QPromise rejected(const QPromiseException& r) noexcept { return Q::reject(r); }
}

#define TEST_FULFILLED(testCase, testValue, test)                                                                      \
                                                                                                                       \
	TEST_F(testCase, AlreadyFulfilled) { this->test(Adapter::resolved(testValue)); }                               \
                                                                                                                       \
	TEST_F(testCase, ImmediatelyFulfilled) {                                                                       \
		auto d = Adapter::deferred();                                                                          \
		this->test(d.promise());                                                                               \
		d.resolve(testValue);                                                                                  \
	}                                                                                                              \
                                                                                                                       \
	TEST_F(testCase, EventuallyFulfilled) {                                                                        \
		auto d = Adapter::deferred();                                                                          \
		this->test(d.promise());                                                                               \
		Adapter::setTimeout([d]() mutable { d.resolve(testValue); }, 50);                                      \
	}

#define TEST_REJECTED(testCase, testReason, test)                                                                      \
                                                                                                                       \
	TEST_F(testCase, AlreadyRejected) { this->test(Adapter::rejected(testReason)); }                               \
                                                                                                                       \
	TEST_F(testCase, ImmediatelyFulfilled) {                                                                       \
		auto d = Adapter::deferred();                                                                          \
		this->test(d.promise());                                                                               \
		d.reject(testReason);                                                                                  \
	}                                                                                                              \
                                                                                                                       \
	TEST_F(testCase, EventuallyRejected) {                                                                         \
		auto d = Adapter::deferred();                                                                          \
		this->test(d.promise());                                                                               \
		Adapter::setTimeout([d]() mutable { d.reject(testReason); }, 50);                                      \
	}

class Test : public testing::Test {
	bool mDone;

protected:
	bool mCalled;

	void done() { mDone = true; }

public:
	void SetUp() override {
		mDone = false;
		mCalled = false;
	}

	void TearDown() override {
		while (!mDone) QCoreApplication::instance()->processEvents();
		EXPECT_TRUE(mCalled);
	}
};

class Aplus_2_1_2_1 : public Test {
public:
	void testFulfilled(auto promise) {
		promise.then([this](Adapter::Value) { mCalled = true; },
		             [this](Adapter::Reason) {
			             ASSERT_FALSE(mCalled);
			             done();
			     });

		Adapter::setTimeout([this]() { done(); }, 100);
	}
};

TEST_FULFILLED(Aplus_2_1_2_1, Adapter::value, testFulfilled);

TEST_F(Aplus_2_1_2_1, TryingToFulfillThenImmediatelyReject) {
	auto d = Adapter::deferred();

	d.promise().then([this](Adapter::Value) { mCalled = true; },
	                 [this](Adapter::Reason) {
		                 EXPECT_FALSE(mCalled);
		                 done();
		         });

	d.resolve(Adapter::value);
	d.reject(Adapter::reason);

	Adapter::setTimeout([this]() { done(); }, 100);
}

TEST_F(Aplus_2_1_2_1, TryingToFulfillThenRejectDelayed) {
	auto d = Adapter::deferred();

	d.promise().then([this](Adapter::Value) { mCalled = true; },
	                 [this](Adapter::Reason) {
		                 EXPECT_FALSE(mCalled);
		                 done();
		         });

	Adapter::setTimeout([d]() mutable {
		d.resolve(Adapter::value);
		d.reject(Adapter::reason);
	}, 50);

	Adapter::setTimeout([this]() { done(); }, 100);
}

TEST_F(Aplus_2_1_2_1, TryingToFulfillImmediatelyThenRejectDelayed) {
	auto d = Adapter::deferred();

	d.promise().then([this](Adapter::Value) { mCalled = true; },
	                 [this](Adapter::Reason) {
		                 EXPECT_FALSE(mCalled);
		                 done();
		         });

	d.resolve(Adapter::value);
	Adapter::setTimeout([d]() mutable { d.reject(Adapter::reason); }, 50);

	Adapter::setTimeout([this]() { done(); }, 100);
}

class Aplus_2_1_3_1 : public Test {
public:
	void testRejected(auto promise) {
		promise.then(
		    [this](Adapter::Value) {
			    EXPECT_FALSE(mCalled);
			    done();
		    },
		    [this](Adapter::Reason) { mCalled = true; });

		Adapter::setTimeout([this]() { done(); }, 100);
	}
};

TEST_REJECTED(Aplus_2_1_3_1, Adapter::reason, testRejected);

TEST_F(Aplus_2_1_3_1, TryingToRejectThenImmediatelyFulfill) {
	auto d = Adapter::deferred();

	d.promise().then(
	    [this](Adapter::Value) {
		    EXPECT_FALSE(mCalled);
		    done();
	    },
	    [this](Adapter::Reason) { mCalled = true; });

	d.reject(Adapter::reason);
	d.resolve(Adapter::value);

	Adapter::setTimeout([this]() { done(); }, 100);
}

TEST_F(Aplus_2_1_3_1, TryingToRejectThenFulfillDelayed) {
	auto d = Adapter::deferred();

	d.promise().then(
	    [this](Adapter::Value) {
		    EXPECT_FALSE(mCalled);
		    done();
	    },
	    [this](Adapter::Reason) { mCalled = true; });

	Adapter::setTimeout([d]() mutable {
		d.reject(Adapter::reason);
		d.resolve(Adapter::value);
	}, 50);

	Adapter::setTimeout([this]() { done(); }, 100);
}

TEST_F(Aplus_2_1_3_1, TryingToRejectImmediatelyThenFulfillDelayed) {
	auto d = Adapter::deferred();

	d.promise().then(
	    [this](Adapter::Value) {
		    EXPECT_FALSE(mCalled);
		    done();
	    },
	    [this](Adapter::Reason) { mCalled = true; });

	d.reject(Adapter::reason);
	Adapter::setTimeout([d]() mutable { d.resolve(Adapter::value); }, 50);

	Adapter::setTimeout([this]() { done(); }, 100);
}

class Aplus_2_2_2_2 : public Test {
};

TEST_F(Aplus_2_2_2_2, FulfilledAfterADelay) {
	auto d = Adapter::deferred();

	d.promise().then([this](Adapter::Value) {
		EXPECT_TRUE(mCalled);
		done();
	});

	Adapter::setTimeout([this, d]() mutable {
		d.resolve(Adapter::value);
		mCalled = true;
	}, 50);
}
}
