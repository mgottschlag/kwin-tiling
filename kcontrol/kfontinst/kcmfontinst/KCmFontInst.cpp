////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CKCmFontInst
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "KCmFontInst.h"
#include "KfiConstants.h"
#include "PrintDialog.h"
#include "SettingsDialog.h"
#ifdef HAVE_XFT
#include "KfiPrint.h"
#include "FcEngine.h"
#endif
#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qsettings.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QGridLayout>
#include <QDropEvent>
#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kdiroperator.h>
#include <kprinter.h>
#include "Misc.h"
#include "KFileFontIconView.h"
#include "KFileFontView.h"
#include <kmenu.h>
#include <ktoolbar.h>
#include <kstdaccel.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kdirlister.h>
#include <kpushbutton.h>
#include <kguiitem.h>
#include <qsplitter.h>

#define CFG_GROUP          "Main Settings"
#define CFG_LISTVIEW       "ListView"
#define CFG_PATH           "Path"
#define CFG_SPLITTER_SIZES "SplitterSizes"
#define CFG_SHOW_BITMAP    "ShowBitmap"
#define CFG_FONT_SIZE      "FontSize"

typedef KGenericFactory<KFI::CKCmFontInst, QWidget> FontInstallFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_fontinst, FontInstallFactory("kcmfontinst"))

