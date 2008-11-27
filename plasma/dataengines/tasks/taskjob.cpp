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
    // only a subset of task operations are exported
    QString operation = operationName();
    if (operation.startsWith("set")) {
        if (operation == "setMaximized") {
            m_source->getTask()->setMaximized(parameters().value("maximized").toBool());
            setResult(true);
            return;
        }
        else if (operation == "setMinimized") {
            m_source->getTask()->setIconified(parameters().value("minimized").toBool());
            setResult(true);
            return;
        }
        else if (operation == "setShaded") {
            m_source->getTask()->setShaded(parameters().value("shaded").toBool());
            setResult(true);
            return;
        }
        else if (operation == "setFullScreen") {
            m_source->getTask()->setFullScreen(parameters().value("fullScreen").toBool());
            setResult(true);
            return;
        }
        else if (operation == "setAlwaysOnTop") {
            m_source->getTask()->setAlwaysOnTop(parameters().value("alwaysOnTop").toBool());
            setResult(true);
            return;
        }
        else if (operation == "setKeptBelowOthers") {
            m_source->getTask()->setKeptBelowOthers(parameters().value("keptBelowOthers").toBool());
            setResult(true);
            return;
        }
    }
    else if (operation.startsWith("toggle")) {
        if (operation == "toggleMaximized") {
            m_source->getTask()->toggleMaximized();
            setResult(true);
            return;
        }
        else if (operation == "toggleMinimized") {
            m_source->getTask()->toggleIconified();
            setResult(true);
            return;
        }
        else if (operation == "toggleShaded") {
            m_source->getTask()->toggleShaded();
            setResult(true);
            return;
        }
        else if (operation == "toggleFullScreen") {
            m_source->getTask()->toggleFullScreen();
            setResult(true);
            return;
        }
        else if (operation == "toggleAlwaysOnTop") {
            m_source->getTask()->toggleAlwaysOnTop();
            setResult(true);
            return;
        }
        else if (operation == "toggleKeptBelowOthers") {
            m_source->getTask()->toggleKeptBelowOthers();
            setResult(true);
            return;
        }
    }
    else {
        if (operation == "restore") {
            m_source->getTask()->restore();
            setResult(true);
            return;
        }
        else if (operation == "raise") {
            m_source->getTask()->raise();
            setResult(true);
            return;
        }
        else if (operation == "lower") {
            m_source->getTask()->lower();
            setResult(true);
            return;
        }
        else if (operation == "activate") {
            m_source->getTask()->activate();
            setResult(true);
            return;
        }
        else if (operation == "activateRaiseOrMaximize") {
            m_source->getTask()->activateRaiseOrIconify();
            setResult(true);
            return;
        }
        else if (operation == "toDesktop") {
            m_source->getTask()->toDesktop(parameters().value("desktop").toInt());
            setResult(true);
            return;
        }
        else if (operation == "toCurrentDesktop") {
            m_source->getTask()->toCurrentDesktop();
            setResult(true);
            return;
        }
    }
    setResult(false);
}

#include "taskjob.moc"
