/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <riccardo@kde.org>           *
 *   Copyright (C) 2007 by Sebastian Kuegler <sebas@kde.org>               *
 *   Copyright (C) 2007 by Luka Renko <lure@kubuntu.org>                   *
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
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QGraphicsSceneHoverEvent>

#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KDialog>
#include <KColorScheme>
#include <KConfigDialog>
#include <KGlobalSettings>

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/phase.h>

Battery::Battery(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_batteryStyle(0),
      m_smallPixelSize(22),
      m_theme(0),
      m_dialog(0),
      m_animId(-1),
      m_alpha(1),
      m_fadeIn(true),
      m_acAnimId(-1),
      m_acAlpha(1),
      m_acFadeIn(false),
      m_batteryAnimId(-1),
      m_batteryAlpha(1),
      m_batteryFadeIn(true),
      m_isHovered(false),
      m_numOfBattery(0)
{
    kDebug() << "Loading applet battery";
    setAcceptsHoverEvents(true);
    setHasConfigurationInterface(true);
    // TODO: minimum size causes size on panel to be huge (do not use for now)
    //setMinimumContentSize(m_smallPixelSize, m_smallPixelSize);
    resize(64, 64);
}

void Battery::init()
{
    KConfigGroup cg = config();
    m_showBatteryString = cg.readEntry("showBatteryString", false);
    m_showMultipleBatteries = cg.readEntry("showMultipleBatteries", true);
    m_drawBackground = cg.readEntry("drawBackground", true);

    // TODO: set background on panel causes 0 height, so do not use it
    if (formFactor() != Plasma::Vertical && formFactor() != Plasma::Horizontal) {
        setDrawStandardBackground(m_drawBackground);
    }
    setWindowFlags(Qt::Window);

    QString svgFile = QString();
    if (cg.readEntry("style", 0) == 0) {
        m_batteryStyle = OxygenBattery;
        svgFile = "widgets/battery-oxygen";
    } else {
        m_batteryStyle = ClassicBattery;
        svgFile = "widgets/battery";
    }
    m_theme = new Plasma::Svg(svgFile, this);
    m_theme->setContentType(Plasma::Svg::SingleImage);
    m_theme->resize(geometry().size());

    m_font = QApplication::font();
    m_font.setWeight(QFont::Bold);

    m_boxAlpha = 128;
    m_boxHoverAlpha = 192;

    readColors();
    connect(Plasma::Theme::self(), SIGNAL(changed()), SLOT(readColors()));

    const QStringList& battery_sources = dataEngine("powermanagement")->query(I18N_NOOP("Battery"))[I18N_NOOP("sources")].toStringList();
    m_numOfBattery = battery_sources.size();

    //connect sources
    connectSources();
    
    foreach (QString battery_source, battery_sources) {
        dataUpdated(battery_source, dataEngine("powermanagement")->query(battery_source));
    }
    dataUpdated(I18N_NOOP("AC Adapter"), dataEngine("powermanagement")->query(I18N_NOOP("AC Adapter")));
}

void Battery::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Vertical) {
            kDebug() << "Vertical FormFactor";
            // TODO: set background(true) on panel causes 0 height, so do not use it
            setDrawStandardBackground(false);
        } else if (formFactor() == Plasma::Horizontal) {
            kDebug() << "Horizontal FormFactor";
            // TODO: set background(true) on panel causes 0 height, so do not use it
            setDrawStandardBackground(false);
        } else if (formFactor() == Plasma::Planar) {
            kDebug() << "Planar FormFactor";
            setDrawStandardBackground(m_drawBackground);
        } else if (formFactor() == Plasma::MediaCenter) {
            kDebug() << "MediaCenter FormFactor";
            setDrawStandardBackground(m_drawBackground);
        } else {
            kDebug() << "Other FormFactor" << formFactor();
            setDrawStandardBackground(m_drawBackground);
        }
    }

    if (constraints & Plasma::SizeConstraint && m_theme) {
        m_theme->resize(geometry().size().toSize());
    }
}


void Battery::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    if (source.startsWith(I18N_NOOP("Battery"))) {
        m_batteries_data[source] = data;
    } else if (source == I18N_NOOP("AC Adapter")) {
        m_acadapter_plugged = data[I18N_NOOP("Plugged in")].toBool();
        showAcAdapter(m_acadapter_plugged);
    } else {
        kDebug() << "Applet::Dunno what to do with " << source;
    }
    update();
}

void Battery::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, parent->windowTitle(), "battery");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    ui.styleGroup->setSelected(m_batteryStyle);
    ui.showBatteryStringCheckBox->setChecked(m_showBatteryString ? Qt::Checked : Qt::Unchecked);
    ui.showMultipleBatteriesCheckBox->setChecked(m_showMultipleBatteries ? Qt::Checked : Qt::Unchecked);
    ui.drawBackgroundCheckBox->setChecked(m_drawBackground ? Qt::Checked : Qt::Unchecked);
}

