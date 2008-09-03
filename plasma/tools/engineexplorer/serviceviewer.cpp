/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

#include "serviceviewer.h"

#include <KDebug>
#include <KStringHandler>

#include "plasma/dataengine.h"
#include "plasma/service.h"
#include "plasma/servicejob.h"

ServiceViewer::ServiceViewer(Plasma::DataEngine *engine, const QString &source, QWidget *parent)
    : KDialog(parent),
      m_engine(engine),
      m_source(source),
      m_service(0)
{
    setWindowTitle(i18n("Service Viewer"));
    QWidget* mainWidget = new QWidget(this);
    setMainWidget(mainWidget);
    setupUi(mainWidget);

    QString engineName = i18n("Unknown");
    QString serviceName = i18n("Unknown");
    if (m_engine) {
        engineName = KStringHandler::capwords(m_engine->name());
        m_service = m_engine->serviceForSource(m_source);
        serviceName = m_service->name();
        updateOperations();
        connect(m_service, SIGNAL(operationsChanged()), this, SLOT(updateOperations()));
        connect(m_service, SIGNAL(finished(Plasma::ServiceJob*)), this,
                SLOT(operationResult(Plasma::ServiceJob*)));
        connect(m_engine, SIGNAL(destroyed(QObject*)), this, SLOT(engineDestroyed()));
    }

    QString title = i18n("DataEngine: <b>%1</b>; Source: <b>%2</b>; Service <b>%3</b>", engineName, m_source, serviceName);
    m_title->setText(title);
    m_operations->setFocus();

    setButtons(KDialog::Close | KDialog::User1);
    setButtonText(KDialog::User1, i18n("Start Operation"));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(startOperation()));
    enableButton(KDialog::User1, false);
    enableButton(KDialog::User2, false);
}

ServiceViewer::~ServiceViewer()
{
    kDebug() << "service viewer destroy!";
    delete m_service;
    m_engine = 0;
}

void ServiceViewer::updateOperations()
{
    if (!m_engine) {
        return;
    }

    bool enable = false;

    m_operations->clear();
    m_operationDescription->clear();

    if (m_operations) {
        QStringList operations = m_service->operationNames();

        if (!operations.isEmpty()) {
            enable = true;

            foreach (const QString& operation, operations) {
                m_operations->addItem(operation);
            }
        }
    }

    m_operations->setEnabled(enable);
    m_operationsLabel->setEnabled(enable);
    m_operationDescription->setEnabled(enable);
}

void ServiceViewer::startOperation()
{
    if (!m_service) {
        return;
    }
    kDebug() << "start operation" << m_operations->currentText();
}

void ServiceViewer::operationSelected(const QString &operation)
{
    if (!m_service) {
        return;
    }
    kDebug() << "selected operation" << operation;
}

void ServiceViewer::operationResult(Plasma::ServiceJob *job)
{
    if (!m_service) {
        return;
    }
    kDebug() << "operation results are in!";
}

void ServiceViewer::engineDestroyed()
{
    m_service = 0;
    m_engine = 0;
    hide();
    deleteLater();
}


#include "serviceviewer.moc"

