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
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "KCmFontInst.h"
#include <qlayout.h>
#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kdiroperator.h>
#include "Global.h"
#include "Misc.h"
#include "FontEngine.h"
#include "KFileFontIconView.h"
#include "KFileFontView.h"
#include "RenameJob.h"
#include <kpopupmenu.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kstdaccel.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kurllabel.h>
#include <kapplication.h>
#include <klibloader.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kdirlister.h>
#include <qsplitter.h>

#define CFG_GROUP          "Main Settings"
#define CFG_LISTVIEW       "ListView"
#define CFG_PATH           "Path"
#define CFG_SPLITTER_SIZES "SplitterSizes"
#define CFG_SIZE           "Size"

//
// Remove any fonts:/ status information added to name
static QString formatName(const QString &name)
{
    QString n(name);

    return n.remove(i18n(KIO_FONTS_DISABLED));
}

typedef KGenericFactory<CKCmFontInst, QWidget> FontInstallFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_fontinst, FontInstallFactory("kcmfontinst"))

CKCmFontInst::CKCmFontInst(QWidget *parent, const char *, const QStringList&)
            : KCModule(parent, "kfontinst"),
              itsAboutData(NULL),
              itsTop(CMisc::root() ? "fonts:/" : QString("fonts:/")+i18n(KIO_FONTS_USER)),
              itsPreview(NULL),
              itsConfig(CGlobal::uiCfgFile())
{
    KGlobal::locale()->insertCatalogue("kfontinst");

    const char *appName=KCmdLineArgs::appName();

    itsEmbeddedAdmin=CMisc::root() && (NULL==appName || strcmp("kcontrol", appName) && 
                     KCmdLineArgs::parsedArgs()->isSet("embed"));
    itsKCmshell=!itsEmbeddedAdmin && NULL!=appName && 0==strcmp("kcmshell", appName) &&
                !KCmdLineArgs::parsedArgs()->isSet("embed");

    itsStatusLabel = new QLabel(this);
    itsStatusLabel->setFrameShape(QFrame::Panel);
    itsStatusLabel->setFrameShadow(QFrame::Sunken);
    itsStatusLabel->setLineWidth(1);

    itsConfig.setGroup(CFG_GROUP);

    KLibFactory *factory=KLibLoader::self()->factory("libkfontviewpart");
    QFrame      *fontsFrame,
                *urlFrame=new QFrame(this);

    if(factory)
    {
        itsSplitter=new QSplitter(this);
        fontsFrame=new QFrame(itsSplitter),
        itsPreview=(KParts::ReadOnlyPart *)factory->create(itsSplitter, "kcmfontinst", "KParts::ReadOnlyPart");
        itsSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QValueList<int> sizes(itsConfig.readIntListEntry(CFG_SPLITTER_SIZES));

        if(2!=sizes.count())
        {
            sizes.clear();
            sizes+=250;
            sizes+=150;
        }
        itsSplitter->setSizes(sizes);
    }
    else
    {
        fontsFrame=new QFrame(this);
        fontsFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        urlFrame->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    }

    QGridLayout *fontsLayout=new QGridLayout(fontsFrame, 1, 1, 0, 1);
    QHBoxLayout *urlLayout=new QHBoxLayout(urlFrame, 1);
    QVBoxLayout *layout=new QVBoxLayout(this, 0, KDialog::spacingHint());
    KToolBar    *toolbar=new KToolBar(this);

    fontsFrame->setLineWidth(0);
    urlFrame->setFrameShadow(QFrame::Sunken);
    urlFrame->setFrameShape(QFrame::StyledPanel);
    urlFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    itsLabel = new KURLLabel(urlFrame);
    itsLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    urlLayout->addItem(new QSpacerItem(4, 4));
    urlLayout->addWidget(new QLabel(i18n("Location: "), urlFrame));
    urlLayout->addItem(new QSpacerItem(4, 4));
    urlLayout->addWidget(itsLabel);
    urlLayout->addItem(new QSpacerItem(4, 4));
    toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    toolbar->setMovingEnabled(false);

    QString previousPath=itsConfig.readEntry(CFG_PATH);
    KURL    url(itsTop);

    if(!previousPath.isEmpty())
    {
        KIO::UDSEntry uds;

        url.setPath(previousPath);
        if(!KIO::NetAccess::stat(url, uds, this))
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
    itsDirOp->dirLister()->setMainWindow(this);
    fontsLayout->addWidget(itsDirOp, 0, 0);

    layout->addWidget(toolbar);
    layout->addWidget(urlFrame);
    layout->addWidget(itsPreview ? itsSplitter : fontsFrame);
    layout->addWidget(itsStatusLabel);

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
        act->setText(i18n("Top Level"));
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
        disconnect(act, SIGNAL(activated()), itsDirOp, SLOT(mkdir()));
        connect(act, SIGNAL(activated()), this, SLOT(createFolder()));
    }

    act=new KAction(i18n("Add Fonts..."), "newfont", 0, this, SLOT(addFonts()), itsDirOp->actionCollection(), "addfonts");
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
    itsDisableAct=new KAction(i18n("Disable"), "button_cancel", 0, this, SLOT(disable()), itsDirOp->actionCollection(),
                              "disable");
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

    if(itsPreview)
    {
        KActionCollection *previewCol=itsPreview->actionCollection();

        if(previewCol && previewCol->count()>0)
        {
            toolbar->insertLineSeparator();

            if((act=previewCol->action("zoomIn")))
                act->plug(toolbar);
            if((act=previewCol->action("zoomOut")))
                act->plug(toolbar);
            if((act=previewCol->action("changeText")))
                act->plug(toolbar);
            if((act=previewCol->action("toggleWaterfall")))
                act->plug(toolbar);

            // For some reason the following always put zoomOut, zoomIn, changeText hmmm :-(
            //for(unsigned int i=0; i<previewCol->count(); ++i)
            //    previewCol->action(i)->plug(toolbar);
        }
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
    connect(itsDirOp, SIGNAL(finishedLoading()), SLOT(loadingFinished()));
    connect(itsDirOp, SIGNAL(dropped(const KFileItem *, QDropEvent *, const KURL::List &)),
                      SLOT(dropped(const KFileItem *, QDropEvent *, const KURL::List &)));
    connect(itsLabel, SIGNAL(leftClickedURL(const QString &)), SLOT(openUrlInBrowser(const QString &)));
    connect(itsDirOp->dirLister(), SIGNAL(infoMessage(const QString &)), SLOT(infoMessage(const QString &)));
    connect(itsDirOp, SIGNAL(updateInformation(int, int)), SLOT(updateInformation(int, int)));

    if(itsKCmshell)
    {
        QSize defSize(450, 380);

        itsSizeHint=itsConfig.readSizeEntry(CFG_SIZE, &defSize);
    }
}

CKCmFontInst::~CKCmFontInst()
{
    if(itsPreview)
    {
        itsConfig.setGroup(CFG_GROUP);
        itsConfig.writeEntry(CFG_SPLITTER_SIZES, itsSplitter->sizes());
        if(itsKCmshell)
            itsConfig.writeEntry(CFG_SIZE, size());
    }
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
                                                    "(c) Craig Drummond, 2000 - 2004"));
        that->itsAboutData->addAuthor("Craig Drummond", I18N_NOOP("Developer and maintainer"), "craig@kde.org");
    }

    return itsAboutData;
}

