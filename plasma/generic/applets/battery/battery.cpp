/***************************************************************************
 *   Copyright 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>          *
 *   Copyright 2007-2009 by Sebastian Kuegler <sebas@kde.org>              *
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
#include <QDesktopWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QDBusPendingCall>
#include <QLabel>
#include <QPropertyAnimation>

#include <KApplication>
#include <KDebug>
#include <KIcon>
#include <KSharedConfig>
#include <KToolInvocation>
#include <KColorScheme>
#include <KConfigDialog>
#include <KGlobalSettings>
#include <KPushButton>
#include <KComboBox>

#include <kworkspace/kworkspace.h>

#include <solid/powermanagement.h>

#include <Plasma/Animator>
#include <Plasma/CheckBox>
#include <Plasma/ComboBox>
#include <Plasma/IconWidget>
#include <Plasma/Extender>
#include <Plasma/ExtenderItem>
#include <Plasma/Label>
#include <Plasma/PopupApplet>
#include <Plasma/PushButton>
#include <Plasma/Separator>
#include <Plasma/Slider>
#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>
#include <qdbuspendingreply.h>


typedef QHash<QString, QVariant> VariantDict;

Battery::Battery(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_isEmbedded(false),
      m_extenderVisible(false),
      m_batteryLabelLabel(0),
      m_batteryInfoLabel(0),
      m_acLabelLabel(0),
      m_acInfoLabel(0),
      m_brightnessSlider(0),
      m_minutes(0),
      m_hours(0),
      m_theme(0),
      m_availableProfiles(StringStringMap()),
      m_numOfBattery(0),
      m_acAdapterPlugged(false),
      m_remainingMSecs(0),
      m_labelAlpha(0),
      m_labelAnimation(0),
      m_acAlpha(0),
      m_acAnimation(0),
      m_ignoreBrightnessChange(false),
      m_inhibitCookies(qMakePair< int, int >(-1, -1))
{
    //kDebug() << "Loading applet battery";
    setAcceptsHoverEvents(true);
    setPopupIcon(QIcon());
    resize(128, 128);
    setAspectRatioMode(Plasma::ConstrainedSquare );
    m_textRect = QRectF();
    m_remainingMSecs = 0;
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/battery-oxygen");
    m_theme->setContainsMultipleImages(true);
    setStatus(Plasma::ActiveStatus);
    //setPassivePopup(true);

    m_labelAnimation = new QPropertyAnimation(this, "labelAlpha");
    m_labelAnimation->setDuration(200);
    m_labelAnimation->setStartValue((qreal)(0.0));
    m_labelAnimation->setEndValue((qreal)(1.0));
    m_labelAnimation->setEasingCurve(QEasingCurve::OutQuad);
    connect(m_labelAnimation, SIGNAL(finished()), this, SLOT(updateBattery()));
    m_acAnimation = new QPropertyAnimation(this, "acAlpha");
    m_acAnimation->setDuration(500);
    m_acAnimation->setStartValue((qreal)(0.0));
    m_acAnimation->setEndValue((qreal)(1.0));
    m_acAnimation->setEasingCurve(QEasingCurve::OutBack);
    m_acAnimation->setDirection(QAbstractAnimation::Backward);
    connect(m_acAnimation, SIGNAL(finished()), this, SLOT(updateBattery()));
}

Battery::~Battery()
{
}

void Battery::init()
{
    setHasConfigurationInterface(true);

    // read config
    configChanged();

    m_theme->resize(contentsRect().size());
    m_font = QApplication::font();
    m_font.setWeight(QFont::Bold);

    m_boxAlpha = 128;
    m_boxHoverAlpha = 192;

    readColors();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(readColors()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(readColors()));
    connect(KGlobalSettings::self(), SIGNAL(appearanceChanged()), SLOT(setupFonts()));

    const QStringList& battery_sources = dataEngine("powermanagement")->query("Battery")["Sources"].toStringList();

    connectSources();

    foreach (const QString &battery_source, battery_sources) {
        dataUpdated(battery_source, dataEngine("powermanagement")->query(battery_source));
    }
    m_numOfBattery = battery_sources.size();
    if (m_numOfBattery == 0) {
        m_acAlpha = 1;
    }

    dataUpdated("AC Adapter", dataEngine("powermanagement")->query("AC Adapter"));

    if (!m_isEmbedded) {
        initPopupWidget();
        // let's show a brightness OSD
        QDBusConnection::sessionBus().connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement",
                                              "brightnessChanged", this, SLOT(updateSlider(float)));
    }

    if (m_acAdapterPlugged) {
        showAcAdapter(true);
    }
}

void Battery::configChanged()
{
    KConfigGroup cg = config();
    m_showBatteryLabel = cg.readEntry("showBatteryString", false);
    m_showRemainingTime = cg.readEntry("showRemainingTime", false);
    m_showMultipleBatteries = cg.readEntry("showMultipleBatteries", false);

    if (m_showBatteryLabel) {
        showLabel(true);
    }
}

void Battery::toolTipAboutToShow()
{
    // for icon
    // void Battery::paintBattery(QPainter *p, const QRect &contentsRect, const int batteryPercent, const bool plugState)
    QString mainText;
    QString subText;
    int batteryCount = 0;
    foreach (const VariantDict &batteryData, m_batteriesData) {
        if (m_numOfBattery == 1) {
            subText.append(i18n("<b>Battery:</b>"));
        } else {
            //kDebug() << "More batteries ...";
            if (!subText.isEmpty()) {
                subText.append("<br/>");
            }

            subText.append(i18nc("tooltip: placeholder is the battery ID", "<b>Battery %1:</b>", batteryCount));
        }

        subText.append(' ').append(stringForState(batteryData));
        ++batteryCount;
    }

    if (!subText.isEmpty()) {
        subText.append("<br/>");
    }

    subText.append(i18nc("tooltip", "<b>AC Adapter:</b>")).append(' ');
    subText.append(m_acAdapterPlugged ? i18nc("tooltip", "Plugged in") : i18nc("tooltip", "Not plugged in"));

    Plasma::ToolTipContent c(mainText, subText, KIcon("battery"));
    Plasma::ToolTipManager::self()->setContent(this, c);
}

void Battery::toolTipHidden()
{
  Plasma::ToolTipManager::self()->clearContent(this);
}

void Battery::updateBattery()
{
    update();
}

bool Battery::isConstrained()
{
    return formFactor() == Plasma::Vertical || formFactor() == Plasma::Horizontal;
}

void Battery::constraintsEvent(Plasma::Constraints constraints)
{
    //kDebug() << "ConstraintsEvent, Dude." << contentsRect();
    if (!m_showMultipleBatteries || m_numOfBattery < 2) {
        setAspectRatioMode(Plasma::Square);
    } else {
        setAspectRatioMode(Plasma::KeepAspectRatio);
    }

    if (constraints & Plasma::FormFactorConstraint) {
        if (isConstrained()) {
            m_theme->setImagePath("icons/battery");
        } else {
            m_theme->setImagePath("widgets/battery-oxygen");
        }
    }

    if (constraints & Plasma::FormFactorConstraint || constraints & Plasma::SizeConstraint) {
        int minWidth = KIconLoader::SizeSmall;
        int minHeight = KIconLoader::SizeSmall;
        bool showToolTips = false;
        if (formFactor() == Plasma::Vertical) {
            if (m_showBatteryLabel) {
                minHeight = qMax(qMax(minHeight, int(m_textRect.height())), int(size().width()));
            } else {
                showToolTips = true;
                minHeight = qMax(minHeight, int(size().width()));
            }
            minWidth = 0;
            //kDebug() << "Vertical FormFactor";
        } else if (formFactor() == Plasma::Horizontal) {
            if (m_showBatteryLabel) {
                minWidth = qMax(qMax(minWidth, int(m_textRect.width())), int(size().height()));
            } else {
                showToolTips = true;
                minWidth = qMax(minWidth, int(size().height()));
            }
            minHeight = 0;
            //kDebug() << "Horizontal FormFactor" << m_textRect.width() << contentsRect().height();
        } else {
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            Plasma::ToolTipManager::self()->unregisterWidget(this);
        }

        if (m_showMultipleBatteries) {
            setMinimumSize(minWidth * m_numOfBattery, minHeight);
        } else {
            setMinimumSize(minWidth, minHeight);
        }
        if (parentLayoutItem() && parentLayoutItem()->isLayout()) {
            static_cast<QGraphicsLayout *>(parentLayoutItem())->invalidate();
        }

        if (showToolTips) {
            Plasma::ToolTipManager::self()->registerWidget(this);
        } else {
            Plasma::ToolTipManager::self()->unregisterWidget(this);
        }

        QSize c(contentsRect().size().toSize());
        if (m_showMultipleBatteries) {
            if (formFactor() == Plasma::Vertical) {
                c.setHeight(size().height() / qMax(1, m_numOfBattery));
            } else if (formFactor() == Plasma::Horizontal) {
                c.setWidth(size().width() / qMax(1, m_numOfBattery));
            }
        }
        m_theme->resize(c);

        m_font.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(),
                                 qRound(contentsRect().height() / 10)));
        update();
    }
}

void Battery::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    if (source == "Battery") {
        m_remainingMSecs = data["Remaining msec"].toULongLong();
        //kDebug() << "Remaining msecs on battery:" << m_remainingMSecs;
    }
    else if (source.startsWith(QLatin1String("Battery"))) {
        m_batteriesData[source] = data;
        //kDebug() << "new battery source" << source;
    } else if (source == "AC Adapter") {
        m_acAdapterPlugged = data["Plugged in"].toBool();
        showAcAdapter(m_acAdapterPlugged);
    } else if (source == "PowerDevil") {
        m_availableProfiles = data["Available profiles"].value< StringStringMap >();
        m_currentProfile = data["Current profile"].toString();
        //kDebug() << "PowerDevil profiles:" << m_availableProfiles << "[" << m_currentProfile << "]";
    } else {
        kDebug() << "Applet::Dunno what to do with " << source;
    }

    Plasma::ItemStatus status = Plasma::PassiveStatus;
    foreach (const Plasma::DataEngine::Data &data, m_batteriesData) {
        if (data.value("Percent", 0).toInt() < 10) {
            status = Plasma::NeedsAttentionStatus;
            break;
        } else if (data["State"].toString() != "NoCharge") {
            status = Plasma::ActiveStatus;
            break;
        }
    }
    setStatus(status);

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
    ui.showBatteryStringCheckBox->setChecked(m_showBatteryLabel ? Qt::Checked : Qt::Unchecked);
    ui.showMultipleBatteriesCheckBox->setChecked(m_showMultipleBatteries ? Qt::Checked : Qt::Unchecked);

    connect(ui.showBatteryStringCheckBox, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.showMultipleBatteriesCheckBox, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
}

void Battery::configAccepted()
{
    KConfigGroup cg = config();

    if (m_showBatteryLabel != ui.showBatteryStringCheckBox->isChecked()) {
        setShowBatteryLabel(!m_showBatteryLabel);
        cg.writeEntry("showBatteryString", m_showBatteryLabel);
    }

    if (m_showMultipleBatteries != ui.showMultipleBatteriesCheckBox->isChecked()) {
        m_showMultipleBatteries = !m_showMultipleBatteries;
        cg.writeEntry("showMultipleBatteries", m_showMultipleBatteries);
        //kDebug() << "Show multiple battery changed: " << m_showMultipleBatteries;
        emit sizeHintChanged(Qt::PreferredSize);
    }

    emit configNeedsSaving();
}

void Battery::readColors()
{
    m_textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    m_boxColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
}

void Battery::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_showBatteryLabel && !m_isEmbedded && !isConstrained()) {
        showLabel(true);
    }

    Applet::hoverEnterEvent(event);
}

void Battery::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_showBatteryLabel && !m_isEmbedded && !isConstrained()) {
        showLabel(false);
    }

    Applet::hoverLeaveEvent(event);
}

void Battery::suspend()
{
    hidePopup();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                      "/org/kde/Solid/PowerManagement",
                                                      "org.kde.Solid.PowerManagement",
                                                      "suspendToRam");
    QDBusPendingReply< QString > reply = QDBusConnection::sessionBus().asyncCall(msg);
}

void Battery::hibernate()
{
    hidePopup();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                      "/org/kde/Solid/PowerManagement",
                                                      "org.kde.Solid.PowerManagement",
                                                      "suspendToDisk");
    QDBusPendingReply< QString > reply = QDBusConnection::sessionBus().asyncCall(msg);
}

void Battery::brightnessChanged(const int brightness)
{
    if (!m_ignoreBrightnessChange) {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                          "/org/kde/Solid/PowerManagement",
                                                          "org.kde.Solid.PowerManagement",
                                                          "setBrightness");
        msg.setArguments(QList<QVariant>() << QVariant::fromValue(brightness));
        QDBusPendingReply< QString > reply = QDBusConnection::sessionBus().asyncCall(msg);
    }
}

void Battery::updateSlider()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                      "/org/kde/Solid/PowerManagement",
                                                      "org.kde.Solid.PowerManagement",
                                                      "brightness");
    QDBusPendingReply< int > reply = QDBusConnection::sessionBus().asyncCall(msg);
    reply.waitForFinished();
    updateSlider(reply.value());
}

void Battery::updateSlider(const float brightness)
{
    if (m_brightnessSlider->value() != (int)brightness) {
        m_ignoreBrightnessChange = true;
        m_brightnessSlider->setValue((int) brightness);
        m_ignoreBrightnessChange = false;
    }
}

void Battery::setEmbedded(const bool embedded)
{
    m_isEmbedded = embedded;
}

static Plasma::Label *createBuddyLabel(QGraphicsWidget *parent)
{
    Plasma::Label *label = new Plasma::Label(parent);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->nativeWidget()->setWordWrap(false);
    return label;
}

static Plasma::Label *createInfoLabel(QGraphicsWidget *parent)
{
    Plasma::Label *label = new Plasma::Label(parent);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->nativeWidget()->setWordWrap(false);
    return label;
}

static Plasma::IconWidget *createButton(QGraphicsWidget *parent)
{
    Plasma::IconWidget *button = new Plasma::IconWidget(parent);
    button->setPreferredIconSize(QSizeF(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    button->setOrientation(Qt::Horizontal);
    button->setDrawBackground(true);
    button->setTextBackgroundColor(QColor(Qt::transparent));
    return button;
}

void Battery::initPopupWidget()
{
    // We only show the popup widget for applets that are not embedded, as that
    // would create infinitve loops, you really don't want a popup widget when
    // the applet is embedded into another applet, such as the battery applet
    // is also embedded into the battery's popup widget.
    if (m_isEmbedded) {
        return;
    }

    int row = 0;

    QGraphicsWidget *controls = new QGraphicsWidget(this);
    setGraphicsWidget(controls);
    QGraphicsGridLayout *controlsLayout = new QGraphicsGridLayout(controls);
    controlsLayout->setColumnStretchFactor(1, 3);
    // Do not call QGraphicsWidget::setMinimumWidth() here: for some reason
    // it prevents the widget from getting wide enough if needed (for
    // example because of wide translations, see bug #258044)
    //
    // Setting the minimum width of a column does not cause such problems,
    // so use that solution as a workaround.
    //
    // This is reproducible with Qt 4.7
    //controls->setMinimumWidth(360);
    controlsLayout->setColumnMinimumWidth(1, 200);

    Plasma::Label *spacer0 = new Plasma::Label(controls);
    spacer0->setMinimumHeight(8);
    spacer0->setMaximumHeight(8);
    controlsLayout->addItem(spacer0, row, 0);
    row++;

    m_batteryLabelLabel = createBuddyLabel(controls);
    m_batteryInfoLabel = createInfoLabel(controls);
    controlsLayout->addItem(m_batteryLabelLabel, row, 0);
    controlsLayout->addItem(m_batteryInfoLabel, row, 1);
    row++;
    m_acLabelLabel = createBuddyLabel(controls);
    m_acInfoLabel = createInfoLabel(controls);
    controlsLayout->addItem(m_acLabelLabel, row, 0);
    controlsLayout->addItem(m_acInfoLabel, row, 1);
    row++;

    m_remainingTimeLabel = createBuddyLabel(controls);
    m_remainingTimeLabel->setText(i18nc("Label for remaining time", "Time Remaining:"));
    m_remainingInfoLabel = createInfoLabel(controls);
    controlsLayout->addItem(m_remainingTimeLabel, row, 0);
    controlsLayout->addItem(m_remainingInfoLabel, row, 1);
    row++;

    m_inhibitLabel = createBuddyLabel(controls);
    m_inhibitLabel->setText(i18nc("Label for power management inhibition", "Power management enabled:"));
    m_inhibitButton = new Plasma::CheckBox(controls);
    m_inhibitButton->setChecked(true);
    controlsLayout->addItem(m_inhibitLabel, row, 0);
    controlsLayout->addItem(m_inhibitButton, row, 1);
    connect(m_inhibitButton, SIGNAL(toggled(bool)), this, SLOT(toggleInhibit(bool)));
    row++;

    Battery *extenderApplet = new Battery(0, QVariantList());
    extenderApplet->setParent(controls);
    extenderApplet->setAcceptsHoverEvents(false);
    extenderApplet->setParentItem(controls);
    extenderApplet->setEmbedded(true);
    extenderApplet->setBackgroundHints(NoBackground);
    extenderApplet->setFlag(QGraphicsItem::ItemIsMovable, false);
    extenderApplet->init();
    extenderApplet->setShowBatteryLabel(false);
    extenderApplet->updateConstraints(Plasma::StartupCompletedConstraint);
    extenderApplet->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    controlsLayout->addItem(extenderApplet, 1, 2, 2, 1);

    m_brightnessLabel = createBuddyLabel(controls);
    m_brightnessLabel->setText(i18n("Screen Brightness:"));
    controlsLayout->addItem(m_brightnessLabel, row, 0);

    m_brightnessSlider = new Plasma::Slider(controls);
    m_brightnessSlider->setRange(0, 100);

    updateSlider();
    m_brightnessSlider->nativeWidget()->setTickInterval(10);
    m_brightnessSlider->setOrientation(Qt::Horizontal);
    connect(m_brightnessSlider, SIGNAL(valueChanged(int)),
            this, SLOT(brightnessChanged(int)));
    controlsLayout->addItem(m_brightnessSlider, row, 1, 1, 2);
    row++;

    QGraphicsLinearLayout *buttonLayout = new QGraphicsLinearLayout;
    buttonLayout->setSpacing(0.0);
    buttonLayout->addStretch();

    // Sleep and Hibernate buttons
    QSet<Solid::PowerManagement::SleepState> sleepstates = Solid::PowerManagement::supportedSleepStates();
    foreach (const Solid::PowerManagement::SleepState &sleepstate, sleepstates) {
        if (sleepstate == Solid::PowerManagement::StandbyState) {
            // Not interesting at this point ...

        } else if (sleepstate == Solid::PowerManagement::SuspendState) {
            Plasma::IconWidget *suspendButton = createButton(controls);
            suspendButton->setIcon("system-suspend");
            suspendButton->setText(i18nc("Suspend the computer to RAM; translation should be short", "Sleep"));
            buttonLayout->addItem(suspendButton);
            //row++;
            connect(suspendButton, SIGNAL(clicked()), this, SLOT(suspend()));
        } else if (sleepstate == Solid::PowerManagement::HibernateState) {
            Plasma::IconWidget *hibernateButton = createButton(controls);
            hibernateButton->setIcon("system-suspend-hibernate");
            hibernateButton->setText(i18nc("Suspend the computer to disk; translation should be short", "Hibernate"));
            buttonLayout->addItem(hibernateButton);
            connect(hibernateButton, SIGNAL(clicked()), this, SLOT(hibernate()));
        }
    }

    // Configure button
    Plasma::IconWidget *configButton = createButton(controls);
    QAction *action = new QAction(this);
    action->setToolTip(i18nc("tooltip on the config button in the popup", "Configure Power Management..."));
    action->setIcon(KIcon("configure"));
    action->setText(i18n("Power Settings"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(openConfig()));
    connect(configButton, SIGNAL(clicked()), this, SLOT(openConfig()));
    addAction("configure_powersave", action);
    configButton->setIcon(action->icon());
    configButton->setToolTip(action->toolTip());
    configButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    configButton->setEnabled(hasAuthorization("LaunchApp"));

    buttonLayout->addItem(configButton);
    buttonLayout->setItemSpacing(0, 0.0);
    buttonLayout->setItemSpacing(1, 0.0);
    buttonLayout->setAlignment(configButton, Qt::AlignRight|Qt::AlignVCenter);

    controlsLayout->addItem(buttonLayout, row, 0, 1, 3);
    controls->setLayout(controlsLayout);

    setupFonts();
}

void Battery::popupEvent(bool show)
{
    m_extenderVisible = show;
    updateStatus();
    PopupApplet::popupEvent(show);
}

void Battery::setupFonts()
{
    if (m_batteryLabelLabel) {
        QFont infoFont = KGlobalSettings::generalFont();
        m_brightnessLabel->setFont(infoFont);

        QFont boldFont = infoFont;
        boldFont.setBold(true);

        m_batteryLabelLabel->setFont(infoFont);
        m_acLabelLabel->setFont(infoFont);
        m_remainingTimeLabel->setFont(infoFont);

        m_batteryInfoLabel->setFont(boldFont);
        m_acInfoLabel->setFont(boldFont);

        m_remainingInfoLabel->setFont(infoFont);
    }
}

QString Battery::stringForState(const QHash<QString, QVariant> &batteryData, bool *chargeChanging)
{
    if (batteryData["Plugged in"].toBool()) {
        const QString state = batteryData["State"].toString();
        if (chargeChanging)
            *chargeChanging = true;

        if (state == "NoCharge") {
            return i18n("%1% (charged)", batteryData["Percent"].toString());
        } else if (state == "Discharging") {
            return i18n("%1% (discharging)", batteryData["Percent"].toString());
        } else  {// charging
            return i18n("%1% (charging)", batteryData["Percent"].toString());
        }
    }

    return i18nc("Battery is not plugged in", "Not present");
}

void Battery::updateStatus()
{
    if (!m_extenderVisible) {
        return;
    }

    QString batteriesLabel;
    QString batteriesInfo;
    if (m_numOfBattery > 0) {
        bool showRemainingTime = m_showRemainingTime;
        int batteryCount = 0;
        foreach (const VariantDict &batteryData, m_batteriesData) {
            if (m_numOfBattery == 1) {
                batteriesLabel.append(i18n("Battery:"));
            } else {
                //kDebug() << "More batteries ...";
                if (!batteriesInfo.isEmpty()) {
                    batteriesLabel.append('\n');
                    batteriesInfo.append('\n');
                }
                batteriesLabel.append(i18nc("Placeholder is the battery ID", "Battery %1:", batteryCount));
            }

            batteriesInfo.append(stringForState(batteryData, &showRemainingTime));
            ++batteryCount;
        }

        m_acLabelLabel->setText(i18n("AC Adapter:"));
        if (m_acAdapterPlugged) {
            m_acInfoLabel->setText(i18n("<b>Plugged in</b>"));
        } else {
            m_acInfoLabel->setText(i18n("<b>Not plugged in</b>"));
        }

        if (batteryCount && showRemainingTime && m_remainingMSecs > 0) {
            m_remainingTimeLabel->show();
            m_remainingInfoLabel->show();
            // we don't have too much accuracy so only give hours and minutes
            int remainingMinutes = m_remainingMSecs - (m_remainingMSecs % 60000);
            QString remText = QString("%1%2%3").arg("<b>", KGlobal::locale()->prettyFormatDuration(remainingMinutes), "</b>");
            m_remainingInfoLabel->setText(remText);
        } else {
            m_remainingTimeLabel->hide();
            m_remainingInfoLabel->hide();
        }
    } else {
        batteriesLabel = i18n("Battery:");
        batteriesInfo = i18nc("Battery is not plugged in", "<b>Not present</b>");
    }

    if (!batteriesInfo.isEmpty()) {
        m_batteryInfoLabel->setText(batteriesInfo);
        m_batteryLabelLabel->setText(batteriesLabel);
        kDebug() << batteriesInfo;
        kDebug() << batteriesLabel;
    }

    if (m_brightnessSlider) {
        updateSlider();
    }
}

void Battery::openConfig()
{
    //kDebug() << "opening powermanagement configuration dialog";
    QStringList args;
    args << QLatin1String("--icon")
        << QLatin1String("preferences-system-power-management")
        << QLatin1String("powerdevilglobalconfig")
        << QLatin1String("powerdevilprofilesconfig");
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

void Battery::showLabel(bool show)
{
    m_labelAnimation->setDirection(show ? QAbstractAnimation::Forward
                                        : QAbstractAnimation::Backward);
    if (m_labelAnimation->state() != QAbstractAnimation::Running) {
        m_labelAnimation->start();
    }
}

void Battery::showAcAdapter(bool show)
{
    if (m_numOfBattery == 0) {
        return;
    }

    QAbstractAnimation::Direction d = show ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
    if (d == m_acAnimation->direction()) {
        return;
    }

    m_acAnimation->setDirection(d);
    if (m_acAnimation->state() != QAbstractAnimation::Running) {
        m_acAnimation->start();
    }
}

QRectF Battery::scaleRectF(const qreal progress, QRectF rect)
{
    if (qFuzzyCompare(progress, qreal(1))) {
        return rect;
    }

    // Scale
    qreal w = rect.width() * progress;
    qreal h = rect.width() * progress;

    // Position centered
    rect.setX(rect.x() + (rect.width() - w) / 2);
    rect.setY(rect.y() + (rect.height() - h) / 2);

    rect.setWidth(w);
    rect.setHeight(h);

    return rect;
}


QFont Battery::setupLabelPainting(const QRect &contentsRect, const QString &labelText)
{
    // Fonts smaller than smallestReadableFont don't make sense.
    QFont font = m_font;
    int orginalPointSize = font.pointSize();
    font.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(), font.pointSize()));
    QFontMetrics fm(font);
    qreal textWidth = fm.width(labelText);

    // Longer texts get smaller fonts, h/v constrained gets the small font we have
    if (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical) {
        font = KGlobalSettings::smallestReadableFont();
        fm = QFontMetrics(font);
        textWidth = (fm.width(labelText)+8);
    } else if (labelText.length() > 4) {
        if (orginalPointSize/1.5 < KGlobalSettings::smallestReadableFont().pointSize()) {
            font.setPointSize((KGlobalSettings::smallestReadableFont().pointSize()));
        } else {
            font.setPointSizeF(orginalPointSize/1.5);
        }
        fm = QFontMetrics(font);
        textWidth = (fm.width(labelText) * 1.2);
    } else {
        // Smaller texts get a wider box
        textWidth = (textWidth * 1.4);
    }

    // Let's find a good position for painting the percentage on top of the battery
    m_textRect = QRectF((contentsRect.left() + (contentsRect.width() - textWidth) / 2),
                         contentsRect.top() + ((contentsRect.height() - (int)fm.height()) / 2 * 0.9),
                         (int)(textWidth),
                         fm.height() * 1.1);
    return font;
}

void Battery::paintLabel(QPainter *p, const QRect &contentsRect, const QString &labelText)
{
    // Store font size, we want to restore it shortly
    QFont font = setupLabelPainting(contentsRect, labelText);
    p->setFont(font);
    // Poor man's highlighting
    m_boxColor.setAlphaF(m_labelAlpha);
    p->setPen(m_boxColor);
    m_boxColor.setAlphaF(m_labelAlpha*0.5);
    p->setBrush(m_boxColor);

    // Find sensible proportions for the rounded corners
    float round_prop = m_textRect.width() / m_textRect.height();

    // Tweak the rounding edge a bit with the proportions of the textbox
    qreal round_radius = 35.0;
    p->drawRoundedRect(m_textRect, round_radius / round_prop, round_radius, Qt::RelativeSize);

    m_textColor.setAlphaF(m_labelAlpha);
    p->setPen(m_textColor);
    p->drawText(m_textRect, Qt::AlignCenter, labelText);
}

void Battery::paintBattery(QPainter *p, const QRect &contentsRect, const int batteryPercent, const bool plugState)
{
    int minSize = qMin(contentsRect.height(), contentsRect.width());
    QRect contentsSquare = QRect(contentsRect.x() + (contentsRect.width() - minSize) / 2, contentsRect.y() + (contentsRect.height() - minSize) / 2, minSize, minSize);

    if (m_theme->hasElement("Battery")) {
        m_theme->paint(p, contentsSquare, "Battery");
    }

    QString fill_element;
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
            m_theme->paint(p, contentsSquare, fill_element);
        } else {
            kDebug() << fill_element << " does not exist in svg";
        }
    }

    if (!qFuzzyCompare(1, m_acAlpha + 1)) {
        m_theme->paint(p, scaleRectF(m_acAlpha, contentsSquare), "AcAdapter");
    }

    if (plugState && m_theme->hasElement("Overlay")) {
        m_theme->paint(p, contentsSquare, "Overlay");
    }
}

void Battery::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    if (m_numOfBattery == 0) {
        paintBattery(p, contentsRect, 0, false);
        return;
    }

    if (m_isEmbedded || m_showMultipleBatteries) {
        // paint each battery with own charge level
        int battery_num = 0;
        int height = contentsRect.height();
        int width = contentsRect.width();
        int xdelta = 0;
        int ydelta = 0;
        if (m_showMultipleBatteries) {
            if (formFactor() == Plasma::Vertical) {
                height = height / m_numOfBattery;
                ydelta = height;
            } else {
                width = width / m_numOfBattery;
                xdelta = width;
            }
        }
        QHashIterator<QString, QHash<QString, QVariant > > batteryData(m_batteriesData);
        while (batteryData.hasNext()) {
            batteryData.next();
            QRect corect = QRect(contentsRect.left() + battery_num * xdelta,
                                 contentsRect.top() + battery_num * ydelta,
                                 width, height);
            // paint battery with appropriate charge level
            paintBattery(p, corect, batteryData.value()["Percent"].toInt(), batteryData.value()["Plugged in"].toBool());

            if (!isConstrained()) {
                // Show the charge percentage with a box on top of the battery
                QString batteryLabel;
                if (batteryData.value()["Plugged in"].toBool()) {
                    int hours = m_remainingMSecs/1000/3600;
                    int minutes = qRound(m_remainingMSecs/60000) % 60;
                    if (!(minutes==0 && hours==0)) {
                        m_minutes= minutes;
                        m_hours= hours;
                    }
                    QString state = batteryData.value()["State"].toString();

                    if (m_remainingMSecs > 0 && (m_showRemainingTime && (state=="Charging" || state=="Discharging"))) {
                        QTime t = QTime(m_hours, m_minutes);
                        KLocale tmpLocale(*KGlobal::locale());
                        tmpLocale.setTimeFormat("%k:%M");
                        batteryLabel = tmpLocale.formatTime(t, false, true); // minutes, hours as duration
                    } else {
                        batteryLabel = i18nc("overlay on the battery, needs to be really tiny", "%1%", batteryData.value()["Percent"].toString());
                    }
                    paintLabel(p, corect, batteryLabel);
                }
            }
            ++battery_num;
        }
    } else {
        // paint only one battery and show cumulative charge level
        int batteryNum = 0;
        int batteryCharge = 0;
        bool hasBattery = false;
        QHashIterator<QString, QHash<QString, QVariant > > batteryData(m_batteriesData);
        while (batteryData.hasNext()) {
            batteryData.next();
            if (batteryData.value()["Plugged in"].toBool()) {
                batteryCharge += batteryData.value()["Percent"].toInt();
                hasBattery = true;
                ++batteryNum;
            }
        }

        if (batteryNum > 0) {
            batteryCharge = batteryCharge / batteryNum;
        }

        // paint battery with appropriate charge level
        paintBattery(p, contentsRect,  batteryCharge, hasBattery);

        // Show the charge percentage with a box on top of the battery
        if (hasBattery && (m_showBatteryLabel || !isConstrained())) {
            QString batteryLabel = i18nc("overlay on the battery, needs to be really tiny", "%1%", batteryCharge);
            paintLabel(p, contentsRect, batteryLabel);
        }
    }
}

void Battery::setShowBatteryLabel(bool show)
{
    if (show != m_showBatteryLabel) {
        m_showBatteryLabel = show;
        //FIXME this is not correct for multiple batteries shown: it needs to figure this out for
        //each and ever battery shown
        QString batteryLabel = i18nc("overlay on the battery, needs to be really tiny", "%1%", 99);
        setupLabelPainting(contentsRect().toRect(), batteryLabel);
        constraintsEvent(Plasma::FormFactorConstraint);
        showLabel(show);
    }
}

void Battery::connectSources()
{
    const QStringList& battery_sources = dataEngine("powermanagement")->query("Battery")["Sources"].toStringList();

    foreach (const QString &battery_source, battery_sources) {
        dataEngine("powermanagement")->connectSource(battery_source, this);
    }

    dataEngine("powermanagement")->connectSource("AC Adapter", this);
    dataEngine("powermanagement")->connectSource("PowerDevil", this);
    dataEngine("powermanagement")->connectSource("Battery", this);

    connect(dataEngine("powermanagement"), SIGNAL(sourceAdded(QString)),
            this,                          SLOT(sourceAdded(QString)));
    connect(dataEngine("powermanagement"), SIGNAL(sourceRemoved(QString)),
            this,                          SLOT(sourceRemoved(QString)));
}

void Battery::sourceAdded(const QString& source)
{
    if (source.startsWith(QLatin1String("Battery")) && source != "Battery") {
        dataEngine("powermanagement")->connectSource(source, this);
        m_numOfBattery++;
        constraintsEvent(Plasma::SizeConstraint);
        update();
    } else if (source == "PowerDevil") {
        dataEngine("powermanagement")->connectSource(source, this);
    }
}

void Battery::sourceRemoved(const QString& source)
{
    if (m_batteriesData.remove(source)) {
        m_numOfBattery--;
        constraintsEvent(Plasma::SizeConstraint);
        update();
    } else if (source == "PowerDevil") {
        dataEngine("powermanagement")->disconnectSource(source, this);
    }
}

void Battery::setLabelAlpha(qreal alpha)
{
    m_labelAlpha = alpha;
    update();
}

qreal Battery::labelAlpha() const
{
    return m_labelAlpha;
}

void Battery::setAcAlpha(qreal alpha)
{
    m_acAlpha = alpha;
    update();
}

QList<QAction*> Battery::contextualActions()
{
    QList<QAction *> rv;
    QAction *act = action("configure_powersave");
    rv << act;
    return rv;
}

qreal Battery::acAlpha() const
{
    return m_acAlpha;
}

void Battery::toggleInhibit(bool toggle)
{
    using namespace Solid::PowerManagement;

    if (m_inhibitCookies.first > 0 && m_inhibitCookies.second > 0 && !toggle) {
        // Release inhibition
        stopSuppressingSleep(m_inhibitCookies.first);
        stopSuppressingScreenPowerManagement(m_inhibitCookies.second);

        m_inhibitCookies = qMakePair< int, int >(-1, -1);
    } else if (m_inhibitCookies.first < 0 && m_inhibitCookies.second < 0 && toggle) {
        // Trigger inhibition
        QString reason = i18n("The battery applet has enabled system-wide inhibition");
        m_inhibitCookies = qMakePair< int, int >(beginSuppressingSleep(reason),
                                                 beginSuppressingScreenPowerManagement(reason));
    } else {
        kWarning() << "The requested action conflicts with the current inhibition state";
    }
}

#include "battery.moc"
