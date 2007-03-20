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

#include "KCmFontInst.h"
#include "KfiConstants.h"
#include "PrintDialog.h"
#include "FcEngine.h"
#include "FontPreview.h"
#include "Misc.h"
#include "FontList.h"
#include "DuplicatesDialog.h"
#include "PreviewSelectAction.h"
#include "FontFilter.h"
#include <QGridLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QProgressBar>
#include <QCoreApplication>
#include <QApplication>
#include <QTextStream>
#include <QMenu>
#include <QComboBox>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <ktoolbar.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kpushbutton.h>
#include <kguiitem.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kprogressdialog.h>
#include <kdirselectdialog.h>
#include <kzip.h>
#include <ktempdir.h>
#include <ktemporaryfile.h>
#include <kicon.h>
#include <kprocess.h>
#include <kactionmenu.h>
#include <ktoggleaction.h>
#include <kstandarddirs.h>
#include <kmenu.h>

#define CFG_GROUP          "Main Settings"
#define CFG_SPLITTER_SIZES "SplitterSizes"
#define CFG_FONT_SIZE      "FontSize"
#define CFG_FONT_MGT_MODE  "MgtMode"
#define CFG_SHOW_PREVIEW   "ShowPreview"

typedef KGenericFactory<KFI::CKCmFontInst, QWidget> FontInstallFactory;
K_EXPORT_COMPONENT_FACTORY(fontinst, FontInstallFactory("fontinst"))

namespace KFI
{

class CPushButton : public KPushButton
{
    public:

    CPushButton(const KGuiItem &item, QWidget *parent)
        : KPushButton(item, parent)
    {
        theirHeight=qMax(theirHeight, height());
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    }

    QSize sizeHint() const
    {
        QSize sh(KPushButton::sizeHint());

        sh.setHeight(theirHeight);
        if(sh.width()<sh.height())
            sh.setWidth(sh.height());
        return sh;
    }

    private:

    static int theirHeight;
};

class CProgressBar : public QProgressBar
{
    public:

    CProgressBar(QWidget *p, int h) : QProgressBar(p), itsHeight((int)(h*0.6))
        { setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); }

    virtual ~CProgressBar() { }

    int height() const     { return itsHeight; }
    QSize sizeHint() const { return QSize(100, itsHeight); }

    private:

    int itsHeight;
};

int CPushButton::theirHeight=0;

