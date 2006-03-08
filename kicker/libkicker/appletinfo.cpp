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

#include <QBuffer>
#include <QFileInfo>
#include <QMimeData>
#include <QDataStream>

#include <kdesktopfile.h>
#include <krandom.h>

#include "appletinfo.h"

class AppletInfo::Private
{
public:
    Private()
      : type(Undefined),
        unique(true),
        hidden(false)
    {}

    QString name;
    QString comment;
    QString icon;
    QString lib;
    QString desktopFile;
    QString configFile;
    AppletType type;
    bool unique;
    bool hidden;
};

AppletInfo::AppletInfo( const QString& deskFile, const QString& configFile, const AppletInfo::AppletType type)
{
    d = new Private;
    d->type = type;
    QFileInfo fi(deskFile);
    d->desktopFile = fi.fileName();

    const char* resource = "applets";
    switch (type)
    {
        case Extension:
            resource = "extensions";
            break;
        case BuiltinButton:
            resource = "builtinbuttons";
            break;
        case SpecialButton:
            resource = "specialbuttons";
            break;
        case Undefined:
        case Applet:
        default:
            break;
    }

    KDesktopFile df(d->desktopFile, true, resource);

    // set the appletssimple attributes
    setName(df.readName());
    setComment(df.readComment());
    setIcon(df.readIcon());

    // library
    setLibrary(df.readEntry("X-KDE-Library"));

    // is it a unique applet?
    setIsUnique(df.readEntry("X-KDE-UniqueApplet", QVariant(false)).toBool());

    // should it be shown in the gui?
    d->hidden = df.readEntry("Hidden", QVariant(false)).toBool();

    if (configFile.isEmpty())
    {
        // generate a config file base name from the library name
        d->configFile = d->lib.toLower();

        if (d->unique)
        {
            d->configFile.append("rc");
        }
        else
        {
            d->configFile.append("_")
                         .append(KRandom::randomString(20).toLower())
                         .append("_rc");
        }
    }
    else
    {
        d->configFile = configFile;
    }
}

AppletInfo::AppletInfo(const AppletInfo &copy)
{
    d = new Private;
    *d = *copy.d;
}

AppletInfo::~AppletInfo()
{
    delete d;
}

AppletInfo& AppletInfo::operator=(const AppletInfo &rhs)
{
    *d = *rhs.d;

    return *this;
}

QString AppletInfo::name() const
{
    return d->name;
}

QString AppletInfo::comment() const
{
    return d->comment;
}

QString AppletInfo::icon() const
{
    return d->icon;
}

AppletInfo::AppletType AppletInfo::type() const
{
    return d->type;
}

QString AppletInfo::library() const
{
    return d->lib;
}

QString AppletInfo::desktopFile() const
{
    return d->desktopFile;
}

QString AppletInfo::configFile() const
{
    return d->configFile;
}

bool AppletInfo::isUniqueApplet() const
{
    return d->unique;
}

bool AppletInfo::isHidden() const
{
    return d->hidden;
}

void AppletInfo::setConfigFile(const QString &cf)
{
    d->configFile = cf;
}

void AppletInfo::setType(AppletType type)
{
    d->type = type;
}

void AppletInfo::populateMimeData(QMimeData* mimeData)
{
    QByteArray a;
    QDataStream s(&a, QIODevice::WriteOnly);
    s << desktopFile() << configFile() << type();
    mimeData->setData("application/x-kde-plasma-AppletInfo", a);
}

bool AppletInfo::canDecode(const QMimeData* mimeData)
{
    return mimeData->hasFormat("application/x-kde-plasma-AppletInfo");
}

AppletInfo AppletInfo::fromMimeData(const QMimeData* mimeData)
{
    QByteArray a = mimeData->data("application/x-kde-plasma-AppletInfo");

    if (a.isEmpty())
    {
        return AppletInfo();
    }

    QBuffer buff(&a);
    buff.open(QIODevice::ReadOnly);
    QDataStream s(&buff);

    QString desktopFile;
    QString configFile;
    int type;
    s >> desktopFile >> configFile >> type;
    AppletInfo info(desktopFile, configFile, (AppletInfo::AppletType)type);
    return info;
}

void AppletInfo::setName(const QString &name)
{
    d->name = name;
}

void AppletInfo::setComment(const QString &comment)
{
    d->comment = comment;
}

void AppletInfo::setIcon(const QString &icon)
{
    d->icon = icon;
}

void AppletInfo::setLibrary(const QString &lib)
{
   d->lib = lib;
}

void AppletInfo::setIsUnique(bool u)
{
    d->unique = u;
}

bool AppletInfo::operator!=( const AppletInfo& rhs) const
{
    return configFile() != rhs.configFile();
}

bool AppletInfo::operator==( const AppletInfo& rhs) const
{
    return configFile() == rhs.configFile();
}

bool AppletInfo::operator<( const AppletInfo& rhs ) const
{
    return name().toLower() < rhs.name().toLower();
}

bool AppletInfo::operator> ( const AppletInfo& rhs ) const
{
    return name().toLower() > rhs.name().toLower();
}

bool AppletInfo::operator<= ( const AppletInfo& rhs ) const
{
    return name().toLower() <= rhs.name().toLower();
}

