/***************************************************************************
 *   Copyright (C) 2010 by Ingomar Wesp <ingomar@wesp.name>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef QUICKLAUNCH_POPUP_H
#define QUICKLAUNCH_POPUP_H

// Qt
#include <Qt>

// KDE
#include <Plasma/Dialog>

using Plasma::Dialog;

namespace Quicklaunch {

class IconArea;
class Quicklaunch;

class Popup : public Dialog {

    Q_OBJECT

public:
    Popup(Quicklaunch *applet);
    ~Popup();

    IconArea *iconArea();
    void show();

private Q_SLOTS:
    void onAppletGeometryChanged();
    void onDisplayedItemCountChanged();
    void onIconClicked();

private:
    void syncSizeAndPosition();

    Quicklaunch *m_applet;
    IconArea *m_iconArea;
};
}

#endif /* QUICKLAUNCH_POPUP_H */
