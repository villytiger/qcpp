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

#ifndef QPROMISE_PRIVATE_EXCEPTION_H
#define QPROMISE_PRIVATE_EXCEPTION_H

#include <exception>
#include <memory>

namespace qpromise {
namespace priv {

class ExceptionPtr {
protected:
	std::function<std::unique_ptr<std::exception> (const std::exception&)> mClone;
	std::unique_ptr<std::exception> mException;

public:
	template<typename E>
	void set(E&& e) {
		mClone = [](const std::exception& e) {return std::make_unique<E>(e);};
		mException = std::make_unique(std::forward<E>(e));
	}

/*	ExceptionPtr& ExceptionPtr(const ExceptionPtr& e)
		: mClone(e.mClone)
		, mException(e.mClone(e.*mException) {
		return this;
		}*/

	std::exception& operator*() {
		return *mException;
	}

	const std::exception& operator*() const {
		return *mException;
	}
};

}
}

#endif
