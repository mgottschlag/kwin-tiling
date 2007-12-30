/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <riccardo@kde.org>           *
 *   Copyright (C) 2007 by Sebastian Kuegler <sebas@kde.org>               *
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
#include <KGlobalSettings>

#include <plasma/svg.h>
#include <plasma/theme.h>

Battery::Battery(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_batteryStyle(0),
      m_theme(0),
      m_dialog(0),
      m_isHovered(0),
      m_numOfBattery(0)
{
    kDebug() << "Loading applet battery";
    setAcceptsHoverEvents(true);
    setHasConfigurationInterface(true);
    setContentSize(200,200);
}

void Battery::init()
{
    KConfigGroup cg = config();
    m_showBatteryString = cg.readEntry("showBatteryString", false);
    m_drawBackground = cg.readEntry("drawBackground", true);
    setDrawStandardBackground(m_drawBackground);

    QString svgFile = QString();
    if (cg.readEntry("style", 0) == 0) {
        m_batteryStyle = OxygenBattery;
        svgFile = "widgets/battery-oxygen";
    } else {
        m_batteryStyle = ClassicBattery;
        svgFile = "widgets/battery";
    }
    m_smallPixelSize = 44;
    m_theme = new Plasma::Svg(svgFile, this);
    m_theme->setContentType(Plasma::Svg::SingleImage);
    m_theme->resize(contentSize());
    setMinimumContentSize(m_smallPixelSize, m_smallPixelSize);
    setContentSize(m_smallPixelSize, m_smallPixelSize);

    m_font = QApplication::font();
    m_font.setWeight(QFont::Bold);

    m_textColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::self()->colors()).foreground().color();
    m_boxColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::self()->colors()).background().color();

    m_boxAlpha = 128;
    m_boxHoverAlpha = 192;
    m_boxColor.setAlpha(m_boxAlpha);
    
    const QStringList& battery_sources = dataEngine("powermanagement")->query(I18N_NOOP("Battery"))[I18N_NOOP("sources")].toStringList();
    m_numOfBattery = battery_sources.size();

    //connect sources
    connectSources();
    
    foreach(QString battery_source, battery_sources) {
        dataUpdated(battery_source, dataEngine("powermanagement")->query(battery_source));
    }
    dataUpdated(I18N_NOOP("AC Adapter"), dataEngine("powermanagement")->query(I18N_NOOP("AC Adapter")));

    setAcceptsHoverEvents(true);
}

void Battery::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Vertical ||
            formFactor() == Plasma::Horizontal) {
            kDebug() << "Horizontal or Vertical FormFactor";
        } else {
            kDebug() << "Other FormFactor";
        }
    }

    if (constraints & Plasma::SizeConstraint && m_theme) {
        m_theme->resize(contentSize().toSize());
        update();
        // Save new size to config
        KConfigGroup cg = config();
        cg.writeEntry("size", (int)(boundingRect().height()));
    }
    updateGeometry();
}

QSizeF Battery::contentSizeHint() const
{
    QSizeF sizeHint = contentSize();
    switch (formFactor()) {
        case Plasma::Vertical:
            sizeHint.setHeight(sizeHint.width()*m_numOfBattery);
            break;
        case Plasma::Horizontal:
        case Plasma::Planar:
            sizeHint.setWidth(sizeHint.height()*m_numOfBattery);
            break;
        default:
            break;
    }
    return sizeHint;
}

Qt::Orientations Battery::expandingDirections() const
{
    return 0;
}

void Battery::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    if (source.startsWith(I18N_NOOP("Battery"))) {
        int battery_percent = data[I18N_NOOP("Percent")].toInt();
        QString battery_percent_label = data[I18N_NOOP("Percent")].toString();
        battery_percent_label.append("%");
        m_batteries_data[source] = qMakePair(battery_percent, battery_percent_label);
        kDebug() << "Applet::Battery::dataUpdated " << m_batteries_data[source].first;
    } else if (source == I18N_NOOP("AC Adapter")) {
        m_acadapter_plugged = data[I18N_NOOP("Plugged in")].toBool();
        kDebug() << "Applet::AC Adapter dataUpdated: " << m_acadapter_plugged;
    } else {
        kDebug() << "Applet::Dunno what to do with " << source;
    }
    update();
}

