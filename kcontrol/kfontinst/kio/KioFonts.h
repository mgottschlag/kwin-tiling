#ifndef __KIO_FONTS_H__
#define __KIO_FONTS_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <fontconfig/fontconfig.h>
#include <time.h>
#include <kio/slavebase.h>
#include <kurl.h>
#include <klocale.h>
#include <QString>
#include <QByteArray>
#include <QHash>
#include <QSet>
#include <QList>
#include <QVariant>
#include "Misc.h"
#include "KfiConstants.h"
#include "DisabledFonts.h"
#include "Server.h"
#include "Helper.h"

namespace KFI
{

class CSuProc;
class CSocket;

class CKioFonts : public KIO::SlaveBase
{
    private:

    enum EFileType
    {
        FILE_UNKNOWN,
        FILE_FONT,
        FILE_METRICS
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
        OP_DELETE,
        OP_ENABLE,
        OP_DISABLE
    };

    class CDirList : public QSet<QString>
    {
        public:

        CDirList()                   { }
        CDirList(const QString &str) { add(str); }
        void add(const QString &d)   { insert(d); }
    };

    struct TFontDetails
    {
        TFontDetails() { }

        CDisabledFonts::TFileList files;
        unsigned long             styleVal;
    };

    typedef QHash<QString, TFontDetails> TFontMap;

    struct TFolder
    {
        TFolder() : disabled(NULL) { }
        void  setLocation(const QString &l, const QString &d, bool sys);

        QString        location;
        CDirList       modified;
        TFontMap       fontMap;   // Maps from "Times New Roman" -> $HOME/.fonts/times.ttf
        CDisabledFonts *disabled;
    };

    public:

    struct TCommand
    {
        TCommand(ECommands c) : cmd(c)                    { }
        TCommand(ECommands c, const QVariant &v) : cmd(c) { args.append(v); }
        ECommands       cmd;
        QList<QVariant> args;
    };

    public:

    CKioFonts(const QByteArray &pool, const QByteArray &app);
    virtual ~CKioFonts();

    static QString     getSect(const QString &f) { return f.section('/', 1, 1); }

    void               listDir(const KUrl &url);
    void               listDir(EFolder folder, KIO::UDSEntry &entry);
    void               stat(const KUrl &url);
    bool               createStatEntry(KIO::UDSEntry &entry, const KUrl &url, EFolder folder);
    bool               createStatEntryReal(KIO::UDSEntry &entry, const KUrl &url, EFolder folder);
    void               get(const KUrl &url);
    void               put(const KUrl &url, int mode, bool overwrite, bool resume);
    void               copy(const KUrl &src, const KUrl &dest, int mode, bool overwrite);
    void               rename(const KUrl &src, const KUrl &dest, bool overwrite);
    void               del(const KUrl &url, bool isFile);

    private:

    QString            getUserName(uid_t uid);
    QString            getGroupName(gid_t gid);
    bool               createFontUDSEntry(KIO::UDSEntry &entry, const QString &name,
                                          const CDisabledFonts::TFileList &patterns,
                                          unsigned long styleVal, bool sys, bool hidden=false);
    bool               createFolderUDSEntry(KIO::UDSEntry &entry, const QString &name, const QString &path,
                                            bool sys);
    bool               putReal(const QString &destOrig, const QByteArray &destOrigC, bool origExists,
                               int mode, bool resume);
    void               modified(int timeout, EFolder folder, bool clearList=true,
                                const CDirList &dirs=CDirList());
    void               special(const QByteArray &a);
    bool               configure(EFolder folder);
    void               doModified();
    QString            getRootPasswd(bool askPasswd=true);
    bool               doRootCmd(QList<TCommand> &cmd, const QString &passwd);
    bool               doRootCmd(const TCommand &cmd, bool askPasswd=true);
    bool               doRootCmd(const TCommand &cmd, const QString &passwd);
    bool               doRootCmd(QList<TCommand> &cmd, bool askPasswd=true)
                           { return doRootCmd(cmd, getRootPasswd(askPasswd)); }
    void               correctUrl(KUrl &url);
    void               clearFontList();
    bool               updateFontList(bool initial=false);
    EFolder            getFolder(const KUrl &url);
    TFontMap::Iterator getMap(const KUrl &url);
    const CDisabledFonts::TFileList * getEntries(const KUrl &url, TFontMap::Iterator &enabledIt,
                                                 CDisabledFonts::TFontList::Iterator &disabledIt);
    QStringList        getFontNameEntries(EFolder folder, const QString &file, bool disabledFonts);
    QMap<int, QString> getFontIndexToNameEntries(EFolder folder, const QString &file);
    QString *          getEntry(EFolder folder, const QString &file, bool full=false);
    EFileType          checkFile(const QString &file, const KUrl &url);
    bool               getSourceFiles(const KUrl &src, CDisabledFonts::TFileList &files,
                                      bool removeSymLinks=true);
    bool               checkDestFile(const KUrl &src, const KUrl &dest, EFolder destFolder,
                                     bool overwrite);
    bool               checkDestFiles(const KUrl &src, QMap<QString, QString> &map, const KUrl &dest,
                                      EFolder destFolder, bool overwrite);
    bool               confirmMultiple(const KUrl &url, const CDisabledFonts::TFileList &files,
                                       EFolder folder, EOp op);
    bool               confirmMultiple(const KUrl &url, const CDisabledFonts::TFileList *patterns,
                                       EFolder folder, EOp op);
    bool               checkUrl(const KUrl &u, bool rootOk=false, bool logError=true);
    bool               checkAllowed(const KUrl &u);
    void               createAfm(const QString &file, bool nrs=false,
                                 const QString &passwd=QString::null);
    int                reconfigTimeout();

    private:

    bool                  itsRoot,
                          itsAddToSysFc;
    QString               itsPasswd;
    QByteArray            itsHelper;
    time_t                itsLastFcCheckTime;
    FcFontSet             *itsFontList;
    TFolder               itsFolders[FOLDER_COUNT];
    QHash<uid_t, QString> itsUserCache;
    QHash<gid_t, QString> itsGroupCache;
    CServer               itsServer;
    CSocket               *itsSocket;
    CSuProc               *itsSuProc;
};

}

#endif
