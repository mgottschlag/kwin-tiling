////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKCmFontInst
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 26/04/2003
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

#include "KCmFontInst.h"
#include <qlayout.h>
#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kdiroperator.h>
#include "Misc.h"
#include "FontEngine.h"
#include "KioFonts.h"
#include "KFileFontView.h"
#include "RenameJob.h"
#include "kfileiconview.h"
#include <kpopupmenu.h>
#include <ktoolbar.h>
#include <qsplitter.h>
#include <kstdaccel.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <kio/job.h>
#include <kio/netaccess.h>

// HACK - Can't sym-link in CVS?
#include "../viewpart/FontPreview.cpp"

#define CFG_GROUP       "Main Settings"
#define CFG_LISTVIEW    "ListView"
#define CFG_PATH        "Path"
#define CFG_DIRSIZE     "DirSize"
#define CFG_PREVIEWSIZE "PreviewSize"

typedef KGenericFactory<CKCmFontInst, QWidget> FontInstallFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_fontinst, FontInstallFactory)

CKCmFontInst::CKCmFontInst(QWidget *parent, const char *, const QStringList&)
            : KCModule(parent, "fontinst"),
              itsAboutData(NULL),
              itsTop(CMisc::root() ? "fonts:/" : QString("fonts:/")+i18n(KIO_FONTS_USER_DIR)),
              itsConfig("kcmfontinstuirc")
{
    KConfigGroupSaver cfgSaver(&itsConfig, CFG_GROUP);
    const char *appName=KCmdLineArgs::appName();

    itsAutoSync=CMisc::root() && (NULL==appName || strcmp("kcontrol", appName));

    itsSplitter=new QSplitter(this);

    QFrame            *fontsFrame=new QGroupBox(itsSplitter),
                      *previewFrame=new QGroupBox(itsSplitter),
                      *urlFrame=new QGroupBox(this);
    QGridLayout       *fontsLayout=new QGridLayout(fontsFrame, 1, 1, 0, 1),
                      *previewLayout=new QGridLayout(previewFrame, 1, 1, 1, 1);
    QHBoxLayout       *urlLayout=new QHBoxLayout(urlFrame, 4);
    QVBoxLayout       *layout=new QVBoxLayout(this, 4);
    KToolBar          *toolbar=new KToolBar(this);

    fontsFrame->setLineWidth(0);
    previewFrame->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
    urlFrame->setFrameShadow(QFrame::Sunken);
    urlFrame->setFrameShape(QFrame::StyledPanel);
    urlFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    itsLabel = new QLabel(urlFrame);
    itsLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    urlLayout->addWidget(new QLabel(i18n("Location: "), urlFrame));
    urlLayout->addWidget(itsLabel);
    toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    toolbar->setMovingEnabled(false);
    itsSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QValueList<int> sizes;

    sizes.append(itsConfig.readNumEntry(CFG_DIRSIZE, 3));
    sizes.append(itsConfig.readNumEntry(CFG_PREVIEWSIZE, 2));
    itsSplitter->setSizes(sizes);

    QString previousPath=itsConfig.readEntry(CFG_PATH);
    KURL    url(itsTop);

    if(!previousPath.isNull())
    {
        KIO::UDSEntry uds;

        url.setPath(previousPath);
        if(!KIO::NetAccess::stat(url, uds))
            url=itsTop;
    }

    itsDirOp = new KDirOperator(url, fontsFrame);
    itsDirOp->setViewConfig(&itsConfig, "ListView Settings");
    itsDirOp->setMinimumSize(QSize(96, 64));

    QStringList mimeTypes;

    mimeTypes << "application/x-font-ttf"
              << "application/x-font-otf"
              << "application/x-font-ttc"
              << "application/x-font-type1"
              << "application/x-font-bdf"
              << "application/x-font-pcf"
              << "application/x-font-snf"
              << "application/x-font-speedo"
              << "fonts/folder"
              << "fonts/system-folder";

    itsDirOp->setMimeFilter(mimeTypes);
    fontsLayout->addWidget(itsDirOp, 0, 0);

    itsPreview = new CFontPreview(previewFrame);
//    itsPreview->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0,
//                              itsPreview->sizePolicy().hasHeightForWidth()));
    itsPreview->setMinimumSize(QSize(96, 64));
    previewLayout->addWidget(itsPreview, 0, 0);

    layout->addWidget(toolbar);
    layout->addWidget(urlFrame);
    layout->addWidget(itsSplitter);

    setButtons(0);
    setUseRootOnlyMsg(false);
    itsDirOp->setMode((KFile::Mode)(KFile::Directory|KFile::Files));

    //
    // Now for the hack!
    KAction          *act;
    KActionMenu      *topMnu=dynamic_cast<KActionMenu *>(itsDirOp->actionCollection()->action("popupMenu"));
    KActionSeparator *sep=new KActionSeparator(itsDirOp->actionCollection(), "separator" );

    itsViewMenuAct=dynamic_cast<KActionMenu *>(itsDirOp->actionCollection()->action("view menu"));
    topMnu->popupMenu()->clear();
    if((itsUpAct=itsDirOp->actionCollection()->action("up")))
    {
        itsUpAct->disconnect(SIGNAL(activated()), itsDirOp, SLOT(cdUp()));
        connect(itsUpAct, SIGNAL(activated()), SLOT(goUp()));
        itsUpAct->plug(toolbar);
        if(url==itsTop)
            itsUpAct->setEnabled(false);
        topMnu->insert(itsUpAct);
    }

    if((act=itsDirOp->actionCollection()->action("back")))
    {
        act->disconnect(SIGNAL(activated()), itsDirOp, SLOT(back()));
        connect(act, SIGNAL(activated()), SLOT(goBack()));
        act->plug(toolbar);
        act->setEnabled(false);
        topMnu->insert(act);
    }

    if((act=itsDirOp->actionCollection()->action("forward")))
    {
        act->disconnect(SIGNAL(activated()), itsDirOp, SLOT(forward()));
        connect(act, SIGNAL(activated()), SLOT(goForward()));
        act->plug(toolbar);
        act->setEnabled(false);
        topMnu->insert(act);
    }

    if((act=itsDirOp->actionCollection()->action("home")))
    {
        act->setIcon("fonts");
        act->setText("Top Level");
        act->disconnect(SIGNAL(activated()), itsDirOp, SLOT(home()));
        connect(act, SIGNAL(activated()), SLOT(gotoTop()));
        act->plug(toolbar);
        topMnu->insert(act);
    }

    if((act=itsDirOp->actionCollection()->action("reload")))
        act->plug(toolbar);

    topMnu->insert(sep);
    toolbar->insertLineSeparator();
    topMnu->insert(itsViewMenuAct);

    if((itsIconAct=dynamic_cast<KRadioAction *>(itsDirOp->actionCollection()->action("short view"))))
    {
        disconnect(itsIconAct, SIGNAL(activated()), itsDirOp, SLOT(slotSimpleView()));
        connect(itsIconAct, SIGNAL(activated()), SLOT(iconView()));
        itsIconAct->plug(toolbar);
    }

    if((itsListAct=dynamic_cast<KRadioAction *>(itsDirOp->actionCollection()->action("detailed view"))))
    {
        disconnect(itsListAct, SIGNAL(activated()), itsDirOp, SLOT(slotDetailedView()));
        connect(itsListAct, SIGNAL(activated()), SLOT(listView()));
        itsListAct->plug(toolbar);
    }

    topMnu->insert(sep);
    toolbar->insertLineSeparator();

    if((act=itsDirOp->actionCollection()->action("mkdir")))
    {
        act->plug(toolbar);
        topMnu->insert(act);
    }

    act=new KAction(i18n("Add Fonts..."), "filenew2", 0, this, SLOT(addFonts()), itsDirOp->actionCollection(), "addfonts");
    act->plug(toolbar);
    topMnu->insert(act);

    if((itsDeleteAct=itsDirOp->actionCollection()->action("delete")))
    {
        itsDeleteAct->plug(toolbar);
        itsDeleteAct->setEnabled(false);
        topMnu->insert(itsDeleteAct);
        disconnect(itsDeleteAct, SIGNAL(activated()), itsDirOp, SLOT(deleteSelected()));
        connect(itsDeleteAct, SIGNAL(activated()), this, SLOT(removeFonts()));
    }

    toolbar->insertLineSeparator();
    itsEnableAct=new KAction(i18n("Enable"), "button_ok", 0, this, SLOT(enable()), itsDirOp->actionCollection(), "enable");
    itsEnableAct->setEnabled(false);
    itsEnableAct->plug(toolbar);
    topMnu->insert(itsEnableAct);
    itsDisableAct=new KAction(i18n("Disable"), "button_cancel", 0, this, SLOT(disable()), itsDirOp->actionCollection(), "disable");
    itsDisableAct->setEnabled(false);
    itsDisableAct->plug(toolbar);
    topMnu->insert(itsDisableAct);
    topMnu->insert(sep);

    if((act=itsDirOp->actionCollection()->action("properties")) )
        topMnu->insert(act);

    if( (itsSepDirsAct=itsDirOp->actionCollection()->action("separate dirs")) &&
        (itsShowHiddenAct=itsDirOp->actionCollection()->action("show hidden")))
    {
        //disconnect(itsViewMenuAct->popupMenu(), SIGNAL(aboutToShow()), itsDirOp, SLOT(insertViewDependentActions()));
        connect(itsViewMenuAct->popupMenu(), SIGNAL(aboutToShow()), SLOT(setupViewMenu()));
        setupViewMenu();
    }

    setUpAct();

    //
    // Set view...
    if(itsConfig.readBoolEntry(CFG_LISTVIEW, true))
        listView();
    else
        iconView();

    itsDirOp->dirLister()->setShowingDotFiles(true);

    //
    // Enter url...
    urlEntered(url);

    connect(itsDirOp, SIGNAL(urlEntered(const KURL &)), SLOT(urlEntered(const KURL &)));
    connect(itsDirOp, SIGNAL(fileHighlighted(const KFileItem *)), SLOT(fileHighlighted(const KFileItem *)));
    connect(itsDirOp, SIGNAL(fileSelected(const KFileItem *)), SLOT(fileSelected(const KFileItem *)));
    connect(itsDirOp, SIGNAL(finishedLoading()), SLOT(loadingFinished()));
}

