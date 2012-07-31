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

#ifndef basictab_h
#define basictab_h

#include <KTabWidget>
#include <KShortcut>
#include <KService>

class KKeySequenceWidget;
class KLineEdit;
class KIconButton;
class QCheckBox;
class QGroupBox;
class QLabel;
class KUrlRequester;
class KService;
class KLineSpellChecking;

class MenuFolderInfo;
class MenuEntryInfo;

class BasicTab : public KTabWidget
{
    Q_OBJECT

public:
    BasicTab( QWidget *parent=0 );

    void apply();

    void updateHiddenEntry( bool show );

Q_SIGNALS:
    void changed( MenuFolderInfo * );
    void changed( MenuEntryInfo * );
    void findServiceShortcut(const KShortcut&, KService::Ptr &);

public Q_SLOTS:
    void setFolderInfo(MenuFolderInfo *folderInfo);
    void setEntryInfo(MenuEntryInfo *entryInfo);
    void slotDisableAction();
protected Q_SLOTS:
    void slotChanged();
    void launchcb_clicked();
    void systraycb_clicked();
    void termcb_clicked();
    void uidcb_clicked();
    void slotCapturedKeySequence(const QKeySequence&);
    void slotExecSelected();
    void onlyshowcb_clicked();
    void hiddenentrycb_clicked();

protected:
    void enableWidgets(bool isDF, bool isDeleted);

protected:
    KLineEdit    *_nameEdit;
    KLineSpellChecking*_commentEdit;
    KLineSpellChecking   *_descriptionEdit;
    KKeySequenceWidget *_keyEdit;
    KUrlRequester *_execEdit, *_pathEdit;
    KLineEdit    *_termOptEdit, *_uidEdit;
    QCheckBox    *_terminalCB, *_uidCB, *_launchCB, *_systrayCB, *_onlyShowInKdeCB, *_hiddenEntryCB;
    KIconButton  *_iconButton;
    QGroupBox    *_path_group, *_term_group, *_uid_group, *general_group_keybind;
    QLabel *_termOptLabel, *_uidLabel, *_pathLabel, *_nameLabel, *_commentLabel, *_execLabel;
    QLabel      *_descriptionLabel;

    MenuFolderInfo *_menuFolderInfo;
    MenuEntryInfo  *_menuEntryInfo;
};

#endif
