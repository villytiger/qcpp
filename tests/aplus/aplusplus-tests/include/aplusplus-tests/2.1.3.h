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

}
