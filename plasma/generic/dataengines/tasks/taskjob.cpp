/*
 * Copyright 2008 Alain Boyer <alainboyer@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "taskjob.h"

TaskJob::TaskJob(TaskSource *source, const QString &operation, QMap<QString, QVariant> &parameters, QObject *parent) :
    ServiceJob(source->objectName(), operation, parameters, parent),
    m_source(source)
{
}

TaskJob::~TaskJob()
{
}

void TaskJob::start()
{
    if (!m_source->task()) {
        return;
    }

    // only a subset of task operations are exported
    const QString operation = operationName();
    if (operation == "setMaximized") {
        m_source->task()->setMaximized(parameters().value("maximized").toBool());
        setResult(true);
        return;
    } else if (operation == "setMinimized") {
        m_source->task()->setIconified(parameters().value("minimized").toBool());
        setResult(true);
        return;
    } else if (operation == "setShaded") {
        m_source->task()->setShaded(parameters().value("shaded").toBool());
        setResult(true);
        return;
    } else if (operation == "setFullScreen") {
        m_source->task()->setFullScreen(parameters().value("fullScreen").toBool());
        setResult(true);
        return;
    } else if (operation == "setAlwaysOnTop") {
        m_source->task()->setAlwaysOnTop(parameters().value("alwaysOnTop").toBool());
        setResult(true);
        return;
    } else if (operation == "setKeptBelowOthers") {
        m_source->task()->setKeptBelowOthers(parameters().value("keptBelowOthers").toBool());
        setResult(true);
        return;
    } else if (operation == "toggleMaximized") {
        m_source->task()->toggleMaximized();
        setResult(true);
        return;
    } else if (operation == "toggleMinimized") {
        m_source->task()->toggleIconified();
        setResult(true);
        return;
    } else if (operation == "toggleShaded") {
        m_source->task()->toggleShaded();
        setResult(true);
        return;
    } else if (operation == "toggleFullScreen") {
        m_source->task()->toggleFullScreen();
        setResult(true);
        return;
    } else if (operation == "toggleAlwaysOnTop") {
        m_source->task()->toggleAlwaysOnTop();
        setResult(true);
        return;
    } else if (operation == "toggleKeptBelowOthers") {
        m_source->task()->toggleKeptBelowOthers();
        setResult(true);
        return;
    } else if (operation == "restore") {
        m_source->task()->restore();
        setResult(true);
        return;
    } else if (operation == "resize") {
        m_source->task()->resize();
        setResult(true);
        return;
    } else if (operation == "move") {
        m_source->task()->move();
        setResult(true);
        return;
    } else if (operation == "raise") {
        m_source->task()->raise();
        setResult(true);
        return;
    } else if (operation == "lower") {
        m_source->task()->lower();
        setResult(true);
        return;
    } else if (operation == "activate") {
        m_source->task()->activate();
        setResult(true);
        return;
    } else if (operation == "activateRaiseOrIconify") {
        m_source->task()->activateRaiseOrIconify();
        setResult(true);
        return;
    } else if (operation == "close") {
        m_source->task()->close();
        setResult(true);
        return;
    } else if (operation == "toDesktop") {
        m_source->task()->toDesktop(parameters().value("desktop").toInt());
        setResult(true);
        return;
    } else if (operation == "toCurrentDesktop") {
        m_source->task()->toCurrentDesktop();
        setResult(true);
        return;
    } else if (operation == "publishIconGeometry") {
        m_source->task()->publishIconGeometry(parameters().value("geometry").toRect());
        setResult(true);
        return;
    }

    setResult(false);
}

#include "taskjob.moc"
