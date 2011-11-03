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
#include <QtGui/QGraphicsGridLayout>
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
#include <KTextBrowser>
#include <KIntSpinBox>
#include <KConfigDialog>
#include <KConfigGroup>

//Plasma
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/Separator>
#include <Plasma/SpinBox>
#include <Plasma/TextBrowser>
#include <Plasma/ToolButton>
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
              eventsDisplay(0),
              jumpToday(0),
              monthMenu(0),
              weekSpinBox(0),
              separator(0)
        {
        }

        bool addDateDetailsToDisplay(QString &html, const QDate &date);

        ToolButton *back;
        Plasma::Label *spacer0;
        Plasma::ToolButton *month;
        Plasma::SpinBox *yearSpinBox;
        Plasma::ToolButton *year;
        Plasma::Label *spacer1;
        Plasma::ToolButton *forward;
        Plasma::CalendarTable *calendarTable;
        Plasma::LineEdit *dateText;
        Plasma::TextBrowser *eventsDisplay;
        ToolButton *jumpToday;
        QMenu *monthMenu;
        Plasma::SpinBox *weekSpinBox;
        Plasma::Separator *separator;
        QGraphicsLinearLayout *layout;
};

Calendar::Calendar(const QDate &date, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new CalendarPrivate())
{
    init(date);
}

Calendar::Calendar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{
    init();
}

Calendar::~Calendar()
{
   delete d->monthMenu;
   delete d;
}

void Calendar::init(const QDate &initialDate)
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    d->layout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    QGraphicsLinearLayout *calendarLayout = new QGraphicsLinearLayout(Qt::Vertical, d->layout);
    QGraphicsLinearLayout *hLayout = new QGraphicsLinearLayout(d->layout);
    QGraphicsLinearLayout *layoutTools = new QGraphicsLinearLayout(d->layout);

    d->calendarTable = new CalendarTable(this);
    d->calendarTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(d->calendarTable, SIGNAL(dateChanged(QDate)), this, SLOT(dateUpdated()));
    connect(d->calendarTable, SIGNAL(dateHovered(QDate)), this, SIGNAL(dateHovered(QDate)));
    connect(d->calendarTable, SIGNAL(dateSelected(QDate)), this, SLOT(displayEvents(QDate)));
    connect(d->calendarTable, SIGNAL(eventsChanged()), this, SLOT(displayEvents()));
    connect(this, SIGNAL(dateHovered(QDate)), this, SLOT(displayEvents(QDate)));

    d->back = new Plasma::ToolButton(this);
    d->back->setText("<");
    d->back->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->back, SIGNAL(clicked()), this, SLOT(prevMonth()));
    hLayout->addItem(d->back);

    hLayout->addStretch();

    d->month = new WheelyToolButton(this);
    d->month->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    d->monthMenu = new QMenu();
    d->month->nativeWidget()->setMenu(d->monthMenu);
    connect(d->month, SIGNAL(clicked()), this, SLOT(monthsPopup()));
    connect(d->month, SIGNAL(wheelUp()), this, SLOT(prevMonth()));
    connect(d->month, SIGNAL(wheelDown()), this, SLOT(nextMonth()));
    hLayout->addItem(d->month);

    d->year = new WheelyToolButton(this);
    d->year->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->year, SIGNAL(wheelUp()), this, SLOT(prevYear()));
    connect(d->year, SIGNAL(wheelDown()), this, SLOT(nextYear()));
    connect(d->year, SIGNAL(clicked()), this, SLOT(showYearSpinBox()));
    hLayout->addItem(d->year);

    d->yearSpinBox = new Plasma::SpinBox(this);
    d->yearSpinBox->setRange(calendar()->year(calendar()->earliestValidDate()), calendar()->year(calendar()->latestValidDate()));
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
    d->jumpToday->nativeWidget()->setToolTip(i18n("Select today"));
    d->jumpToday->nativeWidget()->setMinimumWidth(25);
    connect(d->jumpToday, SIGNAL(clicked()), this, SLOT(goToToday()));
    layoutTools->addItem(d->jumpToday);
    layoutTools->addStretch();

    d->dateText = new Plasma::LineEdit(this);
    connect(d->dateText->nativeWidget(), SIGNAL(returnPressed()), this, SLOT(manualDateChange()));
    layoutTools->addItem(d->dateText);
    layoutTools->addStretch();

    d->weekSpinBox = new Plasma::SpinBox(this);
    d->weekSpinBox->setMinimum(1);
    connect(d->weekSpinBox, SIGNAL(valueChanged(int)), this, SLOT(goToWeek(int)));
    layoutTools->addItem(d->weekSpinBox);

    calendarLayout->addItem(hLayout);
    calendarLayout->addItem(d->calendarTable);
    calendarLayout->addItem(layoutTools);
    d->layout->addItem(calendarLayout);

    setDate(initialDate);
    displayEvents();
}

void Calendar::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
    grabKeyboard();
}

void Calendar::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
    ungrabKeyboard();
}

void Calendar::keyPressEvent(QKeyEvent* event)
{
    switch(event->key()) {
	case Qt::Key_Right :
	    setDate(date().addDays(1));
	    break;
	case Qt::Key_Left :
	    setDate(date().addDays(-1));
	    break;
	case Qt::Key_Up :
	    setDate(date().addDays(-7));
	    break;
	case Qt::Key_Down :
	    setDate(date().addDays(7));
	    break;
	case Qt::Key_PageUp:
	    nextMonth();
	    break;
	case Qt::Key_PageDown:
	    prevMonth();
	    break;
	case Qt::Key_Home:
	    goToToday();
	    break;
	default:
	    break;
    }
}

