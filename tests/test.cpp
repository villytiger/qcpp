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

typedef AsyncTest Test;

void func(int a);

template <typename T = typename priv::FunctionTraits<decltype(func)>::Arg<0>::Type> class A {
public:
	void func();
};
/*
TEST_F(Test, Test) {
	Q::fulfill(10).then([](const QVariant& v) { EXPECT_EQ(10, v.toInt()); });

	auto f = [](int) {};

	qpromise::priv::FunctionTraits<decltype(f)>::Arg<0>::Type a;
	(void)a;
}
*/
