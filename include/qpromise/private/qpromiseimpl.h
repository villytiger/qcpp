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

namespace qpromise {

constexpr auto wrapFulfilled(nullptr_t) noexcept { return propagateValue; }

template <typename F> constexpr auto wrapFulfilled(F&& fulfilled) noexcept { return std::forward<F>(fulfilled); }

template <typename F> QPromise QPromiseBase::then(F&& fulfilled) { return doThen(wrapFulfilled(fulfilled)); }
}
