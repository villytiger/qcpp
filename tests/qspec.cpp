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

#include "asynctest.h"

#include <qpromise/qpromise.h>

using namespace qpromise;

typedef AsyncTest QspecDeferAndWhen;

TEST_F(QspecDeferAndWhen, ResolveBeforeWhen) {
	QSharedPointer<int> turn(new int(0));
        QDeferred deferred;
        deferred.resolve(10);
	Q::when(deferred.promise(), [turn](int value) {
		EXPECT_EQ(1, *turn);
		EXPECT_EQ(10, value);
        });
        (*turn)++;
}

typedef AsyncTest QspecPropagation;

TEST_F(QspecPropagation, PropagateThroughThenWithNoCallback) {
	Q::fulfill(10).then().then([](int r) { EXPECT_EQ(10, r); });
}

TEST_F(QspecPropagation, PropagateThroughThenWithModifyingCallback) {
	Q::fulfill(10).then([](int r) { return r + 10; }).then([](int r) { EXPECT_EQ(20, r); });
}

TEST_F(QspecPropagation, ResolutionIsForwardedThroughDeferredPromise) {
        QDeferred a;
        QDeferred b;
        a.resolve(b.promise());
        b.resolve(10);
        a.promise().then([](int v) {
                EXPECT_EQ(10, v);
        });
}

TEST_F(QspecPropagation, ErrbackRecoversFromException) {
        std::runtime_error error("some error");
	Q::reject(error).then(nullptr, [](const QException& e) {
		(void)e;
		//EXPECT_TRUE(typeid(error) == typeid(e));
                //EXPECT_EQ(error.what(), e.what());
                return 10;
        }).then([](int r) {
                EXPECT_EQ(10, r);
        });
}

/*TEST(QspecPropagation, RejectionPropagatesThroughThenWithNoErrback) {
        bool t = false;
        std::runtime_error error("some error");
        reject(error).then().then(nullptr, [&t, &error](const std::exception& e) {
                EXPECT_TRUE(typeid(error) == typeid(e));
                EXPECT_EQ(error.what(), e.what());
                EXPECT_FALSE(t);
                t = true;
        });
        EXPECT_TRUE(t);
}*/
