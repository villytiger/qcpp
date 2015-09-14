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

// TODO: copying deferred object in setTimeout

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
	void done() { mDone = true; }

public:
	void SetUp() override {
		mDone = false;
	}

	void TearDown() override {
		while (!mDone) QCoreApplication::instance()->processEvents();
	}
};

class Aplus_2_1_2_1 : public Test {
public:
	void testFulfilled(auto promise) {
		auto onFulfilledCalled = std::make_shared<bool>(false);

		promise.then([this, onFulfilledCalled](Adapter::Value) { *onFulfilledCalled = true; },
		             [this, onFulfilledCalled](Adapter::Reason) {
			             ASSERT_FALSE(*onFulfilledCalled);
			             done();
			     });

		Adapter::setTimeout([this]() { done(); }, 100);
	}
};

TEST_FULFILLED(Aplus_2_1_2_1, Adapter::value, testFulfilled);

TEST_F(Aplus_2_1_2_1, TryingToFulfillThenImmediatelyReject) {
	auto d = Adapter::deferred();
	auto onFulfilledCalled = std::make_shared<bool>(false);

	d.promise().then([this, onFulfilledCalled](Adapter::Value) { *onFulfilledCalled = true; },
	                 [this, onFulfilledCalled](Adapter::Reason) {
		                 EXPECT_FALSE(*onFulfilledCalled);
		                 done();
		         });

	d.resolve(Adapter::value);
	d.reject(Adapter::reason);

	Adapter::setTimeout([this]() { done(); }, 100);
}

