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

}
