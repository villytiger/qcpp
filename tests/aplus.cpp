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

// TODO: move from adapter
void nullResolved(const QVariant&) {}

QDeferred deferred() noexcept { return QDeferred(); }
QPromise resolved(const QVariant& v) noexcept { return Q::resolve(v); }
QPromise rejected(const QPromiseException& r) noexcept { return Q::reject(r); }
}

#define SPECIFY(desc) RecordProperty("description", desc)

#define DESCRIBE(testcase, desc)                                                                                       \
	class testcase : public Test {                                                                                 \
	protected:                                                                                                     \
		static auto describe() { return std::vector<std::string>{desc}; }                                      \
                                                                                                                       \
	public:                                                                                                        \
		static void SetUpTestCase() { RecordProperty("description0", desc); }                                  \
	};

#define DESCRIBE_INNER(testcase, parent, desc)                                                                         \
	class testcase : public parent {                                                                               \
	protected:                                                                                                     \
		static auto describe() {                                                                               \
			auto d = parent::describe();                                                                   \
			d.push_back(desc);                                                                             \
			return d;                                                                                      \
		}                                                                                                      \
                                                                                                                       \
	public:                                                                                                        \
		static void SetUpTestCase() {                                                                          \
			auto description = describe();                                                                 \
			for (size_t i = 0; i != description.size(); ++i) {                                             \
				RecordProperty("description" + std::to_string(i), description[i]);                     \
			}                                                                                              \
		}                                                                                                      \
	};

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
	void SetUp() override { mDone = false; }

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

class Aplus_2_2_2_2 : public Test {};

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

class Aplus_2_2_2_3 : public Test {};

