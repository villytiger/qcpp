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

}
