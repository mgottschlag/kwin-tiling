/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *   Copyright (C) 2010 Michel Lafon-Puyo <michel.lafonpuyo@gmail.com>
 *   Copyright (C) 2011, 2012 Shaun Reich <shaun.reich@kdemail.net>
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
 *
 * Note actually it's now set to an inclusive-only mode, far cleaner.
 *
 * Included items only:
 *
 *
 * RAID blocks (there *could* be more if not using mdadm, I think):
 *
 * disk/md<something>/Rate/rio
 * disk/md<something>/Rate/wio
 *
 * SATA disks:
 *
 * disk/sd<something>/Rate/rio
 * disk/sd<something>/Rate/wio
 *
 * IDE/PATA disks:
 *
 * disk/hd<something>/Rate/rio
 * disk/hd<something>/Rate/wio
 *
 */
Hdd_Activity::Hdd_Activity(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args),
    m_regexp("disk/(?:md|sd|hd).*/Rate/(?:rio|wio)")
{
    setHasConfigurationInterface(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_sourceTimer.setSingleShot(true);
    connect(&m_sourceTimer, SIGNAL(timeout()), this, SLOT(sourcesChanged()));
}

Hdd_Activity::~Hdd_Activity()
{
}

void Hdd_Activity::init()
{
    KGlobal::locale()->insertCatalog("plasma_applet_system-monitor");
    setEngine(dataEngine("systemmonitor"));
    setTitle(i18n("Disk Activity"), true);

    configChanged();

    /* At the time this method is running, not all sources may be connected. */
    connect(engine(), SIGNAL(sourceAdded(QString)), this, SLOT(sourceChanged(QString)));
    connect(engine(), SIGNAL(sourceRemoved(QString)), this, SLOT(sourceChanged(QString)));
    foreach (const QString& source, engine()->sources()) {
     // sourceChanged(source);
    }
}

void Hdd_Activity::sourceChanged(const QString& name)
{
    //kDebug() << "######## sourceChanged name: " << name;
    //kDebug() << "###### regexp captures: " << m_regexp.capturedTexts();

    if (m_regexp.indexIn(name) != -1) {
        kDebug() << "######### REGEXP match successful, hopefully. Adding:" << name;

        m_hdds.append(name);

        if (!m_sourceTimer.isActive()) {
            m_sourceTimer.start(0);
        }
    }
}

void Hdd_Activity::sourcesChanged()
{
    kDebug() << "###### sourcesChanged";
    configChanged();
}

void Hdd_Activity::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    //kDebug() << "####### dataUpdated source: " << source << " data: " << data;

//    SM::Plotter *plotter = qobject_cast<SM::Plotter*>(visualization(source));
//    if (plotter) {
//        double value = data["value"].toDouble();
//        QString temp = KGlobal::locale()->formatNumber(value, 1);
//        plotter->addSample(QList<double>() << value);
//
//        if (mode() == SM::Applet::Panel) {
//            setToolTip(source, QString("<tr><td>%1&nbsp;</td><td>%2%</td></tr>")
//            .arg(plotter->title()).arg(temp));
//        }
//    }
}

void Hdd_Activity::createConfigurationInterface(KConfigDialog *parent)
{
    kDebug() << "###### createConfigurationInterface";

    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    m_hddModel.clear();
    m_hddModel.setHorizontalHeaderLabels(QStringList() << i18n("Name"));
    QStandardItem *parentItem = m_hddModel.invisibleRootItem();

    foreach (const QString& hdd, m_hdds) {
        if (m_regexp.indexIn(hdd) != -1) {
            QStandardItem *item1 = new QStandardItem(m_regexp.cap(0));
            item1->setEditable(false);
            item1->setCheckable(true);
            item1->setData(hdd);
            if (sources().contains(hdd)) {
                item1->setCheckState(Qt::Checked);
            }
            parentItem->appendRow(QList<QStandardItem *>() << item1);
        }
    }
    ui.treeView->setModel(&m_hddModel);
    ui.treeView->resizeColumnToContents(0);
    ui.intervalSpinBox->setValue(interval() / 1000.0);
    ui.intervalSpinBox->setSuffix(i18nc("second", " s"));
    parent->addPage(widget, i18n("Hard Disks"), "drive-harddisk");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(ui.treeView, SIGNAL(clicked(QModelIndex)), parent, SLOT(settingsModified()));
    connect(ui.intervalSpinBox, SIGNAL(valueChanged(QString)), parent, SLOT(settingsModified()));
}

void Hdd_Activity::configChanged()
{
    //kDebug() << "#### configChanged m_hdds:" << m_hdds;

    KConfigGroup cg = config();
    QStringList default_hdds = QStringList(); //fucking HACK: m_hdds;

    // default to 2 seconds (2000 ms interval
    setInterval(cg.readEntry("interval", 2.0) * 1000.0);
    setSources(cg.readEntry("hdds", default_hdds));

    if (!m_hdds.isEmpty()) {
        kDebug() << "@@@@ configChanged, reconnecting engine to sources: " << m_hdds;
        setSources(m_hdds);
        connectToEngine();
    }
}

void Hdd_Activity::configAccepted()
{
    kDebug() << "#### configAccepted";
    KConfigGroup cg = config();
    QStandardItem *parentItem = m_hddModel.invisibleRootItem();

    clear();

    for (int i = 0; i < parentItem->rowCount(); ++i) {
        QStandardItem *item = parentItem->child(i, 0);
        if (item) {
            if (item->checkState() == Qt::Checked) {
                appendSource(item->data().toString());
            }
        }
    }

    cg.writeEntry("hdds", sources());

    uint interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);

    emit configNeedsSaving();
}

bool Hdd_Activity::addVisualization(const QString& source)
{
    kDebug() << "#### addVisualization FOR SOURCE:" << source;

    QStringList splits = source.split('/');
    //kDebug() << "### ADD VIS SOURCE SPLITS:" << splits;

    // 0 == "disk" 1 == "sde_(8:64)" 2 == "Rate" 3 == "rio"
    if (splits.count() < 3) {
        return false;
    }

    QString hdd = splits[1];
    //kDebug() << "#### ADD VIS hdd: " << hdd;

    SM::Plotter *plotter = new SM::Plotter(this);
    plotter->setMinMax(0.0, 100.0);
    plotter->setTitle(hdd);
    plotter->setUnit("%");

    appendVisualization(source, plotter);
    setPreferredItemHeight(80);
    return true;
}

#include "hdd_activity.moc"