CalendarTable *Calendar::calendarTable() const
{
    return d->calendarTable;
}

void Calendar::setCalendar(const QString &newCalendarType)
{
    calendarTable()->setCalendar(newCalendarType);
    refreshWidgets();
}

void Calendar::setCalendar(const KCalendarSystem *newCalendar)
{
    calendarTable()->setCalendar(newCalendar);
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
}

const QDate& Calendar::date() const
{
    return calendarTable()->date();
}

void Calendar::clearHolidaysRegions()
{
    calendarTable()->clearHolidaysRegions();
}

void Calendar::addHolidaysRegion(const QString &region, bool daysOff)
{
    calendarTable()->addHolidaysRegion(region, daysOff);
}

QStringList Calendar::holidaysRegions() const
{
    return calendarTable()->holidaysRegions();
}

bool Calendar::isDisplayingDateDetails() const
{
    return calendarTable()->displayHolidays() || calendarTable()->displayEvents();
}

bool Calendar::dateHasDetails(const QDate &date) const
{
    return calendarTable()->dateHasDetails(date);
}

QStringList Calendar::dateDetails(const QDate &date) const
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
    if (isDisplayingDateDetails()) {
        setPreferredSize(440, 250);
    } else {
        setPreferredSize(220, 250);
    }
}

void Calendar::writeConfiguration(KConfigGroup cg)
{
    calendarTable()->writeConfiguration(cg);
}

void Calendar::createConfigurationInterface(KConfigDialog *parent)
{
    calendarTable()->createConfigurationInterface(parent);
}

void Calendar::configAccepted(KConfigGroup cg)
{
    calendarTable()->configAccepted(cg);
    if (isDisplayingDateDetails()) {
        setPreferredSize(440, 250);
    } else {
        setPreferredSize(220, 250);
    }
    displayEvents();
}

void Calendar::manualDateChange()
{
    setDate(calendar()->readDate(((QLineEdit*)sender())->text()));
}

void Calendar::goToToday()
{
    setDate(QDate::currentDate());
}

void Calendar::dateUpdated()
{
    // Ignore the date passed in, only ever show the date to match the CalendarTable
    refreshWidgets();
    emit dateChanged(date());
    displayEvents();
}

void Calendar::displayEvents(const QDate &date)
{
    if (!isDisplayingDateDetails()) {
        if (d->eventsDisplay) {
            kDebug() << "deleting events display!";
            delete d->eventsDisplay;
            d->eventsDisplay = 0;
            delete d->separator;
            d->separator = 0;
        }
        return;
    } else if (!d->eventsDisplay) {
        d->separator = new Plasma::Separator(this);
        d->separator->setOrientation(Qt::Vertical);
        d->layout->addItem(d->separator);

        d->eventsDisplay = new Plasma::TextBrowser(this);
        d->layout->addItem(d->eventsDisplay);
    }

    QString html;

    if (d->addDateDetailsToDisplay(html, date) < 1) {
        QDate dt = calendarTable()->date();
        QDate end = calendarTable()->endDate();

        if (dt.isValid() && end.isValid()) {
            while (dt <= end) {
                d->addDateDetailsToDisplay(html, dt);
                dt = dt.addDays(1);
            }
        }
    }

    d->eventsDisplay->setText(html);
}

bool CalendarPrivate::addDateDetailsToDisplay(QString &html, const QDate &date)
{
    if (!calendarTable->dateHasDetails(date)) {
        return false;
    }

    html += "<b>" + calendarTable->calendar()->formatDate(date, KLocale::LongDate) + "</b>";
    html += "<ul style='-qt-list-indent: 0;'>";

    const QStringList details = calendarTable->dateDetails(date);
    foreach (const QString &detail, details) {
        html+= "<li style='margin-left: 2em;'>" + detail + "</li>";
    }

    html += "</ul>";
    return true;
}

// Update the nav widgets to show the current date in the CalendarTable
void Calendar::refreshWidgets()
{
    d->month->setText(calendar()->monthName(calendar()->month(date()), calendar()->year(date())));
    d->month->setMinimumSize(static_cast<QToolButton*>(d->month->widget())->sizeHint());
    d->year->setText(calendar()->formatDate(date(), KLocale::Year, KLocale::LongNumber));
    d->dateText->setText(calendar()->formatDate(date(),  KLocale::ShortDate));

    // Block the signals to prevent changing the date again
    d->yearSpinBox->blockSignals(true);
    d->yearSpinBox->setRange(calendar()->year(calendar()->earliestValidDate()),
                             calendar()->year(calendar()->latestValidDate()));
    d->yearSpinBox->setValue(calendar()->year(date()));
    d->yearSpinBox->blockSignals(false);

    // Block the signals to prevent changing the date again
    d->weekSpinBox->blockSignals(true);
    d->weekSpinBox->setMaximum(calendar()->weeksInYear(date()));
    d->weekSpinBox->setValue(calendar()->week(date(), KLocale::IsoWeekNumber));
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
    const int year = calendar()->year(date());
    const int monthsInYear = calendar()->monthsInYear(date());

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
    int currWeek = calendar()->week(date(), KLocale::IsoWeekNumber);
    int daysInWeek = calendar()->daysInWeek(date());

    setDate(calendar()->addDays(date(), (newWeek - currWeek) * daysInWeek));
}

void Calendar::showYearSpinBox()
{
    QGraphicsLinearLayout *hLayout = (QGraphicsLinearLayout*)d->year->parentLayoutItem();
    if (!hLayout) {
        // already hidden!
        return;
    }

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
    if (!hLayout) {
        // already hidden!
        return;
    }

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