CKCmFontInst::~CKCmFontInst()
{
    KConfigGroupSaver         cfgSaver(&itsConfig, CFG_GROUP);
    QValueList<int>           list=itsSplitter->sizes();
    QValueList<int>::Iterator it;
    int                       num;

    for(it=list.begin(), num=0; it!=list.end() && num<2; ++it, num++)
        itsConfig.writeEntry(0==num ? CFG_DIRSIZE : CFG_PREVIEWSIZE, *it);

    if(itsAboutData)
        delete itsAboutData;
    delete itsDirOp;

    CGlobal::destroy();
}

const KAboutData * CKCmFontInst::aboutData() const
{
    if(!itsAboutData)
    {
        CKCmFontInst *that = const_cast<CKCmFontInst *>(this);

        that->itsAboutData=new KAboutData("kcmfontinst",
                                          I18N_NOOP("KDE Font Installer"),
                                          0, 0,
                                          KAboutData::License_GPL,
                                          I18N_NOOP("GUI front end to the fonts:/ ioslave.\n"
                                                    "(c) Craig Drummond, 2000 - 2003"));
        that->itsAboutData->addAuthor("Craig Drummond", "Developer and maintainer", "craig@kde.org");
    }

    return itsAboutData;
}

QString CKCmFontInst::quickHelp() const
{
    QString help(i18n("<h1>Font Installer</h1><p> This module allows you to"
                      " install TrueType, Type1, Speedo, and Bitmap"
                      " fonts.</p><p>You may also install fonts using konqueror,"
                      " type fonts:/ into konqueror's location bar - "
                      " and this will display your installed fonts. To install a "
                      " font, simply copy one into the appropriate folder.</p>")),
            rootHelp(i18n("<p><b>NOTE:</b> As you are not logged in as \"root\", any"
                          " fonts installed will only be available to you. To install"
                          " fonts system wide, use the \"Administrator Mode\""
                          " button to run this module as \"root\".</p>"));

    return CMisc::root() ? help : help+rootHelp;
}

