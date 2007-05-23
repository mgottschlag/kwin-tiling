/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#include "engineexplorer.h"

#include <QStandardItemModel>
#include <QVBoxLayout>

#include <KIconLoader>
#include <KIconTheme>

#include "enginemanager.h"

EngineExplorer::EngineExplorer(QWidget* parent)
    : KDialog(parent),
      m_engine(0),
      m_sourceCount(0)
{
    setButtons(KDialog::Close);
    setWindowTitle(i18n("Plasma Engine Explorer"));
    QWidget* mainWidget = new QWidget(this);
    setMainWidget(mainWidget);
    setupUi(mainWidget);

    m_engineManager = new DataEngineManager();
    m_dataModel = new QStandardItemModel(this);
    KIcon pix("plasma");
    int size = IconSize(K3Icon::Dialog);
    m_title->setPixmap(pix.pixmap(size, size));
    connect(m_engines, SIGNAL(activated(QString)), this, SLOT(showEngine(QString)));
    m_data->setModel(m_dataModel);

    listEngines();
}

EngineExplorer::~EngineExplorer()
{
}

void EngineExplorer::updated(const QString& source, const Plasma::DataEngine::Data& data)
{
    QList<QStandardItem*> items = m_dataModel->findItems(source, 0);

    if (items.count() < 1) {
        return;
    }

    QStandardItem* parent = items.first();

    while (parent->hasChildren()) {
        parent->removeRow(0);
    }

    showData(parent, data);
}

void EngineExplorer::listEngines()
{
    m_engines->clear();
    m_engines->addItem("");
    m_engines->addItems(m_engineManager->knownEngines());
}

void EngineExplorer::showEngine(const QString& name)
{
    m_dataModel->clear();
    m_dataModel->setColumnCount(3);
    QStringList headers;
    headers << i18n("DataSource") << i18n("Key") << i18n("Value");
    m_dataModel->setHorizontalHeaderLabels(headers);
    m_engine = 0;
    m_sourceCount = 0;

    if (!m_engineName.isEmpty()) {
        m_engineManager->unloadDataEngine(m_engineName);
    }

    m_engineName = name;
    if (m_engineName.isEmpty()) {
        updateTitle();
        return;
    }

    m_engine = m_engineManager->loadDataEngine(m_engineName);
    if (!m_engine) {
        m_engineName.clear();
        updateTitle();
        return;
    }

    QStringList sources = m_engine->dataSources();

    kDebug() << "showing engine " << m_engine->objectName() << endl;
    kDebug() << "we have " << sources.count() << " data sources" << endl;
    foreach (const QString& source, sources) {
        kDebug() << "adding " << source << endl;
        addSource(source);
    }

    connect(m_engine, SIGNAL(newDataSource(QString)), this, SLOT(addSource(QString)));
    connect(m_engine, SIGNAL(dataSourceRemoved(QString)), this, SLOT(removeSource(QString)));
}

void EngineExplorer::addSource(const QString& source)
{
    QList<QStandardItem*> parentItems;
    QStandardItem* parent = new QStandardItem(source);
    parentItems.append(parent);
    m_dataModel->appendRow(parent);

    kDebug() << "getting data for source " << source << endl;
    Plasma::DataEngine::Data data = m_engine->query(source);
    showData(parent, data);
    m_engine->connectSource(source, this);

    ++m_sourceCount;
    updateTitle();
}

void EngineExplorer::removeSource(const QString& source)
{
    QList<QStandardItem*> items = m_dataModel->findItems(source, 0);

    if (items.count() < 1) {
        return;
    }

    foreach (QStandardItem* item, items) {
        m_dataModel->removeRow(item->row());
    }

    --m_sourceCount;
    updateTitle();
}

void EngineExplorer::showData(QStandardItem* parent, Plasma::DataEngine::Data data)
{
    int rowCount = 0;
    Plasma::DataEngine::DataIterator it(data);
//    parent->insertRows(0, data.count());
//    parent->setColumnCount(3);
    while (it.hasNext()) {
        it.next();
        parent->setChild(rowCount, 1, new QStandardItem(it.key()));
        parent->setChild(rowCount, 2, new QStandardItem(it.value().toString()));
        ++rowCount;
    }
}

void EngineExplorer::updateTitle()
{
    if (!m_engine) {
        m_title->setPixmap(KIcon("plasma").pixmap(IconSize(K3Icon::Dialog)));
        m_title->setText(i18n("Plasma DataEngine Explorer"));
        return;
    }

    m_title->setText(i18nc("The name of the engine followed by the number of data sources",
                           "%1 - %2 data sources", m_engine->objectName(), m_sourceCount));
    if (m_engine->icon().isEmpty()) {
        m_title->setPixmap(KIcon("plasma").pixmap(IconSize(K3Icon::Dialog)));
    } else {
        m_title->setPixmap(KIcon("alarmclock").pixmap(IconSize(K3Icon::Dialog)));
        //m_title->setPixmap(KIcon(m_engine->icon()).pixmap(IconSize(K3Icon::Dialog)));
    }
}

#include "engineexplorer.moc"

