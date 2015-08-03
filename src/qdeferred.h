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

#ifndef QDEFERRED_H
#define QDEFERRED_H

#include "qpromiseglobal.h"
#include "qpromise.h"

#include <QObject>
#include <QSharedPointer>

QPROMISE_BEGIN_NAMESPACE

class QPromise;
class QPromiseBase;
class QPromiseException;

class QDeferred {
	Q_GADGET

	QSharedPointer<QPromiseBase> mPromise;

public:
	QDeferred();

	Q_PROPERTY(QPromise promise READ promise);

	Q_INVOKABLE QPromise promise() const;

	Q_INVOKABLE void resolve(const QVariant& value);

	Q_INVOKABLE void resolve(const QPromise& promise);

	Q_INVOKABLE void reject(const QPromiseException& reason);
};

QPROMISE_END_NAMESPACE

#endif
