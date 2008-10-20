/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef SM_APPLET_HEADER
#define SM_APPLET_HEADER

#include <plasma/applet.h>

#include "sm_export.h"

#define DEFAULT_MINIMUM_WIDTH 200

class Header;
class QGraphicsLinearLayout;

namespace Plasma {
    class Meter;
    class SignalPlotter;
    class Frame;
}

namespace SM {

class SM_EXPORT Applet : public Plasma::Applet
{
    Q_OBJECT
    public:
        enum Mode { Monitor, Panel, Desktop };
        enum Detail { High, Low };

        Applet(QObject *parent, const QVariantList &args);
        ~Applet();

        virtual void constraintsEvent(Plasma::Constraints constraints);
        QSizeF minSize() const { return m_min; };

    signals:
        void geometryChecked();

    protected:
        void connectToEngine();
        void connectSource(const QString& source);
        void disconnectSources();
        void checkGeometry();
        QGraphicsLinearLayout* mainLayout();
        void setTitle(const QString& title, bool spacer = false);
        KConfigGroup persistentConfig() const;
        uint interval() { return m_interval; };
        void setInterval(uint interval) { m_interval = interval; };
        qreal preferredItemHeight() { return m_preferredItemHeight; };
        void setPreferredItemHeight(qreal preferredItemHeight)
                { m_preferredItemHeight = preferredItemHeight; };
        QString title() { return m_title; };
        bool titleSpacer() { return m_titleSpacer; };
        Plasma::Frame* header() { return m_header; };
        QStringList items() { return m_items; };
        void appendItem(const QString& item) { m_items.append(item); };
        void setItems(const QStringList& items) { m_items = items; };
        void clearItems() { m_items.clear(); };
        QStringList connectedSources() { return m_connectedSources; };
        void setEngine(Plasma::DataEngine* engine) { m_engine = engine; };
        Plasma::DataEngine* engine() { return m_engine; };
        Qt::Orientation ratioOrientation() { return m_ratioOrientation; };
        void setRatioOrientation(Qt::Orientation ratioOrientation)
                { m_ratioOrientation = ratioOrientation; };
        void appendKeepRatio(QGraphicsWidget* w) { m_keepRatio.append(w); };
        QHash<QString,Plasma::Meter*> meters() { return m_meters; };
        void appendMeter(const QString& source, Plasma::Meter* meter)
                { m_meters[source] = meter; };
        QHash<QString,Plasma::SignalPlotter*> plotters() { return m_plotters; };
        void appendPlotter(const QString& source, Plasma::SignalPlotter* plotter)
                { m_plotters[source] = plotter; };
        Qt::Orientation orientation() { return m_orientation; };
        Mode mode() { return m_mode; };
        Detail detail() { return m_detail; };
        qreal minimumWidth() { return m_minimumWidth; };
        void setMinimumWidth(qreal minimumWidth) { m_minimumWidth = minimumWidth; };

        virtual bool addMeter(const QString&) { return false; };
        virtual void deleteMeters(QGraphicsLinearLayout* layout = 0);
        virtual void setDetail(Detail detail);
        //QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;

    private:
        uint m_interval;
        qreal m_preferredItemHeight;
        QString m_title;
        bool m_titleSpacer;
        Plasma::Frame* m_header;
        QStringList m_items;
        QStringList m_connectedSources;
        Plasma::DataEngine *m_engine;
        Qt::Orientation m_ratioOrientation;
        QList<QGraphicsWidget*> m_keepRatio;
        QHash<QString, Plasma::Meter*> m_meters;
        QHash<QString, Plasma::SignalPlotter*> m_plotters;
        Qt::Orientation m_orientation;
        Mode m_mode;
        Detail m_detail;
        qreal m_minimumWidth;
        QSizeF m_min;
        QSizeF m_pref;
        QSizeF m_max;

        QGraphicsLinearLayout *m_mainLayout;

        static QHash< QString, QList<uint> > s_configIds;
        uint m_configId;
};

}

#endif