QSize CKCmFontInst::sizeHint() const
{
    return itsKCmshell ? itsSizeHint : KCModule::sizeHint();
}

QString CKCmFontInst::quickHelp() const
{
    QString help(i18n("<h1>Font Installer</h1><p> This module allows you to"
                      " install TrueType, Type1, Speedo, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
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
    itsConfig.setGroup(CFG_GROUP);
    itsConfig.writeEntry(CFG_LISTVIEW, true);
    if(itsEmbeddedAdmin)
        itsConfig.sync();
    itsDirOp->setAcceptDrops(true);
}

void CKCmFontInst::iconView()
{
    CKFileFontIconView *newView=new CKFileFontIconView(itsDirOp, "simple view");

    itsDirOp->setView(newView);
    itsIconAct->setChecked(true);
    itsConfig.setGroup(CFG_GROUP);
    itsConfig.writeEntry(CFG_LISTVIEW, false);
    if(itsEmbeddedAdmin)
        itsConfig.sync();
    itsDirOp->setAcceptDrops(true);
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
    itsConfig.setGroup(CFG_GROUP);
    itsConfig.writeEntry(CFG_PATH, url.path());

    itsEnableAct->setEnabled(false);
    itsDisableAct->setEnabled(false);
    itsLabel->setText(createLocationLabel(url));
    itsLabel->setURL(url.url());
    if(itsEmbeddedAdmin)
        itsConfig.sync();

    updateInformation(0, 0);
}

void CKCmFontInst::fileHighlighted(const KFileItem *item)
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

    if(itsPreview)
    {
        //
        // Generate preview...
        const KFileItem *previewItem=item 
                                       ? item 
                                       : list && 1==list->count() 
                                             ? list->getFirst() 
                                             : NULL;

        if(previewItem && list && list->contains(previewItem))  // OK, check its been selected - not deselected!!!
            itsPreview->openURL(previewItem->url());
    }
}

void CKCmFontInst::loadingFinished()
{
    QListView *lView=dynamic_cast<QListView *>(itsDirOp->view());

    if(lView)
        lView->sort();
    else
    {
        QIconView *iView=dynamic_cast<QIconView *>(itsDirOp->view());

        if(iView)
            iView->sort();
    }
    fileHighlighted(NULL);
}

void CKCmFontInst::addFonts()
{
    KURL::List list=KFileDialog::getOpenURLs(QString::null, "application/x-font-ttf application/x-font-otf "
                                                            "application/x-font-ttc application/x-font-type1 "
                                                            "application/x-font-bdf application/x-font-pcf "
                                                            "application/x-font-snf application/x-font-speedo",
                                             this, i18n("Add Fonts"));

    if(list.count())
        addFonts(list, itsDirOp->url());
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
            files.append(formatName((*it)->text()));
            urls.append((*it)->url());
        }

        bool doIt=false;

        switch(files.count())
        {
            case 0:
                break;
            case 1:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                           i18n("<qt>Do you really want to delete\n <b>'%1'</b>?</qt>").arg(files.first()),
                           i18n("Delete Item"), i18n("Delete"));
            break;
            default:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                           i18n("translators: not called for n == 1", "Do you really want to delete these %n items?",
                                files.count()),
                           files, i18n("Delete Items"), i18n("Delete"));
        }

        if(doIt)
        {
            KURL::List           copy(urls);
            KURL::List::Iterator it;

            //
            // Check if deleting a Type1 font, if so look for corresponding .afm file to also delete
            for(it=urls.begin(); it!=urls.end(); ++it)
                if(CFontEngine::isAType1(QFile::encodeName((*it).path())))
                {
                    const char *others[]={ "afm", "AFM", NULL };

                    for(int i=0; others[i]; ++i)
                    {
                        KURL          afmUrl(*it);
                        KIO::UDSEntry uds;

                        afmUrl.setPath(CMisc::changeExt((*it).path(), others[i]));
                        if(KIO::NetAccess::stat(afmUrl, uds, this))
                        {
                            copy+=afmUrl;
                            break;
                        }
                    }
                }

            KIO::DeleteJob *job = KIO::del(copy, false, true);
            connect(job, SIGNAL(result(KIO::Job *)), this, SLOT(jobResult(KIO::Job *)));
            job->setWindow(this);
            job->setAutoErrorHandlingEnabled(true, this);
        }
    }
}

