#ifndef __KCM_FONT_INST_H__
#define __KCM_FONT_INST_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "GroupList.h"
#include <QStringList>
#include <QSet>
#include <kcmodule.h>
#include <kurl.h>
#include <kconfig.h>
#include <kio/job.h>

class KPushButton;
class KProgressDialog;
class KTreeWidgetSearchLineWidget;
class KTempDir;
class KProcess;
class KZip;
class KSelectAction;
class QLabel;
class QSplitter;
class QComboBox;
class QGridLayout;

namespace KFI
{

class CFontList;
class CFontPreview;
class CUpdateDialog;
class CFontListView;

class CKCmFontInst : public KCModule
{
    Q_OBJECT

    public:

    CKCmFontInst(QWidget *parent=NULL, const QStringList &list=QStringList());
    virtual ~CKCmFontInst();

    public Q_SLOTS:

    void    displayType(int p);
    QString quickHelp() const;
    void    fontSelected(const QModelIndex &index, bool en, bool dis);
    void    groupSelected(const QModelIndex &index);
    void    reload();
    void    addFonts();
    void    deleteFonts();
    void    enableFonts();
    void    disableFonts();
    void    addGroup();
    void    removeGroup();
    void    enableGroup();
    void    disableGroup();
    void    exportGroup();
    void    exportJobResult(KJob *job);
    void    exported(KIO::Job *job, const KUrl &from, const KUrl &to, bool dir, bool renamed);
    void    changeText();
    void    print();
    void    printGroup();
    void    initialJobResult(KJob *job);
    void    jobResult(KJob *job);
    void    infoMessage(KJob *job, const QString &msg);
    void    listingCompleted();
    void    setStatusBar();
    void    addFonts(const QSet<KUrl> &src);
    void    toggleFontManagement(bool on);
    void    selectGroup(int grp);

    private:

    void    print(bool all);
    void    deleteFonts(QStringList &files, KUrl::List &urls, bool hasSys, bool hasUser);
    void    toggleGroup(bool enable);
    void    toggleFonts(bool enable, const QString &grp=QString());
    void    toggleFonts(QStringList &files, KUrl::List &urls, bool enable, const QString &grp,
                        bool hasSys, bool hasUser);
    bool    working(bool displayMsg=true);
    KUrl    baseUrl(bool sys);
    void    selectMainGroup();
    bool    getPasswd(bool required);
    void    setMetaData(KIO::Job *job);

    private:

    QWidget           *itsGroupWidget,
                      *itsFontWidget;
    QComboBox         *itsModeControl,
                      *itsPreviewType;
    QSplitter         *itsSplitter;
    CFontPreview      *itsPreview;
    KConfig           itsConfig;
    QLabel            *itsStatusLabel;
    CFontList         *itsFontList;
    CFontListView     *itsFontListView;
    CGroupList        *itsGroupList;
    CGroupListView    *itsGroupListView;
    KPushButton       *itsMgtMode,
                      *itsDeleteGroupControl,
                      *itsEnableGroupControl,
                      *itsDisableGroupControl,
                      *itsAddFontControl,
                      *itsDeleteFontControl,
                      *itsEnableFontControl,
                      *itsDisableFontControl;
    QString           itsLastStatusBarMsg,
                      itsPasswd;
    KIO::Job          *itsJob;
    KProgressDialog   *itsProgress;
    CUpdateDialog     *itsUpdateDialog;
    KTempDir          *itsTempDir;
    KProcess          *itsPrintProc;
    KZip              *itsExportFile;
    QSet<QString>     itsDeletedFonts;
    KUrl::List        itsModifiedUrls;
};

}

#endif