void Battery::showConfigurationInterface()
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Configure Battery Monitor"));

        QWidget *widget = new QWidget;
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );

    }

    ui.styleGroup->setSelected(m_batteryStyle);

    ui.showBatteryStringCheckBox->setChecked(m_showBatteryString ? Qt::Checked : Qt::Unchecked);
    ui.drawBackgroundCheckBox->setChecked(m_drawBackground ? Qt::Checked : Qt::Unchecked);
    m_dialog->show();
}

void Battery::configAccepted()
{
    KConfigGroup cg = config();
    m_showBatteryString = ui.showBatteryStringCheckBox->checkState() == Qt::Checked;
    cg.writeEntry("showBatteryString", m_showBatteryString);

    m_drawBackground = ui.drawBackgroundCheckBox->checkState() == Qt::Checked;
    setDrawStandardBackground(m_drawBackground);
    cg.writeEntry("drawBackground", m_drawBackground);

    if (ui.styleGroup->selected() != m_batteryStyle) {
        QString svgFile = QString();
        if (ui.styleGroup->selected() == OxygenBattery) {
            svgFile = "widgets/battery-oxygen";
        } else {
            svgFile = "widgets/battery";
        }
        m_batteryStyle = ui.styleGroup->selected();
        delete m_theme;
        m_theme = new Plasma::Svg(svgFile, this);
        kDebug() << "Changing theme to " << svgFile;
        cg.writeEntry("style", m_batteryStyle);
        m_theme->resize(contentSize());
    }

    //reconnect sources
    disconnectSources();
    connectSources();

    //constraintsUpdated(Plasma::AllConstraints);
    update();
    cg.config()->sync();
}

void Battery::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    update();
    Applet::hoverEnterEvent(event);
}

void Battery::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    update();
    Applet::hoverLeaveEvent(event);
}

Battery::~Battery()
{
}

void Battery::paintLabel(QPainter *p, const QRect &contentsRect, const QString& labelText)
{
    // Store font size, we want to restore it shortly
    int original_font_size = m_font.pointSize();

    // Fonts smaller than smallestReadableFont don't make sense.
    m_font.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(), m_font.pointSize()));
    QFontMetrics fm(m_font);
    int text_width = fm.width(labelText);

    // Longer texts get smaller fonts
    if (labelText.length() > 4) {
        if (original_font_size/1.5 < KGlobalSettings::smallestReadableFont().pointSize()) {
            m_font.setPointSize((int)(KGlobalSettings::smallestReadableFont().pointSize()));
        } else {
            m_font.setPointSize((int)(original_font_size/1.5));
        }
        fm = QFontMetrics(m_font);
        text_width = (int)(fm.width(labelText) * 1.2);
    } else {
        // Smaller texts get a wider box
        text_width = (int)(text_width * 1.4);
    }
    if (formFactor() == Plasma::Horizontal ||
        formFactor() == Plasma::Vertical) {
        m_font = KGlobalSettings::smallestReadableFont();
        m_font.setWeight(QFont::Bold);
        fm = QFontMetrics(m_font);
        text_width = (int)(fm.width(labelText)+8);
    } 
    p->setFont(m_font);

    // Let's find a good position for painting the background
    QRect text_rect = QRect(contentsRect.left()+(contentsRect.width()-text_width)/2,
                            contentsRect.top()+((contentsRect.height() - (int)fm.height())/2*0.9),
                            text_width,
                            (int)(fm.height()*1.2));

    // Poor man's highlighting
    if (m_isHovered) {
        m_boxColor.setAlpha(m_boxHoverAlpha);
    }
    p->setBrush(m_boxColor);

    // Find sensible proportions for the rounded corners
    float round_prop = text_rect.width() / text_rect.height();

    // Tweak the rounding edge a bit with the proportions of the textbox
    int round_radius = 35;
    p->drawRoundRect(text_rect, (int)(round_radius/round_prop), round_radius);

    p->setBrush(m_textColor);
    p->drawText(text_rect, Qt::AlignCenter, labelText);

    // Reset font and box
    m_font.setPointSize(original_font_size);
    m_boxColor.setAlpha(m_boxAlpha);
}

