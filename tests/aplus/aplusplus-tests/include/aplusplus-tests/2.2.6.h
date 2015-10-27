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

DESCRIBE(Aplus_2_2_6, "`then` may be called multiple times on the same promise")
DESCRIBE_INNER(Aplus_2_2_6_1, Aplus_2_2_6, "If/when `promise` is fulfilled, all respective `onFulfilled` callbacks"
                                           "must execute in the order of their originating calls to `then`");

class Aplus_2_2_6_1_suite1_base: public Aplus_2_2_6_1 {
public:
	void testFulfilled(auto promise) {
		internal::FunctionMock<Adapter::Value> handler1;
		EXPECT_CALL(handler1.impl(), run(Adapter::value));

		internal::FunctionMock<Adapter::Value> handler2;
		EXPECT_CALL(handler2.impl(), run(Adapter::value));

		internal::FunctionMock<Adapter::Value> handler3;
		EXPECT_CALL(handler3.impl(), run(Adapter::value));

		internal::FunctionMock<Adapter::Reason> spy;
		EXPECT_CALL(spy.impl(), run(Adapter::reason)).Times(0);

                promise.then(handler1, spy);
                promise.then(handler2, spy);
                promise.then(handler3, spy);

                promise.then([this](Adapter::Value value) {
		                EXPECT_EQ(Adapter::value, value);
		                done();
	                });
	}
};

DESCRIBE_INNER(Aplus_2_2_6_1_suite1, Aplus_2_2_6_1_suite1_base, "multiple boring fulfillment handlers");

TEST_FULFILLED(Aplus_2_2_6_1_suite1, Adapter::value, testFulfilled);

}
