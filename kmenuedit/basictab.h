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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __basictab_h__
#define __basictab_h__

#include <qwidget.h>
#include <qstring.h>

#include <klineedit.h>

class KKeyButton;
class KLineEdit;
class KIconButton;
class QCheckBox;
class QGroupBox;
class QLabel;
class KURLRequester;
class KComboBox;

class MenuFolderInfo;
class MenuEntryInfo;

class BasicTab : public QWidget
{
    Q_OBJECT

public:
    BasicTab( QWidget *parent=0, const char *name=0 );

    void apply();

signals:
    void changed( MenuFolderInfo * );
    void changed( MenuEntryInfo * );

public slots:
    void setFolderInfo(MenuFolderInfo *folderInfo);
    void setEntryInfo(MenuEntryInfo *entryInfo);

protected slots:
    void slotChanged();
    void launchcb_clicked();
    void termcb_clicked();
    void uidcb_clicked();
    void slotCapturedShortcut(const KShortcut&);

protected:
    void enableWidgets(bool isDF, bool isDeleted);

protected:
    KLineEdit    *_nameEdit, *_commentEdit;
    KKeyButton   *_keyEdit;
    KURLRequester *_execEdit, *_pathEdit;
    KLineEdit    *_termOptEdit, *_uidEdit;
    QCheckBox    *_terminalCB, *_uidCB, *_launchCB;
    KIconButton  *_iconButton;
    QGroupBox    *_path_group, *_term_group, *_uid_group, *general_group_keybind;
    QLabel *_termOptLabel, *_uidLabel, *_pathLabel, *_nameLabel, *_commentLabel, *_execLabel;

    MenuFolderInfo *_menuFolderInfo;
    MenuEntryInfo  *_menuEntryInfo;
    bool _isDeleted;
};

#endif
