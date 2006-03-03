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

#include <qfileinfo.h>
#include <kdesktopfile.h>
#include <krandom.h>

#include "extensioninfo.h"

class ExtensionInfo::Private
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
    QString id;
    QString lib;
    QString desktopFile;
    QString configFile;
    QString desktopFilePath;
    ExtensionType type;
    bool unique;
    bool hidden;
};

ExtensionInfo::ExtensionInfo( const QString& deskFile, const QString& configFile, const ExtensionInfo::ExtensionType type)
{
    d = new Private;
    d->type = type;
    QFileInfo fi(deskFile);
    d->desktopFilePath = fi.absoluteFilePath();
    d->desktopFile = fi.fileName();

    KDesktopFile df(deskFile);

    // set the appletssimple attributes
    setName(df.readName());
    setComment(df.readComment());
    setIcon(df.readIcon());

    // library
    setLibrary(df.readEntry("X-KDE-Library"));

    // is it a unique applet?
    setIsUnique(df.readEntry("X-KDE-UniqueExtension", QVariant(false)).toBool());

    // should it be shown in the gui?
    d->hidden = df.readEntry("Hidden", QVariant(false)).toBool();

    if (configFile.isEmpty())
    {
        // generate a config file base name from the library name
        d->configFile = d->lib.lower();

        if (d->unique)
        {
            d->configFile.append("rc");
        }
        else
        {
            d->configFile.append("_")
                        .append(KRandom::randomString(20).lower())
                        .append("_rc");
        }
    }
    else
    {
        d->configFile = configFile;
    }
}

ExtensionInfo::ExtensionInfo(const ExtensionInfo &copy)
{
    d = new Private;
    *d = *copy.d;
}

ExtensionInfo::~ExtensionInfo()
{
    delete d;
}

ExtensionInfo& ExtensionInfo::operator=(const ExtensionInfo &rhs)
{
    *d = *rhs.d;

    return *this;
}

QString ExtensionInfo::name() const
{
    return d->name;
}

QString ExtensionInfo::comment() const
{
    return d->comment;
}

QString ExtensionInfo::icon() const
{
    return d->icon;
}

ExtensionInfo::ExtensionType ExtensionInfo::type() const
{
    return d->type;
}

QString ExtensionInfo::library() const
{
    return d->lib;
}

QString ExtensionInfo::desktopFilePath() const
{
    return d->desktopFilePath;
}

QString ExtensionInfo::desktopFile() const
{
    return d->desktopFile;
}

QString ExtensionInfo::configFile() const
{
    return d->configFile;
}

bool ExtensionInfo::isUniqueExtension() const
{
    return d->unique;
}

bool ExtensionInfo::isHidden() const
{
    return d->hidden;
}

void ExtensionInfo::setConfigFile(const QString &cf)
{
    d->configFile = cf;
}

void ExtensionInfo::setType(ExtensionType type)
{
    d->type = type;
}

void ExtensionInfo::setName(const QString &name)
{
    d->name = name;
}

void ExtensionInfo::setComment(const QString &comment)
{
    d->comment = comment;
}

void ExtensionInfo::setIcon(const QString &icon)
{
    d->icon = icon;
}

void ExtensionInfo::setId(const QString &id)
{
    d->id = id;
}

void ExtensionInfo::setLibrary(const QString &lib)
{
   d->lib = lib; 
}

void ExtensionInfo::setIsUnique(bool u)
{
    d->unique = u;
}

bool ExtensionInfo::operator!=( const ExtensionInfo& rhs) const
{
    return configFile() != rhs.configFile();
}

bool ExtensionInfo::operator==( const ExtensionInfo& rhs) const
{
    return configFile() == rhs.configFile();
}

bool ExtensionInfo::operator<( const ExtensionInfo& rhs ) const
{
    return name().lower() < rhs.name().lower();
}

bool ExtensionInfo::operator> ( const ExtensionInfo& rhs ) const
{
    return name().lower() > rhs.name().lower();
}

bool ExtensionInfo::operator<= ( const ExtensionInfo& rhs ) const
{
    return name().lower() <= rhs.name().lower();
}

