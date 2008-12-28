/*
 *   Copyright (C) 2008 Petri Damsten <damu@iki.fi>
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

#include "net.h"
#include <Plasma/SignalPlotter>
#include <Plasma/Theme>
#include <KConfigDialog>
#include <QTimer>
#include <QGraphicsLinearLayout>

SM::Net::Net(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args)
{
    setHasConfigurationInterface(true);
    resize(234 + 20 + 23, 135 + 20 + 25);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

SM::Net::~Net()
{
}

void SM::Net::init()
{
    KConfigGroup cg = config();
    setEngine(dataEngine("systemmonitor"));
    setInterval(cg.readEntry("interval", 2) * 1000);
    setTitle(i18n("Network"));
    if (engine()->sources().count() == 0) {
        connect(engine(), SIGNAL(sourceAdded(QString)), this, SLOT(initLater(const QString)));
    } else {
        parseSources();
    }
}

void SM::Net::parseSources()
{
    QRegExp rx("network/interfaces/(\\w+)/transmitter/data");

    foreach (const QString& s, engine()->sources()) {
        if (rx.indexIn(s) != -1) {
            //kDebug() << rx.cap(1);
            if (rx.cap(1) != "lo") {
                m_interfaces << s;
            }
        }
    }
    KConfigGroup cg = config();
    setItems(cg.readEntry("interfaces", m_interfaces));
    connectToEngine();
}

void SM::Net::initLater(const QString &name)
{
    // How we know all (network) sources are ready???
    if (name == "ps") {
        QTimer::singleShot(0, this, SLOT(parseSources()));
    }
}

bool SM::Net::addMeter(const QString& source)
{
    QStringList l = source.split('/');
    if (l.count() < 3) {
        return false;
    }
    QString interface = l[2];
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    Plasma::SignalPlotter *plotter = new Plasma::SignalPlotter(this);
    QColor color = theme->color(Plasma::Theme::TextColor);
    plotter->addPlot(QColor(((color.red() - 128) * 0.65) + 128,
                            ((color.green() - 128) * 0.65) + 128, 0, color.alpha()));
    plotter->addPlot(QColor(((color.red() - 128) * 0.90) + 128, 0, 0, color.alpha()));
    plotter->setUseAutoRange(true);
    plotter->setThinFrame(false);
    plotter->setShowLabels(false);
    plotter->setShowTopBar(false);
    plotter->setShowVerticalLines(false);
    plotter->setShowHorizontalLines(false);
    plotter->setStackPlots(true);
    plotter->setFontColor(theme->color(Plasma::Theme::HighlightColor));
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(8);
    plotter->setFont(font);
    plotter->setHorizontalLinesColor(theme->color(Plasma::Theme::HighlightColor));
    plotter->setVerticalLinesColor(theme->color(Plasma::Theme::HighlightColor));
    plotter->setHorizontalLinesCount(4);
    //plotter->setSvgBackground("widgets/plot-background");
    plotter->setTitle(interface);
    plotter->setUnit("KiB/s");
    appendPlotter(interface, plotter);
    mainLayout()->addItem(plotter);
    connectSource("network/interfaces/" + interface + "/receiver/data");
    setPreferredItemHeight(80);
    return true;
}

void SM::Net::themeChanged()
{
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    QColor color = theme->color(Plasma::Theme::TextColor);
    foreach (Plasma::SignalPlotter *plotter, plotters().values()) {
        plotter->removePlot(1);
        plotter->removePlot(0);
        plotter->addPlot(QColor(((color.red() - 128) * 0.65) + 128,
                                ((color.green() - 128) * 0.65) + 128, 0, color.alpha()));
        plotter->addPlot(QColor(((color.red() - 128) * 0.90) + 128, 0, 0, color.alpha()));
        plotter->setFontColor(theme->color(Plasma::Theme::HighlightColor));
        plotter->setHorizontalLinesColor(theme->color(Plasma::Theme::HighlightColor));
        plotter->setVerticalLinesColor(theme->color(Plasma::Theme::HighlightColor));
    }
}

void SM::Net::dataUpdated(const QString& source,
                          const Plasma::DataEngine::Data &data)
{
    QString interface = source.split('/')[2];

    m_data[interface] << data["value"].toDouble();
    if (m_data[interface].count() > 1) {
       Plasma::SignalPlotter *plotter = plotters()[interface];
        if (plotter) {
            plotter->addSample(m_data[interface]);
            //plotter->addSample(QList<double>() << 50 << 25);
        }
        m_data[interface].clear();
    }
}

void SM::Net::createConfigurationInterface(KConfigDialog *parent)
{
   QWidget *widget = new QWidget();
   ui.setupUi(widget);
   m_model.clear();
   m_model.setHorizontalHeaderLabels(QStringList() << i18n("Network interface"));
   QStandardItem *parentItem = m_model.invisibleRootItem();


   foreach (const QString& interface, m_interfaces) {
        QString ifname = interface.split('/')[2];
        QStandardItem *item1 = new QStandardItem(ifname);
        item1->setEditable(false);
        item1->setCheckable(true);
        item1->setData(interface);
        if (items().contains(interface)) {
            item1->setCheckState(Qt::Checked);
        }
        parentItem->appendRow(QList<QStandardItem *>() << item1);
   }
   ui.treeView->setModel(&m_model);
   ui.treeView->resizeColumnToContents(0);
   ui.intervalSpinBox->setValue(interval() / 1000);

   parent->addPage(widget, i18n("Interfaces"), "network-workgroup");
   connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
   connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void SM::Net::configAccepted()
{
    KConfigGroup cg = config();
    QStandardItem *parentItem = m_model.invisibleRootItem();

    clearItems();
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        QStandardItem *item = parentItem->child(i, 0);
        if (item) {
            if (item->checkState() == Qt::Checked) {
                appendItem(item->data().toString());
            }
        }
    }
    cg.writeEntry("interfaces", items());

    uint interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);
    interval *= 1000;
    setInterval(interval);
    emit configNeedsSaving();
    connectToEngine();
}

void SM::Net::setDetail(Detail detail)
{
    foreach (const QString& key, plotters().keys()) {
        plotters().value(key)->setShowLabels(detail == SM::Applet::High);
        //plotters().value(key)->setShowTopBar(detail == SM::Applet::High);
        //plotters().value(key)->setShowVerticalLines(detail == SM::Applet::High);
        plotters().value(key)->setShowHorizontalLines(detail == SM::Applet::High);
    }
}

#include "net.moc"
