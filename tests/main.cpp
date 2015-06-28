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

using qpromise::QPromise;
using qpromise::QDeferred;

TEST(IntegrationTest, SuccessVoid) {
	QDeferred<> d;
	d.promise();
	d.resolve();
}

TEST(IntegrationTest, ThenSuccessVoidLambda) {
	bool t = false;
	QDeferred<> d;

	auto p = d.promise();
	p.then([&t]() {
		EXPECT_FALSE(t);
		t = true;
	});

	d.resolve();
	EXPECT_EQ(true, t);
}

TEST(IntegrationTest, ThenErrorVoidLambda) {
	bool t = false;
	QDeferred<> d;

	auto p = d.promise();
	p.then([]() {
		FAIL() << "Expected error callback to be called";
	}, [&t](const std::exception&) {
		EXPECT_FALSE(t);
		t = true;
	});

	d.reject(std::exception());
	EXPECT_EQ(true, t);
}

TEST(IntegrationTest, ThenOmitFirstErrorVoidLambda) {
	bool t = false;
	QDeferred<> d;

	auto p = d.promise();
	p.then([]() {
		FAIL() << "Expected error callback to be called";
	}).fail([&t](const std::exception&) {
		EXPECT_FALSE(t);
		t = true;
	});

	d.reject(std::exception());
	EXPECT_EQ(true, t);
}

TEST(IntegrationTest, ThenSuccessVoidLambdaChain) {
	bool t1 = false;
	bool t2 = false;
	QDeferred<> d;

	auto p = d.promise();
	p.then([&t1, &t2]() {
		EXPECT_FALSE(t1);
		EXPECT_FALSE(t2);
		t1 = true;
	}).then([&t1, &t2]() {
		EXPECT_TRUE(t1);
		EXPECT_FALSE(t2);
		t2 = true;
	});

	d.resolve();
	EXPECT_EQ(true, t1);
	EXPECT_EQ(true, t2);
}

TEST(IntegrationTest, ThenSuccessIntLambda) {
	const int result = 1984;
	bool t = false;
	QDeferred<int> d;

	auto p = d.promise();
	p.then([result, &t](int r) {
		EXPECT_FALSE(t);
		EXPECT_EQ(result, r);
		t = true;
	});

	d.resolve(result);
	EXPECT_EQ(true, t);
}

TEST(IntegrationTest, ThenSuccessIntLambdaChain) {
	const int result1 = 1984;
	const int result2 = 2015;
	bool t1 = false;
	bool t2;
	QDeferred<int> d;

	auto p = d.promise();
	p.then([result1, result2, &t1, &t2](int r) {
		EXPECT_FALSE(t1);
		EXPECT_FALSE(t2);
		EXPECT_EQ(result1, r);
		t1 = true;
		return result2;
	}).then([result2, &t1, &t2](int r) {
		EXPECT_TRUE(t1);
		EXPECT_FALSE(t2);
		EXPECT_EQ(result2, r);
		t2 = true;
	});

	d.resolve(result1);
	EXPECT_EQ(true, t1);
	EXPECT_EQ(true, t2);
}