CKCmFontInst::CKCmFontInst(QWidget *parent, const QStringList&)
            : KCModule(FontInstallFactory::componentData(), parent),
              itsPreview(NULL),
              itsConfig(KFI_UI_CFG_FILE),
              itsJob(NULL),
              itsProgress(NULL),
              itsUpdateDialog(NULL),
              itsTempDir(NULL),
              itsPrintProc(NULL),
              itsExportFile(NULL)
{
    setButtons(0);

    CFcEngine::instance()->readConfig(itsConfig);
    CFcEngine::setBgndCol(QApplication::palette().color(QPalette::Active, QPalette::Base));
    CFcEngine::setTextCol(QApplication::palette().color(QPalette::Active, QPalette::Text));
    KGlobal::locale()->insertCatalog(KFI_CATALOGUE);
    KIconLoader::global()->addAppDir(KFI_NAME);
    KAboutData* about = new KAboutData("fontinst",
         I18N_NOOP("KDE Font Installer"),
         0, 0,
         KAboutData::License_GPL,
         I18N_NOOP("GUI front end to the fonts:/ ioslave.\n"
         "(c) Craig Drummond, 2000 - 2006"));
    about->addAuthor("Craig Drummond", I18N_NOOP("Developer and maintainer"), "craig@kde.org");
    setAboutData(about);

    KConfigGroup cg(&itsConfig, CFG_GROUP);

    itsRunner=new CJobRunner(this);

    itsSplitter=new QSplitter(this);
    itsSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QStringList items;
    QBoxLayout  *mainLayout=new QBoxLayout(QBoxLayout::TopToBottom, this);
    KToolBar    *toolbar=new KToolBar(this);

    itsGroupsWidget=new QWidget(itsSplitter);
    itsFontsWidget=new QWidget(itsSplitter);
    itsPreviewWidget=new QWidget(itsSplitter);

    QGridLayout *groupsLayout=new QGridLayout(itsGroupsWidget),
                *fontsLayout=new QGridLayout(itsFontsWidget);
    QBoxLayout  *previewLayout=new QBoxLayout(QBoxLayout::TopToBottom, itsPreviewWidget);

    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    groupsLayout->setMargin(0);
    groupsLayout->setSpacing(KDialog::spacingHint());
    fontsLayout->setMargin(0);
    fontsLayout->setSpacing(KDialog::spacingHint());

    // Preview...
    previewLayout->setMargin(0);
    previewLayout->setSpacing(KDialog::spacingHint());

    QFrame     *previewFrame=new QFrame(itsPreviewWidget);
    QBoxLayout *previewFrameLayout=new QBoxLayout(QBoxLayout::LeftToRight, previewFrame);

    previewFrameLayout->setMargin(0);
    previewFrameLayout->setSpacing(0);
    previewFrame->setFrameShape(QFrame::StyledPanel);
    previewFrame->setFrameShadow(QFrame::Sunken);
    previewFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    itsPreview=new CFontPreview(previewFrame);
    itsPreview->setWhatsThis(i18n("This displays a preview of the selected font."));
    itsPreview->setContextMenuPolicy(Qt::CustomContextMenu);
    previewFrameLayout->addWidget(itsPreview);

    // Toolbar...
    KActionMenu *settingsMenu=new KActionMenu(KIcon("configure"), i18n("Settings"), this);
    KAction     *changeTextAct=new KAction(KIcon("text"), i18n("Change Preview Text..."), this),
                *duplicateFontsAct=new KAction(KIcon("file-find"), i18n("Scan For Duplicate Fonts..."), this);
                //*validateFontsAct=new KAction(KIcon("checkmark"), i18n("Validate Fonts..."), this);
                //*downloadFontsAct=new KAction(KIcon("go-down"), i18n("Download Fonts..."), this);

    itsToolsMenu=new KActionMenu(KIcon("tool2"), i18n("Tools"), this);
    itsMgtMode=new KToggleAction(KIcon("fonts"),
                                 i18n("Font Management Mode"), this),
    itsShowPreview=new KToggleAction(KIcon("thumbnail-show"), i18n("Show Preview"), this);
    settingsMenu->addAction(itsMgtMode);
    itsMgtMode->setChecked(true);
    settingsMenu->addSeparator();
    settingsMenu->addAction(itsShowPreview);
    settingsMenu->addAction(changeTextAct);
    settingsMenu->setDelayed(false);
    itsToolsMenu->addAction(duplicateFontsAct);
    //itsToolsMenu->addAction(validateFontsAct);
    //itsToolsMenu->addAction(downloadFontsAct);
    itsToolsMenu->setDelayed(false);
    toolbar->addAction(settingsMenu);
    toolbar->addAction(itsToolsMenu);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    if(Misc::root())
        itsModeControl=NULL;
    else
    {
        itsModeControl=new QComboBox(toolbar);
        itsModeAct=toolbar->addWidget(itsModeControl);
    }

    itsFilter=new CFontFilter(toolbar);
    toolbar->addWidget(itsFilter);

    itsPreviewControl=new CPreviewSelectAction(this);
    toolbar->addAction(itsPreviewControl);
    // Details - Groups...
    itsGroupList=new CGroupList(itsGroupsWidget);
    itsGroupListView=new CGroupListView(itsGroupsWidget, itsGroupList);

    //itsGroupListView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    if(itsModeControl)
        for(int i=0; i<3; ++i)
            itsModeControl->addItem(itsGroupList->group((CGroupListItem::EType)i)->name());

    KPushButton *createGroup=new CPushButton(KGuiItem(QString(), "list-add",
                                                      i18n("Create a new group")),
                                             itsGroupsWidget);

    itsDeleteGroupControl=new CPushButton(KGuiItem(QString(), "list-remove",
                                                   i18n("Remove group")),
                                          itsGroupsWidget);

    itsEnableGroupControl=new CPushButton(KGuiItem(QString(), "enablefont",
                                                   i18n("Enable all disabled fonts in the current group")),
                                          itsGroupsWidget);

    itsDisableGroupControl=new CPushButton(KGuiItem(QString(), "disablefont",
                                                    i18n("Disable all enabled fonts in the current group")),
                                           itsGroupsWidget);

    groupsLayout->addWidget(itsGroupListView, 0, 0, 1, 5);
    groupsLayout->addWidget(createGroup, 1, 0);
    groupsLayout->addWidget(itsDeleteGroupControl, 1, 1);
    groupsLayout->addWidget(itsEnableGroupControl, 1, 2);
    groupsLayout->addWidget(itsDisableGroupControl, 1, 3);
    groupsLayout->addItem(new QSpacerItem(itsDisableGroupControl->width(), KDialog::spacingHint(),
                          QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 4);
    // Details - Fonts...
    itsFontList=new CFontList(itsFontsWidget);
    itsFontListView=new CFontListView(itsFontsWidget, itsFontList);
    itsFontListView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    itsFontListView->readConfig(cg);

    itsAddFontControl=new CPushButton(KGuiItem(i18n("Add..."), "newfont",
                                               i18n("Install fonts")),
                                      itsFontsWidget);

    itsDeleteFontControl=new CPushButton(KGuiItem(i18n("Delete..."), "edit-delete",
                                                  i18n("Delete all selected fonts")),
                                         itsFontsWidget);

    itsEnableFontControl=new CPushButton(KGuiItem(i18n("Enable"), "enablefont",
                                                  i18n("Enable all selected fonts")),
                                         itsFontsWidget);

    itsDisableFontControl=new CPushButton(KGuiItem(i18n("Disable"), "disablefont",
                                                   i18n("Disable all selected fonts")),
                                          itsFontsWidget);

    fontsLayout->addWidget(itsFontListView, 0, 0, 1, 5);
    fontsLayout->addWidget(itsAddFontControl, 1, 0);
    fontsLayout->addWidget(itsDeleteFontControl, 1, 1);
    fontsLayout->addWidget(itsEnableFontControl, 1, 2);
    fontsLayout->addWidget(itsDisableFontControl, 1, 3);
    fontsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                         QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 4);

    // Details - Status...
    QWidget    *statusWidget=new QWidget(this);
    QBoxLayout *statusLayout=new QBoxLayout(QBoxLayout::LeftToRight, statusWidget);
    itsStatusLabel = new QLabel(statusWidget);
    itsStatusLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    itsListingProgress=new CProgressBar(statusWidget, itsStatusLabel->height());
    itsListingProgress->setRange(0, 100);
    statusLayout->setMargin(0);
    statusLayout->setSpacing(KDialog::spacingHint());
    statusLayout->addWidget(itsStatusLabel);
    statusLayout->addItem(new QSpacerItem(0, itsListingProgress->height()+4,
                                          QSizePolicy::Fixed, QSizePolicy::Fixed));
    statusLayout->addWidget(itsListingProgress);

    // Layout widgets...
    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(itsSplitter);
    mainLayout->addWidget(statusWidget);
    previewLayout->addWidget(previewFrame);

    // Set size of widgets...
    itsSplitter->setChildrenCollapsible(false);
    itsSplitter->setStretchFactor(0, 0);
    itsSplitter->setStretchFactor(1, 1);
    itsSplitter->setStretchFactor(2, 0);
    toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    QList<int> defaultSizes;

    defaultSizes+=100;
    defaultSizes+=350;
    defaultSizes+=150;

    QList<int> sizes(cg.readEntry(CFG_SPLITTER_SIZES, defaultSizes));

    if(3!=sizes.count())
        sizes=defaultSizes;

    itsSplitter->setSizes(sizes);

    // Connect signals...
    connect(itsFilter, SIGNAL(textChanged(const QString &)), itsFontListView, SLOT(filterText(const QString &)));
    connect(itsFilter, SIGNAL(criteriaChanged(int)), itsFontListView, SLOT(filterCriteria(int)));
    connect(itsGroupListView, SIGNAL(del()), SLOT(removeGroup()));
    connect(itsGroupListView, SIGNAL(print()), SLOT(printGroup()));
    connect(itsGroupListView, SIGNAL(enable()), SLOT(enableGroup()));
    connect(itsGroupListView, SIGNAL(disable()), SLOT(disableGroup()));
    connect(itsGroupListView, SIGNAL(exportGroup()), SLOT(exportGroup()));
    connect(itsGroupListView, SIGNAL(itemSelected(const QModelIndex &)),
           SLOT(groupSelected(const QModelIndex &)));
    connect(itsGroupListView, SIGNAL(unclassifiedChanged()), SLOT(setStatusBar()));
    connect(itsGroupList, SIGNAL(refresh()), SLOT(refreshFamilies()));
    connect(itsFontList, SIGNAL(finished()), SLOT(listingCompleted()));
    connect(itsFontList, SIGNAL(percent(int)), itsListingProgress, SLOT(setValue(int)));
    connect(itsFontList, SIGNAL(status(const QString &)), itsStatusLabel,
            SLOT(setText(const QString &)));
    connect(itsFontListView, SIGNAL(del()), SLOT(deleteFonts()));
    connect(itsFontListView, SIGNAL(print()), SLOT(print()));
    connect(itsFontListView, SIGNAL(enable()), SLOT(enableFonts()));
    connect(itsFontListView, SIGNAL(disable()), SLOT(disableFonts()));
    connect(itsFontListView, SIGNAL(reload()), SLOT(reload()));
    connect(itsFontListView, SIGNAL(fontsDropped(const QSet<KUrl> &)),
           SLOT(addFonts(const QSet<KUrl> &)));
    connect(itsFontListView, SIGNAL(itemSelected(const QModelIndex &, bool, bool)),
           SLOT(fontSelected(const QModelIndex &, bool, bool)));
    connect(itsFontListView, SIGNAL(refresh()), SLOT(setStatusBar()));
    connect(itsGroupListView, SIGNAL(unclassifiedChanged()), itsFontListView, SLOT(refreshFilter()));
    connect(createGroup, SIGNAL(clicked()), SLOT(addGroup()));
    connect(itsDeleteGroupControl, SIGNAL(clicked()), SLOT(removeGroup()));
    connect(itsEnableGroupControl, SIGNAL(clicked()), SLOT(enableGroup()));
    connect(itsDisableGroupControl, SIGNAL(clicked()), SLOT(disableGroup()));
    connect(itsAddFontControl, SIGNAL(clicked()), SLOT(addFonts()));
    connect(itsDeleteFontControl, SIGNAL(clicked()), SLOT(deleteFonts()));
    connect(itsEnableFontControl, SIGNAL(clicked()), SLOT(enableFonts()));
    connect(itsDisableFontControl, SIGNAL(clicked()), SLOT(disableFonts()));
    connect(itsMgtMode, SIGNAL(toggled(bool)), SLOT(toggleFontManagement(bool)));
    connect(itsShowPreview, SIGNAL(toggled(bool)), SLOT(showPreview(bool)));
    connect(changeTextAct, SIGNAL(triggered(bool)), SLOT(changeText()));
    connect(itsPreviewControl, SIGNAL(range(const QList<CFcEngine::TRange> &)),
            itsPreview, SLOT(setUnicodeRange(const QList<CFcEngine::TRange> &)));
    connect(duplicateFontsAct, SIGNAL(triggered(bool)), SLOT(duplicateFonts()));
    //connect(validateFontsAct, SIGNAL(triggered(bool)), SLOT(validateFonts()));
    //connect(downloadFontsAct, SIGNAL(triggered(bool)), SLOT(downloadFonts()));
    if(itsModeControl)
        connect(itsModeControl, SIGNAL(activated(int)), SLOT(selectGroup(int)));

    itsMgtMode->setChecked(cg.readEntry(CFG_FONT_MGT_MODE, false));
    itsShowPreview->setChecked(cg.readEntry(CFG_SHOW_PREVIEW, false));
    showPreview(itsShowPreview->isChecked());
    itsPreviewWidget->setVisible(itsShowPreview->isChecked());
    toggleFontManagement(itsMgtMode->isChecked());
    selectMainGroup();
    itsFontList->scan();
}

