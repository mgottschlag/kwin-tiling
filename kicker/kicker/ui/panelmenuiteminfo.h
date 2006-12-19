/*****************************************************************

Copyright (c) 2003 Aaron J. Seigo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef PANELMENUITEMINFO_H
#define PANELMENUITEMINFO_H

#include <QMenu>
#include <QString>
#include <QByteArray>

#include <kicon.h>

// a little class meant to be used to store menu items for sorting then later
// plugging into a popup menu

class PanelMenuItemInfo
{
    public:
        typedef QList<PanelMenuItemInfo> List;

        PanelMenuItemInfo()
            : m_recvr(0), m_id(-1) {}

        PanelMenuItemInfo(const QString& iconName, const QString& visibleName, const QObject* recvr, const QByteArray& slot, int id = -1)
            : m_icon(iconName), m_name(visibleName), m_slot_(slot), m_recvr(recvr), m_id(id) {}

        PanelMenuItemInfo(const QString& iconName, const QString& visibleName, int id = -1)
            : m_icon(iconName), m_name(visibleName), m_recvr(0), m_id(id) {}

        PanelMenuItemInfo(const PanelMenuItemInfo& c)
            : m_icon(c.m_icon), m_name(c.m_name), m_slot_(c.m_slot_), m_recvr(c.m_recvr), m_id(c.m_id) {}

        PanelMenuItemInfo& operator=(const PanelMenuItemInfo& c)
        {
            m_icon = c.m_icon;
            m_name = c.m_name;
            m_slot_ = c.m_slot_;
            m_recvr = c.m_recvr;
            m_id = c.m_id;
            return *this;
        }

        bool operator<(const PanelMenuItemInfo& rh) const
        {
            return m_name.toLower() < rh.m_name.toLower();
        }

        bool operator<=(const PanelMenuItemInfo& rh) const
        {
            return m_name.toLower() <= rh.m_name.toLower();
        }

        bool operator>(const PanelMenuItemInfo& rh) const
        {
            return m_name.toLower() > rh.m_name.toLower();
        }

        int plug(QMenu* menu)
        {
            if (!m_icon.isEmpty() && m_icon != "unknown")
            {
                if (m_recvr && !m_slot_.isEmpty())
                {
                    return menu->insertItem(KIcon(m_icon), m_name, m_recvr, m_slot_, 0, m_id);
                }

                return menu->insertItem(KIcon(m_icon), m_name, m_id);
            }
            else if (m_recvr && !m_slot_.isEmpty())
            {
                return menu->insertItem(m_name, m_recvr, m_slot_, 0, m_id);
            }

            return menu->insertItem(m_name, m_id);
        }

    private:
        QString m_icon;
        QString m_name;
        QByteArray m_slot_; // HPUX namespace is polluted with m_slot
        const QObject* m_recvr;
        int m_id;
};

#endif

