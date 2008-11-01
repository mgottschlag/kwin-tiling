/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
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

#include "hdd.h"
#include "monitoricon.h"
#include <plasma/widgets/meter.h>
#include <plasma/containment.h>
#include <plasma/theme.h>
#include <KConfigDialog>
#include <QFileInfo>
#include <QGraphicsLinearLayout>

Hdd::Hdd(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args)
{
    setHasConfigurationInterface(true);
    resize(215 + 20 + 23, 99 + 20 + 25);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

Hdd::~Hdd()
{
}

void Hdd::init()
{
    KConfigGroup cg = persistentConfig();
    QString predicateString("IS StorageVolume");
    setEngine(dataEngine("soliddevice"));
    setItems(cg.readEntry("uuids",
             engine()->query(predicateString)[predicateString].toStringList()));
    setInterval(cg.readEntry("interval", 2) * 60 * 1000);

    setTitle(i18n("Disk Space"), true);
    connectToEngine();
}

void Hdd::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    m_hddModel.clear();
    m_hddModel.setHorizontalHeaderLabels(QStringList() << i18n("Mount Point")
                                                       << i18n("Name"));
    QStandardItem *parentItem = m_hddModel.invisibleRootItem();
    Plasma::DataEngine::Data data;
    QString predicateString("IS StorageVolume");

    foreach (const QString& uuid, engine()->query(predicateString)[predicateString].toStringList()) {
        if (!isValidDevice(uuid, &data)) {
            continue;
        }
        QStandardItem *item1 = new QStandardItem(data["File Path"].toString());
        item1->setEditable(false);
        item1->setCheckable(true);
        item1->setData(uuid);
        if (items().contains(uuid)) {
            item1->setCheckState(Qt::Checked);
        }
        QStandardItem *item2 = new QStandardItem(title(uuid, data));
        item2->setEditable(true);
        parentItem->appendRow(QList<QStandardItem *>() << item1 << item2);
    }
    ui.treeView->setModel(&m_hddModel);
    ui.treeView->resizeColumnToContents(0);
    ui.intervalSpinBox->setValue(interval() / 60 / 1000);

    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    parent->addPage(widget, i18n("Partitions"), "drive-harddisk");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void Hdd::configAccepted()
{
    KConfigGroup cg = persistentConfig();
    QStandardItem *parentItem = m_hddModel.invisibleRootItem();

    clearItems();
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        QStandardItem *item = parentItem->child(i, 0);
        if (item) {
            cg.writeEntry(item->data().toString(),
                          parentItem->child(i, 1)->text());
            if (item->checkState() == Qt::Checked) {
                appendItem(item->data().toString());
            }
        }
    }
    cg.writeEntry("uuids", items());

    uint interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);
    interval *= 60 * 1000;
    setInterval(interval);

    emit configNeedsSaving();
    connectToEngine();
}

QString Hdd::title(const QString& uuid, const Plasma::DataEngine::Data &data)
{
    KConfigGroup cg = persistentConfig();
    QString label = cg.readEntry(uuid, "");

    if (label.isEmpty()) {
        label = data["Label"].toString();
        if (label.isEmpty()) {
            QString path = data["File Path"].toString();
            if (path == "/")
                return i18nc("the root filesystem", "root");
            QFileInfo fi(path);
            label = fi.fileName();
            if (label.isEmpty()) {
                label = data["Device"].toString();
                if (label.isEmpty()) {
                    kDebug() << "Disk: " << uuid << " has empty label";
                    label = i18n("Unknown filesystem");
                }
            }
        }
    }
    QVariant accessible = data["Accessible"];
    if (accessible.isValid()) {
        if (accessible.canConvert(QVariant::Bool)) {
            if (!accessible.toBool()) {
                label = i18nc("hard disk label (not mounted or accessible)",
                              "%1 (not accessible)", label);
            }
        }
    }
    return label;
}