void CKCmFontInst::gotoTop()
{
    itsDirOp->setURL(itsTop, true);
    setUpAct();
}

void CKCmFontInst::goUp()
{
    itsDirOp->cdUp();
    setUpAct();
}

void CKCmFontInst::goBack()
{
    itsDirOp->back();
    setUpAct();
}

void CKCmFontInst::goForward()
{
    itsDirOp->forward();
    setUpAct();
}

void CKCmFontInst::listView()
{
    CKFileFontView *newView=new CKFileFontView(itsDirOp, "detailed view");

    itsDirOp->setView(newView);
    itsListAct->setChecked(true);
    KConfigGroupSaver cfgSaver(&itsConfig, CFG_GROUP);
    itsConfig.writeEntry(CFG_LISTVIEW, true);
    if(itsAutoSync)
        itsConfig.sync();
}

void CKCmFontInst::iconView()
{
    KFileIconView *newView=new KFileIconView(itsDirOp, "simple view");

    itsDirOp->setView(newView);
    itsIconAct->setChecked(true);
    KConfigGroupSaver cfgSaver(&itsConfig, CFG_GROUP);
    itsConfig.writeEntry(CFG_LISTVIEW, false);
    if(itsAutoSync)
        itsConfig.sync();
}

void CKCmFontInst::setupViewMenu()
{
    itsViewMenuAct->remove(itsSepDirsAct);
    itsViewMenuAct->remove(itsShowHiddenAct);
}

