#ifndef __KIO_FONTS_H__
#define __KIO_FONTS_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CKioFonts
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <fontconfig/fontconfig.h>
#include <time.h>
#include <kio/slavebase.h>
#include <kurl.h>
#include <klocale.h>
#include <QString>
#include <QMap>
#include "Misc.h"
#include "KfiConstants.h"

namespace KFI
{

class CKioFonts : public KIO::SlaveBase
{
    private:

    enum EConstants
    {
        KFI_PARAMS = 8
    };

    enum EDest
    {
        DEST_UNCHANGED,
        DEST_SYS,
        DEST_USER
    };

    enum EFolder
    {
        FOLDER_SYS,
        FOLDER_USER,

        FOLDER_COUNT
    };

    enum EOp
    {
        OP_COPY,
        OP_MOVE,
        OP_DELETE
    };

    class CDirList : public QStringList
    {
        public:

        CDirList()                                      { }
        CDirList(const QString &str) : QStringList(str) { }

        void add(const QString &d)                      { if (!contains(d)) append(d); }
    };

    struct TFolder
    {
        QString                                 location;
        CDirList                                modified;
        QMap<QString, QList<FcPattern *> > fontMap;   // Maps from "Times New Roman" -> $HOME/.fonts/times.ttf
    };

    public:

    CKioFonts(const QByteArray &pool, const QByteArray &app);
    virtual ~CKioFonts();

    static QString getSect(const QString &f) { return f.section('/', 1, 1); }

    void listDir(const KUrl &url);
    void stat(const KUrl &url);
    bool createStatEntry(KIO::UDSEntry &entry, const KUrl &url, EFolder folder);
    void get(const KUrl &url);
    void put(const KUrl &url, int mode, bool overwrite, bool resume);
    void copy(const KUrl &src, const KUrl &dest, int mode, bool overwrite);
    void rename(const KUrl &src, const KUrl &dest, bool overwrite);
    void del(const KUrl &url, bool isFile);

    private:

    bool     putReal(const QString &destOrig, const QByteArray &destOrigC, bool origExists, int mode, bool resume);
    void     modified(EFolder folder, const CDirList &dirs=CDirList());
    void     special(const QByteArray &a);
    void     createRootRefreshCmd(QByteArray &cmd, const CDirList &dirs=CDirList(), bool reparseCfg=true);
    void     doModified();
    QString  getRootPasswd(bool askPasswd=true);
    bool     doRootCmd(const char *cmd, const QString &passwd);
    bool     doRootCmd(const char *cmd, bool askPasswd=true) { return doRootCmd(cmd, getRootPasswd(askPasswd)); }
    bool     confirmUrl(KUrl &url);
    void     clearFontList();
    bool     updateFontList();
    EFolder  getFolder(const KUrl &url);
    QMap<QString, QList<FcPattern *> >::Iterator getMap(const KUrl &url);
    QList<FcPattern *> * getEntries(const KUrl &url);
    FcPattern * getEntry(EFolder folder, const QString &file, bool full=false);
    bool     checkFile(const QString &file);
    bool     getSourceFiles(const KUrl &src, QStringList &files);
    bool     checkDestFiles(const KUrl &src, QMap<QString, QString> &map, const KUrl &dest, EFolder destFolder, bool overwrite);
    bool     confirmMultiple(const KUrl &url, const QStringList &files, EFolder folder, EOp op);
    bool     confirmMultiple(const KUrl &url, QList<FcPattern *> *patterns, EFolder folder, EOp op);
    bool     checkUrl(const KUrl &u, bool rootOk=false);
    bool     checkAllowed(const KUrl &u);
    void     createAfm(const QString &file, bool nrs=false, const QString &passwd=QString());
    void     reparseConfig();

    private:

    bool         itsRoot,
                 itsCanStorePasswd,
                 itsUsingFcFpe,
                 itsUsingXfsFpe,
                 itsHasSys,
                 itsAddToSysFc;
    QString      itsPasswd;
    unsigned int itsFontChanges;
    EDest        itsLastDest;
    time_t       itsLastDestTime,
                 itsLastFcCheckTime;
    FcFontSet    *itsFontList;
    TFolder      itsFolders[FOLDER_COUNT];
    char         itsNrsKfiParams[KFI_PARAMS],
                 itsNrsNonMainKfiParams[KFI_PARAMS],
                 itsKfiParams[KFI_PARAMS];
};

}

#endif
