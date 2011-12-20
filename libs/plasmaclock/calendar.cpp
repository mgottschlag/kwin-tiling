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
#include <QtCore/QTimer>
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
        CalendarPrivate(Calendar *calendar)
            : q(calendar),
              back(0),
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

        void init(const QDate &date = QDate());
        void refreshWidgets();
        bool addDateDetailsToDisplay(QString &html, const QDate &date);
        void popupMonthsMenu();
        void displayEvents(const QDate &date = QDate());

        Calendar *q;
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
      d(new CalendarPrivate(this))
{
    d->init(date);
}

Calendar::Calendar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new CalendarPrivate(this))
{
    d->init();
}

Calendar::~Calendar()
{
   delete d->monthMenu;
   delete d;
}

void CalendarPrivate::init(const QDate &initialDate)
{
    q->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    layout = new QGraphicsLinearLayout(Qt::Horizontal, q);
    QGraphicsLinearLayout *calendarLayout = new QGraphicsLinearLayout(Qt::Vertical, layout);
    QGraphicsLinearLayout *hLayout = new QGraphicsLinearLayout(layout);
    QGraphicsLinearLayout *layoutTools = new QGraphicsLinearLayout(layout);

    calendarTable = new CalendarTable(q);
    calendarTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QObject::connect(calendarTable, SIGNAL(dateChanged(QDate)), q, SLOT(dateUpdated()));
    QObject::connect(calendarTable, SIGNAL(dateHovered(QDate)), q, SIGNAL(dateHovered(QDate)));
    QObject::connect(calendarTable, SIGNAL(dateSelected(QDate)), q, SLOT(displayEvents(QDate)));
    QObject::connect(calendarTable, SIGNAL(eventsChanged()), q, SLOT(displayEvents()));
    QObject::connect(q, SIGNAL(dateHovered(QDate)), q, SLOT(displayEvents(QDate)));

    back = new Plasma::ToolButton(q);
    back->setText("<");
    back->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(back, SIGNAL(clicked()), q, SLOT(prevMonth()));
    hLayout->addItem(back);

    hLayout->addStretch();

    month = new WheelyToolButton(q);
    month->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    monthMenu = new QMenu();
    QObject::connect(month, SIGNAL(clicked()), q, SLOT(popupMonthsMenu()));
    QObject::connect(month, SIGNAL(pressed()), q, SLOT(popupMonthsMenu()));
    QObject::connect(month, SIGNAL(wheelUp()), q, SLOT(prevMonth()));
    QObject::connect(month, SIGNAL(wheelDown()), q, SLOT(nextMonth()));
    hLayout->addItem(month);

    year = new WheelyToolButton(q);
    year->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(year, SIGNAL(wheelUp()), q, SLOT(prevYear()));
    QObject::connect(year, SIGNAL(wheelDown()), q, SLOT(nextYear()));
    QObject::connect(year, SIGNAL(clicked()), q, SLOT(showYearSpinBox()));
    hLayout->addItem(year);

    yearSpinBox = new Plasma::SpinBox(q);
    yearSpinBox->setRange(calendarTable->calendar()->year(calendarTable->calendar()->earliestValidDate()),
                          calendarTable->calendar()->year(calendarTable->calendar()->latestValidDate()));
    yearSpinBox->hide();
    QObject::connect(yearSpinBox->nativeWidget(), SIGNAL(editingFinished()), q, SLOT(hideYearSpinBox()));

    hLayout->addStretch();

    forward = new Plasma::ToolButton(q);
    forward->setText(">");
    forward->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(forward, SIGNAL(clicked()), q, SLOT(nextMonth()));
    hLayout->addItem(forward);

    jumpToday = new Plasma::ToolButton(q);
    jumpToday->nativeWidget()->setIcon(KIcon("go-jump-today"));
    jumpToday->nativeWidget()->setToolTip(i18n("Select today"));
    jumpToday->nativeWidget()->setMinimumWidth(25);
    QObject::connect(jumpToday, SIGNAL(clicked()), q, SLOT(goToToday()));
    layoutTools->addItem(jumpToday);
    layoutTools->addStretch();

    dateText = new Plasma::LineEdit(q);
    QObject::connect(dateText->nativeWidget(), SIGNAL(returnPressed()), q, SLOT(manualDateChange()));
    layoutTools->addItem(dateText);
    layoutTools->addStretch();

    weekSpinBox = new Plasma::SpinBox(q);
    weekSpinBox->setMinimum(1);
    QObject::connect(weekSpinBox, SIGNAL(valueChanged(int)), q, SLOT(goToWeek(int)));
    layoutTools->addItem(weekSpinBox);

    calendarLayout->addItem(hLayout);
    calendarLayout->addItem(calendarTable);
    calendarLayout->addItem(layoutTools);
    layout->addItem(calendarLayout);

    q->setDate(initialDate);
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
    d->refreshWidgets();
}

