/*
 *   Copyright 2008 Davide Bettio <davide.bettio@kdemail.net>
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

//Plasma
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/SpinBox>
#include <Plasma/ToolButton>
#include <Plasma/DataEngine>

namespace Plasma
{

class CalendarPrivate
{
    public:
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
    init(new CalendarTable(date));
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

Calendar::Calendar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{
    init(new CalendarTable());
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

Calendar::Calendar(CalendarTable *calendarTable, QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{
    init(calendarTable);
}

Calendar::~Calendar()
{
   delete d->monthMenu;
   delete d;
}

void Calendar::init(CalendarTable *calendarTable)
{
    QGraphicsLinearLayout *m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    QGraphicsLinearLayout *m_hLayout = new QGraphicsLinearLayout(m_layout);
    QGraphicsLinearLayout *m_layoutTools = new QGraphicsLinearLayout(m_layout);

    d->calendarTable = calendarTable;
    d->calendarTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->back = new Plasma::ToolButton(this);
    d->back->setText("<");
    d->back->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->back, SIGNAL(clicked()), this, SLOT(prevMonth()));
    m_hLayout->addItem(d->back);

    m_hLayout->addStretch();

    d->month = new Plasma::ToolButton(this);
    d->month->setText(calendar()->monthName(calendar()->month(date()), calendar()->year(date())));
    d->month->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    connect(d->month, SIGNAL(clicked()), this, SLOT(monthsPopup()));
    m_hLayout->addItem(d->month);

    d->year = new Plasma::ToolButton(this);
    d->year->setText(QString::number(calendar()->year(date())));
    d->year->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->year, SIGNAL(clicked()), this, SLOT(showYearSpinBox()));
    m_hLayout->addItem(d->year);

    d->yearSpinBox = new Plasma::SpinBox(this);
    d->yearSpinBox->setRange(calendar()->year(calendar()->earliestValidDate()), calendar()->year(calendar()->latestValidDate()));
    d->yearSpinBox->setValue(calendar()->year(date()));
    d->yearSpinBox->hide();
    connect(d->yearSpinBox->nativeWidget(), SIGNAL(editingFinished()), this, SLOT(hideYearSpinBox()));

    m_hLayout->addStretch();

    d->forward = new Plasma::ToolButton(this);
    d->forward->setText(">");
    d->forward->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->forward, SIGNAL(clicked()), this, SLOT(nextMonth()));
    m_hLayout->addItem(d->forward);

    m_layout->addItem(m_hLayout);

    m_layout->addItem(d->calendarTable);
    connect(d->calendarTable, SIGNAL(dateChanged(const QDate &)), this, SLOT(dateUpdated(const QDate &)));

    d->jumpToday = new Plasma::ToolButton(this);
    d->jumpToday->nativeWidget()->setIcon(KIcon("go-jump-today"));
    d->jumpToday->nativeWidget()->setMinimumWidth(25);
    connect(d->jumpToday, SIGNAL(clicked()), this, SLOT(goToToday()));
    m_layoutTools->addItem(d->jumpToday);
    m_layoutTools->addStretch();

    d->dateText = new Plasma::LineEdit(this);
    connect(d->dateText->nativeWidget(), SIGNAL(returnPressed()), this, SLOT(manualDateChange()));
    m_layoutTools->addItem(d->dateText);
    m_layoutTools->addStretch();

    d->weekSpinBox = new Plasma::SpinBox(this);
    d->weekSpinBox->setMinimum(1);
    d->weekSpinBox->setMaximum(calendar()->weeksInYear(date()));
    connect(d->weekSpinBox, SIGNAL(valueChanged(int)), this, SLOT(goToWeek(int)));
    m_layoutTools->addItem(d->weekSpinBox);

    m_layout->addItem(m_layoutTools);

    d->monthMenu = 0;

    dateUpdated(date());
}

void Calendar::manualDateChange()
{
    setDate(calendar()->readDate(((QLineEdit*)sender())->text()));
}

void Calendar::goToToday()
{
    setDate(QDate::currentDate());
}

bool Calendar::setCalendar(KCalendarSystem *calendar)
{
    return d->calendarTable->setCalendar(calendar);
}

const KCalendarSystem *Calendar::calendar () const
{
    return calendarTable()->calendar();
}

bool Calendar::setDate(const QDate &toDate)
{
    bool ret = d->calendarTable->setDate(toDate);

    //If set date failed force refresh of nav widgets to reset any user entry
    if (!ret) {
        refreshWidgets();
    }

    return ret;
}

const QDate& Calendar::date() const
{
    return calendarTable()->date();
}

void Calendar::dateUpdated(const QDate &newDate)
{
    // Ignore the date passed in, only ever show the date to match the CalendarTable
    Q_UNUSED(newDate);

    refreshWidgets();

    emit dateChanged(date());
}

// Update the nav widgets to show the current date in the CalendarTable
void Calendar::refreshWidgets()
{
    d->month->setText(calendar()->monthName(date()));
    d->month->setMinimumSize(static_cast<QToolButton*>(d->month->widget())->sizeHint());
    d->year->setText(QString::number(calendar()->year(date())));
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

void Calendar::monthsPopup()
{
    delete d->monthMenu;
    d->monthMenu = new QMenu();

    int year = calendar()->year(date());
    int monthsInYear = calendar()->monthsInYear(date());

    for (int i = 1; i <= monthsInYear; i++){
        QAction *tmpAction = new QAction(calendar()->monthName(i, year), d->monthMenu);
        tmpAction->setProperty("month", i);
        connect(tmpAction, SIGNAL(triggered()), this, SLOT(monthTriggered()));
        d->monthMenu->addAction(tmpAction);
    }

    d->monthMenu->popup(QCursor::pos());
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
    d->yearSpinBox->setValue(calendar()->year(date()));
    d->yearSpinBox->setGeometry(d->year->geometry());
    d->year->hide();
    d->yearSpinBox->show();
    d->yearSpinBox->nativeWidget()->setFocus(Qt::MouseFocusReason);
}

void Calendar::hideYearSpinBox()
{
    d->yearSpinBox->hide();

    int newYear = d->yearSpinBox->value();
    int currYear = calendar()->year(date());
    setDate(calendar()->addYears(date(), newYear - currYear));
    d->year->show();
}

CalendarTable *Calendar::calendarTable() const
{
    return d->calendarTable;
}

void Calendar::setDataEngine(Plasma::DataEngine *dataEngine)
{
    d->calendarTable->setDataEngine(dataEngine);
}

void Calendar::setRegion(const QString &region)
{
    d->calendarTable->setRegion(region);
}

QString Calendar::dateProperty(const QDate &date) const
{
    return d->calendarTable->dateProperty(date);
}

}

#include "calendar.moc"
