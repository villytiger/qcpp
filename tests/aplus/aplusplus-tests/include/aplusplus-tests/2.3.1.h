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

DESCRIBE(Aplus_2_3_1,
         "If `promise` and `x` refer to the same object, reject `promise` with a `TypeError' as the reason");

QPromise f(Adapter::Value) {
	return Adapter::deferred().promise();
}

TEST_F(Aplus_2_3_1, test1) {
	SPECIFY("via return from a fulfilled promise");
	auto promisePtr = std::make_shared<std::shared_ptr<QPromise>>();
/*	auto promise = Adapter::resolved(Adapter::value).then([promisePtr](Adapter::Value) -> QPromise {
		return **promisePtr;
		});*/
	auto promise = Adapter::resolved(Adapter::value).then(f);
	*promisePtr = std::make_shared<QPromise>(promise);

	promise.then(Adapter::nullResolved, [this](Adapter::Reason) {
		//            assert(reason instanceof TypeError);
		done();
        });
}
/*
TEST_F(Aplus_2_3_1, test2) {
        SPECIFY("via return from a rejected promise");
        auto promise = Adapter::rejected(Adapter::reason).then(Adapter::nullResolved, []() {
            return promise;
        });

        promise.then(Adapter::nullResolved, [](Adapter::Reason reason) {
//            assert(reason instanceof TypeError);
            done();
        });
}
*/
}
