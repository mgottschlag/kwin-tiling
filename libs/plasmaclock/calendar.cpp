/*
 *   Copyright 2008 Davide Bettio <davide.bettio@kdemail.net>
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

        Plasma::DataEngine *dataEngine;
        QString queryString;
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
    d->dataEngine = 0;
    d->queryString = "";

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
    d->month->setText(d->calendarTable->calendar()->monthName(d->calendarTable->calendar()->month(d->calendarTable->date()), d->calendarTable->calendar()->year(d->calendarTable->date())));
    d->month->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    connect(d->month, SIGNAL(clicked()), this, SLOT(monthsPopup()));
    m_hLayout->addItem(d->month);

    d->year = new Plasma::ToolButton(this);
    d->year->setText(QString::number(d->calendarTable->calendar()->year(d->calendarTable->date())));
    d->year->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->year, SIGNAL(clicked()), this, SLOT(showYearSpinBox()));
    m_hLayout->addItem(d->year);

    d->yearSpinBox = new Plasma::SpinBox(this);
    d->yearSpinBox->setRange(d->calendarTable->calendar()->year(d->calendarTable->calendar()->earliestValidDate()), d->calendarTable->calendar()->year(d->calendarTable->calendar()->latestValidDate()));
    d->yearSpinBox->setValue(d->calendarTable->calendar()->year(d->calendarTable->date()));
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

    d->jumpToday = new Plasma::ToolButton(this);
    d->jumpToday->nativeWidget()->setIcon(KIcon("go-jump-today"));
    d->jumpToday->nativeWidget()->setMinimumWidth(25);
    connect(d->jumpToday, SIGNAL(clicked()), this, SLOT(goToToday()));
    m_layoutTools->addItem(d->jumpToday);
    m_layoutTools->addStretch();

    d->dateText = new Plasma::LineEdit(this);
    connect(d->calendarTable, SIGNAL(dateChanged(const QDate &)), this, SLOT(dateUpdated(const QDate &)));
    connect(d->calendarTable, SIGNAL(displayedMonthChanged(int, int)), this, SLOT(goToMonth(int, int)));
    connect(d->dateText->nativeWidget(), SIGNAL(returnPressed()), this, SLOT(manualDateChange()));
    m_layoutTools->addItem(d->dateText);
    m_layoutTools->addStretch();

    d->weekSpinBox = new Plasma::SpinBox(this);
    d->weekSpinBox->setMinimum(1);
    d->weekSpinBox->setMaximum(d->calendarTable->calendar()->weeksInYear(d->calendarTable->date()));
    connect(d->weekSpinBox, SIGNAL(valueChanged(int)), this, SLOT(goToWeek(int)));
    m_layoutTools->addItem(d->weekSpinBox);

    m_layout->addItem(m_layoutTools);

    d->monthMenu = 0;

    dateUpdated(d->calendarTable->date());
}

void Calendar::manualDateChange()
{
    QDate date = KGlobal::locale()->readDate(((QLineEdit*)sender())->text());
    if(date.isValid()) {
        setDate(date);
    }
}

void Calendar::goToToday()
{
    setDate(QDate::currentDate());
}

bool Calendar::setCalendar(KCalendarSystem *calendar)
{
    return d->calendarTable->setCalendar(calendar);
}

bool Calendar::setDate(const QDate &date)
{
    bool r = d->calendarTable->setDate(date);
    dateUpdated(date);
    return r;
}

const QDate& Calendar::date() const
{
    return d->calendarTable->date();
}

void Calendar::dateUpdated(const QDate &date)
{
    QString formatted = KGlobal::locale()->formatDate( date,  KLocale::ShortDate );
    d->month->setText(d->calendarTable->calendar()->monthName(date));
    d->year->setText(QString::number(d->calendarTable->calendar()->year(date)));
    d->dateText->setText(formatted);
    d->weekSpinBox->setValue(d->calendarTable->calendar()->weekNumber(date));

    emit dateChanged(date);
}

void Calendar::prevMonth()
{
    const KCalendarSystem *calendar = d->calendarTable->calendar();

    QDate tmpDate = d->calendarTable->date();
    QDate newDate;

    int month = calendar->month(tmpDate);
    int year = calendar->year(tmpDate);

    if (month == 1){
        month = 12;
        year--;
    }else{
        month--;
    }

    if (calendar->setYMD(newDate, year, month, calendar->day(tmpDate))){
        setDate(newDate);
    }else if (calendar->setYMD(newDate, year, month, 1)){
        setDate(newDate);
    }

    if (d->dataEngine){
        for (int i = -10; i < 40; i++){
            QDate tmpDate = newDate.addDays(i);
            QString tmpStr = d->queryString + tmpDate.toString(Qt::ISODate);
            if (d->dataEngine->query(tmpStr).value(tmpStr).toBool()){
                setDateProperty(tmpDate);
            }
        }
    }
}

void Calendar::nextMonth()
{
    const KCalendarSystem *calendar = d->calendarTable->calendar();

    QDate tmpDate = d->calendarTable->date();
    QDate newDate;

    int month = calendar->month(tmpDate);
    int year = calendar->year(tmpDate);

    if (month == 12){
        month = 1;
        year++;
    }else{
        month++;
    }

    if (calendar->setYMD(newDate, year, month, calendar->day(tmpDate))){
        setDate(newDate);
    }else if (calendar->setYMD(newDate, year, month, 1)){
        setDate(newDate);
    }

    if (d->dataEngine){
        for (int i = -10; i < 40; i++){
            QDate tmpDate = newDate.addDays(i);
            QString tmpStr = d->queryString + tmpDate.toString(Qt::ISODate);
            if (d->dataEngine->query(tmpStr).value(tmpStr).toBool()){
                setDateProperty(tmpDate);
            }
        }
    }
}

void Calendar::monthsPopup()
{
    delete d->monthMenu;
    d->monthMenu = new QMenu();

    int year = d->calendarTable->calendar()->year(d->calendarTable->date());

    for (int i = 1; i <= 12; i++){
        QAction *tmpAction = new QAction(d->calendarTable->calendar()->monthName(i, year), d->monthMenu);
        tmpAction->setProperty("month", i);
        connect(tmpAction, SIGNAL(triggered()), this, SLOT(monthTriggered()));
        d->monthMenu->addAction(tmpAction);
    }

    d->monthMenu->popup(QCursor::pos());
}

void Calendar::monthTriggered()
{
    QAction *action = dynamic_cast<QAction*> (sender());
    if (!action || action->property("month").type() != QVariant::Int) return;
    int month = action->property("month").toInt();

    const KCalendarSystem *calendar = d->calendarTable->calendar();
    QDate tmpDate = d->calendarTable->date();
    QDate newDate;

    int year = calendar->year(tmpDate);

    if (calendar->setYMD(newDate, year, month, calendar->day(tmpDate))){
        setDate(newDate);
    }else if (calendar->setYMD(newDate, year, month, 1)){
        setDate(newDate);
    }
}

void Calendar::goToWeek(int week)
{
    const KCalendarSystem *calendar = d->calendarTable->calendar();

    if (calendar->weekNumber(d->calendarTable->date()) != week){
        QDate firstDayOfWeek;
        calendar->setYMD(firstDayOfWeek, calendar->year(d->calendarTable->date()), 1, 1);
        int weeksInYear = calendar->weeksInYear(d->calendarTable->date());

        for (int i = 1; i < weeksInYear; i++){
            if (week == calendar->weekNumber(firstDayOfWeek)) //TODO: Check year
                break;

            firstDayOfWeek= calendar->addDays(firstDayOfWeek, calendar->daysInWeek(firstDayOfWeek));
        }

        setDate(firstDayOfWeek);
    }
}

void Calendar::goToMonth(int year, int month)
{
    setDate(QDate(year, month, 1));
}

void Calendar::showYearSpinBox()
{
    d->yearSpinBox->setValue(d->calendarTable->calendar()->year(d->calendarTable->date()));
    d->yearSpinBox->setGeometry(d->year->geometry());
    d->year->hide();
    d->yearSpinBox->show();
    d->yearSpinBox->nativeWidget()->setFocus(Qt::MouseFocusReason);
}

void Calendar::hideYearSpinBox()
{
    d->yearSpinBox->hide();

    const KCalendarSystem *calendar = d->calendarTable->calendar();
    QDate newDate;
    if (calendar->setYMD(newDate, d->yearSpinBox->value(), calendar->month(d->calendarTable->date()), calendar->day(d->calendarTable->date()))){
        setDate(newDate);
    }

    d->year->show();
}

CalendarTable *Calendar::calendarTable() const
{
    return d->calendarTable;
}


//HACK
void Calendar::setDateProperty(QDate date)
{
    d->calendarTable->setDateProperty(date);
}

void Calendar::setDataEngine(Plasma::DataEngine *dataEngine)
{
    d->dataEngine = dataEngine;
}

void Calendar::setQueryString(QString queryString)
{
    d->queryString = queryString;
}

}

#include "calendar.moc"
