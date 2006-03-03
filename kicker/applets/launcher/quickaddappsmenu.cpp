/*****************************************************************

Copyright (c) 2000 Bill Nagel
   based on paneladdappsmenu.cpp which is
   Copyright (c) 1999-2000 the kicker authors

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

#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kglobalsettings.h>
#include <ksycocaentry.h>
#include <kservice.h>
#include <kservicegroup.h>

#include <kdebug.h>
#include "quickaddappsmenu.h"

QuickAddAppsMenu::QuickAddAppsMenu(const QString &label, const QString &relPath, QWidget *target, QWidget *parent, const char *name, const QString &sender)
   : PanelServiceMenu(label, relPath, parent, name)
{
   _targetObject = target;
   _sender = sender;
   connect(this, SIGNAL(addAppBefore(QString,QString)), 
           target, SLOT(addAppBeforeManually(QString,QString)));
}

QuickAddAppsMenu::QuickAddAppsMenu(QWidget *target, QWidget *parent, const QString &sender, const char *name)
   : PanelServiceMenu(QString(), QString(), parent, name)
{
   _targetObject = target;
   _sender = sender;
   connect(this, SIGNAL(addAppBefore(QString,QString)),
           target, SLOT(addAppBeforeManually(QString,QString)));
}

void QuickAddAppsMenu::slotExec(int id)
{
   if (!entryMap_.contains(id)) return;
   KService::Ptr service(KService::Ptr::staticCast(entryMap_[id]));
   emit addAppBefore(locate("apps", service->desktopEntryPath()),_sender);
}


PanelServiceMenu *QuickAddAppsMenu::newSubMenu(const QString &label, const QString &relPath, QWidget *parent, const char *name, const QString &insertInlineHeader)
{
   return new QuickAddAppsMenu(label, relPath, _targetObject, parent, name, _sender);
}
#include "quickaddappsmenu.moc"
