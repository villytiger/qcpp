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

namespace AplusplusTests {

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

}
