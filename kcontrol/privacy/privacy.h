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
  *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  */

#ifndef _PRIVACY_H_
#define _PRIVACY_H_

#include <kcmodule.h>
#include <kaboutdata.h>
#include "kcmprivacydialog.h"
#include "kprivacysettings.h"
#include "kprivacymanager.h"
#include <klistview.h>

class Privacy: public KCModule
{
    Q_OBJECT

public:
    Privacy( QWidget *parent=0, const char *name=0 );
    ~Privacy();

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual int buttons();
    virtual QString quickHelp() const;
    const KAboutData* aboutData() const;


public slots:
    void configChanged();
    void cleanup();
    void selectAll();
    void selectNone();

private:
    KAboutData *myAboutData;
    KCMPrivacyDialog  *cleaningDialog;
    KPrivacySettings  *p3pSettings;
    KPrivacyManager *m_privacymanager;

    QPtrList<QCheckListItem> checklist;

    KListViewItem *generalCLI;
    KListViewItem *webbrowsingCLI;

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