void Calendar::setCalendar(const KCalendarSystem *newCalendar)
{
    calendarTable()->setCalendar(newCalendar);
    d->refreshWidgets();
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
        d->refreshWidgets();
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
    d->displayEvents();
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
    d->refreshWidgets();
    emit dateChanged(date());
    d->displayEvents();
}

void CalendarPrivate::displayEvents(const QDate &date)
{
    if (!q->isDisplayingDateDetails()) {
        if (eventsDisplay) {
            kDebug() << "deleting events display!";
            delete eventsDisplay;
            eventsDisplay = 0;
            delete separator;
            separator = 0;
        }
        return;
    } else if (!eventsDisplay) {
        separator = new Plasma::Separator(q);
        separator->setOrientation(Qt::Vertical);
        layout->addItem(separator);

        eventsDisplay = new Plasma::TextBrowser(q);
        layout->addItem(eventsDisplay);
    }

    QString html;

    if (addDateDetailsToDisplay(html, date) < 1) {
        QDate dt = calendarTable->date();
        QDate end = calendarTable->endDate();

        if (dt.isValid() && end.isValid()) {
            while (dt <= end) {
                addDateDetailsToDisplay(html, dt);
                dt = dt.addDays(1);
            }
        }
    }

    eventsDisplay->setText(html);
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
void CalendarPrivate::refreshWidgets()
{
    const KCalendarSystem *calendar = calendarTable->calendar();
    const QDate date = calendarTable->date();
    month->setText(calendar->monthName(calendar->month(date), calendar->year(date)));
    month->setMinimumSize(static_cast<QToolButton*>(month->widget())->sizeHint());
    year->setText(calendar->formatDate(date, KLocale::Year, KLocale::LongNumber));
    dateText->setText(calendar->formatDate(date,  KLocale::ShortDate));

    // Block the signals to prevent changing the date again
    yearSpinBox->blockSignals(true);
    yearSpinBox->setRange(calendar->year(calendar->earliestValidDate()),
                          calendar->year(calendar->latestValidDate()));
    yearSpinBox->setValue(calendar->year(date));
    yearSpinBox->blockSignals(false);

    // Block the signals to prevent changing the date again
    weekSpinBox->blockSignals(true);
    weekSpinBox->setMaximum(calendar->weeksInYear(date));
    weekSpinBox->setValue(calendar->week(date, KLocale::IsoWeekNumber));
    weekSpinBox->blockSignals(false);
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

void CalendarPrivate::popupMonthsMenu()
{
    monthMenu->clear();
    const KCalendarSystem *calendar = calendarTable->calendar();
    const QDate date = calendarTable->date();
    const int year = calendar->year(date);
    const int monthsInYear = calendar->monthsInYear(date);

    for (int i = 1; i <= monthsInYear; i++){
        QAction *tmpAction = new QAction(calendar->monthName(i, year), monthMenu);
        tmpAction->setProperty("month", i);
        QObject::connect(tmpAction, SIGNAL(triggered()), q, SLOT(monthTriggered()));
        monthMenu->addAction(tmpAction);
    }

    QGraphicsView *view = Plasma::viewFor(month);
    if (view) {
        monthMenu->adjustSize();
        const int x = month->sceneBoundingRect().center().x() - monthMenu->width() / 2;
        QPoint pos(x, month->sceneBoundingRect().bottom());
        pos = view->mapToGlobal(view->mapFromScene(pos));
        QRect r = Kephal::ScreenUtils::screenGeometry(Kephal::ScreenUtils::screenId(view->geometry().center()));
        if (pos.y() + monthMenu->height() > r.bottom()) {
            pos = QPoint(x, month->sceneBoundingRect().top() - monthMenu->height());
            pos = view->mapToGlobal(view->mapFromScene(pos));
        }
        monthMenu->popup(pos);
    } else {
        monthMenu->popup(QCursor::pos());
    }

    month->nativeWidget()->setDown(false);
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
