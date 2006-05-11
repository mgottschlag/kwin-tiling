/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include "menuinfo.h"

#include <QFile>
#include <QWidget>

#include <kapplication.h>
#include <ksimpleconfig.h>
#include <klibloader.h>
#include <kstandarddirs.h>
#include <kpanelmenu.h>
#include <kparts/componentfactory.h>
#include <kauthorized.h>

class MenuInfo::Private
{
public:
    QString name;
    QString comment;
    QString icon;
    QString library;
    QString desktopfile;
};

MenuInfo::MenuInfo(const QString& desktopFile)
{
    d = new Private;
    KSimpleConfig df(locate("data", QString::fromLatin1("kicker/menuext/%1").arg(desktopFile)));
    df.setGroup("Desktop Entry");

    QStringList list = df.readEntry("X-KDE-AuthorizeAction", QStringList() );
    if (kapp && !list.isEmpty())
    {
       for(QStringList::ConstIterator it = list.begin();
           it != list.end();
           ++it)
       {
          if (!KAuthorized::authorize((*it).trimmed()))
             return;
       }
    }

    d->name = df.readEntry("Name");
    d->comment = df.readEntry("Comment");
    d->icon = df.readEntry("Icon");
    d->library = df.readEntry("X-KDE-Library");
    d->desktopfile = desktopFile;
}

MenuInfo::~MenuInfo()
{
    delete d;
}

QString MenuInfo::name() const
{
    return d->name;
}

QString MenuInfo::comment() const
{
    return d->comment;
}

QString MenuInfo::icon() const
{
    return d->icon;
}

QString MenuInfo::library() const
{
    return d->library;
}

QString MenuInfo::desktopFile() const
{
    return d->desktopfile;
}
bool MenuInfo::isValid() const    { return !d->name.isEmpty(); }

KPanelMenu* MenuInfo::load(QWidget *parent)
{
    if (d->library.isEmpty())
        return 0;

    return KLibLoader::createInstance<KPanelMenu>(
               QFile::encodeName( d->library ),
               parent );
}
