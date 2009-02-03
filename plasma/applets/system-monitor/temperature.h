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

#ifndef TEMPERATURE_HEADER
#define TEMPERATURE_HEADER

#include <applet.h>
#include "ui_temperature-config.h"
#include <Plasma/DataEngine>
#include <QStandardItemModel>

namespace Plasma {
    class Meter;
}
class Header;
class QGraphicsLinearLayout;

class Temperature : public SM::Applet
{
    Q_OBJECT
    public:
        Temperature(QObject *parent, const QVariantList &args);
        ~Temperature();

        virtual void init();

    public slots:
        void initLater(const QString &name);
        void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
        void createConfigurationInterface(KConfigDialog *parent);

    private slots:
        void updateSpinBoxSuffix(int interval);
        void configAccepted();
        void parseSources();
        void themeChanged();

    private:
        bool m_showPlotters;
        Ui::config ui;
        QStandardItemModel m_tempModel;

        QString title(const QString& source);
        bool addMeter(const QString& source);
        bool isValidDevice(const QString& uuid, Plasma::DataEngine::Data* data);
};

K_EXPORT_PLASMA_APPLET(sm_temperature, Temperature)

#endif