void CKCmFontInst::createFolder()
{
    //
    // Hack as when create folder, and go back - created folder does not appear!
    KURL prevUrl=itsDirOp->url();

    itsDirOp->mkdir();

    if(prevUrl!=itsDirOp->url())
        itsDirOp->dirLister()->updateDirectory(prevUrl);
    fileHighlighted(NULL);
}

void CKCmFontInst::enable()
{
    enableItems(true);
}

void CKCmFontInst::disable()
{
    enableItems(false);
}

void CKCmFontInst::dropped(const KFileItem *i, QDropEvent *, const KURL::List &urls)
{
    if(urls.count())
        addFonts(urls, i && i->isDir() ?  i->url() : itsDirOp->url());
}

void CKCmFontInst::openUrlInBrowser(const QString &url)
{
    if (kapp)
    {
        QString u(url);

        if (itsEmbeddedAdmin)  // Need to open fonts:/System...
        {
            u.insert(strlen(KIO_FONTS_PROTOCOL)+1, i18n(KIO_FONTS_SYS));
            u.insert(strlen(KIO_FONTS_PROTOCOL)+1, '/');
        }
        kapp->invokeBrowser(u);
    }
}

void CKCmFontInst::infoMessage(const QString &msg)
{
    itsStatusLabel->setText(msg);
}

