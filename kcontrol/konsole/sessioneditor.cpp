/***************************************************************************
                          sessioneditor.cpp  -  description
                             -------------------
    begin                : oct 28 2001
    copyright            : (C) 2001 by Stephan Binner
    email                : binner@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sessioneditor.h"
#include "sessioneditor.moc"

#include <qlabel.h>
#include <qwmatrix.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <kdebug.h>
#include <qcheckbox.h>
#include <kstandarddirs.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <klineeditdlg.h>
#include <qtoolbutton.h>
#include <kicondialog.h>
#include <kstdguiitem.h>
#include <kmessagebox.h>

SessionEditor::SessionEditor(QWidget * parent, const char *name)
:SessionDialog(parent, name)
{
  sesMod=false;
  oldSession=-1;

  KGlobal::locale()->insertCatalogue("konsole"); // For schema and keytab translations

  loadAllKeytab();
  loadAllSchema();
  loadAllSession();
  readSession(0);
  sessionList->setCurrentItem(0);

  connect(sessionList, SIGNAL(highlighted(int)), this, SLOT(readSession(int)));
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveCurrent()));
  connect(removeButton, SIGNAL(clicked()), this, SLOT(removeCurrent()));

  connect(nameLine, SIGNAL(textChanged(const QString&)), this, SLOT(sessionModified(const QString&)));
  connect(directoryLine, SIGNAL(textChanged(const QString&)), this, SLOT(sessionModified(const QString&)));
  connect(executeLine, SIGNAL(textChanged(const QString&)), this, SLOT(sessionModified(const QString&)));
  connect(termLine, SIGNAL(textChanged(const QString&)), this, SLOT(sessionModified(const QString&)));

  connect(previewIcon, SIGNAL(iconChanged(QString)), this, SLOT(iconModified(QString)));

  connect(fontCombo, SIGNAL(activated(int)), this, SLOT(sessionModified(int)));
  connect(keytabCombo, SIGNAL(activated(int)), this, SLOT(sessionModified(int)));
  connect(schemaCombo, SIGNAL(activated(int)), this, SLOT(sessionModified(int)));
  removeButton->setEnabled(sessionList->count()>1);
}

SessionEditor::~SessionEditor()
{
}

void SessionEditor::loadAllKeytab()
{
  QStringList lst = KGlobal::dirs()->findAllResources("data", "konsole/*.keytab");
  keytabCombo->clear();
  keytabFilename.clear();

  keytabCombo->insertItem(i18n("XTerm (XFree 4.x.x)"),0);
  keytabFilename.append(new QString(""));

  int i=1;
  for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
  {
    QString name = (*it);
    QString title = readKeymapTitle(name);

    int j = name.findRev('/');
    if (j > -1)
      name = name.mid(j+1);
    j = name.findRev('.');
    if (j > -1)
      name = name.left(j);
    keytabFilename.append(new QString(name));

    if (title.isNull() || title.isEmpty())
      keytabCombo->insertItem(i18n("untitled"),i);
    else
      keytabCombo->insertItem(title,i);

    i++;
  }
}

QString SessionEditor::readKeymapTitle(const QString & file)
{
  QString fPath = locate("data", "konsole/" + file);

  if (fPath.isNull())
    fPath = locate("data", file);

  if (fPath.isNull())
    return 0;

  FILE *sysin = fopen(QFile::encodeName(fPath), "r");
  if (!sysin)
    return 0;

  char line[100];
  int len;
  while (fscanf(sysin, "%80[^\n]\n", line) > 0)
    if ((len = strlen(line)) > 8)
      if (!strncmp(line, "keyboard", 8)) {
	fclose(sysin);
        if(line[len-1] == '"')
          line[len-1] = '\000';
        QString temp;
        if(line[9] == '"')
          temp=i18n(line+10);
        else
          temp=i18n(line+9);
	return temp;
      }

  return 0;
}

void SessionEditor::loadAllSchema()
{
  QStringList list = KGlobal::dirs()->findAllResources("data", "konsole/*.schema");
  schemaCombo->clear();
  schemaFilename.clear();

  schemaCombo->insertItem(i18n("Konsole Default"),0);
  schemaFilename.append(new QString(""));

  int i = 1;
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {

    QString name = (*it);
    QString title = readSchemaTitle(name);

    int j = name.findRev('/');
    if (j > -1)
      name = name.mid(j+1);
    schemaFilename.append(new QString(name));

    if (title.isNull() || title.isEmpty())
      schemaCombo->insertItem(i18n("untitled"),i);
    else
      schemaCombo->insertItem(title,i);

    i++;
  }
}

QString SessionEditor::readSchemaTitle(const QString & file)
{
  // Code taken from konsole/konsole/schema.cpp
  QString fPath = locate("data", "konsole/" + file);

  if (fPath.isNull())
    fPath = locate("data", file);

  if (fPath.isNull())
    return 0;

  FILE *sysin = fopen(QFile::encodeName(fPath), "r");
  if (!sysin)
    return 0;

  char line[100];
  while (fscanf(sysin, "%80[^\n]\n", line) > 0)
    if (strlen(line) > 5)
      if (!strncmp(line, "title", 5)) {
	fclose(sysin);
	return i18n(line + 6);
      }

  return 0;
}

void SessionEditor::loadAllSession()
{
  QStringList list = KGlobal::dirs()->findAllResources("data", "konsole/*.desktop", false, true);
  sessionList->clear();
  sessionFilename.clear();

  int i = 0;
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {

    QString name = (*it);
    sessionFilename.append(new QString(name));

    KSimpleConfig* co = new KSimpleConfig(name,TRUE);
    co->setDesktopGroup();
    QString sesname = co->readEntry("Name");

    if (sesname.isNull() || sesname.isEmpty())
      sessionList->insertItem(i18n("Unnamed"),i);
    else
      sessionList->insertItem(sesname,i);

    i++;
  }
}

void SessionEditor::readSession(int num)
{
    int i,counter;
    QString str;
    KSimpleConfig* co;

    if(sesMod) {
        disconnect(sessionList, SIGNAL(highlighted(int)), this, SLOT(readSession(int)));

        sessionList->setCurrentItem(oldSession);
        querySave();
        sessionList->setCurrentItem(num);
        connect(sessionList, SIGNAL(highlighted(int)), this, SLOT(readSession(int)));
        sesMod=false;
    }

    if(sessionFilename.at(num))
    {
        co = new KSimpleConfig(*sessionFilename.at(num),TRUE);

        co->setDesktopGroup();
        str = co->readEntry("Name");
        nameLine->setText(str);

        str = co->readEntry("Cwd");
        directoryLine->setText(str);

        str = co->readEntry("Exec");
        executeLine->setText(str);

        str = co->readEntry("Icon","openterm");
        previewIcon->setIcon(str);

        i = co->readUnsignedNumEntry("Font",(unsigned int)-1);
        fontCombo->setCurrentItem(i+1);

        str = co->readEntry("Term","xterm");
        termLine->setText(str);

        str = co->readEntry("KeyTab","");
        i=0;
        counter=0;
        for (QString *it = keytabFilename.first(); it != 0; it = keytabFilename.next()) {
            if (str == (*it))
                i = counter;
            counter++;
        }
        keytabCombo->setCurrentItem(i);

        str = co->readEntry("Schema","");
        i=0;
        counter=0;
        for (QString *it = schemaFilename.first(); it != 0; it = schemaFilename.next()) {
            if (str == (*it))
                i = counter;
            counter++;
        }
        schemaCombo->setCurrentItem(i);
    }
    sesMod=false;
    oldSession=num;
}

void SessionEditor::querySave()
{
    int result = KMessageBox::questionYesNo(this,
                         i18n("The session has been modified.\n"
			"Do you want to save the changes?"),
			i18n("Session Modified"),
			KStdGuiItem::save(),
			KStdGuiItem::discard());
    if (result == KMessageBox::Yes)
    {
        saveCurrent();
    }
}

void SessionEditor::saveCurrent()
{
  QString base;
  if (sessionList->currentText() == nameLine->text()) {
    base = *sessionFilename.at(sessionList->currentItem());
    int j = base.findRev('/');
    if (j > -1)
      base = base.mid(j+1);
  }
  else
    base = nameLine->text().stripWhiteSpace().simplifyWhiteSpace()+".desktop";
  
  KLineEditDlg dlg(i18n("Filename:"), base, this);
  dlg.setCaption(i18n("Save Session"));
  if (!dlg.exec()) return;

  QString name=dlg.text();
  QString fullpath;
  if (name[0] == '/')
    fullpath = name;
  else
    fullpath = KGlobal::dirs()->saveLocation("data", "konsole/") + name;

  KSimpleConfig* co = new KSimpleConfig(fullpath);
  co->setDesktopGroup();
  co->writeEntry("Type","KonsoleApplication");
  co->writeEntry("Name",nameLine->text());
  co->writeEntry("Cwd",directoryLine->text());
  co->writeEntry("Exec",executeLine->text());
  co->writeEntry("Icon",previewIcon->icon());
  if (fontCombo->currentItem()==0)
    co->writeEntry("Font","");
  else
    co->writeEntry("Font",fontCombo->currentItem()-1);
  co->writeEntry("Term",termLine->text());
  co->writeEntry("KeyTab",*keytabFilename.at(keytabCombo->currentItem()));
  co->writeEntry("Schema",*schemaFilename.at(schemaCombo->currentItem()));
  co->sync();

  sesMod=false;
  loadAllSession();
  readSession(0);
  sessionList->setCurrentItem(0);
  removeButton->setEnabled(sessionList->count()>1);
}

void SessionEditor::removeCurrent()
{
  QString base = *sessionFilename.at(sessionList->currentItem());
  if (!QFile::remove(base)) {
    KMessageBox::error(this,
      i18n("Cannot remove the session.\nMaybe it is a system session\n"),
      i18n("Error Removing Session"));
    return;
  }
  loadAllSession();
  readSession(0);
  sessionList->setCurrentItem(0);
  removeButton->setEnabled(sessionList->count()>1);
}

void SessionEditor::sessionModified()
{
  sesMod=true;
  emit changed();
}

void SessionEditor::sessionModified(int)
{
  sesMod=true;
  emit changed();
}

void SessionEditor::sessionModified(const QString&)
{
  sesMod=true;
  emit changed();
}

void SessionEditor::iconModified(QString)
{
  sesMod=true;
  emit changed();
}