void Battery::configAccepted()
{
    KConfigGroup cg = config();
    m_showBatteryString = ui.showBatteryStringCheckBox->checkState() == Qt::Checked;
    showLabel(m_showBatteryString);
    cg.writeEntry("showBatteryString", m_showBatteryString);

    bool old_showMultipleBatteries = m_showMultipleBatteries;
    m_showMultipleBatteries = ui.showMultipleBatteriesCheckBox->checkState() == Qt::Checked;
    cg.writeEntry("showMultipleBatteries", m_showMultipleBatteries);

    m_drawBackground = ui.drawBackgroundCheckBox->checkState() == Qt::Checked;
    cg.writeEntry("drawBackground", m_drawBackground);

    // TODO: set background on panel causes 0 height, so do not use it
    if (formFactor() != Plasma::Vertical && formFactor() != Plasma::Horizontal) {
        setDrawStandardBackground(m_drawBackground);
    }

    if (ui.styleGroup->selected() != m_batteryStyle) {
        QString svgFile = QString();
        if (ui.styleGroup->selected() == OxygenBattery) {
            svgFile = "widgets/battery-oxygen";
        } else {
            svgFile = "widgets/battery";
        }
        if (m_acadapter_plugged) {
            showAcAdapter(false);
        }
        showBattery(false);
        m_batteryStyle = ui.styleGroup->selected();
        delete m_theme;
        m_theme = new Plasma::Svg(svgFile, this);
        kDebug() << "Changing theme to " << svgFile;
        cg.writeEntry("style", m_batteryStyle);
        m_theme->resize(geometry().size());
        if (m_acadapter_plugged) {
            showAcAdapter(true);
        }
        showBattery(true);
    }

    if (m_numOfBattery > 1 && old_showMultipleBatteries != m_showMultipleBatteries) {
        kDebug() << "Show multiple battery changed: " << m_showMultipleBatteries;
        updateGeometry();
    }

    //reconnect sources
    disconnectSources();
    connectSources();

    update();
    emit configNeedsSaving();
}