CKCmFontInst::~CKCmFontInst()
{
    KConfigGroup cg(&itsConfig, CFG_GROUP);

    cg.writeEntry(CFG_SPLITTER_SIZES, itsSplitter->sizes());
    cg.writeEntry(CFG_FONT_MGT_MODE, itsMgtMode->isChecked());
    cg.writeEntry(CFG_SHOW_PREVIEW, itsShowPreview->isChecked());
    itsFontListView->writeConfig(cg);
    delete itsTempDir;
    delete itsPrintProc;
    delete itsExportFile;
}

QString CKCmFontInst::quickHelp() const
{
    return Misc::root()
               ? i18n("<h1>Font Installer</h1><p> This module allows you to"
                      " install TrueType, Type1, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
                      " font, simply copy one into the folder.</p>")
               : i18n("<h1>Font Installer</h1><p> This module allows you to"
                      " install TrueType, Type1, and Bitmap"
                      " fonts.</p><p>You may also install fonts using Konqueror:"
                      " type fonts:/ into Konqueror's location bar"
                      " and this will display your installed fonts. To install a"
                      " font, simply copy it into the appropriate folder - "
                      " \"%1\" for fonts available to just yourself, or "
                      " \"%2\" for system-wide fonts (available to all).</p>",
                      i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS));
}

