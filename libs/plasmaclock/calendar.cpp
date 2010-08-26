/*
 *   Copyright 2008,2010 Davide Bettio <davide.bettio@kdemail.net>
 *   Copyright 2009 John Layt <john@layt.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "calendar.h"

//Qt
#include <QtCore/QDate>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsView>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QSpinBox>
#include <QtGui/QToolButton>

//KDECore
#include <KCalendarSystem>
#include <KDebug>
#include <KGlobal>
#include <KIcon>
#include <KLineEdit>
#include <KLocale>
#include <KIntSpinBox>
#include <KConfigDialog>
#include <KConfigGroup>

//Plasma
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/SpinBox>
#include <Plasma/ToolButton>
#include <Plasma/ToolTipManager>
#include <Plasma/DataEngine>

#include <kephal/screens.h>

#include "wheelytoolbutton.h"

namespace Plasma
{

static const int s_yearWidgetIndex = 3;

class CalendarPrivate
{
    public:
        CalendarPrivate()
            : back(0),
              spacer0(0),
              month(0),
              yearSpinBox(0),
              year(0),
              spacer1(0),
              forward(0),
              calendarTable(0),
              dateText(0),
              jumpToday(0),
              monthMenu(0),
              weekSpinBox(0)
        {
        }

        ToolButton *back;
        Plasma::Label *spacer0;
        Plasma::ToolButton *month;
        Plasma::SpinBox *yearSpinBox;
        Plasma::ToolButton *year;
        Plasma::Label *spacer1;
        Plasma::ToolButton *forward;
        Plasma::CalendarTable *calendarTable;
        Plasma::LineEdit *dateText;
        ToolButton *jumpToday;
        QMenu *monthMenu;
        Plasma::SpinBox *weekSpinBox;
};

Calendar::Calendar(const QDate &date, QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{
    init(new CalendarTable(date, this));
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

Calendar::Calendar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{
    init(new CalendarTable(this));
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

Calendar::Calendar(CalendarTable *calendarTable, QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{
    init(calendarTable ? calendarTable : new CalendarTable(this));
}

Calendar::~Calendar()
{
   delete d->monthMenu;
   delete d;
}

void Calendar::init(CalendarTable *calendarTable)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    QGraphicsLinearLayout *hLayout = new QGraphicsLinearLayout(layout);
    QGraphicsLinearLayout *layoutTools = new QGraphicsLinearLayout(layout);

    d->calendarTable = calendarTable;
    d->calendarTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(d->calendarTable, SIGNAL(dateChanged(const QDate &)), this, SLOT(dateUpdated(const QDate &)));
    connect(d->calendarTable, SIGNAL(dateHovered(const QDate &)), this, SIGNAL(dateHovered(const QDate &)));
    connect(d->calendarTable, SIGNAL(dateSelected(const QDate &)), this, SLOT(showTip(const QDate &)));
    connect(this, SIGNAL(dateHovered(const QDate &)), this, SLOT(showTip(const QDate &)));

    d->back = new Plasma::ToolButton(this);
    d->back->setText("<");
    d->back->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->back, SIGNAL(clicked()), this, SLOT(prevMonth()));
    hLayout->addItem(d->back);

    hLayout->addStretch();

    d->month = new WheelyToolButton(this);
    d->month->setText(calendar()->monthName(calendar()->month(date()), calendar()->year(date())));
    d->month->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    d->monthMenu = new QMenu();
    d->month->nativeWidget()->setMenu(d->monthMenu);
    connect(d->month, SIGNAL(clicked()), this, SLOT(monthsPopup()));
    connect(d->month, SIGNAL(wheelUp()), this, SLOT(prevMonth()));
    connect(d->month, SIGNAL(wheelDown()), this, SLOT(nextMonth()));
    hLayout->addItem(d->month);

    d->year = new WheelyToolButton(this);
    d->year->setText(calendar()->yearString(date()));
    d->year->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->year, SIGNAL(wheelUp()), this, SLOT(prevYear()));
    connect(d->year, SIGNAL(wheelDown()), this, SLOT(nextYear()));
    connect(d->year, SIGNAL(clicked()), this, SLOT(showYearSpinBox()));
    hLayout->addItem(d->year);

    d->yearSpinBox = new Plasma::SpinBox(this);
    d->yearSpinBox->setRange(calendar()->year(calendar()->earliestValidDate()), calendar()->year(calendar()->latestValidDate()));
    d->yearSpinBox->setValue(calendar()->year(date()));
    d->yearSpinBox->hide();
    connect(d->yearSpinBox->nativeWidget(), SIGNAL(editingFinished()), this, SLOT(hideYearSpinBox()));

    hLayout->addStretch();

    d->forward = new Plasma::ToolButton(this);
    d->forward->setText(">");
    d->forward->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->forward, SIGNAL(clicked()), this, SLOT(nextMonth()));
    hLayout->addItem(d->forward);

    d->jumpToday = new Plasma::ToolButton(this);
    d->jumpToday->nativeWidget()->setIcon(KIcon("go-jump-today"));
    d->jumpToday->nativeWidget()->setMinimumWidth(25);
    connect(d->jumpToday, SIGNAL(clicked()), this, SLOT(goToToday()));
    layoutTools->addItem(d->jumpToday);
    layoutTools->addStretch();

    d->dateText = new Plasma::LineEdit(this);
    d->dateText->setText(calendar()->formatDate(date(),  KLocale::ShortDate));
    connect(d->dateText->nativeWidget(), SIGNAL(returnPressed()), this, SLOT(manualDateChange()));
    layoutTools->addItem(d->dateText);
    layoutTools->addStretch();

    d->weekSpinBox = new Plasma::SpinBox(this);
    d->weekSpinBox->setMinimum(1);
    d->weekSpinBox->setMaximum(calendar()->weeksInYear(date()));
    connect(d->weekSpinBox, SIGNAL(valueChanged(int)), this, SLOT(goToWeek(int)));
    layoutTools->addItem(d->weekSpinBox);

    layout->addItem(hLayout);
    layout->addItem(d->calendarTable);
    layout->addItem(layoutTools);

    dateUpdated(date());
}

CalendarTable *Calendar::calendarTable() const
{
    return d->calendarTable;
}

void Calendar::setCalendar(const QString &newCalendarType)
{
    calendarTable()->setCalendar(newCalendarType);
    d->weekSpinBox->setMaximum(calendar()->weeksInYear(date()));
    d->yearSpinBox->setRange(calendar()->year(calendar()->earliestValidDate()),
                             calendar()->year(calendar()->latestValidDate()));
    refreshWidgets();
}

void Calendar::setCalendar(const KCalendarSystem *newCalendar)
{
    calendarTable()->setCalendar(newCalendar);
    d->weekSpinBox->setMaximum(calendar()->weeksInYear(date()));
    d->yearSpinBox->setRange(calendar()->year(calendar()->earliestValidDate()),
                             calendar()->year(calendar()->latestValidDate()));
    refreshWidgets();
}

const KCalendarSystem *Calendar::calendar() const
{
    return calendarTable()->calendar();
}

void Calendar::setDate(const QDate &toDate)
{
    d->calendarTable->setDate(toDate);

    //If set date failed force refresh of nav widgets to reset any user entry
    //If set date successful refresh will be triggered through signal/slot
    if (d->calendarTable->date() != toDate) {
        refreshWidgets();
    }

    d->weekSpinBox->setMaximum(calendar()->weeksInYear(date()));
}

const QDate& Calendar::date() const
{
    return calendarTable()->date();
}

void Calendar::setDataEngine(Plasma::DataEngine *dataEngine)
{
    calendarTable()->setDataEngine(dataEngine);
}

const Plasma::DataEngine *Calendar::dataEngine() const
{
    return calendarTable()->dataEngine();
}

void Calendar::setDisplayHolidays(bool showHolidays)
{
    calendarTable()->setDisplayHolidays(showHolidays);
}

bool Calendar::displayHolidays()
{
    return calendarTable()->displayHolidays();
}

void Calendar::setHolidaysRegion(const QString &region)
{
    calendarTable()->setHolidaysRegion(region);
}

QString Calendar::holidaysRegion() const
{
    return calendarTable()->holidaysRegion();
}

bool Calendar::dateHasDetails(const QDate &date) const
{
    return calendarTable()->dateHasDetails(date);
}

QString Calendar::dateDetails(const QDate &date) const
{
    return calendarTable()->dateDetails(date);
}

void Calendar::setAutomaticUpdateEnabled(bool automatic)
{
    calendarTable()->setAutomaticUpdateEnabled(automatic);
}

bool Calendar::isAutomaticUpdateEnabled() const
{
    return calendarTable()->isAutomaticUpdateEnabled();
}

void Calendar::setCurrentDate(const QDate &date)
{
    calendarTable()->setCurrentDate(date);  
}

const QDate& Calendar::currentDate() const
{
    return calendarTable()->currentDate();
}

void Calendar::applyConfiguration(KConfigGroup cg)
{
    calendarTable()->applyConfiguration(cg);
}

void Calendar::writeConfiguration(KConfigGroup cg)
{
    calendarTable()->writeConfiguration(cg);
}

void Calendar::createConfigurationInterface(KConfigDialog *parent)
{
    calendarTable()->createConfigurationInterface(parent);
}

void Calendar::applyConfigurationInterface()
{
    calendarTable()->applyConfigurationInterface();
}

void Calendar::configAccepted(KConfigGroup cg)
{
    calendarTable()->configAccepted(cg);
}

void Calendar::manualDateChange()
{
    setDate(calendar()->readDate(((QLineEdit*)sender())->text()));
}

void Calendar::goToToday()
{
    setDate(QDate::currentDate());
}

void Calendar::dateUpdated(const QDate &newDate)
{
    // Ignore the date passed in, only ever show the date to match the CalendarTable
    Q_UNUSED(newDate);
    refreshWidgets();
    emit dateChanged(date());
}

void Calendar::showTip(const QDate &date)
{
    QGraphicsWidget *item = parentWidget();
    if (!item) {
        item = this;
    }

    if (dateHasDetails(date)) {
        const QString details = dateDetails(date);
        Plasma::ToolTipContent content(calendar()->formatDate(date),
                                       details,
                                       KIcon("view-pim-calendar"));
        content.setAutohide(false);
        Plasma::ToolTipManager::self()->setContent(item, content);
        Plasma::ToolTipManager::self()->show(item);
    } else {
        if (Plasma::ToolTipManager::self()->isVisible(item)) {
            Plasma::ToolTipManager::self()->hide(item);
        }
        Plasma::ToolTipManager::self()->setContent(item, Plasma::ToolTipContent());
    }
}

// Update the nav widgets to show the current date in the CalendarTable
void Calendar::refreshWidgets()
{
    d->month->setText(calendar()->monthName(calendar()->month(date()), calendar()->year(date())));
    d->month->setMinimumSize(static_cast<QToolButton*>(d->month->widget())->sizeHint());
    d->year->setText(calendar()->yearString(date()));
    d->dateText->setText(calendar()->formatDate(date(),  KLocale::ShortDate));

    // Block the signals to prevent changing the date again
    d->weekSpinBox->blockSignals(true);
    d->weekSpinBox->setValue(calendar()->weekNumber(date()));
    d->weekSpinBox->blockSignals(false);
}

void Calendar::prevMonth()
{
    setDate(calendar()->addMonths(date(), -1));
}

void Calendar::nextMonth()
{
    setDate(calendar()->addMonths(date(), 1));
}

void Calendar::prevYear()
{
    setDate(calendar()->addYears(date(), -1));
}

void Calendar::nextYear()
{
    setDate(calendar()->addYears(date(), 1));
}

void Calendar::monthsPopup()
{
    d->monthMenu->clear();
    int year = calendar()->year(date());
    int monthsInYear = calendar()->monthsInYear(date());

    for (int i = 1; i <= monthsInYear; i++){
        QAction *tmpAction = new QAction(calendar()->monthName(i, year), d->monthMenu);
        tmpAction->setProperty("month", i);
        connect(tmpAction, SIGNAL(triggered()), this, SLOT(monthTriggered()));
        d->monthMenu->addAction(tmpAction);
    }

    QGraphicsView *view = Plasma::viewFor(d->month);
    if (view) {
        d->monthMenu->adjustSize();
        const int x = d->month->sceneBoundingRect().center().x() - d->monthMenu->width() / 2;
        QPoint pos(x, d->month->sceneBoundingRect().bottom());
        pos = view->mapToGlobal(view->mapFromScene(pos));
        QRect r = Kephal::ScreenUtils::screenGeometry(Kephal::ScreenUtils::screenId(view->geometry().center()));
        if (pos.y() + d->monthMenu->height() > r.bottom()) {
            pos = QPoint(x, d->month->sceneBoundingRect().top() - d->monthMenu->height());
            pos = view->mapToGlobal(view->mapFromScene(pos));
        }
        d->monthMenu->popup(pos);
    } else {
        d->monthMenu->popup(QCursor::pos());
    }
}

void Calendar::monthTriggered()
{
    QAction *action = dynamic_cast<QAction*> (sender());

    if (action && action->property("month").type() == QVariant::Int) {
        int newMonth = action->property("month").toInt();
        int currMonth = calendar()->month(date());
        setDate(calendar()->addMonths(date(), newMonth - currMonth));
    }
}

void Calendar::goToWeek(int newWeek)
{
    int currWeek = calendar()->weekNumber(date());
    int daysInWeek = calendar()->daysInWeek(date());

    setDate(calendar()->addDays(date(), (newWeek - currWeek) * daysInWeek));
}

void Calendar::showYearSpinBox()
{
    QGraphicsLinearLayout *hLayout = (QGraphicsLinearLayout*)d->year->parentLayoutItem();

    d->year->hide();
    hLayout->removeItem(d->year);
    d->yearSpinBox->setValue(calendar()->year(date()));
    d->yearSpinBox->setMinimumWidth(d->yearSpinBox->preferredSize().width());
    hLayout->insertItem(s_yearWidgetIndex, d->yearSpinBox);
    hLayout->activate();
    d->yearSpinBox->show();
    d->yearSpinBox->nativeWidget()->setFocus(Qt::MouseFocusReason);
}

void Calendar::hideYearSpinBox()
{
    QGraphicsLinearLayout *hLayout = (QGraphicsLinearLayout*)d->yearSpinBox->parentLayoutItem();
    hLayout->removeItem(d->yearSpinBox);
    hLayout->insertItem(s_yearWidgetIndex, d->year);
    d->yearSpinBox->hide();

    int newYear = d->yearSpinBox->value();
    int currYear = calendar()->year(date());
    setDate(calendar()->addYears(date(), newYear - currYear));
    d->year->show();
}

}

#include "calendar.moc"