namespace KFI
{

CKCmFontInst::CKCmFontInst(QWidget *parent, const char *, const QStringList&)
    : KCModule( FontInstallFactory::instance(), parent),
#ifdef HAVE_XFT
              itsPreview(NULL),
#endif
              itsConfig(KFI_UI_CFG_FILE)
{
    KGlobal::locale()->insertCatalog(KFI_CATALOGUE);

    KAboutData* about = new KAboutData("kcmfontinst",
         I18N_NOOP("KDE Font Installer"),
         0, 0,
         KAboutData::License_GPL,
         I18N_NOOP("GUI front end to the fonts:/ ioslave.\n"
         "(c) Craig Drummond, 2000 - 2004"));
    about->addAuthor("Craig Drummond", I18N_NOOP("Developer and maintainer"), "craig@kde.org");
    setAboutData(about);

    const char *appName=KCmdLineArgs::appName();

    itsEmbeddedAdmin=Misc::root() && (NULL==appName || strcmp("kcontrol", appName) &&
                     KCmdLineArgs::parsedArgs()->isSet("embed"));

    itsStatusLabel = new QLabel(this);
    itsStatusLabel->setFrameShape(QFrame::Panel);
    itsStatusLabel->setFrameShadow(QFrame::Sunken);
    itsStatusLabel->setLineWidth(1);

    itsConfig.setGroup(CFG_GROUP);

    QFrame      *fontsFrame;
#ifdef HAVE_XFT
    KLibFactory *factory=KLibLoader::self()->factory("libkfontviewpart");

    if(factory)
    {
        itsSplitter=new QSplitter(this);
        fontsFrame=new QFrame(itsSplitter),
        itsPreview=(KParts::ReadOnlyPart *)factory->create(itsSplitter, "kcmfontinst", "KParts::ReadOnlyPart");
        itsSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QList<int> sizes(itsConfig.readEntry(CFG_SPLITTER_SIZES,QList<int>()));

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
#endif
        fontsFrame=new QFrame(this);
        fontsFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
#ifdef HAVE_XFT
    }
#endif

    QGridLayout *fontsLayout=new QGridLayout(fontsFrame, 1, 1, 0, 1);
    QVBoxLayout *layout=new QVBoxLayout(this, 0, KDialog::spacingHint());
    KToolBar    *toolbar=new KToolBar(this);
    bool        showBitmap(itsConfig.readEntry(CFG_SHOW_BITMAP, false));

    fontsFrame->setLineWidth(0);
    toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    toolbar->setMovable(false);

    QString previousPath=itsConfig.readEntry(CFG_PATH);

    itsDirOp = new KDirOperator(Misc::root() ? QString("fonts:/") : QString("fonts:/")+i18n(KFI_KIO_FONTS_USER),
                                fontsFrame);
	KConfigGroup confGroup(&itsConfig, "ListView Settings");
    itsDirOp->setViewConfig(&confGroup);
    itsDirOp->setMinimumSize(QSize(96, 64));
    setMimeTypes(showBitmap);
    itsDirOp->dirLister()->setMainWindow(this);
    itsDirOp->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    fontsLayout->addMultiCellWidget(itsDirOp, 0, 0, 0, 1);

    KPushButton *button=new KPushButton(KGuiItem(i18n("Add Fonts..."), "newfont"), fontsFrame);
    connect(button, SIGNAL(clicked()), SLOT(addFonts()));
    button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    fontsLayout->addWidget(button, 1, 0);
	fontsLayout->addItem(new QSpacerItem(4, 4, QSizePolicy::Expanding, QSizePolicy::Minimum),0,0);

    layout->addWidget(toolbar);
#ifdef HAVE_XFT
    layout->addWidget(itsPreview ? itsSplitter : fontsFrame);
#else
    layout->addWidget(fontsFrame);
#endif
    layout->addWidget(itsStatusLabel);

    setButtons(0);
    setRootOnlyMsg(i18n("<b>The fonts shown are your personal fonts.</b><br>To see (and install) "
                        "system-wide fonts, click on the \"Administrator Mode\" button below."));
    setUseRootOnlyMsg(true);
    itsDirOp->setMode(KFile::Files);

    //
    // Now for the hack!
    KAction     *act;
    KActionMenu *topMnu=dynamic_cast<KActionMenu *>(itsDirOp->actionCollection()->action("popupMenu"));
    Q_ASSERT(topMnu);

    itsViewMenuAct=dynamic_cast<KActionMenu *>(itsDirOp->actionCollection()->action("view menu"));
    Q_ASSERT(itsViewMenuAct);

    topMnu->popupMenu()->clear();
    connect(topMnu->popupMenu(), SIGNAL(aboutToShow()), SLOT(setupMenu()));
    if((act=itsDirOp->actionCollection()->action("up")))
        act->disconnect(SIGNAL(activated()), itsDirOp, SLOT(cdUp()));
    if((act=itsDirOp->actionCollection()->action("home")))
        act->disconnect(SIGNAL(activated()), itsDirOp, SLOT(home()));
    if((act=itsDirOp->actionCollection()->action("back")))
        act->disconnect(SIGNAL(activated()), itsDirOp, SLOT(back()));
    if((act=itsDirOp->actionCollection()->action("forward")))
        act->disconnect(SIGNAL(activated()), itsDirOp, SLOT(forward()));

    if((act=itsDirOp->actionCollection()->action("reload")))
        act->plug(toolbar);

    topMnu->insert(itsViewMenuAct);

    if((itsIconAct=qobject_cast<KToggleAction *>(itsDirOp->actionCollection()->action("short view"))))
    {
        disconnect(itsIconAct, SIGNAL(triggered(bool)), itsDirOp, SLOT(slotSimpleView()));
        connect(itsIconAct, SIGNAL(triggered(bool)), SLOT(iconView()));
        itsIconAct->plug(toolbar);
    }

    if((itsListAct=dynamic_cast<KToggleAction *>(itsDirOp->actionCollection()->action("detailed view"))))
    {
        disconnect(itsListAct, SIGNAL(activated()), itsDirOp, SLOT(slotDetailedView()));
        connect(itsListAct, SIGNAL(activated()), SLOT(listView()));
        itsListAct->plug(toolbar);
    }

    itsShowBitmapAct=new KToggleAction(i18n("Show Bitmap Fonts"), "font_bitmap", 0, this, SLOT(filterFonts()),
                                       itsDirOp->actionCollection(), "showbitmap");
    itsShowBitmapAct->setChecked(showBitmap);
    itsShowBitmapAct->plug(toolbar);

    toolbar->addSeparator();

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

    toolbar->addSeparator();
    act=new KAction(i18n("Configure..."), "configure", 0, this, SLOT(configure()), itsDirOp->actionCollection(), "configure");
    act->plug(toolbar);
#ifdef HAVE_XFT
    toolbar->addSeparator();
    act=new KAction(i18n("Print..."), "fileprint", 0, this, SLOT(print()), itsDirOp->actionCollection(), "print");
    act->plug(toolbar);
#endif

    if( (itsSepDirsAct=itsDirOp->actionCollection()->action("separate dirs")) &&
        (itsShowHiddenAct=itsDirOp->actionCollection()->action("show hidden")))
    {
        //disconnect(itsViewMenuAct->popupMenu(), SIGNAL(aboutToShow()), itsDirOp, SLOT(insertViewDependentActions()));
        connect(itsViewMenuAct->popupMenu(), SIGNAL(aboutToShow()), SLOT(setupViewMenu()));
        setupViewMenu();
    }

#ifdef HAVE_XFT
    if(itsPreview)
    {
        KActionCollection *previewCol=itsPreview->actionCollection();

        if(previewCol && previewCol->count()>0 && (act=previewCol->action("changeText")))
            act->plug(toolbar);
    }
#endif

    //
    // Set view...
    if(itsConfig.readEntry(CFG_LISTVIEW, true))
        listView();
    else
        iconView();

    itsDirOp->dirLister()->setShowingDotFiles(true);

    connect(itsDirOp, SIGNAL(fileHighlighted(const KFileItem *)), SLOT(fileHighlighted(const KFileItem *)));
    connect(itsDirOp, SIGNAL(finishedLoading()), SLOT(loadingFinished()));
    connect(itsDirOp, SIGNAL(dropped(const KFileItem *, QDropEvent *, const KUrl::List &)),
                      SLOT(dropped(const KFileItem *, QDropEvent *, const KUrl::List &)));
    connect(itsDirOp->dirLister(), SIGNAL(infoMessage(const QString &)), SLOT(infoMessage(const QString &)));
    connect(itsDirOp, SIGNAL(updateInformation(int, int)), SLOT(updateInformation(int, int)));
}

CKCmFontInst::~CKCmFontInst()
{
#ifdef HAVE_XFT
    if(itsPreview)
    {
        itsConfig.setGroup(CFG_GROUP);
        itsConfig.writeEntry(CFG_SPLITTER_SIZES, itsSplitter->sizes());
    }
#endif
    delete itsDirOp;
}

void CKCmFontInst::setMimeTypes(bool showBitmap)
{
    QStringList mimeTypes;

    mimeTypes << "application/x-font-ttf"
              << "application/x-font-otf"
              << "application/x-font-ttc"
              << "application/x-font-type1";
    if(showBitmap)
        mimeTypes << "application/x-font-pcf"
                  << "application/x-font-bdf";

    itsDirOp->setMimeFilter(mimeTypes);
}

void CKCmFontInst::filterFonts()
{
    setMimeTypes(itsShowBitmapAct->isChecked());
    itsDirOp->rereadDir();
    itsConfig.setGroup(CFG_GROUP);
    itsConfig.writeEntry(CFG_SHOW_BITMAP, itsShowBitmapAct->isChecked());
    if(itsEmbeddedAdmin)
        itsConfig.sync();
}

QString CKCmFontInst::quickHelp() const
{
    return Misc::root()
               ? i18n("<h1>Font Installer</h1><p> This module allows you to"
                      //" install TrueType, Type1, Speedo, and Bitmap"
                      " install TrueType, Type1, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
                      " font, simply copy one into the folder.</p>")
               : i18n("<h1>Font Installer</h1><p> This module allows you to"
                      //" install TrueType, Type1, Speedo, and Bitmap"
                      " install TrueType, Type1, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
                      " font, simply copy it into the appropriate folder - "
                      " \"Personal\" for fonts available to just yourself, or "
                      " \"System\" for system-wide fonts (available to all).</p>"
                      "<p><b>NOTE:</b> As you are not logged in as \"root\", any"
                      " fonts installed will only be available to you. To install"
                      " fonts system-wide, use the \"Administrator Mode\""
                      " button to run this module as \"root\".</p>");
}

void CKCmFontInst::listView()
{
    CKFileFontView *newView=new CKFileFontView( itsDirOp );
    newView->setObjectName( "detailed view" );

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

void CKCmFontInst::setupMenu()
{
    itsDirOp->setupMenu(KDirOperator::SortActions|/*KDirOperator::FileActions|*/KDirOperator::ViewActions);
}

void CKCmFontInst::setupViewMenu()
{
    itsViewMenuAct->remove(itsSepDirsAct);
    itsViewMenuAct->remove(itsShowHiddenAct);
}

void CKCmFontInst::fileHighlighted(const KFileItem *item)
{
    const KFileItemList *list=itsDirOp->selectedItems();

    itsDeleteAct->setEnabled(list && list->count());

#ifdef HAVE_XFT
    if(itsPreview)
    {
        //
        // Generate preview...
        const KFileItem *previewItem=item ? item
                                    : list && 1==list->count()
                                             ? list->first()
                                             : NULL;

        if(previewItem && list && list->contains(const_cast<KFileItem *>(previewItem)))  // OK, check its been selected - not deselected!!!
            itsPreview->openURL(previewItem->url());
    }
#endif
}

void CKCmFontInst::loadingFinished()
{
    Q3ListView *lView=dynamic_cast<Q3ListView *>(itsDirOp->view());

    if(lView)
        lView->sort();
    else
    {
        Q3IconView *iView=dynamic_cast<Q3IconView *>(itsDirOp->view());

        if(iView)
            iView->sort();
    }
    fileHighlighted(NULL);
}

void CKCmFontInst::addFonts()
{
    KUrl::List list=KFileDialog::getOpenURLs(QString(), "application/x-font-ttf application/x-font-otf "
                                                            "application/x-font-ttc application/x-font-type1 "
                                                            "application/x-font-pcf application/x-font-bdf",
                                                            //"application/x-font-snf application/x-font-speedo",
                                             this, i18n("Add Fonts"));

    if(list.count())
        addFonts(list, itsDirOp->url());
}

void CKCmFontInst::removeFonts()
{
    if(itsDirOp->selectedItems()->isEmpty())
        KMessageBox::information(this, i18n("You did not select anything to delete."), i18n("Nothing to Delete"));
    else
    {
        KUrl::List            urls;
        QStringList           files;
        const KFileItemList list = *(itsDirOp->selectedItems());

        KFileItemList::const_iterator kit = list.begin();
        const KFileItemList::const_iterator kend = list.end();
        for ( ; kit != kend; ++kit )
        {
            files.append((*kit)->text());
            urls.append((*kit)->url());
        }

        bool doIt=false;

        switch(files.count())
        {
            case 0:
                break;
            case 1:
                doIt = KMessageBox::Continue==KMessageBox::warningContinueCancel(this,
                           i18n("<qt>Do you really want to delete\n <b>'%1'</b>?</qt>", files.first()),
			   i18n("Delete Font"), KStdGuiItem::del());
            break;
            default:
                doIt = KMessageBox::Continue==KMessageBox::warningContinueCancelList(this,
                           i18np("Do you really want to delete this font?", "Do you really want to delete these %n fonts?",
                                files.count()),
			   files, i18n("Delete Fonts"), KStdGuiItem::del());
        }

        if(doIt)
        {
            KIO::DeleteJob *job = KIO::del(urls, false, true);
            connect(job, SIGNAL(result(KIO::Job *)), this, SLOT(jobResult(KIO::Job *)));
            job->setWindow(this);
            job->setAutoErrorHandlingEnabled(true, this);
        }
    }
}

void CKCmFontInst::configure()
{
    CSettingsDialog(this).exec();
}

void CKCmFontInst::print()
{
#ifdef HAVE_XFT
    KFileItemList list;
    bool          ok=false;

    for (KFileItem *item=itsDirOp->view()->firstFileItem(); item && !ok; item=itsDirOp->view()->nextItem(item))
        if(Print::printable(item->mimetype()))
            ok=true;

    if(ok)
    {
        const KFileItemList *list=itsDirOp->selectedItems();
        bool                select=false;

        if(list)
        {
            KFileItemList::const_iterator kit = list->begin();
            const KFileItemList::const_iterator kend = list->end();
            for ( ; kit != kend && !select; ++kit )
                if(Print::printable((*kit)->mimetype()))
                    select=true;
        }

        CPrintDialog dlg(this);

        itsConfig.setGroup(CFG_GROUP);
        if(dlg.exec(select, itsConfig.readEntry(CFG_FONT_SIZE, 1)))
        {
            static const int constSizes[]={0, 12, 18, 24, 36, 48};

            QStringList       items;
            QVector<int> sizes;
            CFcEngine         engine;

            if(dlg.outputAll())
            {
                for (KFileItem *item=itsDirOp->view()->firstFileItem(); item; item=itsDirOp->view()->nextItem(item))
                    items.append(item->name());
            }
            else
            {
                KFileItemList::const_iterator kit = list->begin();
                const KFileItemList::const_iterator kend = list->end();
                for(; kit!=kend; ++kit)
                    items.append((*kit)->name());
            }
	    int cs = dlg.chosenSize(); // can return -1
	    if (cs >= 0 && cs < sizeof(constSizes)/sizeof(int))
	    {
                  Print::printItems(items, constSizes[cs], this, engine);
                  itsConfig.writeEntry(CFG_FONT_SIZE, cs);
                  if(itsEmbeddedAdmin)
                        itsConfig.sync();
	    }
        }
    }
    else
        KMessageBox::information(this, i18n("There are no printable fonts.\nYou can only print non-bitmap fonts."),
                                 i18n("Cannot Print"));
#endif
}

void CKCmFontInst::dropped(const KFileItem *i, QDropEvent *, const KUrl::List &urls)
{
    if(urls.count())
        addFonts(urls, i && i->isDir() ?  i->url() : itsDirOp->url());
}

void CKCmFontInst::infoMessage(const QString &msg)
{
    itsStatusLabel->setText(msg);
}

static QString family(const QString &name)
{
    int commaPos=name.indexOf(',');

    return -1==commaPos ? name : name.left(commaPos);
}

void CKCmFontInst::updateInformation(int, int fonts)
{
    KIO::filesize_t size=0;
    QString         text(i18np("One Font", "%n Fonts", fonts));
    QStringList     families;

    if(fonts>0)
    {
        KFileItem *item=NULL;

        for (item=itsDirOp->view()->firstFileItem(); item; item=itsDirOp->view()->nextItem(item))
        {
            QString fam(family(item->text()));

            size+=item->size();
            if(-1==families.indexOf(fam))
                families+=fam;
        }
    }

    if(fonts>0)
    {
        text+=" ";
        text+=i18n("(%1 Total)", KIO::convertSize(size));
    }
    text+=" - ";
    text+=i18np("One Family", "%n Families", families.count());
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
        KMessageBox::information(this,
#ifdef HAVE_XFT
                                 i18n("<p>Please note that any open applications will need to be restarted in order "
                                      "for any changes to be noticed.<p><p>(You will also have to restart this application "
                                      "in order to use its print function on any newly installed fonts.)</p>"),
#else
                                 i18n("Please note that any open applications will need to be restarted in order "
                                      "for any changes to be noticed."),
#endif
                                 i18n("Success"), "KFontinst_WarnAboutFontChangesAndOpenApps");
    }
}

void CKCmFontInst::addFonts(const KUrl::List &src, const KUrl &dest)
{
    if(src.count())
    {
        KUrl::List                copy(src);
        KUrl::List::ConstIterator it;

        //
        // Check if font has any associated AFM or PFM file...
        for(it=src.begin(); it!=src.end(); ++it)
        {
            KUrl::List associatedUrls;

            Misc::getAssociatedUrls(*it, associatedUrls);
            copy+=associatedUrls;
        }

        KIO::CopyJob *job=KIO::copy(copy, dest, true);
        connect(job, SIGNAL(result(KIO::Job *)), this, SLOT(jobResult(KIO::Job *)));
        job->setWindow(this);
        job->setAutoErrorHandlingEnabled(true, this);
    }
}

}

#include "KCmFontInst.moc"
