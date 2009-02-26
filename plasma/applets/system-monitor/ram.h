/*
 *   Copyright (C) 2008 Petri Damsten <damu@iki.fi>
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

#ifndef CPU_HEADER
#define CPU_HEADER

#include "ui_ram-config.h"
#include "ui_cpu-config-adv.h"
#include "applet.h"
#include <Plasma/DataEngine>
#include <QStandardItemModel>

class QStandardItemModel;

namespace SM {

class Ram : public Applet
{
    Q_OBJECT
    public:
        Ram(QObject *parent, const QVariantList &args);
        ~Ram();

        virtual void init();
        virtual bool addMeter(const QString&);
        virtual void createConfigurationInterface(KConfigDialog *parent);
        virtual void setDetail(Detail detail);

    public slots:
        void dataUpdated(const QString &name,
                         const Plasma::DataEngine::Data &data);
        void initLater(const QString &name);
        void configAccepted();

    private:
        Ui::config ui;
        Ui::configAdv uiAdv;
        QStandardItemModel m_model;
        QStringList m_memories;
        bool m_showTopBar;
        bool m_showBackground;
        QColor m_graphColor;
        QHash<QString, QString> m_html;
        QHash<QString, double> m_max;

    private slots:
        void parseSources();
        void themeChanged();
        void updateSpinBoxSuffix(int interval);
};
}

K_EXPORT_PLASMA_APPLET(sm_ram, SM::Ram)

#endif
