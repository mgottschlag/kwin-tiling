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

#ifndef HWINFO_HEADER
#define HWINFO_HEADER

#include <applet.h>
#include <plasma/dataengine.h>

namespace Plasma {
    class WebView;
    class IconWidget;
}
class Header;
class QGraphicsLinearLayout;

class HWInfo : public SM::Applet
{
    Q_OBJECT
    public:
        HWInfo(QObject *parent, const QVariantList &args);
        ~HWInfo();

        virtual void init();
        virtual bool addMeter(const QString&);

    public slots:
        void dataUpdated(const QString &name,
                     const Plasma::DataEngine::Data &data);

    private:
        void connectToEngine();

        Plasma::WebView *m_info;
        Plasma::IconWidget *m_icon;
        QString m_cpu;
        QString m_gpu;
        QString m_audio;
        QString m_net;
};

K_EXPORT_PLASMA_APPLET(sm_hwinfo, HWInfo)

#endif