void CKCmFontInst::fontSelected(const QModelIndex &index, bool en, bool dis)
{
    itsDeleteFontControl->setEnabled(false);
    itsDisableFontControl->setEnabled(en);
    itsEnableFontControl->setEnabled(dis);

    if(index.isValid())
    {
        CFontModelItem *mi=static_cast<CFontModelItem *>(index.internalPointer());
        CFontItem      *font=NULL;

        if(mi->parent())
            font=static_cast<CFontItem *>(index.internalPointer());
        else
            font=(static_cast<CFamilyItem *>(index.internalPointer()))->regularFont();

        if(font)
        {
            if(itsPreview->width()>6)
            {
                KUrl url(font->isEnabled()
                            ? font->url()
                            : font->fileName().isEmpty()
                                ? font->url() : KUrl::fromPath(font->fileName()));

                itsPreview->showFont(url, font->isEnabled() ? font->name() : QString(),
                                     font->styleInfo(), font->isEnabled() ? 1 : font->index());
            }
            itsDeleteFontControl->setEnabled(true);
        }
    }
}

void CKCmFontInst::reload()
{
    if(!working(false))
    {
        itsListingProgress->show();
        itsFontList->scan();
        itsGroupList->rescan();
    }
}

void CKCmFontInst::addFonts()
{
    if(!working())
    {
        QString filter("application/x-font-ttf application/x-font-otf "
                       "application/x-font-ttc application/x-font-type1");

        if(itsMgtMode->isChecked())
        {
            if(FC::bitmapsEnabled())
                filter+=" application/x-font-pcf application/x-font-bdf";
            filter+=" fonts/group";
        }
        filter+=" fonts/package";

        KUrl::List list=KFileDialog::getOpenUrls(KUrl(), filter, this, i18n("Add Fonts"));

        if(list.count())
        {
            QSet<KUrl>           urls;
            KUrl::List::Iterator it(list.begin()),
                                 end(list.end());

            for(; it!=end; ++it)
            {
                if(KFI_KIO_FONTS_PROTOCOL!=(*it).protocol()) // Dont try to install from fonts:/ !!!
                {
                    KUrl url(KIO::NetAccess::mostLocalUrl(*it, this));

                    if(url.isLocalFile())
                    {
                        QString file(url.path());

                        bool package=Misc::isPackage(file),
                             group=!package && Misc::isGroup(file);

                        if(package || group) // If its a package or group - we need to unzip 1st...
                        {
                            KZip zip(url.path());

                            delete itsTempDir;
                            itsTempDir=NULL;

                            if(zip.open(QIODevice::ReadOnly))
                            {
                                const KArchiveDirectory *zipDir=zip.directory();

                                if(zipDir)
                                {
                                    QStringList fonts(zipDir->entries());

                                    if(fonts.count())
                                    {
                                        QStringList::ConstIterator it(fonts.begin()),
                                                                   end(fonts.end());
 
                                        for(; it!=end; ++it)
                                        {
                                            const KArchiveEntry *entry=zipDir->entry(*it);

                                            if(entry && entry->isFile())
                                            {
                                                if(!itsTempDir)
                                                {
                                                    itsTempDir=new KTempDir(KStandardDirs::locateLocal("tmp",
                                                                            KFI_TMP_DIR_PREFIX));
                                                    itsTempDir->setAutoRemove(true);
                                                }

                                                ((KArchiveFile *)entry)->copyTo(itsTempDir->name());

                                                if(group && KFI_GROUPS_FILE==entry->name())
                                                {
                                                    itsGroupList->merge(itsTempDir->name()+entry->name());
                                                    ::unlink(QFile::encodeName(itsTempDir->name()+
                                                                               entry->name()));
                                                }
                                                else
                                                {
                                                    QString name(entry->name());

                                                    //
                                                    // Cant install hidden fonts, therefore need to
                                                    // unhide 1st!
                                                    if(Misc::isHidden(name))
                                                    {
                                                        ::rename(QFile::encodeName(itsTempDir->name()+
                                                                                   name).data(),
                                                                 QFile::encodeName(itsTempDir->name()+
                                                                                  name.mid(1)).data());
                                                        name=name.mid(1);
                                                    }

                                                    KUrl url(itsTempDir->name()+name);

                                                    if(!Misc::isMetrics(name))
                                                        urls.insert(url);
                                               }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if(!Misc::isMetrics(url))
                            urls.insert(url);
                    }
                    else if(!Misc::isMetrics(url))
                        urls.insert(url);
                }
            }
            if(urls.count())
                addFonts(urls);
        }
    }
}

void CKCmFontInst::groupSelected(const QModelIndex &index)
{
    CGroupListItem *grp=NULL;

    if(index.isValid())
        grp=static_cast<CGroupListItem *>(index.internalPointer());

    itsFontListView->setFilterGroup(grp);
    setStatusBar();

    //
    // Check fonts listed within group are still valid!
    if(grp && grp->isStandard() && !grp->validated())
    {
        QSet<QString>           remList;
        QSet<QString>::Iterator it(grp->families().begin()),
                                end(grp->families().end());

        for(; it!=end; ++it)
            if(!itsFontList->hasFamily(*it))
                remList.insert(*it);
        it=remList.begin();
        end=remList.end();
        for(; it!=end; ++it)
            itsGroupList->removeFromGroup(grp, *it);
        grp->setValidated();
    }
}

void CKCmFontInst::print(bool all)
{
    //
    // In order to support printing of newly installed/enabled fonts, the actual printing
    // is carried out by the kfontinst helper app. This way we know Qt's font list will be
    // up to date.
    if(!working() && (!itsPrintProc || !itsPrintProc->isRunning()))
    {
        QSet<Misc::TFont> fonts;

        itsFontListView->getPrintableFonts(fonts, !all);

        if(fonts.count())
        {
            CPrintDialog dlg(this);
            KConfigGroup cg(&itsConfig, CFG_GROUP);

            if(dlg.exec(cg.readEntry(CFG_FONT_SIZE, 1)))
            {
                static const int constSizes[]={0, 12, 18, 24, 36, 48};
                QSet<Misc::TFont>::ConstIterator it(fonts.begin()),
                                                 end(fonts.end());
                KTemporaryFile                   tmpFile;
                bool                             useGrpFile(fonts.count()>64),
                                                 startProc(true);

                if(!itsPrintProc)
                    itsPrintProc=new KProcess;
                else
                    itsPrintProc->clearArguments();

                *itsPrintProc << KFI_APP;
                //
                // If we have lots of fonts to print, pass kfontinst a tempory groups file to print
                // instead of passing font by font...
                if(useGrpFile)
                {
                    if(tmpFile.open())
                    {
                        QTextStream str(&tmpFile);

                        for(; it!=end; ++it)
                            str << (*it).family << endl
                                << (*it).styleInfo << endl;

                        *itsPrintProc << "-p"
                                      << QString().sprintf("0x%x", (unsigned int)topLevelWidget()->winId())
                                      << KGlobal::caption().toUtf8()
                                      << QString().setNum(constSizes[dlg.chosenSize() < 6
                                                           ? dlg.chosenSize() : 2])
                                      << tmpFile.fileName()
                                      << "y"; // y => implies kfontinst will remove our tmp file
                    }
                    else
                    {
                        KMessageBox::error(this, i18n("Failed to save list of fonts to print."));
                        startProc=false;
                    }
                }
                else
                {
                    *itsPrintProc << "-P"
                                  << QString().sprintf("0x%x", (unsigned int)topLevelWidget()->winId())
                                  << KGlobal::caption().toUtf8()
                                  << QString().setNum(constSizes[dlg.chosenSize()<6 ? dlg.chosenSize() : 2]);

                    for(; it!=end; ++it)
                        *itsPrintProc << (*it).family.toUtf8()
                                      << QString().setNum((*it).styleInfo);
                }

                if(startProc)
                    if(itsPrintProc->start(KProcess::DontCare))
                    {
                        if(useGrpFile)
                            tmpFile.setAutoRemove(false);
                    }
                    else
                        KMessageBox::error(this, i18n("Failed to start font printer."));
                cg.writeEntry(CFG_FONT_SIZE, dlg.chosenSize());
            }
        }
        else
            KMessageBox::information(this, i18n("There are no printable fonts.\n"
                                                "You can only print non-bitmap "
                                                 "and enabled fonts."),
                                     i18n("Cannot Print"));
    }
}

void CKCmFontInst::deleteFonts()
{
    CJobRunner::ItemList urls;
    QStringList          fontNames;
    QSet<Misc::TFont>    fonts;
    bool                 hasSys(false);

    itsDeletedFonts.clear();
    itsFontListView->getFonts(urls, fontNames, &fonts, &hasSys, true);

    if(urls.isEmpty())
        KMessageBox::information(this, i18n("You did not select anything to delete."),
                                       i18n("Nothing to Delete"));
    else
    {
        QSet<Misc::TFont>::ConstIterator it(fonts.begin()),
                                         end(fonts.end());

        for(; it!=end; ++it)
            itsDeletedFonts.insert((*it).family);

        deleteFonts(urls, fontNames, hasSys);
    }
}

void CKCmFontInst::enableFonts()
{
    toggleFonts(true);
}

void CKCmFontInst::disableFonts()
{
    toggleFonts(false);
}

void CKCmFontInst::addGroup()
{
    bool    ok;
    QString name(KInputDialog::getText(i18n("Create New Group"),
                                       i18n("Please enter the name of the new group:"),
                                       i18n("New Group"), &ok, this));

    if(ok && !name.isEmpty())
        itsGroupList->createGroup(name);
}

void CKCmFontInst::removeGroup()
{
    if(itsGroupList->removeGroup(itsGroupListView->currentIndex()))
        selectMainGroup();
}

void CKCmFontInst::enableGroup()
{
    toggleGroup(true);
}

void CKCmFontInst::disableGroup()
{
    toggleGroup(false);
}

void CKCmFontInst::exportGroup()
{
    if(!working())
    {
        QModelIndex index(itsGroupListView->currentIndex());

        if(index.isValid())
        {
            CGroupListItem *grp=static_cast<CGroupListItem *>(index.internalPointer());

            if(grp)
            {
                CJobRunner::ItemList items;
                QStringList          fontNames;

                itsFontListView->getFonts(items, fontNames, NULL, NULL, false, true, true);

                if(items.count())
                {
                    KUrl::List urls;

                    CJobRunner::ItemList::ConstIterator it(items.begin()),
                                                        end(items.end());

                    for(; it!=end; ++it)
                        urls.append(*it);

                    QString name(grp->name());
                    KUrl    dir=KDirSelectDialog::selectDirectory(KUrl(), true, this,
                                                                  i18n("Select Export Folder For \"%1\"", name));

                    if(!dir.isEmpty())
                    {
                        QString file(Misc::dirSyntax(dir.path())+name+KFI_FONTS_GROUP);

                        if(!Misc::fExists(file) ||
                           KMessageBox::Yes==KMessageBox::warningYesNo(this,
                               i18n("<p>A font group package named <b>%1</b> already exists!</p>"
                                    "<p>Do you wish to overwrite this?</p>", file),
                               i18n("Exported Group Exists"),
                               KGuiItem(i18n("Overwrite")), KStandardGuiItem::cancel()))
                        {
                            delete itsTempDir;
                            itsTempDir=new KTempDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));

                            if(itsTempDir && !itsTempDir->name().isEmpty())
                            {
                                itsTempDir->setAutoRemove(true);
                                itsExportFile=new KZip(file);

                                if(itsExportFile && itsExportFile->open(IO_WriteOnly))
                                {
                                    if(itsGroupList->save(itsTempDir->name()+KFI_GROUPS_FILE, grp))
                                    {
                                        itsExportFile->addLocalFile(itsTempDir->name()+KFI_GROUPS_FILE,
                                                                    KFI_GROUPS_FILE);
                                        ::unlink(QFile::encodeName(itsTempDir->name()+KFI_GROUPS_FILE));
                                        itsJob=KIO::copy(urls, itsTempDir->name(), true);
                                        connect(itsJob, SIGNAL(result(KJob *)),
                                                SLOT(exportJobResult(KJob *)));
                                        connect(itsJob, SIGNAL(copyingDone(KIO::Job *, const KUrl &,
                                                               const KUrl &, time_t, bool, bool)),
                                                SLOT(exported(KIO::Job *, const KUrl &, const KUrl &)));
                                    }
                                    else
                                    {
                                        KMessageBox::error(this, i18n("Could not save group details!"));
                                        delete itsTempDir;
                                        itsTempDir=NULL;
                                        delete itsExportFile;
                                        itsExportFile=NULL;
                                    }
                                }
                                else
                                {
                                    KMessageBox::error(this, i18n("Could not create %1",
                                                                  dir.path()+name+KFI_FONTS_GROUP));
                                    delete itsExportFile;
                                    itsExportFile=NULL;
                                    delete itsTempDir;
                                    itsTempDir=NULL;
                                }
                            }
                            else
                            {
                                KMessageBox::error(this, i18n("Could not create temporary folder to save "
                                                              "group into."));
                                delete itsTempDir;
                                itsTempDir=NULL;
                            }
                        }
                    }
                }
                else
                    KMessageBox::error(this, i18n("<qt>Group <b>%1</b> has no fonts!</b>",
                                                  grp->name()));
            }
        }
    }
}

void CKCmFontInst::exportJobResult(KJob *job)
{
    bool ok=job && 0==job->error();

    itsJob=NULL;

    if(!ok && itsExportFile)
    {
        itsExportFile->close();
        unlink(QFile::encodeName(itsExportFile->fileName()));
    }
    delete itsExportFile;
    delete itsTempDir;
    itsExportFile=NULL;
    itsTempDir=NULL;
}

void CKCmFontInst::exported(KIO::Job *, const KUrl &, const KUrl &to)
{
    if(itsExportFile)
    {
        QString file(to.fileName());

        if(Misc::isPackage(file))
        {
            KZip zip(to.path());

            if(zip.open(IO_ReadOnly))
            {
                const KArchiveDirectory *zipDir=zip.directory();

                if(zipDir)
                {
                    QStringList fonts(zipDir->entries());
                    KTempDir    tmpDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));

                    if(fonts.count() && !tmpDir.name().isEmpty())
                    {
                        QStringList::ConstIterator it(fonts.begin()),
                                                   end(fonts.end());

                        tmpDir.setAutoRemove(true);
                        for(; it!=end; ++it)
                        {
                            const KArchiveEntry *entry=zipDir->entry(*it);

                            if(entry && entry->isFile() &&
                               (NULL==itsExportFile->directory() ||
                               (NULL==itsExportFile->directory()->entry(entry->name()))))
                            {
                                ((KArchiveFile *)entry)->copyTo(tmpDir.name());
                                itsExportFile->addLocalFile(tmpDir.name()+entry->name(),
                                                            entry->name());
                            }
                        }
                    }
                }
            }
        }
        else
        {
            QString destFile(Misc::unhide(Misc::getFile(to.path())));

            if (NULL==itsExportFile->directory() ||
                NULL==itsExportFile->directory()->entry(destFile))
            itsExportFile->addLocalFile(to.path(), destFile);
        }
        ::unlink(QFile::encodeName(to.path()));
    }
}

void CKCmFontInst::changeText()
{
    if(!working())
    {
        bool             status;
        QRegExpValidator validator(QRegExp(".*"), 0L);
        QString          oldStr(CFcEngine::instance()->getPreviewString()),
                         newStr(KInputDialog::getText(i18n("Preview String"),
                                                      i18n("Please enter new string:"),
                                                      oldStr, &status, this, &validator));

        if(status && oldStr!=newStr)
        {
            CFcEngine::instance()->setPreviewString(newStr);
            itsFontList->forceNewPreviews();

            if(itsPreview->width()>6)
                itsPreview->showFont();
        }
    }
}

void CKCmFontInst::showPreview(bool s)
{
    itsPreviewWidget->setVisible(s);
    itsPreviewControl->setVisible(itsMgtMode->isOn() && s);
}

void CKCmFontInst::duplicateFonts()
{
    CDuplicatesDialog    dlg(this, itsRunner, itsFontList);
    CJobRunner::ItemList update;

    dlg.exec();

    if(dlg.modifiedUser())
        update.append(baseUrl(false));
    if(!Misc::root() && dlg.modifiedSys())
        update.append(baseUrl(true));

    if(update.count())
        doCmd(CJobRunner::CMD_UPDATE, update, KUrl());
}

//void CKCmFontInst::validateFonts()
//{
//}

//void CKCmFontInst::downloadFonts()
//{
//}

void CKCmFontInst::print()
{
    print(false);
}

void CKCmFontInst::printGroup()
{
    print(true);
}

void CKCmFontInst::listingCompleted()
{
    if(itsDeletedFonts.count())
    {
        QSet<QString>::Iterator it(itsDeletedFonts.begin()),
                                end(itsDeletedFonts.end());

        for(; it!=end; ++it)
            if(!itsFontList->hasFamily(*it))
                itsGroupList->removeFamily(*it);

        itsDeletedFonts.clear();
    }

    refreshFamilies();
    itsListingProgress->hide();
    itsFontListView->selectFirstFont();
}

void CKCmFontInst::refreshFamilies()
{
    QSet<QString> enabledFamilies,
                  disabledFamilies,
                  partialFamilies;

    itsFontList->getFamilyStats(enabledFamilies, disabledFamilies, partialFamilies);
    itsGroupList->updateStatus(enabledFamilies, disabledFamilies, partialFamilies);
    setStatusBar();
}

void CKCmFontInst::setStatusBar()
{
    int  enabled=0,
         disabled=0,
         partial=0;
    bool selectedEnabled=false,
         selectedDisabled=false;

    if(0==itsFontList->families().count())
        itsStatusLabel->setText(i18n("No fonts"));
    else
    {
        itsFontListView->stats(enabled, disabled, partial);
        itsFontListView->selectedStatus(selectedEnabled, selectedDisabled);

        QString text(i18np("1 Font", "%1 Fonts", enabled+disabled+partial));

        if(disabled||partial)
            text+=i18n(" (%1 Enabled, %2 Disabled, %3 Partial)", enabled, disabled, partial);

        itsStatusLabel->setText(text);
    }

    bool isStd(itsGroupListView->isStandard());

    itsAddFontControl->setEnabled(!isStd);
    itsDeleteGroupControl->setEnabled(isStd);
    itsEnableGroupControl->setEnabled(disabled||partial);
    itsDisableGroupControl->setEnabled(isStd && (enabled||partial));

    itsGroupListView->controlMenu(itsDeleteGroupControl->isEnabled(),
                                  itsEnableGroupControl->isEnabled(),
                                  itsDisableGroupControl->isEnabled(), enabled||partial,
                                  enabled||partial||disabled);

    itsEnableFontControl->setEnabled(selectedDisabled);
    itsDisableFontControl->setEnabled(selectedEnabled);
    itsDeleteFontControl->setEnabled(selectedEnabled||selectedDisabled);
}

void CKCmFontInst::addFonts(const QSet<KUrl> &src)
{
    if(!working() && src.count() && !itsGroupListView->isStandard())
    {
        KUrl dest;

        if(Misc::root())
            dest=baseUrl(true);
        else
        {
            switch(getCurrentGroupType())
            {
                case CGroupListItem::ALL:
                case CGroupListItem::UNCLASSIFIED:
                    switch(KMessageBox::questionYesNoCancel(this,
                           i18n("Do you wish to install the font(s) for personal use "
                                "(only usable by you), or "
                                "system-wide (usable by all users)?"),
                           i18n("Where to Install"), KGuiItem(i18n(KFI_KIO_FONTS_USER)),
                                KGuiItem(i18n(KFI_KIO_FONTS_SYS))))
                    {
                        case KMessageBox::Yes:
                            dest=baseUrl(false);
                            break;
                        case KMessageBox::No:
                            if(itsRunner->getAdminPasswd(this))
                                dest=baseUrl(true);
                            else
                                return;
                            break;
                        default:
                        case KMessageBox::Cancel:
                            return;
                    }
                    break;
                case CGroupListItem::PERSONAL:
                    dest=baseUrl(false);
                    break;
                case CGroupListItem::SYSTEM:
                    if(itsRunner->getAdminPasswd(this))
                        dest=baseUrl(true);
                    else
                        return;
                    break;
                default:
                    return;
            }
        }

        QSet<KUrl>                copy;
        QSet<KUrl>::ConstIterator it,
                                  end(src.end());

        //
        // Check if font has any associated AFM or PFM file...
        itsStatusLabel->setText(i18n("Looking for any associated files..."));

        if(!itsProgress)
        {
            itsProgress=new KProgressDialog(this, i18n("Scanning Files..."),
                                            i18n("Looking for additional files to install..."), true);
            itsProgress->setAutoReset(true);
            itsProgress->setAutoClose(true);
        }

        itsProgress->setAllowCancel(false);
        itsProgress->setMinimumDuration(500);
        itsProgress->progressBar()->show();
        itsProgress->progressBar()->setRange(0, src.size());
        itsProgress->progressBar()->setValue(0);

        int steps=src.count()<200 ? 1 : src.count()/10;
        for(it=src.begin(); it!=end; ++it)
        {
            KUrl::List associatedUrls;

            itsProgress->setLabel(i18n("Looking for files associated with %1", (*it).prettyUrl()));
            itsProgress->progressBar()->setValue(itsProgress->progressBar()->value()+1);
            if(1==steps || 0==(itsProgress->progressBar()->value()%steps))
            {
                bool dialogVisible(itsProgress->isVisible());
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                if(dialogVisible && !itsProgress->isVisible()) // User closed dialog! re-open!!!
                    itsProgress->show();
            }

            CJobRunner::getAssociatedUrls(*it, associatedUrls, false, this);
            copy.insert(*it);

            KUrl::List::Iterator aIt(associatedUrls.begin()),
                                 aEnd(associatedUrls.end());

            for(; aIt!=aEnd; ++aIt)
                copy.insert(*aIt);
        }
        itsProgress->close();

        CJobRunner::ItemList installUrls;

        end=copy.end();
        for(it=copy.begin(); it!=end; ++it)
            installUrls.append(*it);

        itsStatusLabel->setText(i18n("Installing font(s)..."));
        doCmd(CJobRunner::CMD_INSTALL, installUrls, dest);
    }
}

void CKCmFontInst::toggleFontManagement(bool on)
{
    if(working())
        itsMgtMode->setChecked(!on);
    else
    {
        if(!on)
            itsPreviewControl->setStd();
        itsPreviewControl->setVisible(on && itsShowPreview->isOn());
        itsToolsMenu->setVisible(on);
        itsFontListView->setMgtMode(on);
        itsFilter->setMgtMode(on);
        if(itsModeControl)
            itsModeAct->setVisible(!on);
        itsEnableFontControl->setVisible(on);
        itsDisableFontControl->setVisible(on);
        selectMainGroup();
        itsGroupsWidget->setVisible(on);
        setStatusBar();
    }
}

void CKCmFontInst::selectGroup(int grp)
{
    CGroupListItem::EType t((CGroupListItem::EType)grp);
    QModelIndex           current(itsGroupListView->currentIndex());

    if(current.isValid())
    {
        CGroupListItem *grpItem=static_cast<CGroupListItem *>(current.internalPointer());

        if(grpItem && t==grpItem->type())
            return;
        else
            itsGroupListView->selectionModel()->select(current,
                                                       QItemSelectionModel::Deselect);
    }

    QModelIndex idx(itsGroupList->index(t));

    itsGroupListView->selectionModel()->select(idx, QItemSelectionModel::Select);
    itsGroupListView->setCurrentIndex(idx);
    groupSelected(idx);
    if(itsModeControl)
        itsModeControl->setCurrentItem(grp);
    itsFontListView->refreshFilter();
}

void CKCmFontInst::deleteFonts(CJobRunner::ItemList &urls, const QStringList &fonts, bool hasSys)
{
    if(!working() && urls.count())
    {
        bool doIt=false;

        switch(fonts.count())
        {
            case 0:
                break;
            case 1:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                           i18n("<p>Do you really want to "
                                "delete</p></p>\'<b>%1</b>\'?</p>", fonts.first()),
                           i18n("Delete Font"), KStandardGuiItem::del());
            break;
            default:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                           i18np("Do you really want to delete this font?",
                                 "Do you really want to delete these %1 fonts?",
                                 fonts.count()),
                           fonts, i18n("Delete Fonts"), KStandardGuiItem::del());
        }

        if(doIt && (!hasSys || itsRunner->getAdminPasswd(this)))
        {
            itsStatusLabel->setText(i18n("Deleting font(s)..."));
            doCmd(CJobRunner::CMD_DELETE, urls, KUrl());
        }
    }
}

