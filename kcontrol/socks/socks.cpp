/**
 * socks.cpp
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <qframe.h>
#include <qfile.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qfileinfo.h>
#include <qcheckbox.h>

#include <kfiledialog.h>
#include <klineedit.h>
#include <klistview.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kmessagebox.h>


#include "socks.h"


KSocksConfig::KSocksConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
QGridLayout *grid;
QString whatstr;
QFrame *frame;

  frame = new QFrame(this);
  grid = new QGridLayout(frame, 13, 5, KDialog::marginHint(),
                                       KDialog::spacingHint());

  // First widget: "Enable SOCKS support?"
  _c_enableSocks = new QCheckBox(i18n("&Enable SOCKS Support"), frame);
  connect(_c_enableSocks, SIGNAL(clicked()), this, SLOT(enableChanged()));
  whatstr = i18n("Check this to enable SOCKS4 and SOCKS5 support in KDE"
                 " applications and I/O subsystems.");
  QWhatsThis::add(_c_enableSocks, whatstr);
  grid->addMultiCellWidget(_c_enableSocks, 0, 0, 0, 4);

  // Next: List of SOCKS client libraries
  bg = new QVButtonGroup(i18n("SOCKS Implementation"), frame);
  _c_detect = new QRadioButton(i18n("Autodetect"), bg);
  _c_NEC = new QRadioButton(i18n("Use NEC SOCKS"), bg);
  _c_Dante = new QRadioButton(i18n("Use Dante"), bg);
  _c_custom = new QRadioButton(i18n("Custom..."), bg);
  bg->insert(_c_detect, 1);
  bg->insert(_c_NEC, 2);
  bg->insert(_c_Dante, 3);
  bg->insert(_c_custom, 4);
  whatstr = i18n("If you select Autodetect, then KDE will automatically search"
                 " for an implementation of SOCKS on your computer.");
  QWhatsThis::add(_c_detect, whatstr);
  whatstr = i18n("This will force KDE to use NEC SOCKS if it can be found.");
  QWhatsThis::add(_c_NEC, whatstr);
  whatstr = i18n("This will force KDE to use Dante if it can be found.");
  QWhatsThis::add(_c_Dante, whatstr);
  whatstr = i18n("Select custom if you wish to use an unlisted SOCKS library."
                 "  Please note that this may not always work as it depends"
                 " on the API of the library which you specify (below).");
  QWhatsThis::add(_c_custom, whatstr);
  connect(bg, SIGNAL(clicked(int)), this, SLOT(methodChanged(int)));
  grid->addMultiCellWidget(bg, 1, 5, 0, 3);

  // The custom library
  _c_customLabel = new QLabel(i18n("Path to custom library..."), frame);
  grid->addMultiCellWidget(_c_customLabel, 6, 6, 0, 2);
  _c_customPath = new KLineEdit(frame);
  grid->addMultiCellWidget(_c_customPath, 7, 7, 0, 3);
  whatstr = i18n("Enter the path to an unsupported SOCKS library.");
  QWhatsThis::add(_c_customPath, whatstr);
  _c_customChoose = new QPushButton("...", frame);
  grid->addWidget(_c_customChoose, 7, 4);
  whatstr = i18n("Open a file selection dialog to choose the SOCKS library.");
  QWhatsThis::add(_c_customChoose, whatstr);
  connect(_c_customChoose, SIGNAL(clicked()), this, SLOT(chooseCustomLib()));
  connect(_c_customPath, SIGNAL(textChanged(const QString&)),
                     this, SLOT(customPathChanged(const QString&)));

  // Additional libpaths
  QLabel *lbl = new QLabel(i18n("Additional library paths to search..."), frame);
  grid->addMultiCellWidget(lbl, 8, 8, 0, 4);
  whatstr = i18n("Here you can specify additional directories to search for the SOCKS libraries.  /usr/lib, /usr/local/lib and /opt/socks5/lib are already searched by default.");
  QWhatsThis::add(lbl, whatstr);
  _c_newPath = new KLineEdit(frame);
  grid->addMultiCellWidget(_c_newPath, 9, 9, 0, 2);
  connect(_c_newPath, SIGNAL(returnPressed(const QString&)), 
          this, SLOT(addThisLibrary(const QString&)));
  connect(_c_newPath, SIGNAL(textChanged(const QString&)), 
          this, SLOT(libTextChanged(const QString&)));
  QWhatsThis::add(_c_newPath, whatstr);
  _c_add = new QPushButton(i18n("&Add"), frame);
  grid->addWidget(_c_add, 9, 3);
  connect(_c_add, SIGNAL(clicked()), this, SLOT(addLibrary()));
  _c_add->setEnabled(false);
  _c_remove = new QPushButton(i18n("&Remove"), frame);
  grid->addWidget(_c_remove, 9, 4);
  connect(_c_remove, SIGNAL(clicked()), this, SLOT(removeLibrary()));
  _c_remove->setEnabled(false);
  _c_libs = new KListView(frame);
  grid->addMultiCellWidget(_c_libs, 10, 11, 0, 4);
  _c_libs->addColumn(i18n("Path"));
  connect(_c_libs, SIGNAL(selectionChanged()), this, SLOT(libSelection()));
  whatstr = i18n("This is the list of additional paths that will be searched.");
  QWhatsThis::add(_c_libs, whatstr);
  

  // The "Test" button
  _c_test = new QPushButton(i18n("&Test"), frame);
  connect(_c_test, SIGNAL(clicked()), this, SLOT(testClicked()));
  whatstr = i18n("Click here to test SOCKS support.");
  QWhatsThis::add(_c_test, whatstr);
  grid->addWidget(_c_test, 12, 4);
  

  frame->resize(frame->sizeHint());

  config = new KConfig("ksocksrc", false, false);
  load();
}

KSocksConfig::~KSocksConfig()
{
    delete config;
}

void KSocksConfig::configChanged()
{
    emit changed(true);
}

void KSocksConfig::enableChanged()
{
  KMessageBox::information(NULL,
                           i18n("These changes will only apply to newly "
                                "started applications."),
                           i18n("SOCKS Support"),
                           "SOCKSdontshowagain");
  emit changed(true);
}


void KSocksConfig::methodChanged(int id)
{
  if (id == 4) {
    _c_customLabel->setEnabled(true);
    _c_customPath->setEnabled(true);
    _c_customChoose->setEnabled(true);
  } else {
    _c_customLabel->setEnabled(false);
    _c_customPath->setEnabled(false);
    _c_customChoose->setEnabled(false);
  }
  emit changed(true);
}


void KSocksConfig::customPathChanged(const QString&)
{
  emit changed(true);
}


void KSocksConfig::testClicked()
{
  KMessageBox::information(NULL,
                           "Sorry, this isn't implemented yet.",
                           i18n("SOCKS Support"));
}


void KSocksConfig::chooseCustomLib()
{
  QString newFile = KFileDialog::getOpenFileName();
  if (newFile.length() > 0) {
    _c_customPath->setText(newFile);
    emit changed(true);
  }
}



void KSocksConfig::libTextChanged(const QString& lib)
{
   if (lib.length() > 0)
      _c_add->setEnabled(true);
   else _c_add->setEnabled(false);
}


void KSocksConfig::addThisLibrary(const QString& lib)
{
   if (lib.length() > 0) {
      new QListViewItem(_c_libs, lib);
      _c_newPath->clear();
      _c_add->setEnabled(false);
      _c_newPath->setFocus();
      emit changed(true);
   }
}


void KSocksConfig::addLibrary()
{
   addThisLibrary(_c_newPath->text());
}


void KSocksConfig::removeLibrary()
{
 QListViewItem *thisitem = _c_libs->selectedItem();
   _c_libs->takeItem(thisitem);
   delete thisitem;
   _c_libs->clearSelection();
   _c_remove->setEnabled(false);
   emit changed(true);
}


void KSocksConfig::libSelection()
{
   _c_remove->setEnabled(true);
}


void KSocksConfig::load()
{
  _c_enableSocks->setChecked(config->readBoolEntry("Enable SOCKS", false));
  int id = config->readNumEntry("SOCKS Method", 1); 
  bg->setButton(id);
  if (id == 4) {
    _c_customLabel->setEnabled(true);
    _c_customPath->setEnabled(true);
    _c_customChoose->setEnabled(true);
  } else {
    _c_customLabel->setEnabled(false);
    _c_customPath->setEnabled(false);
    _c_customChoose->setEnabled(false);
  }
  _c_customPath->setText(config->readEntry("Custom Lib", ""));

  QListViewItem *thisitem;
  while ((thisitem = _c_libs->firstChild())) {
     _c_libs->takeItem(thisitem);
     delete thisitem;
  }

  QStringList libs = config->readListEntry("Lib Path");
  for(QStringList::Iterator it = libs.begin();
                            it != libs.end();
                            ++it ) {
     new QListViewItem(_c_libs, *it);
  }
  _c_libs->clearSelection();
  _c_remove->setEnabled(false);
 emit changed(false);
}

void KSocksConfig::save()
{
QStringList libs;
  config->writeEntry("Enable SOCKS", _c_enableSocks->isChecked());
  config->writeEntry("SOCKS Method", bg->id(bg->selected()));
  config->writeEntry("Custom Lib", _c_customPath->text());
  QListViewItem *thisitem = _c_libs->firstChild();
  while (thisitem) {
    libs << thisitem->text(0);
    thisitem = thisitem->itemBelow();
  }
  config->writeEntry("Lib Path", libs);
  config->sync();

  emit changed(false);
}

void KSocksConfig::defaults()
{

  _c_enableSocks->setChecked(false);
  bg->setButton(1);
    _c_customLabel->setEnabled(false);
    _c_customPath->setEnabled(false);
    _c_customChoose->setEnabled(false);
  _c_customPath->setText("");
  QListViewItem *thisitem;
  while ((thisitem = _c_libs->firstChild())) {
     _c_libs->takeItem(thisitem);
     delete thisitem;
  }
  _c_newPath->clear();
  _c_add->setEnabled(false);
  _c_remove->setEnabled(false);
  emit changed(true);
}

QString KSocksConfig::quickHelp() const
{
  return i18n("<h1>socks</h1> This module allows you to configure KDE support"
     " for a SOCKS server or proxy.");
}


extern "C"
{
  KCModule *create_socks(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmsocks");
    return new KSocksConfig(parent, name);
  };
}


#include "socks.moc"