bool Hdd::addMeter(const QString& source)
{
    Plasma::Meter *w;
    Plasma::DataEngine *engine = dataEngine("soliddevice");
    Plasma::DataEngine::Data data;
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();

    if (!engine) {
        return false;
    }
    if (!isValidDevice(source, &data)) {
        // do not try to show hard drives and swap partitions.
        return false;
    }
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Horizontal);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->setSpacing(5);

    w = new Plasma::Meter(this);
    w->setMeterType(Plasma::Meter::BarMeterHorizontal);
    if (mode() != SM::Applet::Panel) {
        MonitorIcon *icon = new MonitorIcon(this);
        m_icons.insert(source, icon);
        icon->setImage("drive-harddisk");
        if (data["Accessible"].toBool()) {
            QStringList overlays;
            overlays << QString("emblem-mounted");
            icon->setOverlays(overlays);
        }
        layout->addItem(icon);
    } else {
        w->setSvg("system-monitor/hdd_panel");
    }
    QColor text = theme->color(Plasma::Theme::TextColor);
    QColor background = theme->color(Plasma::Theme::BackgroundColor);
    QColor darkerText((text.red() + background.red()) / 2,
                      (text.green() + background.green()) / 2,
                      (text.blue() + background.blue()) / 2,
                      (text.alpha() + background.alpha()) / 2);
    w->setLabel(0, title(source, data));
    w->setLabelColor(0, theme->color(Plasma::Theme::TextColor));
    w->setLabelColor(1, darkerText);
    w->setLabelColor(2, darkerText);
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(9);
    w->setLabelFont(0, font);
    font.setPointSizeF(7.5);
    w->setLabelFont(1, font);
    w->setLabelFont(2, font);
    w->setLabelAlignment(0, Qt::AlignVCenter | Qt::AlignLeft);
    w->setLabelAlignment(1, Qt::AlignVCenter | Qt::AlignRight);
    w->setLabelAlignment(2, Qt::AlignVCenter | Qt::AlignCenter);
    w->setMaximum(data["Size"].toULongLong() / (1024 * 1024));
    layout->addItem(w);
    appendMeter(source, w);
    mainLayout()->addItem(layout);
    dataUpdated(source, data);
    setPreferredItemHeight(layout->preferredSize().height());

    QString disk = data["Parent UDI"].toString();

    m_diskMap[disk] << w;
    if (!connectedSources().contains(disk)) {
        data = engine->query(disk);
        dataUpdated(disk, data);
        connectSource(disk);
    }
    return true;
}

void Hdd::deleteMeters(QGraphicsLinearLayout* layout)
{
    Applet::deleteMeters(layout);
    m_diskMap.clear();
}

bool Hdd::isValidDevice(const QString& uuid, Plasma::DataEngine::Data* data)
{
    Plasma::DataEngine *engine = dataEngine("soliddevice");
    if (engine) {
        *data = engine->query(uuid);
        /*
        kDebug() << uuid << data->value("Device").toString() <<
                            data->value("Usage").toString() <<
                            data->value("File System Type").toString() <<
                            data->value("Size").toString();
        */
        if ((data->value("Usage").toString() != "File System" &&
             data->value("Usage").toString() != "Raid") ||
            data->value("File System Type").toString() == "swap") {
            QStringList list = items();
            list.removeAll(uuid);
            setItems(list);
            return false;
        }
        return true;
    }
    return false;
}

void Hdd::dataUpdated(const QString& source,
                      const Plasma::DataEngine::Data &data)
{
    if (m_diskMap.keys().contains(source) && mode() != SM::Applet::Panel) {
        if (data.keys().contains("Temperature")) {
            QList<Plasma::Meter *> widgets = m_diskMap[source];
            foreach (Plasma::Meter *w, widgets) {
                w->setLabel(2, QString("%1\xb0%2").arg(data["Temperature"].toString())
                                                  .arg(data["Temperature Unit"].toString()));
            }
        }
    } else {
        Plasma::Meter *w = meters().value(source);
        if (!w) {
            return;
        }
        qulonglong size = qulonglong(data["Size"].toULongLong() /
                          (1024 * 1024));
        qlonglong availBytes = 0;
        QVariant freeSpace = data["Free Space"];
        if (freeSpace.isValid()) {
            if (freeSpace.canConvert(QVariant::LongLong)) {
                availBytes = qlonglong(freeSpace.toLongLong());
                w->setValue(size - availBytes / (1024 * 1024));
            }
        }
        else {
            w->setValue(0);
        }
        if (mode() != SM::Applet::Panel) {
            w->setLabel(1, i18n("%1", KGlobal::locale()->formatByteSize(
                                    availBytes)));
            QStringList overlays;
            if (data["Accessible"].toBool()) {
                overlays << "emblem-mounted";
            }
            m_icons[source]->setOverlays(overlays);
        }
    }
}

#include "hdd.moc"