void CKCmFontInst::toggleGroup(bool enable)
{
    QModelIndex idx(itsGroupListView->currentIndex());

    if(idx.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(idx.internalPointer());

        if(grp)
            toggleFonts(enable, grp->name());
    }
}

void CKCmFontInst::toggleFonts(bool enable, const QString &grp)
{
    if(!working())
    {
        CJobRunner::ItemList urls;
        QStringList          fonts;
        bool                 hasSys(false);

        itsFontListView->getFonts(urls, fonts, NULL, &hasSys, grp.isEmpty(), !enable, enable);

        if(urls.isEmpty())
            KMessageBox::information(this,
                                     enable ? i18n("You did not select anything to enable.")
                                            : i18n("You did not select anything to disable."),
                                     enable ? i18n("Nothing to Enable") : i18n("Nothing to Disable"));
        else
            toggleFonts(urls, fonts, enable, grp, hasSys);
    }
}

void CKCmFontInst::toggleFonts(CJobRunner::ItemList &urls, const QStringList &fonts, bool enable,
                               const QString &grp, bool hasSys)
{
    bool doIt=false;

    switch(fonts.count())
    {
        case 0:
            break;
        case 1:
            doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                       grp.isEmpty()
                            ? enable ? i18n("<p>Do you really want to "
                                            "enable</p><p>\'<b>%1</b>\'?</p>", fonts.first())
                                     : i18n("<p>Do you really want to "
                                            "disable</p><p>\'<b>%1</b>\'?</p>", fonts.first())
                            : enable ? i18n("<p>Do you really want to "
                                            "enable</p><p>\'<b>%1</b>\', "
                                            "contained within group \'<b>%2</b>\'?</p>",
                                            fonts.first(), grp)
                                     : i18n("<p>Do you really want to "
                                            "disable</p><p>\'<b>%1</b>\', "
                                            "contained within group \'<b>%2</b>\'?</p>",
                                            fonts.first(), grp),
                       enable ? i18n("Enable Font") : i18n("Disable Font"),
                       enable ? KGuiItem(i18n("Enable"), "enablefont", i18n("Enable font"))
                              : KGuiItem(i18n("Disable"), "disablefont", i18n("Disable font")));
            break;
        default:
            doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                       grp.isEmpty()
                            ? enable ? i18np("Do you really want to enable this font?",
                                             "Do you really want to enable these %1 fonts?",
                                             urls.count())
                                     : i18np("Do you really want to disable this font?",
                                             "Do you really want to disable these %1 fonts?",
                                             urls.count())
                            : enable ? i18np("<p>Do you really want to enable this font "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             "<p>Do you really want to enable these %1 fonts "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             urls.count(), grp)
                                     : i18np("<p>Do you really want to disable this font "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             "<p>Do you really want to disable these %1 fonts "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             urls.count(), grp),
                       fonts,
                       enable ? i18n("Enable Fonts") : i18n("Disable Fonts"),
                       enable ? KGuiItem(i18n("Enable"), "enablefont", i18n("Enable fonts"))
                              : KGuiItem(i18n("Disable"), "disablefont", i18n("Disable fonts")));
    }

    if(doIt && (!hasSys || itsRunner->getAdminPasswd(this)))
    {
        if(enable)
            itsStatusLabel->setText(i18n("Enabling font(s)..."));
        else
            itsStatusLabel->setText(i18n("Disabling font(s)..."));

        doCmd(enable ? CJobRunner::CMD_ENABLE : CJobRunner::CMD_DISABLE, urls, KUrl());
    }
}

