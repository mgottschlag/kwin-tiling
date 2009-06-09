/*
 *   Copyright 2009 Artur Duque de Souza <morpheuz@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SEARCHLAUNCH_SERVICE_H
#define SEARCHLAUNCH_SERVICE_H

#include "salengine.h"

#include <Plasma/Service>
#include <Plasma/ServiceJob>

using namespace Plasma;


class SearchLaunchService : public Plasma::Service
{
    Q_OBJECT

public:
    SearchLaunchService(SearchLaunchEngine *engine);
    ServiceJob *createJob(const QString &operation,
                          QMap<QString, QVariant> &parameters);

private:
    SearchLaunchEngine *m_engine;
};

#endif // SEARCHLAUNCH_SERVICE_H
