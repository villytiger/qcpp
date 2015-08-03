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

#include <memory>

#include <QPromise/QDeferred>

QVariant dummy("dummy");
QPromiseException exception;

class Aplus_2_1_2_1 : public AsyncTest {
protected:
	std::shared_ptr<bool> mFulfilledCalled;

public:
	Aplus_2_1_2_1() : mFulfilledCalled(new bool(false)) {}

	void TearDown() override {
		AsyncTest::TearDown();
		EXPECT_TRUE(*mFulfilledCalled);
	}
};

TEST_F(Aplus_2_1_2_1, TryingToFulfillThenImmediatelyReject) {
	QDeferred d;

	d.promise().then([this](const QVariant&) { *mFulfilledCalled = true; },
	                 [this](const QPromiseException&) { EXPECT_FALSE(*mFulfilledCalled); });

	d.resolve(dummy);
	d.reject(exception);
}

TEST_F(Aplus_2_1_2_1, TryingToFulfillThenRejectDelayed) {
	QDeferred d;

	d.promise().then([this](const QVariant&) { *mFulfilledCalled = true; },
	                 [this](const QPromiseException&) {
		                 EXPECT_FALSE(*mFulfilledCalled);
		                 // done();
		         });

	Q::nextTick(function() {
		d.resolve(dummy);
		d.reject(dummy);
	}, 50);
	setTimeout(done, 100);
});
