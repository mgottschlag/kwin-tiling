/***************************************************************************
 *   Copyright 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>          *
 *   Copyright 2007-2008 by Sebastian Kuegler <sebas@kde.org>              *
 *   Copyright 2007 by Luka Renko <lure@kubuntu.org>                       *
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

#include "battery.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>

#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KToolInvocation>
#include <KDialog>
#include <KColorScheme>
#include <KConfigDialog>
#include <KGlobalSettings>
#include <KPushButton>

#include <kworkspace/kworkspace.h>

#include <solid/control/powermanager.h>
#include <solid/powermanagement.h>

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/Animator>
#include <Plasma/Extender>
#include <Plasma/ExtenderItem>
#include <Plasma/PopupApplet>
#include <Plasma/Label>
#include <Plasma/Slider>
#include <Plasma/PushButton>
#include <Plasma/CheckBox>
#include <Plasma/ComboBox>
#include <Plasma/IconWidget>


Battery::Battery(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_isEmbedded(false),
      m_extenderVisible(false),
      m_controlsLayout(0),
      m_batteryLayout(0),
      m_statusLabel(0),
      m_batteryLabel(0),
      m_profileLabel(0),
      m_profileCombo(0),
      m_brightnessSlider(0),
      m_theme(0),
      m_availableProfiles(QStringList()),
      m_currentProfile(0),
      m_animId(-1),
      m_alpha(1),
      m_fadeIn(false),
      m_acAnimId(-1),
      m_acAlpha(1),
      m_acFadeIn(false),
      m_batteryAnimId(-1),
      m_batteryAlpha(1),
      m_batteryFadeIn(true),
      m_isHovered(false),
      m_firstRun(true),
      m_numOfBattery(0),
      m_acadapter_plugged(false),
      m_remainingMSecs(0)
{
    kDebug() << "Loading applet battery";
    setAcceptsHoverEvents(true);
    setHasConfigurationInterface(true);
    setPopupIcon(QIcon());
    resize(128, 128);
    setAspectRatioMode(Plasma::ConstrainedSquare );
    m_textRect = QRectF();
    m_remainingMSecs = 0;
    m_extenderApplet = 0;
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/battery-oxygen");
    m_theme->setContainsMultipleImages(false);
}

void Battery::init()
{
    KConfigGroup cg = config();
    m_showBatteryString = cg.readEntry("showBatteryString", false);
    m_showRemainingTime = cg.readEntry("showRemainingTime", false);
    m_showMultipleBatteries = cg.readEntry("showMultipleBatteries", !m_isEmbedded);

    showBattery(false);
    m_theme->resize(contentsRect().size());
    if (m_acadapter_plugged) {
        showAcAdapter(true);
    }
    showBattery(true);

    m_theme->resize(contentsRect().size());
    m_font = QApplication::font();
    m_font.setWeight(QFont::Bold);

    m_boxAlpha = 128;
    m_boxHoverAlpha = 192;

    readColors();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(readColors()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(readColors()));

    const QStringList& battery_sources = dataEngine("powermanagement")->query("Battery")["sources"].toStringList();

    //connect sources
    connectSources();

    foreach (const QString &battery_source, battery_sources) {
        //kDebug() << "BatterySource:" << battery_source;
        dataUpdated(battery_source, dataEngine("powermanagement")->query(battery_source));
    }
    m_numOfBattery = battery_sources.size();

    dataUpdated("AC Adapter", dataEngine("powermanagement")->query("AC Adapter"));

    if (!m_isEmbedded) {
        Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(extender());
        eItem->setName("powermanagement");
        initBatteryExtender(eItem);
        extender()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    }
}

void Battery::constraintsEvent(Plasma::Constraints constraints)
{
    //kDebug() << "ConstraintsEvent, Dude." << contentsRect();
    if (!m_showMultipleBatteries || m_numOfBattery < 2) {
        setAspectRatioMode(Plasma::Square);
    } else {
        setAspectRatioMode(Plasma::KeepAspectRatio);
    }
    int minWidth;
    int minHeight;

    if (constraints & (Plasma::FormFactorConstraint | Plasma::SizeConstraint)) {
        if (formFactor() == Plasma::Vertical) {
            if (!m_showMultipleBatteries) {
                minHeight = qMax(m_textRect.height(), size().width());
            } else {
                minHeight = qMax(m_textRect.height(), size().width()*m_numOfBattery);
            }
            setMinimumWidth(0);
            setMinimumHeight(minHeight);
            //kDebug() << "Vertical FormFactor";
        } else if (formFactor() == Plasma::Horizontal) {
            if (!m_showMultipleBatteries) {
                minWidth = qMax(m_textRect.width(), size().height());
            } else {
                minWidth = qMax(m_textRect.width(), size().height()*m_numOfBattery);
            }
            setMinimumWidth(minWidth);
            setMinimumHeight(0);
            //kDebug() << "Horizontal FormFactor" << m_textRect.width() << contentsRect().height();
        } else {
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        }

        m_theme->resize(contentsRect().size().toSize());
        m_font.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(),
                                 qRound(contentsRect().height() / 10)));
        update();
    }
}

void Battery::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    if (source.startsWith("Battery")) {
        m_batteries_data[source] = data;
    } else if (source == "AC Adapter") {
        m_acadapter_plugged = data["Plugged in"].toBool();
        showAcAdapter(m_acadapter_plugged);
    } else if (source == "PowerDevil") {
        m_availableProfiles = data["availableProfiles"].toStringList();
        m_currentProfile = data["currentProfile"].toString();
        //kDebug() << "PowerDevil profiles:" << m_availableProfiles << "[" << m_currentProfile << "]";
    } else {
        kDebug() << "Applet::Dunno what to do with " << source;
    }
    if (source == "Battery0") {
        m_remainingMSecs  = data["Remaining msec"].toInt();
        //kDebug() << "Remaining msecs on battery:" << m_remainingMSecs;
    }

    updateStatus();
    update();
}

void Battery::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), Applet::icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    ui.showBatteryStringCheckBox->setChecked(m_showBatteryString ? Qt::Checked : Qt::Unchecked);
    if (m_showRemainingTime) {
        ui.showTimeRadioButton->setChecked(Qt::Checked);
    } else {
        ui.showPercentageRadioButton->setChecked(Qt::Checked);
    }
    ui.showMultipleBatteriesCheckBox->setChecked(m_showMultipleBatteries ? Qt::Checked : Qt::Unchecked);
}

void Battery::configAccepted()
{
    KConfigGroup cg = config();

    if (m_showRemainingTime != ui.showTimeRadioButton->isChecked()) {
        // kDebug() << "config changed";
        m_showRemainingTime = !m_showRemainingTime;
        cg.writeEntry("showRemainingTime", m_showRemainingTime);
        // kDebug() << m_showRemainingTime;
        if (m_showBatteryString && m_showBatteryString == ui.showBatteryStringCheckBox->isChecked()) {
            showLabel(m_showBatteryString);
        }
    }

    if (m_showBatteryString != ui.showBatteryStringCheckBox->isChecked()) {
        m_showBatteryString = !m_showBatteryString;
        cg.writeEntry("showBatteryString", m_showBatteryString);
        showLabel(m_showBatteryString);
    }

    if (m_showMultipleBatteries != ui.showMultipleBatteriesCheckBox->isChecked()) {
        m_showMultipleBatteries = !m_showMultipleBatteries;
        cg.writeEntry("showMultipleBatteries", m_showMultipleBatteries);
        kDebug() << "Show multiple battery changed: " << m_showMultipleBatteries;
        emit sizeHintChanged(Qt::PreferredSize);
    }

    emit configNeedsSaving();
}

void Battery::readColors()
{
    m_textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    m_boxColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    m_boxColor.setAlpha(m_boxAlpha);
}

void Battery::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    showLabel(true);
    //showAcAdapter(false); // to test the animation without constant plugging
    //showBattery(false); // to test the animation without constant plugging
    m_isHovered = true;
    Applet::hoverEnterEvent(event);
}

void Battery::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_showBatteryString && !m_isEmbedded) {
        showLabel(false);
    }
    //showAcAdapter(true); // to test the animation without constant plugging
    //showBattery(true); // to test the animation without constant plugging
    //m_isHovered = false;
    Applet::hoverLeaveEvent(event);
}

Battery::~Battery()
{
}

void Battery::suspend()
{
    hidePopup();
    QDBusConnection dbus( QDBusConnection::sessionBus() );
    QDBusInterface iface( "org.kde.kded", "/modules/powerdevil", "org.kde.PowerDevil", dbus );
    iface.call( "suspend", Solid::Control::PowerManager::ToRam );
}

void Battery::hibernate()
{
    hidePopup();
    QDBusConnection dbus( QDBusConnection::sessionBus() );
    QDBusInterface iface( "org.kde.kded", "/modules/powerdevil", "org.kde.PowerDevil", dbus );
    iface.call( "suspend", Solid::Control::PowerManager::ToDisk );
}

void Battery::brightnessChanged(const int brightness)
{
    Solid::Control::PowerManager::setBrightness(brightness);
}

void Battery::updateSlider(const float brightness)
{
    if (m_brightnessSlider->value() != (int)brightness) {
        m_brightnessSlider->setValue((int) brightness);
    }
}

void Battery::setFullBrightness()
{
    brightnessChanged(100);
    updateSlider(100);
}

void Battery::setEmbedded(const bool embedded)
{
    m_isEmbedded = embedded;
}

void Battery::initBatteryExtender(Plasma::ExtenderItem *item)
{
    // We only show the extender for applets that are not embedded, as
    // that would create infinitve loops, you really don't want an applet
    // extender when the applet is embedded into another applet, such
    // as the battery applet is also embedded into the battery's extender.
    if (!m_isEmbedded) {
        int row = 0;
        int rowHeight = 20;
        int columnWidth = 120;

        QGraphicsWidget *controls = new QGraphicsWidget(item);
        //controls->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        //controls->resize(500, 500);
        m_controlsLayout = new QGraphicsGridLayout(controls);
        m_controlsLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_controlsLayout->setColumnPreferredWidth(0, rowHeight);
        m_controlsLayout->setColumnMinimumWidth(1, 2*columnWidth);
        m_controlsLayout->setColumnPreferredWidth(2, rowHeight);
        m_controlsLayout->setHorizontalSpacing(0);

        m_batteryLayout = new QGraphicsGridLayout(m_controlsLayout);

        //m_batteryLayout->setColumnPreferredWidth(0, 100);
        m_batteryLayout->setColumnPreferredWidth(1, columnWidth);
        //m_batteryLayout->setRowPreferredHeight(row, 60);
        m_batteryLabel = new Plasma::Label(controls);
        m_batteryLabel->setMinimumSize(200, 80);
        m_batteryLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        //m_batteryLabel->nativeWidget()->setWordWrap(false);
        m_batteryLabel->nativeWidget()->setAlignment(Qt::AlignTop);
        // FIXME: larger fonts screw up this label
        m_batteryLayout->addItem(m_batteryLabel, 0, 0, 1, 1, Qt::AlignLeft);

        Battery *m_extenderApplet = static_cast<Battery*>(Plasma::Applet::load("battery"));
        if (m_extenderApplet) {
            m_extenderApplet->setParent(this);
            m_extenderApplet->setAcceptsHoverEvents(false);
            m_extenderApplet->setParentItem(controls);
            m_extenderApplet->setEmbedded(true);
            m_extenderApplet->setMinimumSize(80, 80); // TODO: Multiple batteries?
            m_extenderApplet->resize(80, 80);
            m_extenderApplet->setBackgroundHints(NoBackground);
            m_extenderApplet->setFlag(QGraphicsItem::ItemIsMovable, false);
            m_extenderApplet->init();
            m_extenderApplet->showBatteryLabel(true);
            m_batteryLayout->addItem(m_extenderApplet, 0, 1, 1, 1, Qt::AlignRight);
            m_extenderApplet->updateConstraints(Plasma::StartupCompletedConstraint);
        }

        m_controlsLayout->addItem(m_batteryLayout, row, 0, 1, 3);
        row++;

        Plasma::Label *brightnessLabel = new Plasma::Label(controls);
        brightnessLabel->setText(i18n("Screen Brightness"));
        brightnessLabel->nativeWidget()->setWordWrap(false);
        m_controlsLayout->addItem(brightnessLabel, row, 0, 1, 3);
        brightnessLabel->nativeWidget()->setWordWrap(false);
        row++;

        m_brightnessSlider = new Plasma::Slider(controls);
        m_brightnessSlider->setRange(0, 100);
        m_brightnessSlider->setValue(Solid::Control::PowerManager::brightness());
        m_brightnessSlider->nativeWidget()->setTickInterval(10);
        m_brightnessSlider->setOrientation(Qt::Horizontal);
        connect(m_brightnessSlider, SIGNAL(valueChanged(int)),
                this, SLOT(brightnessChanged(int)));

        Solid::Control::PowerManager::Notifier *notifier = Solid::Control::PowerManager::notifier();

        connect(notifier, SIGNAL(brightnessChanged(float)),
                this, SLOT(updateSlider(float)));
        m_controlsLayout->addItem(m_brightnessSlider, row, 1, 1, 1);

        Plasma::IconWidget *brightnessIcon = new Plasma::IconWidget(controls);
        brightnessIcon->setIcon("ktip");
        connect(brightnessIcon, SIGNAL(clicked()),
                this, SLOT(setFullBrightness()));
        brightnessIcon->setDrawBackground(true);
        brightnessIcon->setMinimumSize(rowHeight, rowHeight);
        m_controlsLayout->addItem(brightnessIcon, row, 2, 1, 1);
        m_controlsLayout->setRowSpacing(row, 10);
        row++;

        m_profileLabel = new Plasma::Label(controls);
        m_profileLabel->setText(i18n("Power Profile"));
        m_controlsLayout->addItem(m_profileLabel, row, 0, 1, 3);
        row++;

        m_profileCombo = new Plasma::ComboBox(controls);
        // This is necessary until Qt task #217874 is fixed
        m_profileCombo->setZValue(100);
        connect(m_profileCombo, SIGNAL(activated(QString)),
                this, SLOT(setProfile(QString)));

        m_controlsLayout->addItem(m_profileCombo, row, 1, 1, 2);
        row++;

        Plasma::Label *actionsLabel = new Plasma::Label(controls);
        actionsLabel->setText(i18n("Actions"));
        actionsLabel->nativeWidget()->setWordWrap(false);
        m_controlsLayout->addItem(actionsLabel, row, 0, 1, 3);
        row++;

        QGraphicsGridLayout *actionsLayout = new QGraphicsGridLayout(m_controlsLayout);
        actionsLayout->setColumnSpacing(0, 0);
        actionsLayout->setColumnSpacing(1, 0);

        // Sleep and Hibernate buttons
        QSet<Solid::PowerManagement::SleepState> sleepstates = Solid::PowerManagement::supportedSleepStates();
        foreach (const Solid::PowerManagement::SleepState &sleepstate, sleepstates) {
            if (sleepstate == Solid::PowerManagement::StandbyState) {
                // Not interesting at this point ...

            } else if (sleepstate == Solid::PowerManagement::SuspendState) {
                Plasma::IconWidget *suspendButton = new Plasma::IconWidget(controls);
                suspendButton->setIcon("system-suspend");
                suspendButton->setText(i18n("Sleep"));
                suspendButton->setOrientation(Qt::Horizontal);
                suspendButton->setMaximumHeight(36);
                suspendButton->setDrawBackground(true);
                actionsLayout->addItem(suspendButton, 0, 0);
                connect(suspendButton, SIGNAL(clicked()), this, SLOT(suspend()));
                actionsLayout->setColumnSpacing(0, 20);
            } else if (sleepstate == Solid::PowerManagement::HibernateState) {
                Plasma::IconWidget *hibernateButton = new Plasma::IconWidget(controls);
                hibernateButton->setIcon("system-suspend-hibernate");
                hibernateButton->setText(i18n("Hibernate"));
                hibernateButton->setOrientation(Qt::Horizontal);
                hibernateButton->setMaximumHeight(36);
                hibernateButton->setDrawBackground(true);
                actionsLayout->addItem(hibernateButton, 0, 1);
                connect(hibernateButton, SIGNAL(clicked()), this, SLOT(hibernate()));
            }
        }
        m_controlsLayout->addItem(actionsLayout, row, 1, 1, 2);
        m_controlsLayout->setRowSpacing(row, 10);
        row++;

        // More settings button
        Plasma::IconWidget *configButton = new Plasma::IconWidget(controls);
        configButton->setText(i18n("More..."));
        configButton->setOrientation(Qt::Horizontal);
        configButton->setMaximumHeight(36);
        configButton->setDrawBackground(true);
        configButton->setIcon("preferences-system-power-management");
        //configButton->nativeWidget()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        connect(configButton, SIGNAL(clicked()), this, SLOT(openConfig()));

        //QGraphicsGridLayout *moreLayout = new QGraphicsGridLayout(m_controlsLayout);
        //moreLayout->setColumnPreferredWidth(0, columnWidth);
        actionsLayout->addItem(configButton, 1, 1, Qt::AlignLeft);

        //m_controlsLayout->addItem(moreLayout, row, 1, 1, 2);

        controls->setLayout(m_controlsLayout);
        item->setWidget(controls);
        item->setTitle(i18n("Power Management"));
    }
}

void Battery::popupEvent(bool show)
{
    m_extenderVisible = show;
    updateStatus();
}

void Battery::updateStatus()
{
    if (!m_extenderVisible) {
        return;
    }

    QString batteryLabelText = QString("<br />");
    if (m_numOfBattery && m_batteryLabel) {
        QHashIterator<QString, QHash<QString, QVariant > > battery_data(m_batteries_data);
        int bnum = 0;

        while (battery_data.hasNext()) {
            bnum++;
            battery_data.next();
            QString state = battery_data.value()["State"].toString();
            m_remainingMSecs = battery_data.value()["Remaining msec"].toInt();
            kDebug() << "time left:" << m_remainingMSecs;
            if (state == "Discharging" && m_remainingMSecs > 0) {

                // FIXME: Somehow, m_extenderApplet is null here, so the label never becomes visible
                if (m_extenderApplet) {
                    m_extenderApplet->showBatteryLabel(true);
                }

                // we don't have too much accuracy so only give hours and minutes
                batteryLabelText.append(i18n("Time remaining: <b>%1</b><br />", KGlobal::locale()->prettyFormatDuration(m_remainingMSecs)));
            } else {
                if (m_extenderApplet) {
                    m_extenderApplet->showBatteryLabel(false);
                }
                if (m_numOfBattery == 0) {
                    //kDebug() << "zero batteries ...";
                } else if (m_numOfBattery == 1) {
                    if (battery_data.value()["Plugged in"].toBool()) {
                        if (state == "NoCharge") {
                            batteryLabelText.append(i18n("<b>Battery:</b> %1% (fully charged)<br />", battery_data.value()["Percent"].toString()));
                        } else if (state == "Discharging") {
                            batteryLabelText.append(i18nc("Shown when a time estimate is not available", "<b>Battery:</b> %1% (discharging)<br />", battery_data.value()["Percent"].toString()));
                        } else {
                            batteryLabelText.append(i18n("<b>Battery:</b> %1% (charging)<br />", battery_data.value()["Percent"].toString()));
                        }
                    } else {
                        batteryLabelText.append(i18nc("Battery is not plugged in", "<b>Battery:</b> not present<br />"));
                    }
                } else {
                    //kDebug() << "More batteries ...";
                    if (state == "NoCharge") {
                        batteryLabelText.append(i18n("<b>Battery %1:</b> %2% (fully charged)<br />", bnum, battery_data.value()["Percent"].toString()));
                    } else if (state == "Discharging") {
                        batteryLabelText.append(i18n("<b>Battery %1:</b> %2% (discharging)<br />", bnum, battery_data.value()["Percent"].toString()));
                    } else {
                        batteryLabelText.append(i18n("<b>Battery %1:</b> %2% (charging)<br />", bnum, battery_data.value()["Percent"].toString()));
                    }
                }
            }
        }

        if (m_acadapter_plugged) {
            batteryLabelText.append(i18n("<b>AC Adapter:</b> Plugged in"));
        } else {
            batteryLabelText.append(i18n("<b>AC Adapter:</b> Not plugged in"));
        }
    } else {
        batteryLabelText.append(i18nc("Battery is not plugged in", "<b>Battery:</b> not present<br />"));
    }
    //kDebug() << batteryLabelText;
    if (m_batteryLabel) {
        m_batteryLabel->setText(batteryLabelText);
    }
    if (!m_availableProfiles.empty() && m_profileCombo) {
        m_profileCombo->clear();
        m_profileCombo->addItem(m_currentProfile);
        foreach (const QString &p, m_availableProfiles) {
            if (m_currentProfile != p) {
                m_profileCombo->addItem(p);
            }
        }
    }

    if (m_profileLabel && m_profileCombo) {
        if (m_availableProfiles.empty()) {
            m_profileCombo->hide();
            m_profileLabel->hide();
        } else {
            m_profileCombo->show();
            m_profileLabel->show();
        }
    }

    if (m_brightnessSlider) {
        m_brightnessSlider->setValue(Solid::Control::PowerManager::brightness());
        kDebug() << "Updating brightness:" << Solid::Control::PowerManager::brightness();
    }
    //kDebug() << "SIZE LABEL" << m_batteryLabel->size() << m_batteryLabel->preferredSize() << m_batteryLabel->preferredSize();
    m_controlsLayout->setColumnMinimumWidth(1,280);
    m_batteryLayout->setColumnMinimumWidth(0,200);
    m_batteryLayout->invalidate();
    m_controlsLayout->invalidate();
}

void Battery::openConfig()
{
    kDebug() << "opening powermanagement configuration dialog";
    QStringList args;
    args << "powerdevilconfig";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

void Battery::setProfile(const QString &profile)
{
    if (m_currentProfile != profile) {
        kDebug() << "Changing power profile to " << profile;
        QDBusConnection dbus( QDBusConnection::sessionBus() );
        QDBusInterface iface( "org.kde.kded", "/modules/powerdevil", "org.kde.PowerDevil", dbus );
        iface.call( "refreshStatus" );
        iface.call( "setProfile", profile );
    }
}

void Battery::showLabel(bool show)
{
    if (m_fadeIn == show) {
        return;
    }
    m_fadeIn = show;
    const int FadeInDuration = 150;

    if (m_animId != -1) {
        Plasma::Animator::self()->stopCustomAnimation(m_animId);
    }
    m_animId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,
                                                      Plasma::Animator::EaseOutCurve, this,
                                                      "animationUpdate");
}

QRectF Battery::scaleRectF(const qreal progress, QRectF rect)
{
    if (progress == 1) {
        return rect;
    }
    // Scale
    qreal w = rect.width()*progress;
    qreal h = rect.width()*progress;

    // Position centered
    rect.setX((rect.width() - w)/2);
    rect.setY((rect.height() - h)/2);

    rect.setWidth(w);
    rect.setHeight(h);

    return rect;
}

void Battery::showAcAdapter(bool show)
{
    if (m_acFadeIn == show) {
        return;
    }
    m_acFadeIn = show;
    const int FadeInDuration = 600;
    // As long as the animation is running, we fake it's still plugged in so it gets
    // painted in paintInterface()
    m_acadapter_plugged = true;

    if (m_acAnimId != -1) {
        Plasma::Animator::self()->stopCustomAnimation(m_acAnimId);
    }
    m_acAnimId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,
                                                      Plasma::Animator::EaseOutCurve, this,
                                                      "acAnimationUpdate");
}

void Battery::showBattery(bool show)
{
    if (m_batteryFadeIn == show) {
        return;
    }
    m_batteryFadeIn = show;
    const int FadeInDuration = 300;

    if (m_batteryAnimId != -1) {
        Plasma::Animator::self()->stopCustomAnimation(m_batteryAnimId);
    }
    m_batteryAnimId = Plasma::Animator::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,
                                                      Plasma::Animator::EaseOutCurve, this,
                                                      "batteryAnimationUpdate");
}

void Battery::animationUpdate(qreal progress)
{
    if (progress == 1) {
        m_animId = -1;
    }
    if (!m_fadeIn) {
        qreal new_alpha = m_fadeIn ? progress : 1 - progress;
        m_alpha = qMin(new_alpha, m_alpha);
    } else {
        m_alpha = m_fadeIn ? progress : 1 - progress;
    }
    m_alpha = qMax(qreal(0.0), m_alpha);
    update();
}

void Battery::acAnimationUpdate(qreal progress)
{
    if (progress == 1) {
        m_acAnimId = -1;
    }
    m_acAlpha = m_acFadeIn ? progress : 1 - progress;
    // During the fadeout animation, we had set it to true (and lie)
    // now the animation has ended, we _really_ set it to not show the adapter
    if (!m_acFadeIn && (progress == 1)) {
        m_acadapter_plugged = false;
        updateStatus();
    }
    update();
}

void Battery::batteryAnimationUpdate(qreal progress)
{
    if (progress == 1) {
        m_batteryAnimId = -1;
    }
    m_batteryAlpha = m_batteryFadeIn ? progress : 1 - progress;
    update();
}

void Battery::paintLabel(QPainter *p, const QRect &contentsRect, const QString& labelText)
{
    // Store font size, we want to restore it shortly
    int original_font_size = m_font.pointSize();

    // Fonts smaller than smallestReadableFont don't make sense.
    m_font.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(), m_font.pointSize()));
    QFontMetrics fm(m_font);
    qreal text_width = fm.width(labelText);

    // Longer texts get smaller fonts
    if (labelText.length() > 4) {
        if (original_font_size/1.5 < KGlobalSettings::smallestReadableFont().pointSize()) {
            m_font.setPointSize((KGlobalSettings::smallestReadableFont().pointSize()));
        } else {
            m_font.setPointSizeF(original_font_size/1.5);
        }
        fm = QFontMetrics(m_font);
        text_width = (fm.width(labelText) * 1.2);
    } else {
        // Smaller texts get a wider box
        text_width = (text_width * 1.4);
    }
    if (formFactor() == Plasma::Horizontal ||
        formFactor() == Plasma::Vertical) {
        m_font = KGlobalSettings::smallestReadableFont();
        m_font.setWeight(QFont::Bold);
        fm = QFontMetrics(m_font);
        text_width = (fm.width(labelText)+8);
    }
    p->setFont(m_font);

    // Let's find a good position for painting the percentage on top of the battery
    m_textRect = QRectF(qMax(qreal(0.0), contentsRect.left() + (contentsRect.width() - text_width) / 2),
                            contentsRect.top() + ((contentsRect.height() - (int)fm.height()) / 2 * 0.9),
                            qMin(contentsRect.width(), (int)text_width),
                            fm.height() * 1.2 );
    //kDebug() << contentsRect << m_textRect;
    //p->setBrush(QColor("green"));
    //p->drawRect(m_textRect);

    if (m_firstRun) {
        m_firstRun = false;
        return;
    }
    // Poor man's highlighting
    m_boxColor.setAlphaF(m_alpha);
    p->setPen(m_boxColor);
    m_boxColor.setAlphaF(m_alpha*0.5);
    p->setBrush(m_boxColor);

    // Find sensible proportions for the rounded corners
    float round_prop = m_textRect.width() / m_textRect.height();

    // Tweak the rounding edge a bit with the proportions of the textbox
    qreal round_radius = 35.0;
    p->drawRoundedRect(m_textRect, round_radius / round_prop, round_radius, Qt::RelativeSize);

    m_textColor.setAlphaF(m_alpha);
    p->setPen(m_textColor);
    p->drawText(m_textRect, Qt::AlignCenter, labelText);

    // Reset font and box
    m_font.setPointSize(original_font_size);
    m_boxColor.setAlpha(m_boxAlpha);
}

void Battery::paintBattery(QPainter *p, const QRect &contentsRect, const int batteryPercent, const bool plugState)
{
    if (m_theme->hasElement("Battery")) {
        m_theme->paint(p, scaleRectF(m_batteryAlpha, contentsRect), "Battery");
    }

    QString fill_element = QString();
    if (plugState) {
        if (batteryPercent > 95) {
            fill_element = "Fill100";
        } else if (batteryPercent > 80) {
            fill_element = "Fill80";
        } else if (batteryPercent > 50) {
            fill_element = "Fill60";
        } else if (batteryPercent > 20) {
            fill_element = "Fill40";
        } else if (batteryPercent > 10) {
            fill_element = "Fill20";
        } // Don't show a fillbar below 11% charged
    } else {
        fill_element = "Unavailable";
    }
    //kDebug() << "plugState:" << plugState;

    // Now let's find out which fillstate to show
    if (!fill_element.isEmpty()) {
        if (m_theme->hasElement(fill_element)) {
            m_theme->paint(p, scaleRectF(m_batteryAlpha, contentsRect), fill_element);
        } else {
            kDebug() << fill_element << " does not exist in svg";
        }
    }

    if (m_acadapter_plugged) {
        //QRectF ac_rect = QRectF(contentsRect.topLeft(), QSizeF(contentsRect.width()*m_acAlpha, contentsRect.height()*m_acAlpha));
        m_theme->paint(p, scaleRectF(m_acAlpha, contentsRect), "AcAdapter");
    }

    // For small FormFactors, we're drawing a shadow
    if (formFactor() == Plasma::Vertical ||
        formFactor() == Plasma::Horizontal) {
        if (plugState) {
            m_theme->paint(p, contentsRect, "Shadow");
        }
    }
    if (plugState && m_theme->hasElement("Overlay")) {
        m_theme->paint(p, scaleRectF(m_batteryAlpha, contentsRect), "Overlay");
    }
}

void Battery::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    if (m_numOfBattery == 0) {
        QRectF ac_contentsRect(contentsRect.topLeft(), QSizeF(qMax(qreal(0.0), contentsRect.width() * m_acAlpha), qMax(qreal(0.0), contentsRect.height() * m_acAlpha)));
        if (m_acadapter_plugged) {
            m_theme->paint(p, ac_contentsRect, "AcAdapter");
        }
        paintBattery(p, contentsRect, 0, false);
        return;
    }

    if (m_isEmbedded || m_showMultipleBatteries || m_firstRun) {
        // paint each battery with own charge level
        int battery_num = 0;
        int width = contentsRect.width()/m_numOfBattery;
        QHashIterator<QString, QHash<QString, QVariant > > battery_data(m_batteries_data);
        while (battery_data.hasNext()) {
            battery_data.next();
            QRect corect = QRect(contentsRect.left()+battery_num*width,
                                 contentsRect.top(),
                                 width, contentsRect.height());

            // paint battery with appropriate charge level
            paintBattery(p, corect, battery_data.value()["Percent"].toInt(), battery_data.value()["Plugged in"].toBool());

            if (m_showBatteryString || m_isHovered || m_firstRun) {
                // Show the charge percentage with a box on top of the battery
                QString batteryLabel;
                if (battery_data.value()["Plugged in"].toBool()) {
                    // kDebug() << m_showRemainingTime;
                    if (!m_showRemainingTime || m_remainingMSecs==0) {
                        batteryLabel = battery_data.value()["Percent"].toString();
                        batteryLabel.append("%");
                    } else {
                        m_remainingMSecs = battery_data.value()["Remaining msec"].toInt();
                        int hours = m_remainingMSecs/1000/3600;
                        int minutes = qRound(m_remainingMSecs/60000) % 60;
                        QTime t = QTime(hours, minutes);
                        KLocale tmpLocale(*KGlobal::locale());
                        tmpLocale.setTimeFormat("%k:%M");
                        batteryLabel = tmpLocale.formatTime(t, false, true); // minutes, hours as duration
                    }
                    // kDebug() << batteryLabel;
                    paintLabel(p, corect, batteryLabel);
                }
            }
            ++battery_num;
        }
    } else {
        // paint only one battery and show cumulative charge level
        int battery_num = 0;
        int battery_charge = 0;
        bool has_battery = false;
        QHashIterator<QString, QHash<QString, QVariant > > battery_data(m_batteries_data);
        while (battery_data.hasNext()) {
            battery_data.next();
            if (battery_data.value()["Plugged in"].toBool()) {
                battery_charge += battery_data.value()["Percent"].toInt();
                has_battery = true;
                ++battery_num;
            }
        }
        if (battery_num > 0) {
            battery_charge = battery_charge / battery_num;
        }
        // paint battery with appropriate charge level
        paintBattery(p, contentsRect,  battery_charge, has_battery);
        if (m_showBatteryString || m_isHovered) {
            // Show the charge percentage with a box on top of the battery
            QString batteryLabel;
            if (has_battery) {
                batteryLabel = QString::number(battery_charge);
                batteryLabel.append("%");
                paintLabel(p, contentsRect, batteryLabel);
            }
        }
    }
}

void Battery::showBatteryLabel(bool show)
{
    kDebug() << show;
    if (show != m_showBatteryString) {
        showLabel(show);
        m_showBatteryString = show;
    }
}

void Battery::connectSources()
{
    const QStringList& battery_sources = dataEngine("powermanagement")->query("Battery")["sources"].toStringList();

    foreach (const QString &battery_source, battery_sources) {
        dataEngine("powermanagement")->connectSource(battery_source, this);
    }

    dataEngine("powermanagement")->connectSource("AC Adapter", this);
    dataEngine("powermanagement")->connectSource("PowerDevil", this);

    connect(dataEngine("powermanagement"), SIGNAL(sourceAdded(QString)),
            this,                          SLOT(sourceAdded(QString)));
    connect(dataEngine("powermanagement"), SIGNAL(sourceRemoved(QString)),
            this,                          SLOT(sourceRemoved(QString)));
}

void Battery::sourceAdded(const QString& source)
{
    if (source.startsWith("Battery") && source != "Battery") {
        dataEngine("powermanagement")->connectSource(source, this);
        m_numOfBattery++;
        constraintsEvent(Plasma::SizeConstraint);
        update();
    }
    if (source == "PowerDevil") {
        dataEngine("powermanagement")->connectSource(source, this);
    }
}

void Battery::sourceRemoved(const QString& source)
{
    if (m_batteries_data.remove(source)) {
        m_numOfBattery--;
        constraintsEvent(Plasma::SizeConstraint);
        update();
    }
    if (source == "PowerDevil") {
        dataEngine("powermanagement")->disconnectSource(source, this);
    }
}

#include "battery.moc"
