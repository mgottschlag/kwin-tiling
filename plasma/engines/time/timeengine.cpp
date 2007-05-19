/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <QDate>
#include <QTime>
#include <QTimer>

#include <KDebug>

#include "timeengine.h"

TimeEngine::TimeEngine(QObject* parent, const QStringList& args)
    : Plasma::DataEngine(parent)
{
    //TODO: we need to add the ability to configure this so that can limit
    //      it to minutes only as well as set which TZs to publish
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    m_timer->start(1000);
}

TimeEngine::~TimeEngine()
{
}

void TimeEngine::init()
{
}

void TimeEngine::updateTime()
{
    //TODO: should these keys be translated? probably not, methinks.
    setData("time", i18n("Local"), QTime::currentTime());
    setData("date", i18n("Date"), QDate::currentDate());
}

#include "timeengine.moc"