bool CKCmFontInst::working(bool displayMsg)
{
return false;
    if(NULL!=itsJob || itsFontList->active())
    {
        if(displayMsg)
            KMessageBox::error(this, i18n("Sorry, the installer is currently busy. Please wait for the "
                                          "current operation to terminate."));
        return true;
    }

    return false;
}

KUrl CKCmFontInst::baseUrl(bool sys)
{
    return Misc::root()
        ? KUrl(KFI_KIO_FONTS_PROTOCOL":/")
        : sys
            ? KUrl(KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS"/")
            : KUrl(KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_USER"/");
}

void CKCmFontInst::selectMainGroup()
{
    selectGroup(Misc::root() || itsMgtMode->isOn()
                    ? CGroupListItem::ALL : CGroupListItem::PERSONAL);
}

void CKCmFontInst::doCmd(CJobRunner::ECommand cmd, const CJobRunner::ItemList &urls, const KUrl &dest)
{
    itsFontList->setAutoUpdate(false);
    itsRunner->exec(cmd, urls, dest);
    CFcEngine::instance()->setDirty();
    setStatusBar();
    itsFontList->scan();
    itsFontList->setAutoUpdate(true);
    delete itsTempDir;
    itsTempDir=NULL;
}

CGroupListItem::EType CKCmFontInst::getCurrentGroupType()
{
    if(itsMgtMode->isOn())
        return itsGroupListView->getType();
    else if(itsModeControl)
        return (CGroupListItem::EType)itsModeControl->currentItem();

    return CGroupListItem::ALL;
}

}

#include "KCmFontInst.moc"