static QString createLocationLabel(const KURL &url)
{
    QString               ret("<p>fonts:/");
    QStringList           list(QStringList::split('/', url.path()));
    QStringList::Iterator it;

    for(it=list.begin(); it!=list.end(); ++it)
    {
        if(QChar('.')==(*it)[0])
        {
            ret+=QString("<i>");
            ret+=*it;
            ret+=QString("</i>");
        }
        else
            ret+=*it;
        ret+=QChar('/');
    }

    ret+=QString("</p>");
    return ret;
}

void CKCmFontInst::urlEntered(const KURL &url)
{
    KConfigGroupSaver cfgSaver(&itsConfig, CFG_GROUP);

    itsConfig.writeEntry(CFG_PATH, url.path());

    itsEnableAct->setEnabled(false);
    itsDisableAct->setEnabled(false); 
    itsLabel->setText(createLocationLabel(url));
    if(itsAutoSync)
        itsConfig.sync();
}

void CKCmFontInst::fileHighlighted(const KFileItem *)
{
    const KFileItemList *list=itsDirOp->selectedItems();

    if(list && list->count())
    {
        KFileItemListIterator it(*list);
        bool                  doneEn=false,
                              doneDis=false;

        while(it.current() && !doneEn && !doneDis)
        {
            if(!doneEn && QChar('.')==(*it)->url().fileName()[0])
            {
                itsEnableAct->setEnabled(true);
                doneEn=true;
            }
            else if(!doneDis)
            {
                itsDisableAct->setEnabled(true);
                doneDis=true;
            }

            if(!doneDis)
                itsDisableAct->setEnabled(false);
            if(!doneEn)
                itsEnableAct->setEnabled(false);

            ++it;
        }
        itsDeleteAct->setEnabled(true);
    }
    else
    {
        itsDeleteAct->setEnabled(false);
        itsEnableAct->setEnabled(false);
        itsDisableAct->setEnabled(false);
    }
}

void CKCmFontInst::fileSelected(const KFileItem *item)
{
    if(item)  // OK, check its been selected - not deselected!!!
    {
        const KFileItemList *list=itsDirOp->selectedItems();

        if(list)
        {
            KFileItemListIterator it(*list);

            while(it.current())
            {
                if((*it)==item)
                {
                    if(CFontEngine::isAFont(QFile::encodeName(item->url().path())))
                        itsPreview->showFont(item->url());
                    break;
                }
                ++it;
            }
        }
    }
}

void CKCmFontInst::loadingFinished()
{
printf("loadingFinished\n");
}

void CKCmFontInst::addFonts()
{
    KURL::List list=KFileDialog::getOpenURLs(QString::null, "application/x-font-ttf application/x-font-otf application/x-font-ttc "
                                                            "application/x-font-type1 application/x-font-bdf application/x-font-pcf "
                                                            "application/x-font-snf application/x-font-speedo",
                                             this, i18n("Install Fonts"));

    if(list.count())
    {
        KURL::List           copy(list);
        KURL::List::Iterator it;

        //
        // Check if installing a Type1 font, if so look for corresponding .afm file to also install
        for(it=list.begin(); it!=list.end(); ++it)
            if(CFontEngine::isAType1(QFile::encodeName((*it).path())))
            {
                QString       afm(CMisc::changeExt((*it).path(), "afm"));
                KURL          afmUrl(*it),
                              destUrl(QString("fonts:/")+itsDirOp->url().path()+CMisc::getFile(afm));
                KIO::UDSEntry uds;

                afmUrl.setPath(afm);
                if(KIO::NetAccess::stat(afmUrl, uds) && !KIO::NetAccess::stat(destUrl, uds))
                    copy+=afmUrl;
            }

        KIO::CopyJob *job=KIO::copy(copy, itsDirOp->url(), true);
        job->setAutoErrorHandlingEnabled(true, this);
    }
}

