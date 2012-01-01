/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *   Copyright (C) 2010 Michel Lafon-Puyo <michel.lafonpuyo@gmail.com>
 *   Copyright (C) 2011 Shaun Reich <shaun.reich@kdemail.net>
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

#include "hdd_activity.h"
#include "monitoricon.h"
#include "plotter.h"

#include <KDebug>
#include <KConfigDialog>
#include <KColorUtils>
#include <QFileInfo>
#include <QGraphicsLinearLayout>

#include <Plasma/Meter>
#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>

/**
 * Examples of what the regexp has to handle...
 * It's going to end up matching everything.
 *
 * I can limit it later though, to probably just
 * the entries below.
 *
 * disk/sda3_(8:3)/Rate/totalio
 * disk/md0_(9:0)/Rate/rio
 * disk/md0_(9:0)/Rate/wio
 *
 * Thought Process:
 * The theory is that on startup, we find all possible sources that match
 * that pattern. There will be no entries checked by default though;
 * therefore the user will configure it by loading the settings page,
 * which will be populated by that list of possible sources.
 *
 * The user hits OK, we put all checked entries into a list of entries to watch.
 * We'll watch only those and provide data accordingly.
 *
 * Then just be sure to handle configChanged() properly.
 */
Hdd_Activity::Hdd_Activity(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args),
      m_regexp("disk/.*/Rate/.*")
{
    kDebug() << "###### HDD ACTIVITY CTOR";
    setHasConfigurationInterface(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

Hdd_Activity::~Hdd_Activity()
{
}

void Hdd_Activity::init()
{
    kDebug() << "######## HDD ACTIVITY APPLET INIT";
    KGlobal::locale()->insertCatalog("plasma_applet_system-monitor");
    setEngine(dataEngine("systemmonitor"));
    setTitle(i18n("Disk Activity"), true);

    configChanged();

    /* At the time this method is running, not all sources may be connected. */
    connect(engine(), SIGNAL(sourceAdded(QString)), this, SLOT(sourceChanged(QString)));
    connect(engine(), SIGNAL(sourceRemoved(QString)), this, SLOT(sourceChanged(QString)));
    foreach (const QString& source, engine()->sources()) {
        sourceChanged(source);
    }
}

void Hdd_Activity::sourceChanged(const QString& name)
{
    kDebug() << "######## sourceChanged name: " << name;
    kDebug() << "$$$$$$$ regexp captures: " << m_regexp.capturedTexts();

    if (m_regexp.indexIn(name) != -1) {
        kDebug() << "######### REGEXP match successful, hopefully";
        //kDebug() << m_regexp.cap(1);
//        m_cpus << name;
//        if (!m_sourceTimer.isActive()) {
//            m_sourceTimer.start(0);
//        }
    }
}

void Hdd_Activity::sourcesChanged()
{
    kDebug() << "###### sourcesChanged";
    configChanged();
}

void Hdd_Activity::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    kDebug() << "####### dataUpdated source: " << source << " data: " << data;
   // SM::Plotter *plotter = qobject_cast<SM::Plotter*>(visualization(source));
   // if (plotter) {
   //     double value = data["value"].toDouble();
   //     QString temp = KGlobal::locale()->formatNumber(value, 1);
   //     plotter->addSample(QList<double>() << value);
   //     if (mode() == SM::Applet::Panel) {
   //         setToolTip(source, QString("<tr><td>%1&nbsp;</td><td>%2%</td></tr>")
   //         .arg(plotter->title()).arg(temp));
   //     }
   // }
}

void Hdd_Activity::createConfigurationInterface(KConfigDialog *parent)
{
    kDebug() << "###### createConfigurationInterface";
//    QWidget *widget = new QWidget();
//    ui.setupUi(widget);
//    m_hddModel.clear();
//    m_hddModel.setHorizontalHeaderLabels(QStringList() << i18n("Mount Point")
//                                                       << i18n("Name"));
//    QStandardItem *parentItem = m_hddModel.invisibleRootItem();
//    Plasma::DataEngine::Data data;
//    QString predicateString("IS StorageVolume");
//
//    foreach (const QString& uuid, engine()->query(predicateString)[predicateString].toStringList()) {
//        if (!isValidDevice(uuid, &data)) {
//            continue;
//        }
//        QStandardItem *item1 = new QStandardItem(filePath(data));
//        item1->setEditable(false);
//        item1->setCheckable(true);
//        item1->setData(uuid);
//        if (sources().contains(uuid)) {
//            item1->setCheckState(Qt::Checked);
//        }
//        QStandardItem *item2 = new QStandardItem(hddTitle(uuid, data));
//        item2->setData(guessHddTitle(data));
//        item2->setEditable(true);
//        parentItem->appendRow(QList<QStandardItem *>() << item1 << item2);
//    }
//
//    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
//    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
//    connect(ui.treeView, SIGNAL(clicked(QModelIndex)), parent, SLOT(settingsModified()));
//    connect(ui.intervalSpinBox, SIGNAL(valueChanged(QString)), parent, SLOT(settingsModified()));
//
//    QWidget *widget = new QWidget();
//    ui.setupUi(widget);
//    m_model.clear();
//    m_model.setHorizontalHeaderLabels(QStringList() << i18n("CPU"));
//    QStandardItem *parentItem = m_model.invisibleRootItem();
//
//    foreach (const QString& cpu, m_cpus) {
//        if (m_regexp.indexIn(cpu) != -1) {
//            QStandardItem *item1 = new QStandardItem(cpuTitle(m_regexp.cap(1)));
//            item1->setEditable(false);
//            item1->setCheckable(true);
//            item1->setData(cpu);
//            if (sources().contains(cpu)) {
//                item1->setCheckState(Qt::Checked);
//            }
//            parentItem->appendRow(QList<QStandardItem *>() << item1);
//        }
//    }
//
//    ui.treeView->setModel(&m_hddModel);
//    ui.treeView->resizeColumnToContents(0);
//    ui.intervalSpinBox->setValue(interval() / 1000.0);
//    ui.intervalSpinBox->setSuffix(i18nc("second", " s"));
//    parent->addPage(widget, i18n("Partitions"), "drive-harddisk");
//
//    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
//    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
//    connect(ui.treeView, SIGNAL(clicked(QModelIndex)), parent, SLOT(settingsModified()));
//    connect(ui.intervalSpinBox, SIGNAL(valueChanged(QString)), parent, SLOT(settingsModified()));
}

void Hdd_Activity::configChanged()
{
    kDebug() << "#### configChanged";
    //KConfigGroup cg = config();

//    KConfigGroup cg = config();
//    QStringList default_cpus;
//
//    if(m_cpus.contains("cpu/system/TotalLoad")) {
//        default_cpus << "cpu/system/TotalLoad";
//    } else {
//        default_cpus = m_cpus;
//    }
//
//    setInterval(cg.readEntry("interval", 2.0) * 1000.0);
//    setSources(cg.readEntry("cpus", default_cpus));
 //   connectToEngine();
}

void Hdd_Activity::configAccepted()
{
    kDebug() << "#### configAccepted";
//    KConfigGroup cg = config();
//    KConfigGroup cgGlobal = globalConfig();
//
//    clear();
//
//    for (int i = 0; i < parentItem->rowCount(); ++i) {
//        QStandardItem *item = parentItem->child(i, 0);
//        if (item) {
//            if (item->checkState() == Qt::Checked) {
//                appendSource(item->data().toString());
//            }
//        }
//    }
//
//    cg.writeEntry("cpus", sources());
//
//    uint interval = ui.intervalSpinBox->value();
//    cg.writeEntry("interval", interval);
//
//    emit configNeedsSaving();
}

bool Hdd_Activity::addVisualization(const QString& source)
{
    kDebug() << "#### addVisualization";
    return true;
}

#include "hdd_activity.moc"
