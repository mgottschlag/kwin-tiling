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

#ifndef NET_HEADER
#define NET_HEADER

#include "ui_net-config.h"
#include "applet.h"
#include <Plasma/DataEngine>
#include <QStandardItemModel>

class QStandardItemModel;

namespace SM {

class Net : public Applet
{
    Q_OBJECT
    public:
        Net(QObject *parent, const QVariantList &args);
        ~Net();

        virtual void init();
        virtual bool addMeter(const QString&);
        virtual void createConfigurationInterface(KConfigDialog *parent);
        virtual void setDetail(Detail detail);

    public slots:
        void configAccepted();
        void dataUpdated(const QString &name,
                         const Plasma::DataEngine::Data &data);
        void initLater(const QString &name);

    private:
       Ui::config ui;
       QStandardItemModel m_model;
       QStringList m_interfaces;
       QMap<QString, QList<double> > m_data;

    private slots:
        void parseSources();
        void themeChanged();
	void updateSpinBoxSuffix(int interval);
};
}

K_EXPORT_PLASMA_APPLET(sm_net, SM::Net)

#endif
