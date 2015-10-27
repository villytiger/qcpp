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

#include <qpromise/qpromise.h>

#include <gtest/gtest.h>

using namespace qpromise;

TEST(Memory, QDeferredMoveConstructor) {
	QDeferred<> d;
	auto d2 = std::move(d);
	(void)d2;
}

TEST(Memory, QPromiseMoveConstructor) {
	QDeferred<> d;
	auto p = d.promise();
	auto p2 = std::move(p);
	(void)p2;
}

TEST(Memory, ResolveWithAnotherPromiseThenDestroy) {
	bool t = false;
	QDeferred<> a;
	{
		QDeferred<> b;
		b.resolve(a.promise());
		b.promise().then([&t]() {
			EXPECT_FALSE(t);
			t = true;
		});
	}
	a.resolve();
	EXPECT_TRUE(t);
}
