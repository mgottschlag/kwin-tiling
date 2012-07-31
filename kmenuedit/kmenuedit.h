/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef kmenuedit_h
#define kmenuedit_h

#include <KXmlGuiWindow>

class QSplitter;
class KAction;
class BasicTab;
class TreeView;

class KMenuEdit : public KXmlGuiWindow
{
    Q_OBJECT

public:
    KMenuEdit();
    ~KMenuEdit();

    void selectMenu(const QString &menu);
    void selectMenuEntry(const QString &menuEntry);

    // dbus method
    void restoreSystemMenu();

protected:
    void setupView();
    void setupActions();
    bool queryClose();

protected Q_SLOTS:
    void slotSave();
    void slotChangeView();
    void slotRestoreMenu();
    void slotConfigure();

protected:
    TreeView           *m_tree;
    BasicTab           *m_basicTab;
    QSplitter          *m_splitter;

    KAction *m_actionDelete;
    bool m_showHidden;
};

#endif
