/**
  * privacy.h
  *
  * Copyright (c) 2003 Ralf Hoelzer <ralf@well.com>
  *
  *  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU Lesser General Public License as published
  *  by the Free Software Foundation; either version 2.1 of the License, or
  *  (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU Lesser General Public License for more details.
  *
  *  You should have received a copy of the GNU Lesser General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
  */

#ifndef _PRIVACY_H_
#define _PRIVACY_H_

#include <kcmodule.h>
#include <klistview.h>

#include "kcmprivacydialog.h"
#include "kprivacymanager.h"
#include "kprivacysettings.h"

class Privacy: public KCModule
{
    Q_OBJECT

public:
    Privacy( QWidget *parent=0, const char *name=0 );
    ~Privacy();

    virtual void load();
    virtual void save();
    virtual void defaults();

public slots:
    void cleanup();
    void selectAll();
    void selectNone();

private:
    KCMPrivacyDialog  *cleaningDialog;
    KPrivacySettings  *p3pSettings;
    KPrivacyManager *m_privacymanager;

    QPtrList<QCheckListItem> checklist;

    KListViewItem *generalCLI;
    KListViewItem *webbrowsingCLI;

    QCheckListItem *clearThumbnails;	
    QCheckListItem *clearRunCommandHistory;
    QCheckListItem *clearAllCookies;
    QCheckListItem *clearSavedClipboardContents;
    QCheckListItem *clearWebHistory;
    QCheckListItem *clearWebCache;
    QCheckListItem *clearFormCompletion;
    QCheckListItem *clearRecentDocuments;
    QCheckListItem *clearQuickStartMenu;
    QCheckListItem *clearFavIcons;
    //QCheckListItem *clearFileDialogHistory;


};

#endif
