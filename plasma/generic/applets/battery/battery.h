/***************************************************************************
 *   Copyright (C) 2007-2009 by Sebastian Kuegler <sebas@kde.org>          *
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

#ifndef BATTERY_H
#define BATTERY_H

#include <QGraphicsSceneHoverEvent>
#include <QGraphicsGridLayout>
#include <QPropertyAnimation>

#include <Plasma/Applet>
#include <Plasma/Animator>
#include <Plasma/DataEngine>
#include <Plasma/PopupApplet>
#include "ui_batteryConfig.h"

typedef QMap< QString, QString > StringStringMap;

namespace Plasma
{
    class Svg;
    class Label;
    class ExtenderItem;
    class ComboBox;
    class Slider;
    class CheckBox;
}

class Battery : public Plasma::PopupApplet
{
    Q_OBJECT
    Q_PROPERTY(qreal labelAlpha READ labelAlpha WRITE setLabelAlpha )
    Q_PROPERTY(qreal acAlpha READ acAlpha WRITE setAcAlpha)

    public:
        Battery(QObject *parent, const QVariantList &args);
        ~Battery();

        void init();
        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            const QRect &contents);
        void setPath(const QString&);
        Qt::Orientations expandingDirections() const;
        void constraintsEvent(Plasma::Constraints constraints);
        void popupEvent(bool show);
        void setShowBatteryLabel(bool show);

        qreal labelAlpha() const;
        void setLabelAlpha(qreal alpha);
        qreal acAlpha() const;
        void setAcAlpha(qreal alpha);

        QList<QAction*> contextualActions();

    public Q_SLOTS:
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void configChanged();
        void toolTipAboutToShow();
        void toolTipHidden();

    protected Q_SLOTS:
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void configAccepted();
        void readColors();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);
        void setEmbedded(const bool embedded);

    private Q_SLOTS:
        //void animationUpdate(qreal progress);
        //void acAnimationUpdate(qreal progress);
        //void batteryAnimationUpdate(qreal progress);
        void sourceAdded(const QString &source);
        void sourceRemoved(const QString &source);
        void brightnessChanged(const int brightness);
        void updateSlider(int brightness);
        void updateSlider();
        void openConfig();
        void suspend();
        void hibernate();
        void updateBattery();
        void setupFonts();
        void toggleInhibit(bool toggle);

    private:
        void initPopupWidget();
        void updateStatus();
        bool isConstrained();
        QString stringForState(const QHash<QString, QVariant> &batteryData, bool *chargeChanging = 0);
        QFont setupLabelPainting(const QRect &contentsRect, const QString &batterLabel);

        /* Paint battery with proper charge level */
        void paintBattery(QPainter *p, const QRect &contentsRect, const int batteryPercent, const bool plugState);
        /* Paint a label on top of the battery */
        void paintLabel(QPainter *p, const QRect &contentsRect, const QString& labelText);
        /* Scale in/out Battery. */
        //void showBattery(bool show);
        /* Scale in/out Ac Adapter. */
        void showAcAdapter(bool show);
        /* Fade in/out the label above the battery. */
        void showLabel(bool show);
        /* Scale in a QRectF */
        QRectF scaleRectF(qreal progress, QRectF rect);

        /* Prevent creating infinite loops by embedding applets inside applets */
        bool m_isEmbedded;
        bool m_extenderVisible;

        Plasma::Label *m_batteryLabelLabel;
        Plasma::Label *m_batteryInfoLabel;
        Plasma::Label *m_acLabelLabel;
        Plasma::Label *m_acInfoLabel;
        Plasma::Label *m_remainingTimeLabel;
        Plasma::Label *m_remainingInfoLabel;
        Plasma::Label *m_statusLabel;
        Plasma::Label *m_brightnessLabel;
        Plasma::Label* m_inhibitLabel;
        Plasma::Slider *m_brightnessSlider;
        Plasma::CheckBox *m_inhibitButton;
        QGraphicsLinearLayout *m_buttonLayout;
        Plasma::IconWidget *m_suspendButton;
        Plasma::IconWidget *m_hibernateButton;

        /* Show multiple batteries with individual icons and charge info? */
        bool m_showMultipleBatteries;
        /* Should the battery charge information be shown on top? */
        bool m_showBatteryLabel;
        /* Should that info be percentage (false) or time (true)? */
        bool m_showRemainingTime;
        int m_minutes;
        int m_hours;
        QSizeF m_size;
        int m_pixelSize;
        Plasma::Svg* m_theme;

        StringStringMap m_availableProfiles;
        QString m_currentProfile;
        QStringList m_suspendMethods;

        // Configuration dialog
        Ui::batteryConfig ui;

        int m_batteryAnimId;

        // Internal data
        QList<QVariant> batterylist;
        QList<QVariant> acadapterlist;
        QHash<QString, QHash<QString, QVariant> > m_batteriesData;
        QFont m_font;
        bool m_firstRun;
        QColor m_boxColor;
        QColor m_textColor;
        QRectF m_textRect;
        int m_boxAlpha;
        int m_boxHoverAlpha;
        int m_numOfBattery;
        bool m_acAdapterPlugged;
        qulonglong m_remainingMSecs;

        qreal m_labelAlpha;
        QPropertyAnimation *m_labelAnimation;
        qreal m_acAlpha;
        QPropertyAnimation *m_acAnimation;

        bool m_ignoreBrightnessChange;

        QPair< int, int > m_inhibitCookies;
};

Q_DECLARE_METATYPE(StringStringMap)
K_EXPORT_PLASMA_APPLET(battery, Battery)

#endif
