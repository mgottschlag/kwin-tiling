#ifndef __KIOFONTS_H__
#define __KIOFONTS_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKioFonts
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 05/03/2003
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

#include <kio/slavebase.h>
#include <kurl.h>
#include <klocale.h>
#include <qstring.h>
#include <qcstring.h>
#include <qfile.h>
#include <qstringlist.h>
#include "Misc.h"
#include "KfiConfig.h"
#include "Global.h"

class CKioFonts : public KIO::SlaveBase
{
    private:

    enum EDest
    {
        DEST_UNCHANGED,
        DEST_SYS,
        DEST_USER
    };

    public:

    CKioFonts(const QCString &pool, const QCString &app);
    virtual ~CKioFonts();

    void listDir(const KURL &url);
    int  getSize(const QStringList &top, const QString &sub, bool sys=true);
    void listDir(const QStringList &top, const QString &sub, const KURL &url, bool sys=true);
    void stat(const KURL &url);
    bool createStatEntry(KIO::UDSEntry &entry, const KURL &url, bool sys=true);
    void get(const KURL &url);
    void put(const KURL &url, int mode, bool overwrite, bool resume);
    void copy(const KURL &src, const KURL &dest, int mode, bool overwrite);
    void rename(const KURL &src, const KURL &dest, bool overwrite);
    void mkdir(const KURL &url, int permissions);
    void chmod(const KURL &url, int permissions);
    void del(const KURL &url, bool isFile);

    private:

    bool    putReal(const QString &destOrig, const QCString &destOrigC, bool origExists,
                    int mode, bool resume);
    bool    addDir(const QString &ds);
    void    cfgDir(const QString &ds, const QString &sub);
    void    syncDirs();
    void    deletedDir(const QString &d, bool sys=false);
    void    addedDir(const QString &d, bool sys=false);
    void    modifiedDir(const QString &d, bool sys=false);
    void    special(const QByteArray &a);
    void    doModifiedDirs();
    QString getRootPasswd(bool askPasswd=true);
    bool    doRootCmd(const char *cmd, const QString &passwd);
    bool    doRootCmd(const char *cmd, bool askPasswd=true) { return doRootCmd(cmd, getRootPasswd(askPasswd)); }
    bool    confirmUrl(KURL &url);

    static QString convertUrl(const KURL &url, bool checkExists);

    private:

    QStringList itsModifiedDirs,
                itsModifiedSysDirs;
    QString     itsPasswd;
    bool        itsCanStorePasswd;
    uint        itsNewFonts;
    EDest       itsLastDest;
    time_t      itsLastDestTime;
};

#endif
