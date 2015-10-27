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

#include <memory>

namespace AplusplusTests {
namespace internal {

template<typename T>
class FunctionMockImpl {
public:
	MOCK_METHOD1_T(run, void(T));
};

template<typename T>
class FunctionMock {
private:
	std::shared_ptr<FunctionMockImpl<T>> mImpl;

public:
	void operator()(T v) {
		mImpl->run(v);
	}

	FunctionMockImpl<T>& impl() {
		return *mImpl;
	}
};

}
}
