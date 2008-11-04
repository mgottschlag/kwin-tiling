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

#include "cpu.h"
#include <Plasma/SignalPlotter>
#include <Plasma/Theme>
#include <KConfigDialog>
#include <QTimer>
#include <QGraphicsLinearLayout>

SM::Cpu::Cpu(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args)
{
    setHasConfigurationInterface(true);
    resize(234 + 20 + 23, 135 + 20 + 25);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

SM::Cpu::~Cpu()
{
}

void SM::Cpu::init()
{
    KConfigGroup cg = config();
    setEngine(dataEngine("systemmonitor"));
    setInterval(cg.readEntry("interval", 2) * 1000);
    setTitle(i18n("CPU"));
    if (engine()->sources().count() == 0) {
        connect(engine(), SIGNAL(sourceAdded(QString)), this, SLOT(initLater(const QString)));
    } else {
        parseSources();
    }
}

void SM::Cpu::parseSources()
{
    QRegExp rx("cpu/(\\w+)/TotalLoad");

    foreach (const QString& s, engine()->sources()) {
        if (rx.indexIn(s) != -1) {
            //kDebug() << rx.cap(1);
            m_cpus << s;
        }
    }
    KConfigGroup cg = config();
    setItems(cg.readEntry("cpus", QStringList() << "cpu/system/TotalLoad"));
    connectToEngine();
}

void SM::Cpu::initLater(const QString &name)
{
    // How we know all (cpu) sources are ready???
    if (name == "system/uptime") {
        QTimer::singleShot(0, this, SLOT(parseSources()));
    }
}

bool SM::Cpu::addMeter(const QString& source)
{
    QStringList l = source.split('/');
    if (l.count() < 3) {
        return false;
    }
    QString cpu = l[2];
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    Plasma::SignalPlotter *plotter = new Plasma::SignalPlotter(this);
    plotter->addPlot(theme->color(Plasma::Theme::TextColor));
    plotter->setUseAutoRange(false);
    plotter->setVerticalRange(0.0, 100.0);
    plotter->setThinFrame(false);
    plotter->setShowLabels(false);
    plotter->setShowTopBar(false);
    plotter->setShowVerticalLines(false);
    plotter->setShowHorizontalLines(false);
    plotter->setFontColor(theme->color(Plasma::Theme::HighlightColor));
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(8);
    plotter->setFont(font);
    plotter->setHorizontalLinesColor(theme->color(Plasma::Theme::HighlightColor));
    plotter->setVerticalLinesColor(theme->color(Plasma::Theme::HighlightColor));
    plotter->setHorizontalLinesCount(4);
    //plotter->setSvgBackground("widgets/plot-background");
    plotter->setTitle(cpu);
    plotter->setUnit("%");
    appendPlotter(source, plotter);
    mainLayout()->addItem(plotter);
    setPreferredItemHeight(80);
    return true;
}

void SM::Cpu::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Plasma::SignalPlotter *plotter = plotters()[source];
    if (plotter) {
        plotter->addSample(QList<double>() << data["value"].toDouble());
    }
}

void SM::Cpu::createConfigurationInterface(KConfigDialog *parent)
{
   QWidget *widget = new QWidget();
   ui.setupUi(widget);
   m_model.clear();
   m_model.setHorizontalHeaderLabels(QStringList() << i18n("CPU"));
   QStandardItem *parentItem = m_model.invisibleRootItem();
   QRegExp rx("cpu/(\\w+)/TotalLoad");


   foreach (const QString& cpu, m_cpus) {
      if (rx.indexIn(cpu) != -1) {
         QStandardItem *item1 = new QStandardItem(rx.cap(1));
         item1->setEditable(false);
         item1->setCheckable(true);
         item1->setData(cpu);
         if (items().contains(cpu)) {
            item1->setCheckState(Qt::Checked);
         }
         parentItem->appendRow(QList<QStandardItem *>() << item1);
      }
   }
   ui.treeView->setModel(&m_model);
   ui.treeView->resizeColumnToContents(0);
   ui.intervalSpinBox->setValue(interval() / 1000);

   parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
   parent->addPage(widget, i18n("CPUs"), "cpu");
   connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
   connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void SM::Cpu::configAccepted()
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
    cg.writeEntry("cpus", items());

    uint interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);
    interval *= 1000;
    setInterval(interval);

    emit configNeedsSaving();
    connectToEngine();
}

void SM::Cpu::setDetail(Detail detail)
{
    foreach (const QString& key, plotters().keys()) {
        plotters().value(key)->setShowLabels(detail == SM::Applet::High);
        //plotters().value(key)->setShowTopBar(detail == SM::Applet::High);
        //plotters().value(key)->setShowVerticalLines(detail == SM::Applet::High);
        plotters().value(key)->setShowHorizontalLines(detail == SM::Applet::High);
    }
}

#include "cpu.moc"