void CKCmFontInst::updateInformation(int dirs, int fonts)
{
    KIO::filesize_t size=0;
    QString         text(i18n("One Item", "%n Items", dirs+fonts));

    if(fonts>0)
    {
        KFileItem *item=NULL;

        for (item=itsDirOp->view()->firstFileItem(); item; item=itsDirOp->view()->nextItem(item))
            if(item->isFile())
                size+=item->size();
    }

    text+=" - ";
    text+=i18n("One Font", "%n Fonts", fonts);
    if(fonts>0)
    {
        text+=" ";
        text+=i18n("(%1 Total)").arg(KIO::convertSize(size));
    }
    text+=" - ";
    text+=i18n("One Folder", "%n Folders", dirs);
    itsStatusLabel->setText(text);
}

void CKCmFontInst::jobResult(KIO::Job *job)
{
    //
    // Force an update of the view. For some reason the view is not automatically updated when
    // run in embedded mode - e.g. from the "Admin" mode button on KControl.
    if(job && 0==job->error())
    {
        itsDirOp->dirLister()->updateDirectory(itsDirOp->url());
        KMessageBox::information(this, i18n("Please note that any open applications will need to be restarted in order "
                                            "for any changes to be noticed."),
                                 i18n("Success"), "KFontinst_WarnAboutFontChangesAndOpenApps");
    }
}

void CKCmFontInst::setUpAct()
{
    if(!CMisc::root() && (itsDirOp->url().path()==(QString("/")+i18n(KIO_FONTS_USER)) ||
                          itsDirOp->url().path()==(QString("/")+i18n(KIO_FONTS_USER)+QString("/")) ) )
        itsUpAct->setEnabled(false);
}

void CKCmFontInst::enableItems(bool enable)
{
    const KFileItemList *items=itsDirOp->selectedItems();
    KURL::List          urls;
    QStringList         files;

    if (items->isEmpty())
        KMessageBox::information(this,
                                enable ? i18n("You didn't select anything to enable.") 
                                       : i18n("You didn't select anything to disable."),
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
                files.append(formatName((*it)->text()));
            }
        }
    }

    bool doIt=false;

    switch(files.count())
    {
        case 0:
            break;
        case 1:
            doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                       enable ? i18n("<qt>Do you really want to enable\n <b>'%1'</b>?</qt>").arg(files.first())
                              : i18n("<qt>Do you really want to disable\n <b>'%1'</b>?</qt>").arg(files.first()),
                       enable ? i18n("Enable Item") : i18n("Disable Item"),
                       enable ? i18n("Enable") : i18n("Disable"));
            break;
        default:
            doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                       enable ? i18n("translators: not called for n == 1", "Do you really want to enable these %n items?",
                                     files.count())
                              : i18n("translators: not called for n == 1", "Do you really want to disable these %n items?",
                                     files.count()),
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
        // Check if enabling/disabling a Type1 font, if so look for corresponding .afm file to also enable/disable
        for(it=urls.begin(); it!=urls.end(); ++it)
            if(CFontEngine::isAType1(QFile::encodeName((*it).path())))
            {
                KURL          afmUrl(*it);
                KIO::UDSEntry uds;

                afmUrl.setPath(CMisc::changeExt((*it).path(), "afm"));
                if(KIO::NetAccess::stat(afmUrl, uds, this))
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
        connect(job, SIGNAL(result(KIO::Job *)), this, SLOT(jobResult(KIO::Job *)));
        job->setWindow(this);
        job->setAutoErrorHandlingEnabled(true, this);
    }
}

void CKCmFontInst::addFonts(const KURL::List &src, const KURL &dest)
{
    if(src.count())
    {
        KURL::List                copy(src);
        KURL::List::ConstIterator it;

        //
        // Check if installing a Type1 font, if so look for corresponding .afm file to also install
        for(it=src.begin(); it!=src.end(); ++it)
            if(CFontEngine::isAType1(QFile::encodeName((*it).path())))
            {
                QString       afm(CMisc::changeExt((*it).path(), "afm"));
                KURL          afmUrl(*it),
                              destUrl(QString("fonts:/")+dest.path()+CMisc::getFile(afm));
                KIO::UDSEntry uds;

                afmUrl.setPath(afm);
                if(KIO::NetAccess::stat(afmUrl, uds, this) && !KIO::NetAccess::stat(destUrl, uds, this) &&
                   -1==copy.findIndex(afmUrl))
                    copy+=afmUrl;
            }

        KIO::CopyJob *job=KIO::copy(copy, dest, true);
        connect(job, SIGNAL(result(KIO::Job *)), this, SLOT(jobResult(KIO::Job *)));
        job->setWindow(this);
        job->setAutoErrorHandlingEnabled(true, this);
    }
}

#include "KCmFontInst.moc"