void CKCmFontInst::removeFonts()
{
    if(itsDirOp->selectedItems()->isEmpty())
        KMessageBox::information(this, i18n("You didn't select anything to delete."), i18n("Nothing to delete"));
    else
    {
        KURL::List            urls;
        QStringList           files;
        KFileItemListIterator it(*(itsDirOp->selectedItems()));

        for(; it.current(); ++it)
        {
            files.append((*it)->url().prettyURL());
            urls.append((*it)->url());
        }

        bool doIt=false;

        switch(files.count())
        {
            case 0:
                break;
            case 1:
                doIt = KMessageBox::Continue==KMessageBox::warningContinueCancel(this,
                           i18n("<qt>Do you really want to delete\n <b>'%1'</b>?</qt>").arg(files.first()),
                           i18n("Delete Item"),
                           i18n("Delete"));
            break;
            default:
                doIt = KMessageBox::Continue==KMessageBox::warningContinueCancelList(this,
                           i18n("translators: not called for n == 1", "Do you really want to delete these %n items?", files.count()),
                           files,
                           i18n("Delete Items"),
                           i18n("Delete"));
        }

        if(doIt)
        {
            KURL::List           copy(urls);
            KURL::List::Iterator it;

            //
            // Check if installign a Type1 font, if so look for corresponding .afm file to also delete
            for(it=urls.begin(); it!=urls.end(); ++it)
                if(CFontEngine::isAType1(QFile::encodeName((*it).path())))
                {
                    KURL          afmUrl(*it);
                    KIO::UDSEntry uds;

                    afmUrl.setPath(CMisc::changeExt((*it).path(), "afm"));
                    if(KIO::NetAccess::stat(afmUrl, uds))
                        copy+=afmUrl;
                }

            KIO::DeleteJob *job = KIO::del(copy, false, true);
            job->setAutoErrorHandlingEnabled(true, this);
        }
    }
}

void CKCmFontInst::enable()
{
    enableItems(true);
}

void CKCmFontInst::disable()
{
    enableItems(false);
}

void CKCmFontInst::setUpAct()
{
    if(!CMisc::root() && (itsDirOp->url().path()==(QString("/")+i18n(KIO_FONTS_USER_DIR)) ||
                          itsDirOp->url().path()==(QString("/")+i18n(KIO_FONTS_USER_DIR)+QString("/")) ) )
        itsUpAct->setEnabled(false);
}

void CKCmFontInst::enableItems(bool enable)
{
    const KFileItemList *items=itsDirOp->selectedItems();
    KURL::List          urls;
    QStringList         files;

    if (items->isEmpty())
        KMessageBox::information(this,
                                enable ? i18n("You didn't select anything to enable.") : i18n("You didn't select anything to disable."),
                                enable ? i18n("Nothing to enable") : i18n("Nothing to disable"));
    else
    {
        KFileItemListIterator it(*items);

        for (; it.current(); ++it)
        {
            KURL url = (*it)->url();
            bool disabled=QChar('.')==(*it)->url().fileName()[0];

            if((enable && disabled) || (!enable && !disabled))
            {
                urls.append(url);
                files.append(url.prettyURL());
            }
        }
    }

    bool doIt=false;

    switch(files.count())
    {
        case 0:
            break;
        case 1:
            doIt = KMessageBox::Continue==KMessageBox::warningContinueCancel(this,
                       enable ? i18n("<qt>Do you really want to enable\n <b>'%1'</b>?</qt>").arg(files.first())
                              : i18n("<qt>Do you really want to disable\n <b>'%1'</b>?</qt>").arg(files.first()),
                       enable ? i18n("Enable Item") : i18n("Disable Item"),
                       enable ? i18n("Enable") : i18n("Disable"));
            break;
        default:
            doIt = KMessageBox::Continue==KMessageBox::warningContinueCancelList(this,
                       enable ? i18n("translators: not called for n == 1", "Do you really want to enable these %n items?", files.count())
                              : i18n("translators: not called for n == 1", "Do you really want to disable these %n items?", files.count()),
                       files,
                       enable ? i18n("Enable Items") : i18n("Disable Items"),
                       enable ? i18n("Enable") : i18n("Disable"));
    }

    if(doIt)
    {
        KURL::List::Iterator    it;
        KURL::List              copy(urls);
        CRenameJob::Entry::List renameList;

        //
        // Check if installign a Type1 font, if so look for corresponding .afm file to also enable/disable
        for(it=urls.begin(); it!=urls.end(); ++it)
            if(CFontEngine::isAType1(QFile::encodeName((*it).path())))
            {
                KURL          afmUrl(*it);
                KIO::UDSEntry uds;

                afmUrl.setPath(CMisc::changeExt((*it).path(), "afm"));
                if(KIO::NetAccess::stat(afmUrl, uds))
                    copy+=afmUrl;
            }

        for(it=copy.begin(); it!=copy.end(); ++it)
        {
            KURL url(*it);

            url.setFileName(enable
                         ? CMisc::getFile((*it).path()).mid(1)
                         : QChar('.')+CMisc::getFile((*it).path()));
            renameList.append(CRenameJob::Entry(*it, url));
        }

        CRenameJob *job = new CRenameJob(renameList, true);
        job->setAutoErrorHandlingEnabled(true, this);
    }
}

#include "KCmFontInst.moc"
