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

//Plasma
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/ToolButton>

namespace Plasma
{

class CalendarPrivate
{
    public:
        ToolButton *back;
        Plasma::Label *spacer0;
        Plasma::ToolButton *month;
        #ifdef COOL_SPINBOX
            SpinBox *year;
        #else
            Plasma::Label *year;
        #endif
        Plasma::Label *spacer1;
        Plasma::ToolButton *forward;
        Plasma::CalendarTable *calendarTable;
        Plasma::LineEdit *dateText;
        ToolButton *jumpToday;
        QMenu *monthMenu;
        QSpinBox *weekSpinBox;
};

//TODO
Calendar::Calendar(const QDate &, QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{

}

Calendar::Calendar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{
    QGraphicsLinearLayout *m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    QGraphicsLinearLayout *m_hLayout = new QGraphicsLinearLayout(m_layout);
    QGraphicsLinearLayout *m_layoutTools = new QGraphicsLinearLayout(m_layout);

    d->calendarTable = new Plasma::CalendarTable(this);
    d->calendarTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(d->calendarTable, SIGNAL(displayedMonthChanged(int, int)), this, SLOT(displayedMonthChanged(int, int)));

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

    #ifdef COOL_SPINBOX
        QGraphicsProxyWidget *yearProxy = new QGraphicsProxyWidget(this);
        d->year = new SpinBox();
        d->year->setRange(d->calendarTable->calendar()->year(d->calendarTable->calendar()->earliestValidDate()), d->calendarTable->calendar()->year(d->calendarTable->calendar()->latestValidDate()));
        d->year->setValue(d->calendarTable->calendar()->year(d->calendarTable->date()));
        yearProxy->setWidget(d->year);
        m_hLayout->addItem(yearProxy);
    #else
        d->year = new Plasma::Label;
        d->year->setText(QString::number(d->calendarTable->calendar()->year(d->calendarTable->date())));
        d->year->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_hLayout->addItem(d->year);
    #endif

    m_hLayout->addStretch();

    d->forward = new Plasma::ToolButton(this);
    d->forward->setText(">");
    d->forward->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->forward, SIGNAL(clicked()), this, SLOT(nextMonth()));
    m_hLayout->addItem(d->forward);

    m_layout->addItem(m_hLayout);

    m_layout->addItem(d->calendarTable);

    d->dateText = new Plasma::LineEdit(this);
    //d->dateText->nativeWidget()->setReadOnly(true);
    connect(d->calendarTable, SIGNAL(dateChanged(const QDate &)), this, SLOT(dateUpdated(const QDate &)));
    connect(d->dateText->nativeWidget(), SIGNAL(returnPressed()), this, SLOT(manualDateChange()));
    
    d->jumpToday = new Plasma::ToolButton(this);
    d->jumpToday->nativeWidget()->setIcon(KIcon("go-jump-today"));
    d->jumpToday->nativeWidget()->setMinimumWidth(25);
    connect(d->jumpToday, SIGNAL(clicked()), this, SLOT(goToToday()));

    d->weekSpinBox = new QSpinBox();
    QGraphicsProxyWidget *spinProxy = new QGraphicsProxyWidget(this);
    spinProxy->setWidget(d->weekSpinBox);
    d->weekSpinBox->setAttribute(Qt::WA_NoSystemBackground);
    d->weekSpinBox->setMinimum(1);
    d->weekSpinBox->setMaximum(d->calendarTable->calendar()->weeksInYear(d->calendarTable->date()));
    connect(d->weekSpinBox, SIGNAL(valueChanged(int)), this, SLOT(goToWeek(int)));

    m_layoutTools->addItem(d->jumpToday);
    m_layoutTools->addStretch();
    m_layoutTools->addItem(d->dateText);
    m_layoutTools->addStretch();
    m_layoutTools->addItem(spinProxy);
    m_layout->addItem(m_layoutTools);

    d->monthMenu = 0;

    dateUpdated(d->calendarTable->date());
}

Calendar::~Calendar()
{
   delete d->monthMenu;
   delete d;
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
    dateUpdated(date);
    return d->calendarTable->setDate(date);
}

const QDate& Calendar::date() const
{
    return d->calendarTable->date();
}

void Calendar::displayedMonthChanged(int calendarSystemYear, int calendarSystemMonth)
{
    d->month->setText(d->calendarTable->calendar()->monthName(calendarSystemMonth, calendarSystemYear));
    
    #ifdef COOL_SPINBOX
        d->year->setValue(calendarSystemYear);
    #else
        d->year->setText(QString::number(calendarSystemYear));
    #endif
}

void Calendar::dateUpdated(const QDate &date)
{
    QString formatted = KGlobal::locale()->formatDate( date,  KLocale::ShortDate );
    d->dateText->setText(formatted);
    d->weekSpinBox->setValue(d->calendarTable->calendar()->weekNumber(date));
}

void Calendar::prevMonth()
{
    QDate tmpDate = d->calendarTable->date();
    QDate newDate;

    int month = d->calendarTable->calendar()->month(tmpDate);
    int year = d->calendarTable->calendar()->year(tmpDate);

    if (month == 1){
        month = 12;
        year--;
    }else{
        month--;
    }

    if (d->calendarTable->calendar()->setYMD(newDate, year, month, d->calendarTable->calendar()->day(tmpDate))){
        setDate(newDate);
    }else if (d->calendarTable->calendar()->setYMD(newDate, year, month, 1)){
        setDate(newDate);
    }
}

void Calendar::nextMonth()
{
    QDate tmpDate = d->calendarTable->date();
    QDate newDate;

    int month = d->calendarTable->calendar()->month(tmpDate);
    int year = d->calendarTable->calendar()->year(tmpDate);

    if (month == 12){
        month = 1;
        year++;
    }else{
        month++;
    }

    if (d->calendarTable->calendar()->setYMD(newDate, year, month, d->calendarTable->calendar()->day(tmpDate))){
        setDate(newDate);
    }else if (d->calendarTable->calendar()->setYMD(newDate, year, month, 1)){
        setDate(newDate);
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

    QDate tmpDate = d->calendarTable->date();
    QDate newDate;

    int year = d->calendarTable->calendar()->year(tmpDate);

    if (d->calendarTable->calendar()->setYMD(newDate, year, month, d->calendarTable->calendar()->day(tmpDate))){
        setDate(newDate);
    }else if (d->calendarTable->calendar()->setYMD(newDate, year, month, 1)){
        setDate(newDate);
    }
}

void Calendar::goToWeek(int week)
{
    if (d->calendarTable->calendar()->weekNumber(d->calendarTable->date()) != week){
        QDate firstDayOfWeek;
        d->calendarTable->calendar()->setYMD(firstDayOfWeek, d->calendarTable->calendar()->year(d->calendarTable->date()), 1, 1);
        int weeksInYear = d->calendarTable->calendar()->weeksInYear(d->calendarTable->date());
        int year = 0;
        int i;

        for (i = 1; i < weeksInYear; i++){
            if (week == d->calendarTable->calendar()->weekNumber(firstDayOfWeek, &year))
                break;

            firstDayOfWeek= d->calendarTable->calendar()->addDays(firstDayOfWeek, d->calendarTable->calendar()->daysInWeek(firstDayOfWeek));
        }

        setDate(firstDayOfWeek);
    }
}

CalendarTable *Calendar::calendarTable() const
{
    return d->calendarTable;
}
}

#include "calendar.moc"
