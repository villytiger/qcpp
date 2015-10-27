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

}