TEST_F(Aplus_2_2_2_3, AlreadyFulfilled) {
	auto timesCalled = std::make_shared<int>(0);

	Adapter::resolved(Adapter::value)
	    .then([this, timesCalled](Adapter::Value) {
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
	Adapter::setTimeout([d]() mutable { d.resolve(Adapter::value); }, 50);
}

TEST_F(Aplus_2_2_2_3, WhenMultipleThenCallsAreMadeSpacedApartInTime) {
	auto d = Adapter::deferred();
	auto timesCalled = std::shared_ptr<std::array<int, 3>>(new std::array<int, 3>{0, 0, 0});

	d.promise().then([timesCalled](Adapter::Value) { EXPECT_EQ(1, ++((*timesCalled)[0])); });

	Adapter::setTimeout([d, timesCalled]() {
		d.promise().then([timesCalled](Adapter::Value) { EXPECT_EQ(1, ++((*timesCalled)[1])); });
	}, 50);

	Adapter::setTimeout([this, d, timesCalled]() {
		d.promise().then([this, timesCalled](Adapter::Value) {
			EXPECT_EQ(1, ++((*timesCalled)[2]));
			done();
		});
	}, 100);

	Adapter::setTimeout([d]() mutable { d.resolve(Adapter::value); }, 150);
}

TEST_F(Aplus_2_2_2_3, WhenThenIsInterleavedWithFulfillment) {
	auto d = Adapter::deferred();
	auto timesCalled = std::shared_ptr<std::array<int, 2>>(new std::array<int, 2>{0, 0});

	d.promise().then([timesCalled](Adapter::Value) { EXPECT_EQ(1, ++((*timesCalled)[0])); });

	d.resolve(Adapter::value);

	d.promise().then([this, timesCalled](Adapter::Value) {
		EXPECT_EQ(1, ++((*timesCalled)[1]));
		done();
	});
}

DESCRIBE(Aplus_2_2_3,
         "it must be called after `promise` is rejected, with `promise`’s rejection reason as its first argument.");
DESCRIBE_INNER(Aplus_2_2_3_2, Aplus_2_2_3, "it must not be called before `promise` is rejected");

TEST_F(Aplus_2_2_3_2, test1) {
	SPECIFY("rejected after a delay");
	auto d = Adapter::deferred();
	auto isRejected = std::make_shared<bool>(false);

	d.promise().then([this, isRejected](Adapter::Value) {
		EXPECT_TRUE(*isRejected);
		done();
	});

	Adapter::setTimeout([d, isRejected]() mutable {
		d.resolve(Adapter::value);
		*isRejected = true;
	}, 50);
}

TEST_F(Aplus_2_2_3_2, test2) {
	SPECIFY("never rejected");
	auto d = Adapter::deferred();
	auto onRejectedCalled = std::make_shared<bool>(false);

	d.promise().then(Adapter::nullResolved, [this, onRejectedCalled](Adapter::Reason) {
		*onRejectedCalled = true;
		done();
	});

	Adapter::setTimeout([this, onRejectedCalled]() {
		EXPECT_FALSE(*onRejectedCalled);
		done();
	}, 150);
}

DESCRIBE(Aplus_2_2_3_3, "it must not be called more than once");

// TODO: rejected test

TEST_F(Aplus_2_2_3_3, test1) {
	SPECIFY("already-rejected");
	auto timesCalled = std::make_shared<int>(0);

	Adapter::rejected(Adapter::reason)
	    .then(Adapter::nullResolved, [this, timesCalled](Adapter::Reason) {
		    EXPECT_EQ(1, ++(*timesCalled));
		    done();
	    });
}

TEST_F(Aplus_2_2_3_3, test2) {
	SPECIFY("trying to reject a pending promise more than once, immediately");
	auto d = Adapter::deferred();
	auto timesCalled = std::make_shared<int>(0);

	d.promise().then(Adapter::nullResolved, [this, timesCalled](Adapter::Reason) {
		EXPECT_EQ(1, ++(*timesCalled));
		done();
	});

	d.reject(Adapter::reason);
	d.reject(Adapter::reason);
}

TEST_F(Aplus_2_2_3_3, test3) {
	SPECIFY("trying to reject a pending promise more than once, delayed");
	auto d = Adapter::deferred();
	auto timesCalled = std::make_shared<int>(0);

	d.promise().then(Adapter::nullResolved, [this, timesCalled](Adapter::Reason) {
		EXPECT_EQ(1, ++(*timesCalled));
		done();
	});

	Adapter::setTimeout([d]() mutable {
		d.reject(Adapter::reason);
		d.reject(Adapter::reason);
	}, 50);
}

TEST_F(Aplus_2_2_3_3, test4) {
	SPECIFY("trying to reject a pending promise more than once, immediately then delayed");
	auto d = Adapter::deferred();
	auto timesCalled = std::make_shared<int>(0);
	;

	d.promise().then(Adapter::nullResolved, [this, timesCalled](Adapter::Reason) {
		EXPECT_EQ(1, ++(*timesCalled));
		done();
	});

	d.reject(Adapter::reason);
	Adapter::setTimeout([d]() mutable { d.reject(Adapter::reason); }, 50);
}

TEST_F(Aplus_2_2_3_3, test5) {
	SPECIFY("when multiple `then` calls are made, spaced apart in time");
	auto d = Adapter::deferred();
	auto timesCalled = std::shared_ptr<std::array<int, 3>>(new std::array<int, 3>{0, 0, 0});

	d.promise().then(Adapter::nullResolved,
	                 [timesCalled](Adapter::Reason) { EXPECT_EQ(1, ++((*timesCalled)[0])); });

	Adapter::setTimeout([d, timesCalled]() {
		d.promise().then(Adapter::nullResolved,
		                 [timesCalled](Adapter::Reason) { EXPECT_EQ(1, ++((*timesCalled)[1])); });
	}, 50);

	Adapter::setTimeout([this, d, timesCalled]() {
		d.promise().then(Adapter::nullResolved, [this, timesCalled](Adapter::Reason) {
			EXPECT_EQ(1, ++((*timesCalled)[2]));
			done();
		});
	}, 100);

	Adapter::setTimeout([d]() mutable { d.reject(Adapter::reason); }, 150);
}

TEST_F(Aplus_2_2_3_3, test6) {
	SPECIFY("when `then` is interleaved with rejection");
	auto d = Adapter::deferred();
	auto timesCalled = std::shared_ptr<std::array<int, 2>>(new std::array<int, 2>{0, 0});

	d.promise().then(Adapter::nullResolved,
	                 [timesCalled](Adapter::Reason) { EXPECT_EQ(1, ++((*timesCalled)[0])); });

	d.reject(Adapter::reason);

	d.promise().then(Adapter::nullResolved, [this, timesCalled](Adapter::Reason) {
		EXPECT_EQ(1, ++((*timesCalled)[1]));
		done();
	});
}

DESCRIBE(
    Aplus_2_2_4,
    "`onFulfilled` or `onRejected` must not be called until the execution context stack contains only platform code");
DESCRIBE_INNER(Aplus_2_2_4_suite1, Aplus_2_2_4, "`then` returns before the promise becomes fulfilled or rejected");

// TODO: test fullfilled, rejected

DESCRIBE_INNER(Aplus_2_2_4_suite2, Aplus_2_2_4, "Clean-stack execution ordering tests (fulfillment case)");

TEST_F(Aplus_2_2_4_suite2, test1) {
	SPECIFY("when `onFulfilled` is added immediately before the promise is fulfilled");
	auto d = Adapter::deferred();
	auto onFulfilledCalled = std::make_shared<bool>(false);

	d.promise().then([onFulfilledCalled](Adapter::Value) { *onFulfilledCalled = true; });

	d.resolve(Adapter::value);

	EXPECT_FALSE(*onFulfilledCalled);
	done();
}

TEST_F(Aplus_2_2_4_suite2, test2) {
	SPECIFY("when `onFulfilled` is added immediately after the promise is fulfilled");
	auto d = Adapter::deferred();
	auto onFulfilledCalled = std::make_shared<bool>(false);

	d.resolve(Adapter::value);

	d.promise().then([onFulfilledCalled](Adapter::Value) { *onFulfilledCalled = true; });

	EXPECT_FALSE(*onFulfilledCalled);
	done();
}

TEST_F(Aplus_2_2_4_suite2, test3) {
	SPECIFY("when one `onFulfilled` is added inside another `onFulfilled`");
	auto promise = Adapter::resolved(Adapter::value);
	auto firstOnFulfilledFinished = std::make_shared<bool>(false);

	promise.then([this, promise, firstOnFulfilledFinished](Adapter::Value) mutable {
		promise.then([this, firstOnFulfilledFinished](Adapter::Value) {
			EXPECT_TRUE(*firstOnFulfilledFinished);
			done();
		});
		*firstOnFulfilledFinished = true;
	});
}

TEST_F(Aplus_2_2_4_suite2, test4) {
	SPECIFY("when `onFulfilled` is added inside an `onRejected`");
	auto promise = Adapter::rejected(Adapter::reason);
	auto promise2 = Adapter::resolved(Adapter::value);
	auto firstOnRejectedFinished = std::make_shared<bool>(false);

	promise.then(Adapter::nullResolved, [this, promise2, firstOnRejectedFinished](Adapter::Reason) mutable {
		promise2.then([this, firstOnRejectedFinished](Adapter::Value) {
			EXPECT_TRUE(*firstOnRejectedFinished);
			done();
		});
		*firstOnRejectedFinished = true;
	});
}

TEST_F(Aplus_2_2_4_suite2, test5) {
	SPECIFY("when the promise is fulfilled asynchronously");
	auto d = Adapter::deferred();
	auto firstStackFinished = std::make_shared<bool>(false);

	Adapter::setTimeout([d, firstStackFinished]() mutable {
		d.resolve(Adapter::value);
		*firstStackFinished = true;
	}, 0);

	d.promise().then([this, firstStackFinished](Adapter::Value) {
		EXPECT_TRUE(*firstStackFinished);
		done();
	});
}

DESCRIBE(Aplus_2_2_4_suite3, "Clean-stack execution ordering tests (rejection case)");

TEST_F(Aplus_2_2_4_suite3, test1) {
	SPECIFY("when `onRejected` is added immediately before the promise is rejected");
	auto d = Adapter::deferred();
	auto onRejectedCalled = std::make_shared<bool>(false);

	d.promise().then(Adapter::nullResolved, [onRejectedCalled](Adapter::Reason) { *onRejectedCalled = true; });

	d.reject(Adapter::reason);

	EXPECT_FALSE(*onRejectedCalled);
	done();
}

TEST_F(Aplus_2_2_4_suite3, test2) {
	SPECIFY("when `onRejected` is added immediately after the promise is rejected");
	auto d = Adapter::deferred();
	auto onRejectedCalled = std::make_shared<bool>(false);

	d.reject(Adapter::reason);

	d.promise().then(Adapter::nullResolved, [onRejectedCalled](Adapter::Reason) { *onRejectedCalled = true; });

	EXPECT_FALSE(*onRejectedCalled);
	done();
}

TEST_F(Aplus_2_2_4_suite3, test3) {
	SPECIFY("when `onRejected` is added inside an `onFulfilled`");
	auto promise = Adapter::resolved(Adapter::value);
	auto promise2 = Adapter::rejected(Adapter::reason);
	auto firstOnFulfilledFinished = std::make_shared<bool>(false);

	promise.then([this, promise2, firstOnFulfilledFinished](Adapter::Value) mutable {
		promise2.then(Adapter::nullResolved, [this, firstOnFulfilledFinished](Adapter::Reason) {
			EXPECT_TRUE(*firstOnFulfilledFinished);
			done();
		});
		*firstOnFulfilledFinished = true;
	});
}

TEST_F(Aplus_2_2_4_suite3, test4) {
	SPECIFY("when one `onRejected` is added inside another `onRejected`");
	auto promise = Adapter::rejected(Adapter::reason);
	auto firstOnRejectedFinished = std::make_shared<bool>(false);

	promise.then(Adapter::nullResolved, [this, promise, firstOnRejectedFinished](Adapter::Reason) mutable {
		promise.then(Adapter::nullResolved, [this, firstOnRejectedFinished](Adapter::Reason) {
			EXPECT_TRUE(*firstOnRejectedFinished);
			done();
		});
		*firstOnRejectedFinished = true;
	});
}

TEST_F(Aplus_2_2_4_suite3, test5) {
	SPECIFY("when the promise is rejected asynchronously");
	auto d = Adapter::deferred();
	auto firstStackFinished = std::make_shared<bool>(false);

	Adapter::setTimeout([d, firstStackFinished]() mutable {
		d.reject(Adapter::reason);
		*firstStackFinished = true;
	}, 0);

	d.promise().then(Adapter::nullResolved, [this, firstStackFinished](Adapter::Reason) {
		EXPECT_TRUE(*firstStackFinished);
		done();
	});
}

DESCRIBE(Aplus_2_2_6, "`then` may be called multiple times on the same promise")
DESCRIBE_INNER(Aplus_2_2_6_1, Aplus_2_2_6, "If/when `promise` is fulfilled, all respective `onFulfilled` callbacks"
                                           "must execute in the order of their originating calls to `then`");
DESCRIBE_INNER(Aplus_2_2_6_1_suite1, Aplus_2_2_6_1, "multiple boring fulfillment handlers");

TEST_FULFILLED(Aplus_2_2_6_1_suite1, Adapter::value, testFulfilled);

	class BaseResolver {
	public:
		virtual std::decay<Adapter::Value> resolve(Adapter::Value) const = 0;
	};

	class Resolver {
	public:
		MOCK_CONST_METHOD1(resolve, std::decay<Adapter::Value>(Adapter::Value));
	};

class asd {
	void testFulfilled(auto promise) {
                var handler1 = sinon.stub().returns(other);
                var handler2 = sinon.stub().returns(other);
                var handler3 = sinon.stub().returns(other);

                var spy = sinon.spy();
                promise.then(handler1, spy);
                promise.then(handler2, spy);
                promise.then(handler3, spy);

                promise.then(function (value) {
                    assert.strictEqual(value, sentinel);

                    sinon.assert.calledWith(handler1, sinon.match.same(sentinel));
                    sinon.assert.calledWith(handler2, sinon.match.same(sentinel));
                    sinon.assert.calledWith(handler3, sinon.match.same(sentinel));
                    sinon.assert.notCalled(spy);

                    done();
                });
            });
};
}
