#ifndef __UI_CONFIG_H__
#define __UI_CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CUiConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 19/06/2002
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include <qstring.h>
#include <qstringlist.h>
#include <qsize.h>
#include <kconfig.h>

//
// This class is used to store UI configuration - ie. data that is always saved at the end of the
// session, regardless of whether "Apply" was selected or not.
//
class CUiConfig : public KConfig
{
    public:

    enum EMode
    {
        BASIC,
        ADVANCED,
        ADVANCED_PLUS_FS
    };

    CUiConfig();
    virtual ~CUiConfig();

    EMode               getMode()         { return itsMode; }
    const QStringList & getOpenInstDirs() { return itsOpenInstDirs; } 
    const QStringList & getOpenFsDirs()   { return itsOpenFsDirs; }
    const QString &     getInstTopItem()  { return itsInstTopItem; }
    const QString &     getFsTopItem()    { return itsFsTopItem; }
    const QSize &       getMainSize()     { return itsMainSize; }

    bool isAdvancedMode()                 { return BASIC!=itsMode; }

    void setMode(EMode m);
    void addOpenInstDir(const QString &d);
    void removeOpenInstDir(const QString &d);
    void addOpenFsDir(const QString &d);
    void removeOpenFsDir(const QString &d);
    void setInstTopItem(const QString &s);
    void setFsTopItem(const QString &s);
    void setMainSize(const QSize &s);

    private:

    void write(const QString &key, const QSize &value);
    void write(const QString &key, const QStringList &value);
    void write(const QString &key, const QString &value);
    void write(const QString &key, int value);
    void storeInList(QStringList &list, const QString &s);

    private:

    EMode       itsMode;
    QStringList itsOpenInstDirs,
                itsOpenFsDirs;
    QString     itsInstTopItem,
                itsFsTopItem;
    bool        itsAutoSync,
                itsKCmShell;
    QSize       itsMainSize;
};

#endif