void Battery::readColors()
{
    m_textColor = Plasma::Theme::self()->textColor();
    m_boxColor = Plasma::Theme::self()->backgroundColor();
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
    if (!m_showBatteryString) {
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

void Battery::showLabel(bool show)
{
    if (m_fadeIn == show) {
        return;
    }
    m_fadeIn = show;
    const int FadeInDuration = 150;

    if (m_animId != -1) {
        Plasma::Phase::self()->stopCustomAnimation(m_animId);
    }
    m_animId = Plasma::Phase::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,
                                                      Plasma::Phase::EaseOutCurve, this,
                                                      "animationUpdate");
}

QRectF Battery::scaleRectF(const qreal progress, QRectF rect) {
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
    const int FadeInDuration = 300;
    // As long as the animation is running, we fake it's still plugged in so it gets
    // painted in paintInterface()
    m_acadapter_plugged = true;

    if (m_acAnimId != -1) {
        Plasma::Phase::self()->stopCustomAnimation(m_acAnimId);
    }
    m_acAnimId = Plasma::Phase::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,
                                                      Plasma::Phase::EaseOutCurve, this,
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
        Plasma::Phase::self()->stopCustomAnimation(m_batteryAnimId);
    }
    m_batteryAnimId = Plasma::Phase::self()->customAnimation(40 / (1000 / FadeInDuration), FadeInDuration,
                                                      Plasma::Phase::EaseOutCurve, this,
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

    // Let's find a good position for painting the background
    QRectF text_rect = QRectF(qMax(0.0, contentsRect.left() + (contentsRect.width() - text_width) / 2),
                            contentsRect.top() + ((contentsRect.height() - (int)fm.height()) / 2 * 0.9),
                            qMin(geometry().width(), text_width),
                            fm.height() * 1.2 );

    // Poor man's highlighting
    m_boxColor.setAlphaF(m_alpha);
    p->setPen(m_boxColor);
    m_boxColor.setAlphaF(m_alpha*0.5);
    p->setBrush(m_boxColor);

    // Find sensible proportions for the rounded corners
    float round_prop = text_rect.width() / text_rect.height();

    // Tweak the rounding edge a bit with the proportions of the textbox
    qreal round_radius = 35.0;
    p->drawRoundedRect(text_rect, round_radius / round_prop, round_radius, Qt::RelativeSize);

    m_textColor.setAlphaF(m_alpha);
    p->setPen(m_textColor);
    p->drawText(text_rect, Qt::AlignCenter, labelText);

    // Reset font and box
    m_font.setPointSize(original_font_size);
    m_boxColor.setAlpha(m_boxAlpha);
}

void Battery::paintBattery(QPainter *p, const QRect &contentsRect, const int batteryPercent, const bool plugState)
{
    QString fill_element = QString();
    if (plugState && m_theme->elementExists("Battery")) {
        m_theme->paint(p, scaleRectF(m_batteryAlpha, contentsRect), "Battery");
    
        if (m_batteryStyle == OxygenBattery) {
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
        } else { // OxyenStyle
            if (batteryPercent > 95) {
                fill_element = "Fill100";
            } else if (batteryPercent > 90) {
                fill_element = "Fill90";
            } else if (batteryPercent > 80) {
                fill_element = "Fill80";
            } else if (batteryPercent > 70) {
                fill_element = "Fill70";
            } else if (batteryPercent > 55) {
                fill_element = "Fill60";
            } else if (batteryPercent > 40) {
                fill_element = "Fill50";
            } else if (batteryPercent > 30) {
                fill_element = "Fill40";
            } else if (batteryPercent > 20) {
                fill_element = "Fill30";
            } else if (batteryPercent > 10) {
                fill_element = "Fill20";
            } else if (batteryPercent >= 5) {
                fill_element = "Fill10";
            } // Lower than 5%? Show no fillbar.
        }
    }
    //kDebug() << "plugState:" << plugState;

    // Now let's find out which fillstate to show
    if (plugState && !fill_element.isEmpty()) {
        if (m_theme->elementExists(fill_element)) {
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
    if (plugState && m_theme->elementExists("Overlay")) {
        m_theme->paint(p, scaleRectF(m_batteryAlpha, contentsRect), "Overlay");
    }
}

void Battery::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    if (m_numOfBattery == 0) {
        QRectF ac_contentsRect(contentsRect.topLeft(), QSizeF(contentsRect.width() * m_acAlpha, contentsRect.height() * m_acAlpha));
        if (m_acadapter_plugged) {
            m_theme->paint(p, ac_contentsRect, "AcAdapter");
        }
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            // Show that there's no battery
            paintLabel(p, contentsRect, I18N_NOOP("n/a"));
        }
        return;
    }
   
    if (m_showMultipleBatteries) {
        // paint each battery with own charge level
        int battery_num = 0;
        int width = contentsRect.width()/m_numOfBattery;
        QHashIterator<QString, QHash<QString, QVariant > > battery_data(m_batteries_data);
        while (battery_data.hasNext()) {
            battery_data.next();
            QRect corect = QRect(contentsRect.left()+battery_num*width, 
                                 contentsRect.top(), 
                                 width, geometry().size().toSize().height());
       
            // paint battery with appropriate charge level
            paintBattery(p, corect, battery_data.value()[I18N_NOOP("Percent")].toInt(), battery_data.value()[I18N_NOOP("Plugged in")].toBool());
                
            if (m_showBatteryString || m_isHovered) {
                // Show the charge percentage with a box on top of the battery
                QString batteryLabel;
                if (battery_data.value()[I18N_NOOP("Plugged in")].toBool()) {
                    batteryLabel = battery_data.value()[I18N_NOOP("Percent")].toString();
                    batteryLabel.append("%");
                } else {
                    batteryLabel = I18N_NOOP("n/a");
                }
                paintLabel(p, corect, batteryLabel);
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
            if (battery_data.value()[I18N_NOOP("Plugged in")].toBool()) {
                battery_charge += battery_data.value()[I18N_NOOP("Percent")].toInt();
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
            if(has_battery) {
                batteryLabel = QString::number(battery_charge);
                batteryLabel.append("%");
            } else {
                batteryLabel = I18N_NOOP("n/a");
            }
            paintLabel(p, contentsRect, batteryLabel);
        }
    }
}

void Battery::connectSources() {
    const QStringList& battery_sources = dataEngine("powermanagement")->query(I18N_NOOP("Battery"))[I18N_NOOP("sources")].toStringList();
    
    foreach (QString battery_source, battery_sources) {
        dataEngine("powermanagement")->connectSource(battery_source, this);
    }
    
    dataEngine("powermanagement")->connectSource(I18N_NOOP("AC Adapter"), this);
}

void Battery::disconnectSources()
{
    const QStringList& battery_sources = dataEngine("powermanagement")->query(I18N_NOOP("Battery"))[I18N_NOOP("sources")].toStringList();
    
    foreach (QString battery_source ,battery_sources) {
        dataEngine("powermanagement")->disconnectSource(battery_source, this);
    }
    
    dataEngine("powermanagement")->disconnectSource(I18N_NOOP("AC Adapter"), this);
}

#include "battery.moc"
