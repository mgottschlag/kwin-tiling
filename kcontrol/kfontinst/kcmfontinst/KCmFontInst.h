#ifndef __KCM_FONT_INST_H__
#define __KCM_FONT_INST_H__

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "GroupList.h"
#include "JobRunner.h"
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
class KToggleAction;
class KActionMenu;
class QLabel;
class QSplitter;
class QComboBox;
class QGridLayout;

namespace KFI
{

class CFontFilter;
class CFontList;
class CFontPreview;
class CUpdateDialog;
class CFontListView;
class CProgressBar;
class CPreviewSelectAction;

class CKCmFontInst : public KCModule
{
    Q_OBJECT

    public:

    CKCmFontInst(QWidget *parent=NULL, const QStringList &list=QStringList());
    virtual ~CKCmFontInst();

    public Q_SLOTS:

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
    void    exported(KIO::Job *job, const KUrl &from, const KUrl &to);
    void    changeText();
    void    showPreview(bool s);
    void    duplicateFonts();
    void    print();
    void    printGroup();
    void    listingCompleted();
    void    refreshFamilies();
    void    setStatusBar();
    void    addFonts(const QSet<KUrl> &src);
    void    toggleFontManagement(bool on);
    void    selectGroup(int grp);

    private:

    void    print(bool all);
    void    deleteFonts(CJobRunner::ItemList &urls, const QStringList &fonts, bool hasSys);
    void    toggleGroup(bool enable);
    void    toggleFonts(bool enable, const QString &grp=QString());
    void    toggleFonts(CJobRunner::ItemList &urls, const QStringList &fonts, bool enable, const QString &grp,
                        bool hasSys);
    bool    working(bool displayMsg=true);
    KUrl    baseUrl(bool sys);
    void    selectMainGroup();
    void    doCmd(CJobRunner::ECommand cmd, const CJobRunner::ItemList &urls, const KUrl &dest);
    CGroupListItem::EType getCurrentGroupType();

    private:

    QWidget              *itsGroupsWidget,
                         *itsFontsWidget,
                         *itsPreviewWidget;
    QComboBox            *itsModeControl;
    QAction              *itsModeAct;
    QSplitter            *itsSplitter;
    CFontPreview         *itsPreview;
    KConfig              itsConfig;
    QLabel               *itsStatusLabel;
    CProgressBar         *itsListingProgress;
    CFontList            *itsFontList;
    CFontListView        *itsFontListView;
    CGroupList           *itsGroupList;
    CGroupListView       *itsGroupListView;
    CPreviewSelectAction *itsPreviewControl;
    KToggleAction        *itsMgtMode,
                         *itsShowPreview;
    KActionMenu          *itsToolsMenu;
    KPushButton          *itsDeleteGroupControl,
                         *itsEnableGroupControl,
                         *itsDisableGroupControl,
                         *itsAddFontControl,
                         *itsDeleteFontControl,
                         *itsEnableFontControl,
                         *itsDisableFontControl;
    CFontFilter          *itsFilter;
    QString              itsLastStatusBarMsg;
    KIO::Job             *itsJob;
    KProgressDialog      *itsProgress;
    CUpdateDialog        *itsUpdateDialog;
    KTempDir             *itsTempDir;
    KProcess             *itsPrintProc;
    KZip                 *itsExportFile;
    QSet<QString>        itsDeletedFonts;
    KUrl::List           itsModifiedUrls;
    CJobRunner           *itsRunner;
};

}

#endif