void Battery::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );

    bool showString = true;

    if (formFactor() == Plasma::Vertical ||
        formFactor() == Plasma::Horizontal) {
        showString = false;
    };

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    if (m_numOfBattery == 0) {
        m_theme->paint(p, contentsRect, "AcAdapter");
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            // Show that there's no battery
            paintLabel(p, contentsRect, I18N_NOOP("0"));
        }
        return;
    }
    
    int width = contentsRect.width()/m_numOfBattery;
    int battery_num = 0;
    QMap<QString, QPair<int, QString > >::iterator it_battery_data;
    for (it_battery_data = m_batteries_data.begin(); 
         it_battery_data != m_batteries_data.end();
         ++it_battery_data)
    {
        QRect corect = QRect(contentsRect.left()+battery_num*width, contentsRect.top(), width, contentSizeHint().toSize().height());
    
        if (m_theme->elementExists("Battery")) {
            m_theme->paint(p, corect, "Battery");
        }
    
        // Now let's find out which fillstate to show
        QString fill_element = QString();
    
          if (m_batteryStyle == OxygenBattery) {
              if (it_battery_data->first > 95) {
                fill_element = "Fill100";
            } else if (it_battery_data->first > 80) {
                fill_element = "Fill80";
            } else if (it_battery_data->first > 50) {
                fill_element = "Fill60";
            } else if (it_battery_data->first > 20) {
                fill_element = "Fill40";
            } else if (it_battery_data->first > 10) {
                fill_element = "Fill20";
            } // Don't show a fillbar below 11% charged
        } else { // OxyenStyle
            if (it_battery_data->first > 95) {
                fill_element = "Fill100";
            } else if (it_battery_data->first > 90) {
                fill_element = "Fill90";
            } else if (it_battery_data->first > 80) {
                fill_element = "Fill80";
            } else if (it_battery_data->first > 70) {
                fill_element = "Fill70";
            } else if (it_battery_data->first > 55) {
                fill_element = "Fill60";
            } else if (it_battery_data->first > 40) {
                fill_element = "Fill50";
            } else if (it_battery_data->first > 30) {
                fill_element = "Fill40";
            } else if (it_battery_data->first > 20) {
                fill_element = "Fill30";
            } else if (it_battery_data->first > 10) {
                fill_element = "Fill20";
            } else if (it_battery_data->first >= 5) {
                fill_element = "Fill10";
            } // Lower than 5%? Show no fillbar.
        }
        if (!fill_element.isEmpty()) {
            if (m_theme->elementExists(fill_element)) {
                m_theme->paint(p, corect, fill_element);
            } else {
                kDebug() << fill_element << " does not exist in svg";
            }
        }
        
    

        if (m_acadapter_plugged) {
            m_theme->paint(p, corect, "AcAdapter");
        }
    
        // Only show batterystring when we're huge
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            showString = m_showBatteryString;
        }
    
        // For small FormFactors, we're drawing a shadow,
        // but no text.
        if (formFactor() == Plasma::Vertical ||
            formFactor() == Plasma::Horizontal) {
            m_theme->paint(p, corect, "Shadow");
        }
        if (m_theme->elementExists("Overlay")) {
            m_theme->paint(p, corect, "Overlay");
        }
    
        if (showString || m_isHovered) {
            // Show the charge percentage with a box
            // on top of the battery, but only for plasmoids bigger than ....
            if (width >= 44) {
                paintLabel(p, corect, it_battery_data->second);
            }
        }
        
        ++battery_num;
    }
}

void Battery::connectSources() {
    const QStringList& battery_sources = dataEngine("powermanagement")->query(I18N_NOOP("Battery"))[I18N_NOOP("sources")].toStringList();
    
    foreach(QString battery_source ,battery_sources) {
        dataEngine("powermanagement")->connectSource(battery_source, this);
        kDebug() << "Battery Applet:: " << battery_source <<" connected";
    }
    
    dataEngine("powermanagement")->connectSource(I18N_NOOP("AC Adapter"), this);
}

void Battery::disconnectSources()
{
    const QStringList& battery_sources = dataEngine("powermanagement")->query(I18N_NOOP("Battery"))[I18N_NOOP("sources")].toStringList();
    
    foreach(QString battery_source ,battery_sources) {
        dataEngine("powermanagement")->disconnectSource(battery_source, this);
        kDebug() << "Battery Applet:: " << battery_source <<" disconnected";
    }
    
    dataEngine("powermanagement")->disconnectSource(I18N_NOOP("AC Adapter"), this);
}

#include "battery.moc"
