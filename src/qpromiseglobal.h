// Copyright (C) 2015 Ilya Lyubimov
//
// This file is part of qpromise.
//
//  QPromise is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  QPromise is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with QPromise.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QPROMISEGLOBAL_H
#define QPROMISEGLOBAL_H

#ifndef QPROMISE_NAMESPACE

#define QPROMISE_BEGIN_NAMESPACE
#define QPROMISE_END_NAMESPACE

#else

#define QPROMISE_BEGIN_NAMESPACE namespace QPROMISE_NAMESPACE {
#define QPROMISE_END_NAMESPACE }

#endif

#endif
