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

#include <gtest/gtest.h>

#include <qpromise/qpromise.h>

using namespace qpromise;

TEST(QspecPropagation, PropagateThroughThenWithNoCallback) {
	bool t = false;
	fulfill(10)
		.then()
		.then([&t](int r) {
		EXPECT_EQ(10, r);
		t = true;
	});
	EXPECT_TRUE(t);
}

TEST(QspecPropagation, PropagateThroughThenWithModifyingCallback) {
	bool t = false;
	fulfill(10)
		.then([](int r) {
		return r + 10;
	}).then([&t](int r) {
		EXPECT_EQ(20, r);
		EXPECT_FALSE(t);
		t = true;
	});
	EXPECT_TRUE(t);
}

TEST(QspecPropagation, ResolutionIsForwardedThroughDeferredPromise) {
	bool t = false;
	QDeferred<int> a;
	QDeferred<int> b;
	a.resolve(b.promise());
	b.resolve(10);
	a.promise().then([&t](int v) {
		EXPECT_EQ(10, v);
		t = true;
	});
	EXPECT_TRUE(t);
}

/*TEST(QspecPropagation, ErrbackRecoversFromException) {
	//bool t = false;
	std::runtime_error error("some error");
	reject<int>(error).then(nullptr, [](const std::exception&) {
		return 10;
	});
	reject<int>(error).then(nullptr, [&error](const std::exception& e) {
		EXPECT_TRUE(typeid(error) == typeid(e));
		EXPECT_EQ(error.what(), e.what());
		return 10;
	}).then([&t](int r) {
		EXPECT_EQ(10, r);
		EXPECT_FALSE(t);
		t = true;
	});
	EXPECT_TRUE(t);
}*/

/*TEST(QspecPropagation, ResolutionIsForwardedThroughDeferredPromise) {
	QDeferred<int> a;
	QDeferred<int> c;
	{
		QDeferred<int> b;
		b.resolve(a.promise());
		c.resolve(b.promise());
	}
	c.then([](int r) {
		EXPECT_EQ(10, r);
	});
	a.resolve(10);
}

void a() {
	QDeferred<int> a;
	{
		QDeferred<int> b;

	}
	a.resolve(10);
}
*/
