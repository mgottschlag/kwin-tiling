/***************************************************************************
 *   Copyright (C) 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>      *
 *   Copyright (C) 2007-2008 by Sebastian Kuegler <sebas@kde.org>          *
 *   Copyright (C) 2008-2009 by Davide Bettio <davide.bettio@kdemail.net>  *
 *   Copyright (C) 2009 by John Layt <john@layt.net>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "clockapplet.h"

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QSpinBox>
#include <QtGui/QClipboard>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsView>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingCall>
#include <QtCore/QDate>
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnectionInterface>

#include <KColorScheme>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KDatePicker>
#include <KMenu>
#include <KDebug>
#include <KDialog>
#include <KGlobalSettings>
#include <KRun>
#include <KLocale>
#include <KPassivePopup>
#include <KService>
#include <KServiceTypeTrader>
#include <KTimeZone>
#include <KToolInvocation>
#include <KMessageBox>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/DataEngine>
#include <Plasma/Dialog>
#include <Plasma/Extender>
#include <Plasma/ExtenderItem>
#include <Plasma/Theme>
#include <Plasma/Label>

#include "calendar.h"

#include "ui_timezonesConfig.h"
#include "ui_generalConfig.h"

class ClockApplet::Private
{
public:
    Private(ClockApplet *clockapplet)
        : q(clockapplet),
          timezone(ClockApplet::localTimezoneUntranslated()),
          clipboardMenu(0),
          adjustSystemTimeAction(0),
          label(0),
          calendarWidget(0),
          forceTzDisplay(false)
    {}

    ClockApplet *q;
    Ui::timezonesConfig ui;
    Ui::generalConfig generalUi;
    QString timezone;
    QString defaultTimezone;
    QPoint clicked;
    QStringList selectedTimezones;
    KMenu *clipboardMenu;
    QAction *adjustSystemTimeAction;
    QString prettyTimezone;
    Plasma::Label *label;
    Plasma::Calendar *calendarWidget;
    int announceInterval;
    int prevHour;
    int prevMinute;
    bool forceTzDisplay : 1;

    void addTzToTipText(QString &subText, const QString& tz)
    {
        Plasma::DataEngine::Data data = q->dataEngine("time")->query(tz);

        if (tz == "UTC")  {
            subText.append("<br><b>UTC</b>&nbsp;");
        } else {
            subText.append("<br><b>")
                   .append(data["Timezone City"].toString().replace('_', "&nbsp;"))
                   .append("</b> ");
        }

        subText.append(KGlobal::locale()->formatTime(data["Time"].toTime(), false).replace(' ', "&nbsp;"))
               .append(",&nbsp;")
               .append(q->calendar()->formatDate(data["Date"].toDate()).replace(' ', "&nbsp;"));
    }

    void createCalendarExtender()
    {
        if (!q->extender()->hasItem("calendar")) {
            Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(q->extender());
            eItem->setName("calendar");
            q->initExtenderItem(eItem);
        }
    }

    void createToday()
    {
        QString tmpStr = "isHoliday:" + calendarWidget->holidaysRegion() + ':' + QDate::currentDate().toString(Qt::ISODate);
        bool isHoliday = q->dataEngine("calendar")->query(tmpStr).value(tmpStr).toBool();

        Plasma::ExtenderItem *todayExtender = q->extender()->item("today");

        if (!todayExtender && isHoliday) {
            Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(q->extender());
            eItem->setName("today");
            q->initExtenderItem(eItem);
        } else if (todayExtender && !isHoliday) {
            todayExtender->destroy();
        }
    }

    void setPrettyTimezone()
    {
        QString timezonetranslated = i18n(timezone.toUtf8().data());
        if (timezone == "UTC")  {
            prettyTimezone = timezonetranslated;
        } else if (!q->isLocalTimezone()) {
            QStringList tzParts = timezonetranslated.split('/', QString::SkipEmptyParts);
            if (tzParts.count() == 1) {
                prettyTimezone = timezonetranslated;
            } else if (tzParts.count() > 0) {
                prettyTimezone = tzParts.last();
            } else {
                prettyTimezone = timezonetranslated;
            }
        } else {
            prettyTimezone = localTimezone();
        }

        prettyTimezone = prettyTimezone.replace('_', ' ');
    }
};

ClockApplet::ClockApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      d(new Private(this))
{
    setPopupIcon(QIcon());
    setPassivePopup(true);
}

ClockApplet::~ClockApplet()
{
    delete d->clipboardMenu;
    delete d;
}

void ClockApplet::speakTime(const QTime &time)
{
    if (!d->announceInterval) {
        return;
    }

    if (time.minute() != d->prevMinute && (time.minute() % d->announceInterval) == 0) {
        d->prevHour = time.hour();
        d->prevMinute = time.minute();

        // If KTTSD not running, start it.
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kttsd")) {
            QString error;
            if (KToolInvocation::startServiceByDesktopName("kttsd", QStringList(), &error)) {
                KPassivePopup::message(i18n("Starting KTTSD Failed"), error, static_cast<QWidget *>(0));
                return;
            }
        }

        QDBusInterface ktts("org.kde.kttsd", "/KSpeech", "org.kde.KSpeech");
        ktts.asyncCall("setApplicationName", "plasmaclock");
        QString text;
        if (time.minute() == 0) {
            if (KGlobal::locale()->use12Clock()) {
                if (time.hour() < 12) {
                    text = i18ncp("Text sent to the text to speech service "
                                     "when minutes==0 and it is AM",
                                 "It is 1 o clock a m",
                                 "It is %1 o clock a m",
                                 time.hour());
                } else {
                    text = i18ncp("Text sent to the text to speech service "
                                     "when minutes==0 and it is PM",
                                 "It is 1 o clock p m",
                                 "It is %1 o clock p m",
                                 time.hour()-12);
                }
            } else {
                text = i18ncp("Text sent to the text to speech service "
                                 "when minutes==0 and it is the 24 hour clock",
                                 "It is 1 o clock",
                                 "It is %1 o clock",
                                 time.hour());
            }
        } else {
            if (KGlobal::locale()->use12Clock()) {
                if (time.hour() < 12) {
                    text = i18nc("Text sent to the text to speech service for AM",
                                "It is %1:%2 a m",
                                time.hour(),
                                time.minute());
                } else {
                    text = i18nc("Text sent to the text to speech service for PM",
                                "It is %1:%2 p m",
                                time.hour()-12,
                                time.minute());
                }
            } else {
                text = i18nc("Text sent to the text to speech service for the 24 hour clock",
                                "It is %1:%2",
                                time.hour(),
                                time.minute());
            }
        }
        ktts.asyncCall("say", text, 0);
    }
}

void ClockApplet::toolTipAboutToShow()
{
    updateTipContent();
}

void ClockApplet::toolTipHidden()
{
    Plasma::ToolTipManager::self()->clearContent(this);
}

void ClockApplet::updateTipContent()
{
    Plasma::ToolTipContent tipData;

    // the main text contains the current timezone's time and date
    Plasma::DataEngine::Data data = dataEngine("time")->query(currentTimezone());
    QString mainText = d->prettyTimezone + ' ';
    mainText += KGlobal::locale()->formatTime(data["Time"].toTime(), false) + "<br>";
    QDate tipDate = data["Date"].toDate();
    mainText += calendar()->formatDate(tipDate);
    tipData.setMainText(mainText);

    QString subText;
    if (!isLocalTimezone()) {
        d->addTzToTipText(subText, localTimezone());
    }

    foreach (const QString &tz, getSelectedTimezones()) {
        if (tz == currentTimezone()) {
            continue;
        }

        d->addTzToTipText(subText, tz);
    }

    if (d->calendarWidget->dateHasDetails(tipDate)) {
        QString property = d->calendarWidget->dateDetails(tipDate);
        QString countryString = KGlobal::locale()->countryCodeToName(d->calendarWidget->holidaysRegion());
        if (countryString.isEmpty()) {
            subText.append("<br>").append(property);
        } else {
            subText.append("<br><b>").append(countryString).append("</b> ").append(property);
        }
    }

    tipData.setSubText(subText);

    // query for custom content
    Plasma::ToolTipContent customContent = toolTipContent();
    if (customContent.image().isNull()) {
        tipData.setImage(KIcon(icon()).pixmap(IconSize(KIconLoader::Desktop)));
    } else {
        tipData.setImage(customContent.image());
    }

    if (!customContent.mainText().isEmpty()) {
        // add their main text
        tipData.setMainText(customContent.mainText() + "<br>" + tipData.mainText());
    }

    if (!customContent.subText().isEmpty()) {
        // add their sub text
        tipData.setSubText(customContent.subText() + "<br>" + tipData.subText());
    }

    tipData.setAutohide(false);
    Plasma::ToolTipManager::self()->setContent(this, tipData);
}

void ClockApplet::updateClockApplet()
{
    bool updateSelectedDate = (d->calendarWidget->currentDate() == d->calendarWidget->date());

    Plasma::DataEngine::Data data = dataEngine("time")->query(currentTimezone());
    d->calendarWidget->setCurrentDate(data["Date"].toDate());
    
    if (updateSelectedDate){
        d->calendarWidget->setDate(d->calendarWidget->currentDate());
    }
}

Plasma::ToolTipContent ClockApplet::toolTipContent()
{
    return Plasma::ToolTipContent();
}

void ClockApplet::createConfigurationInterface(KConfigDialog *parent)
{
    createClockConfigurationInterface(parent);

    QWidget *generalWidget = new QWidget();
    d->generalUi.setupUi(generalWidget);
    parent->addPage(generalWidget, i18nc("General configuration page", "General"), Applet::icon());
    d->generalUi.interval->setValue(d->announceInterval);

    d->calendarWidget->createConfigurationInterface(parent);

    QWidget *widget = new QWidget();
    d->ui.setupUi(widget);
    d->ui.searchLine->addTreeWidget(d->ui.timeZones);

    parent->addPage(widget, i18n("Time Zones"), "preferences-desktop-locale");

    foreach (const QString &tz, d->selectedTimezones) {
        d->ui.timeZones->setSelected(tz, true);
    }

    updateClockDefaultsTo();
    int defaultSelection = d->ui.clockDefaultsTo->findData(d->defaultTimezone);
    if (defaultSelection < 0) {
        defaultSelection = 0; //if it's something unexpected default to local
        kDebug() << d->defaultTimezone << "not in list!?";
    }
    d->ui.clockDefaultsTo->setCurrentIndex(defaultSelection);

    connect(d->ui.timeZones, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(updateClockDefaultsTo()));
}

void ClockApplet::createClockConfigurationInterface(KConfigDialog *parent)
{
    Q_UNUSED(parent)
}

void ClockApplet::clockConfigChanged()
{

}

void ClockApplet::configChanged()
{
    if (isUserConfiguring()) {
        configAccepted();
    }

    KConfigGroup cg = config();
    d->selectedTimezones = cg.readEntry("timeZones", QStringList());
    d->timezone = cg.readEntry("timezone", d->timezone);
    d->defaultTimezone = cg.readEntry("defaultTimezone", d->timezone);
    d->forceTzDisplay = d->timezone != d->defaultTimezone;
    d->setPrettyTimezone();
    d->announceInterval = cg.readEntry("announceInterval", 0);

    clockConfigChanged();

    if (isUserConfiguring()) {
        constraintsEvent(Plasma::SizeConstraint);
        update();
    } else {
        // d->calendarWidget->configAccepted(cg); is called in configAccepted(), 
        // as is setCurrentTimezone
        // so we only need to do this in the case where the user hasn't been
        // configuring things
        d->calendarWidget->applyConfiguration(cg);
        Plasma::DataEngine::Data data = dataEngine("time")->query(d->timezone);
        d->calendarWidget->setDate(data["Date"].toDate());
    }
}

void ClockApplet::clockConfigAccepted()
{

}

void ClockApplet::configAccepted()
{
    KConfigGroup cg = config();

    cg.writeEntry("timeZones", d->ui.timeZones->selection());

    if (d->ui.clockDefaultsTo->currentIndex() == 0) {
        //The first position in ui.clockDefaultsTo is "Local"
        d->defaultTimezone = localTimezoneUntranslated();
    } else {
        d->defaultTimezone = d->ui.clockDefaultsTo->itemData(d->ui.clockDefaultsTo->currentIndex()).toString();
    }

    cg.writeEntry("defaultTimezone", d->defaultTimezone);
    QString cur = currentTimezone();
    setCurrentTimezone(d->defaultTimezone);
    changeEngineTimezone(cur, d->defaultTimezone);

    d->calendarWidget->configAccepted(cg);

    cg.writeEntry("announceInterval", d->generalUi.interval->value());

    clockConfigAccepted();

    emit configNeedsSaving();
}

void ClockApplet::updateClockDefaultsTo()
{
    QString oldSelection = d->ui.clockDefaultsTo->currentText();
    d->ui.clockDefaultsTo->clear();
    d->ui.clockDefaultsTo->addItem(localTimezone(), localTimezone());
    foreach(const QString &tz, d->ui.timeZones->selection()) {
        d->ui.clockDefaultsTo->addItem(KTimeZoneWidget::displayName(KTimeZone(tz)), tz);
    }
    int newPosition = d->ui.clockDefaultsTo->findText(oldSelection);
    if (newPosition >= 0) {
        d->ui.clockDefaultsTo->setCurrentIndex(newPosition);
    }
    if (d->ui.clockDefaultsTo->count() > 1) {
        d->ui.clockDefaultsTo->setEnabled(true);
    } else {
        // Only "Local" in ui.clockDefaultsTo
        d->ui.clockDefaultsTo->setEnabled(false);
    }
}

void ClockApplet::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    // reimplemented by subclasses to get the new data
    Q_UNUSED(oldTimezone);
    Q_UNUSED(newTimezone);
}

bool ClockApplet::shouldDisplayTimezone() const
{
    return d->forceTzDisplay;
}

QList<QAction *> ClockApplet::contextualActions()
{
    if (!d->clipboardMenu) {
        d->clipboardMenu = new KMenu(i18n("C&opy to Clipboard"));
        d->clipboardMenu->setIcon(KIcon("edit-copy"));
        connect(d->clipboardMenu, SIGNAL(aboutToShow()), this, SLOT(updateClipboardMenu()));
        connect(d->clipboardMenu, SIGNAL(triggered(QAction*)), this, SLOT(copyToClipboard(QAction*)));

        KService::List offers = KServiceTypeTrader::self()->query("KCModule", "Library == 'kcm_clock'");
        if (!offers.isEmpty() && hasAuthorization("LaunchApp")) {
            d->adjustSystemTimeAction = new QAction(this);
            d->adjustSystemTimeAction->setText(i18n("Adjust Date and Time..."));
            d->adjustSystemTimeAction->setIcon(KIcon(icon()));
            connect(d->adjustSystemTimeAction, SIGNAL(triggered()), this, SLOT(launchTimeControlPanel()));
        }
    }

    QList<QAction*> contextualActions;
    contextualActions << d->clipboardMenu->menuAction();

    if (d->adjustSystemTimeAction) {
        contextualActions << d->adjustSystemTimeAction;
    }
    return contextualActions;
}

void ClockApplet::launchTimeControlPanel()
{
    KService::List offers = KServiceTypeTrader::self()->query("KCModule", "Library == 'kcm_clock'");
    if (offers.isEmpty()) {
        kDebug() << "fail";
        return;
    }

    KUrl::List urls;
    KService::Ptr service = offers.first();
    KRun::run(*service, urls, 0);
}

void ClockApplet::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (d->selectedTimezones.count() < 1) {
        return;
    }

    QString newTimezone;

    if (isLocalTimezone()) {
        if (event->delta() > 0) {
            newTimezone = d->selectedTimezones.last();
        } else {
            newTimezone = d->selectedTimezones.first();
        }
    } else {
        int current = d->selectedTimezones.indexOf(currentTimezone());

        if (event->delta() > 0) {
            int previous = current - 1;
            if (previous < 0) {
                newTimezone = localTimezoneUntranslated();
            } else {
                newTimezone = d->selectedTimezones.at(previous);
            }
        } else {
            int next = current + 1;
            if (next > d->selectedTimezones.count() - 1) {
                newTimezone = localTimezoneUntranslated();
            } else {
                newTimezone = d->selectedTimezones.at(next);
            }
        }
    }

    QString cur = currentTimezone();
    setCurrentTimezone(newTimezone);
    changeEngineTimezone(cur, newTimezone);
    update();
}

void ClockApplet::initExtenderItem(Plasma::ExtenderItem *item)
{
    if (item->name() == "calendar") {
        item->setTitle(i18n("Calendar"));
        item->setIcon("view-pim-calendar");
        item->setWidget(d->calendarWidget);
    } else if (item->name() == "today") {
        item->setTitle(i18n("Today"));
        item->setIcon("view-pim-calendar");
        d->label = new Plasma::Label();
        item->setWidget(d->label);
    }
}

void ClockApplet::init()
{
    Plasma::ToolTipManager::self()->registerWidget(this);

    d->calendarWidget = new Plasma::Calendar();
    d->calendarWidget->setAutomaticUpdateEnabled(false);
    d->calendarWidget->setMinimumSize(QSize(230, 220));
    d->calendarWidget->setDataEngine(dataEngine("calendar"));
    d->createCalendarExtender();

    extender();

    configChanged();
    QTimer::singleShot(0, this, SLOT(createToday()));
}

void ClockApplet::popupEvent(bool show)
{
    if (!show) {
        return;
    }

    Plasma::DataEngine::Data data = dataEngine("time")->query(currentTimezone());
    d->calendarWidget->setDate(data["Date"].toDate());
}

void ClockApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setPassivePopup(formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical);
    }
}

void ClockApplet::setCurrentTimezone(const QString &tz)
{
    if (d->timezone == tz) {
        return;
    }

    if (tz == localTimezone()) {
        // catch people accidentally passing in the translation of "Local"
        d->timezone = localTimezoneUntranslated();
    } else {
        d->timezone = tz;
    }

    d->forceTzDisplay = d->timezone != d->defaultTimezone;
    d->setPrettyTimezone();

    Plasma::DataEngine::Data data = dataEngine("time")->query(d->timezone);
    d->calendarWidget->setDate(data["Date"].toDate());

    KConfigGroup cg = config();
    cg.writeEntry("timezone", d->timezone);
    emit configNeedsSaving();
}

QString ClockApplet::currentTimezone() const
{
    return d->timezone;
}

QString ClockApplet::prettyTimezone() const
{
    return d->prettyTimezone;
}

QStringList ClockApplet::getSelectedTimezones() const
{
    return d->selectedTimezones;
}

bool ClockApplet::isLocalTimezone() const
{
    return d->timezone == localTimezoneUntranslated();
}

QString ClockApplet::localTimezone()
{
    return i18nc("Local time zone", "Local");
}

QString ClockApplet::localTimezoneUntranslated()
{
    return "Local";
}

void ClockApplet::updateClipboardMenu()
{
    d->clipboardMenu->clear();
    QList<QAction*> actions;
    Plasma::DataEngine::Data data = dataEngine("time")->query(currentTimezone());
    QDateTime dateTime = QDateTime(data["Date"].toDate(), data["Time"].toTime());

    d->clipboardMenu->addAction(calendar()->formatDate(dateTime.date(), KLocale::LongDate));
    d->clipboardMenu->addAction(calendar()->formatDate(dateTime.date(), KLocale::ShortDate));
    // Display ISO Date format if not already displayed
    if (KGlobal::locale()->dateFormatShort() != "%Y-%m-%d") {
        d->clipboardMenu->addAction(calendar()->formatDate(dateTime.date(), "%Y-%m-%d"));
    }

    QAction *sep0 = new QAction(this);
    sep0->setSeparator(true);
    d->clipboardMenu->addAction(sep0);

    d->clipboardMenu->addAction(KGlobal::locale()->formatTime(dateTime.time(), false));
    d->clipboardMenu->addAction(KGlobal::locale()->formatTime(dateTime.time(), true));

    QAction *sep1 = new QAction(this);
    sep1->setSeparator(true);
    d->clipboardMenu->addAction(sep1);

    KLocale tempLocale(*KGlobal::locale());
    tempLocale.setCalendar(calendar()->calendarType());
    d->clipboardMenu->addAction(tempLocale.formatDateTime(dateTime, KLocale::LongDate));
    d->clipboardMenu->addAction(tempLocale.formatDateTime(dateTime, KLocale::LongDate, true));
    d->clipboardMenu->addAction(tempLocale.formatDateTime(dateTime, KLocale::ShortDate));
    d->clipboardMenu->addAction(tempLocale.formatDateTime(dateTime, KLocale::ShortDate, true));
    // Display ISO DateTime format if not already displayed
    if (tempLocale.dateFormatShort() != "%Y-%m-%d") {
        tempLocale.setDateFormatShort("%Y-%m-%d");
        d->clipboardMenu->addAction(tempLocale.formatDateTime(dateTime, KLocale::ShortDate, true));
    }

    QAction *sep2 = new QAction(this);
    sep2->setSeparator(true);
    d->clipboardMenu->addAction(sep2);

    QStringList calendars = KCalendarSystem::calendarSystems();
    foreach ( const QString &cal, calendars ) {
        if (cal != calendar()->calendarType()) {
            KCalendarSystem *tempCal = KCalendarSystem::create(cal);
            QString text = tempCal->formatDate(dateTime.date(), KLocale::LongDate) + " (" + KCalendarSystem::calendarLabel(cal) + ')';
            d->clipboardMenu->addAction(text);
            text = tempCal->formatDate(dateTime.date(), KLocale::ShortDate) + " (" + KCalendarSystem::calendarLabel(cal) + ')';
            d->clipboardMenu->addAction(text);
            delete tempCal;
        }
    }
}

void ClockApplet::copyToClipboard(QAction* action)
{
    QString text = action->text();
    text.remove(QChar('&'));

    QApplication::clipboard()->setText(text);
}

const KCalendarSystem *ClockApplet::calendar() const
{
    return d->calendarWidget->calendar();
}

#include "clockapplet.moc"
