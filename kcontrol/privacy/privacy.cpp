/**
  * privacy.cpp
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

#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtabwidget.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdialog.h>
#include <kconfig.h>
#include <klistview.h>
#include <ktextedit.h>


#include "privacy.h"



Privacy::Privacy(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
  m_privacymanager = new KPrivacyManager();

  QBoxLayout *top = new QVBoxLayout(this, 0, KDialog::spacingHint());

  // add this once the P3P stuff is finished
  //QTabWidget *privacyTabs = new QTabWidget(this, "privacytabs");
  
  cleaningDialog = new KCMPrivacyDialog(this);
  //p3pSettings    = new KPrivacySettings(this);

  top->addWidget(cleaningDialog); 

  //top->addWidget(privacyTabs);
  //privacyTabs->addTab(cleaningDialog, i18n("Cleanup"));
  //privacyTabs->addTab(p3pSettings, i18n("Privacy Settings"));


  KListView *sw = cleaningDialog->privacyListView;

  sw->addColumn(i18n("Privacy Settings"));

  generalCLI     = new KListViewItem(sw, i18n("General") );
  webbrowsingCLI = new KListViewItem(sw, i18n("Webbrowsing") );

  generalCLI->setOpen(true);
  webbrowsingCLI->setOpen(true);

  clearRunCommandHistory = new QCheckListItem(generalCLI,
      i18n("Clear Run Command History"),QCheckListItem::CheckBox);
  clearAllCookies = new QCheckListItem(webbrowsingCLI,
      i18n("Clear all cookies"),QCheckListItem::CheckBox);
  clearSavedClipboardContents = new QCheckListItem(generalCLI,
      i18n("Clear saved clipboard contents"),QCheckListItem::CheckBox);
  clearWebHistory = new QCheckListItem(webbrowsingCLI,
      i18n("Clear web history"),QCheckListItem::CheckBox);
  clearWebCache = new QCheckListItem(webbrowsingCLI,
      i18n("Clear web cache"),QCheckListItem::CheckBox);
  clearStoredPasswords = new QCheckListItem(webbrowsingCLI,
      i18n("Clear stored passwords"),QCheckListItem::CheckBox);

  clearRunCommandHistory->setEnabled(true);


  connect(sw, SIGNAL(selectionChanged()), SLOT(configChanged()));


  connect(cleaningDialog->cleanupButton, SIGNAL(clicked()), SLOT(cleanup()));

  load();
};


Privacy::~Privacy()
{
}


void Privacy::load()
{
  KConfig *c = new KConfig("kprivacyrc", false, false);
  
  // get general privacy settings
  { 
    KConfigGroupSaver saver(c, "Cleaning");

    clearRunCommandHistory->setOn(c->readBoolEntry("ClearRunCommandHistory", true));
    clearAllCookies->setOn(c->readBoolEntry("ClearAllCookies", true));
    clearSavedClipboardContents->setOn(c->readBoolEntry("ClearSavedClipboardContents", true));
    clearWebHistory->setOn(c->readBoolEntry("ClearWebHistory", true));
    clearWebCache->setOn(c->readBoolEntry("ClearWebCache", true));
    clearStoredPasswords->setOn(c->readBoolEntry("ClearStoredPasswords", true));
  }

  { 
    KConfigGroupSaver saver(c, "P3P");

    // TODO: add P3P settings here
  }

  delete c;
  emit changed(false);
  m_moduleChanged = false;

}


void Privacy::defaults()
{
  clearRunCommandHistory->setOn(false);
  clearAllCookies->setOn(false);
  clearSavedClipboardContents->setOn(false);
  clearStoredPasswords->setOn(false);
  clearWebCache->setOn(false);
  clearWebHistory->setOn(false);

  m_moduleChanged = true;
  emit changed(true);
}


void Privacy::save()
{
  if ( !m_moduleChanged )
    return;
  KConfig *c = new KConfig("kprivacyrc", false, false);
  {
    KConfigGroupSaver saver(c, "Cleaning");

    c->writeEntry("ClearRunCommandHistory", clearRunCommandHistory->isOn());
    c->writeEntry("ClearAllCookies", clearAllCookies->isOn());
    c->writeEntry("ClearSavedClipboardContents", clearSavedClipboardContents->isOn());
    c->writeEntry("ClearWebCache", clearWebCache->isOn());
    c->writeEntry("ClearWebHistory", clearWebHistory->isOn());
    c->writeEntry("ClearStoredPasswords", clearStoredPasswords->isOn());
  }

  {
    KConfigGroupSaver saver(c, "P3P");

    // TODO: add settings for P3P 
  }

  c->sync();

  delete c;
  m_moduleChanged = false;
  emit changed(false);

}


int Privacy::buttons()
{
  return KCModule::Default|KCModule::Apply|KCModule::Help;
}


void Privacy::configChanged()
{
  m_moduleChanged = true;
  emit changed(true);
}

void Privacy::cleanup()
{
  cleaningDialog->statusTextEdit->clear();
  cleaningDialog->statusTextEdit->setText(i18n("Starting cleanup.."));


  if(clearRunCommandHistory->isOn()) {
    cleaningDialog->statusTextEdit->append(i18n("Clearing Run Command history.."));
    if(!m_privacymanager->clearRunCommandHistory())
      cleaningDialog->statusTextEdit->append(i18n("Clearing of Run Command history failed."));
  }

  if(clearSavedClipboardContents->isOn()) {
    cleaningDialog->statusTextEdit->append(i18n("Clearing saved clipboard contents.."));
    if(!m_privacymanager->clearSavedClipboardContents())
      cleaningDialog->statusTextEdit->append(i18n("Clearing of saved clipboard content failed."));
  }

  if(clearStoredPasswords->isOn()) {
    cleaningDialog->statusTextEdit->append(i18n("Clearing stored passwords.."));
    if(!m_privacymanager->clearStoredPasswords())
      cleaningDialog->statusTextEdit->append(i18n("Clearing of stored passwords failed."));
  }
  if(clearAllCookies->isOn()) {
    cleaningDialog->statusTextEdit->append(i18n("Clearing cookies.."));
    if(!m_privacymanager->clearAllCookies())
      cleaningDialog->statusTextEdit->append(i18n("Clearing cookies failed."));
  }
  if(clearWebCache->isOn()) {
    cleaningDialog->statusTextEdit->append(i18n("Clearing web cache.."));
    if(!m_privacymanager->clearWebCache())
      cleaningDialog->statusTextEdit->append(i18n("Clearing of web cache failed."));
  }

  if(clearWebHistory->isOn()) {
    cleaningDialog->statusTextEdit->append(i18n("Clearing web history.."));
    if(!m_privacymanager->clearWebHistory())
      cleaningDialog->statusTextEdit->append(i18n("Clearing of web history failed."));
  }


  cleaningDialog->statusTextEdit->append(i18n("Clean up finished."));

}


QString Privacy::quickHelp() const
{
  return i18n("The privacy module allows a user to erase traces which KDE leaves on"
              "the system, such as command histories or browser caches.");
}

const KAboutData* Privacy::aboutData() const
{

  KAboutData *about =
    new KAboutData(I18N_NOOP("kcm_privacy"), I18N_NOOP("KDE Privacy Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 2003 Ralf Hoelzer"));

  about->addAuthor("Ralf Hoelzer", 0, "ralf@well.com");

  return about;
}


extern "C"
{

  KCModule *create_privacy(QWidget *parent, const char * /*name*/)
  {
    KGlobal::locale()->insertCatalogue("privacy");
    return new Privacy(parent, "Privacy");
  }
}

#include "privacy.moc"
