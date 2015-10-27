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
#include <gmock/gmock.h>

#include "macro.h"
#include "function-mock.h"

namespace AplusplusTests {

class Test : public testing::Test {
	bool mDone;

protected:
	void done() { mDone = true; }

public:
	void SetUp() override { mDone = false; }

	void TearDown() override {
		while (!mDone) QCoreApplication::instance()->processEvents();
	}
};


}

#include "2.1.2.h"
#include "2.1.3.h"
#include "2.2.2.h"
#include "2.2.3.h"
#include "2.2.4.h"
//#include "2.2.6.h"
#include "2.3.1.h"