TEST_F(Aplus_2_1_2_1, TryingToFulfillThenRejectDelayed) {
	auto d = Adapter::deferred();
	auto onFulfilledCalled = std::make_shared<bool>(false);

	d.promise().then([this, onFulfilledCalled](Adapter::Value) { *onFulfilledCalled = true; },
	                 [this, onFulfilledCalled](Adapter::Reason) {
		                 EXPECT_FALSE(*onFulfilledCalled);
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
	auto onFulfilledCalled = std::make_shared<bool>(false);

	d.promise().then([this, onFulfilledCalled](Adapter::Value) { *onFulfilledCalled = true; },
	                 [this, onFulfilledCalled](Adapter::Reason) {
		                 EXPECT_FALSE(*onFulfilledCalled);
		                 done();
		         });

	d.resolve(Adapter::value);
	Adapter::setTimeout([d]() mutable { d.reject(Adapter::reason); }, 50);

	Adapter::setTimeout([this]() { done(); }, 100);
}

class Aplus_2_1_3_1 : public Test {
public:
	void testRejected(auto promise) {
		auto onRejectedCalled = std::make_shared<bool>(false);

		promise.then(
			[this, onRejectedCalled](Adapter::Value) {
			    EXPECT_FALSE(*onRejectedCalled);
			    done();
		    },
			[this, onRejectedCalled](Adapter::Reason) { *onRejectedCalled = true; });

		Adapter::setTimeout([this]() { done(); }, 100);
	}
};

TEST_REJECTED(Aplus_2_1_3_1, Adapter::reason, testRejected);

TEST_F(Aplus_2_1_3_1, TryingToRejectThenImmediatelyFulfill) {
	auto d = Adapter::deferred();
	auto onRejectedCalled = std::make_shared<bool>(false);

	d.promise().then(
		[this, onRejectedCalled](Adapter::Value) {
		    EXPECT_FALSE(*onRejectedCalled);
		    done();
	    },
		[this, onRejectedCalled](Adapter::Reason) { *onRejectedCalled = true; });

	d.reject(Adapter::reason);
	d.resolve(Adapter::value);

	Adapter::setTimeout([this]() { done(); }, 100);
}

TEST_F(Aplus_2_1_3_1, TryingToRejectThenFulfillDelayed) {
	auto d = Adapter::deferred();
	auto onRejectedCalled = std::make_shared<bool>(false);

	d.promise().then(
		[this, onRejectedCalled](Adapter::Value) {
		    EXPECT_FALSE(*onRejectedCalled);
		    done();
	    },
		[this, onRejectedCalled](Adapter::Reason) { *onRejectedCalled = true; });

	Adapter::setTimeout([d]() mutable {
		d.reject(Adapter::reason);
		d.resolve(Adapter::value);
	}, 50);

	Adapter::setTimeout([this]() { done(); }, 100);
}

TEST_F(Aplus_2_1_3_1, TryingToRejectImmediatelyThenFulfillDelayed) {
	auto d = Adapter::deferred();
	auto onRejectedCalled = std::make_shared<bool>(false);

	d.promise().then(
		[this, onRejectedCalled](Adapter::Value) {
		    EXPECT_FALSE(*onRejectedCalled);
		    done();
	    },
		[this, onRejectedCalled](Adapter::Reason) { *onRejectedCalled = true; });

	d.reject(Adapter::reason);
	Adapter::setTimeout([d]() mutable { d.resolve(Adapter::value); }, 50);

	Adapter::setTimeout([this]() { done(); }, 100);
}

class Aplus_2_2_2_2 : public Test {
};

TEST_F(Aplus_2_2_2_2, FulfilledAfterADelay) {
	auto d = Adapter::deferred();
	auto isFulfilled = std::make_shared<bool>(false);

	d.promise().then([this, isFulfilled](Adapter::Value) {
		EXPECT_TRUE(*isFulfilled);
		done();
	});

	Adapter::setTimeout([d, isFulfilled]() mutable {
		d.resolve(Adapter::value);
		*isFulfilled = true;
	}, 50);
}

TEST_F(Aplus_2_2_2_2, NeverFulfilled) {
	auto d = Adapter::deferred();
	auto isFulfilled = std::make_shared<bool>(false);

	d.promise().then([this, isFulfilled](Adapter::Value) {
		*isFulfilled = true;
		done();
	});

	Adapter::setTimeout([this, isFulfilled]() mutable {
		EXPECT_FALSE(*isFulfilled);
		done();
	}, 150);
}

class Aplus_2_2_2_3 : public Test {
};

TEST_F(Aplus_2_2_2_3, AlreadyFulfilled) {
	auto timesCalled = std::make_shared<int>(0);

	Adapter::resolved(Adapter::value).then([this, timesCalled](Adapter::Value) {
		EXPECT_EQ(1, ++(*timesCalled));
		done();
	});
}

TEST_F(Aplus_2_2_2_3, TryingToFulfillAPendingPromiseMoreThanOnceImmediately) {
	auto d = Adapter::deferred();
	auto timesCalled = std::make_shared<int>(0);

	d.promise().then([this, timesCalled](Adapter::Value) {
			EXPECT_EQ(1, ++(*timesCalled));
			done();
		});

	d.resolve(Adapter::value);
	d.resolve(Adapter::value);
}

TEST_F(Aplus_2_2_2_3, TryingToFulfillAPendingPromiseMoreThanOnceDelayed) {
	auto d = Adapter::deferred();
	auto timesCalled = std::make_shared<int>(0);

	d.promise().then([this, timesCalled](Adapter::Value) {
			EXPECT_EQ(1, ++(*timesCalled));
			done();
		});

	Adapter::setTimeout([d]() mutable {
			d.resolve(Adapter::value);
			d.resolve(Adapter::value);
		}, 50);
}

TEST_F(Aplus_2_2_2_3, TryingToFulfillAPendingPromiseMoreThanOnceImmediatelyThenDelayed) {
	auto d = Adapter::deferred();
	auto timesCalled = std::make_shared<int>(0);

	d.promise().then([this, timesCalled](Adapter::Value) {
			EXPECT_EQ(1, ++(*timesCalled));
			done();
		});

	d.resolve(Adapter::value);
	Adapter::setTimeout([d]() mutable {
			d.resolve(Adapter::value);
		}, 50);
}

TEST_F(Aplus_2_2_2_3, WhenMultipleThenCallsAreMadeSpacedApartInTime) {
	auto d = Adapter::deferred();
	auto timesCalled = std::shared_ptr<std::array<int, 3>>(new std::array<int, 3>{0, 0, 0});

	d.promise().then([timesCalled](Adapter::Value) {
			EXPECT_EQ(1, ++((*timesCalled)[0]));
	});

	Adapter::setTimeout([d, timesCalled]() {
			d.promise().then([timesCalled](Adapter::Value) {
					EXPECT_EQ(1, ++((*timesCalled)[1]));
				});
		}, 50);

	Adapter::setTimeout([this, d, timesCalled]() {
			d.promise().then([this, timesCalled](Adapter::Value) {
					EXPECT_EQ(1, ++((*timesCalled)[2]));
					done();
				});
		}, 100);
}

/*	specify("when multiple `then` calls are made, spaced apart in time", function (done) {
            var d = deferred();
            var timesCalled = [0, 0, 0];

            d.promise.then(function onFulfilled() {
                assert.strictEqual(++timesCalled[0], 1);
            });

            setTimeout(function () {
                d.promise.then(function onFulfilled() {
                    assert.strictEqual(++timesCalled[1], 1);
                });
            }, 50);

            setTimeout(function () {
                d.promise.then(function onFulfilled() {
                    assert.strictEqual(++timesCalled[2], 1);
                    done();
                });
            }, 100);

            setTimeout(function () {
                d.resolve(dummy);
            }, 150);
	    });*/
}
