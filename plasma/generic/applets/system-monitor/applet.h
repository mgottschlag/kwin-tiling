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

#include <Plasma/Applet>

#include "sm_export.h"

#define MINIMUM 24
#define PREFERRED 200

class QGraphicsLinearLayout;

namespace Plasma {
    class Meter;
    class SignalPlotter;
    class Frame;
    class IconWidget;
}

namespace SM {

class Plotter;

class SM_EXPORT Applet : public Plasma::Applet
{
    Q_OBJECT
    public:
        enum Mode { Monitor, Panel, Desktop };
        enum Detail { High, Low };

        Applet(QObject *parent, const QVariantList &args);
        ~Applet();

        virtual void constraintsEvent(Plasma::Constraints constraints);
        void save(KConfigGroup &config) const;
        void saveConfig(KConfigGroup &config);

    public Q_SLOTS:
        void toolTipAboutToShow();

    signals:
        void geometryChecked();

    protected:
        qreal preferredItemHeight() { return m_preferredItemHeight; };
        void setPreferredItemHeight(qreal preferredItemHeight)
                { m_preferredItemHeight = preferredItemHeight; };
        QStringList items() { return m_items; };
        void appendItem(const QString& item) { m_items.append(item); };
        void setItems(const QStringList& items) { m_items = items; };
        void clearItems() { m_items.clear(); };

        KConfigGroup config();
        void connectToEngine();
        void connectSource(const QString& source);
        QStringList connectedSources();
        void disconnectSources();
        void checkGeometry();
        QGraphicsLinearLayout* mainLayout();
        Plasma::DataEngine* engine();
        void setEngine(Plasma::DataEngine* engine);
        QHash<QString, SM::Plotter*> plotters();
        void appendPlotter(const QString& source, SM::Plotter* plotter);
        uint interval();
        void setInterval(uint interval);
        QString title();
        void setTitle(const QString& title, bool spacer = false);
        QHash<QString, QString> tooltips() const;
        void setToolTip(const QString &source, const QString &tipContent);
        Mode mode();
        virtual bool addMeter(const QString&);
        void displayNoAvailableSources();
        virtual void deleteMeters();
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    private:
        uint m_interval;
        qreal m_preferredItemHeight;
        QString m_title;
        bool m_titleSpacer;
        Plasma::Frame* m_header;
        QStringList m_items;
        QStringList m_connectedSources;
        Plasma::DataEngine *m_engine;
        QHash<QString, SM::Plotter*> m_plotters;
        QHash<QString, QString> m_toolTips;
        Qt::Orientation m_orientation;
        Plasma::IconWidget *m_noSourcesIcon;
        Mode m_mode;
        QGraphicsLinearLayout *m_mainLayout;
        Plasma::Applet *m_configSource;
};

}

#endif
