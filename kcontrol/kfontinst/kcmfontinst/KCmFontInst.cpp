/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include <QtGui/QGridLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSplitter>
#include <QtGui/QProgressBar>
#include <QtCore/QCoreApplication>
#include <QtGui/QApplication>
#include <QtCore/QTextStream>
#include <QtGui/QComboBox>
#include <QtCore/QProcess>
#include <QtGui/QPainter>
#include <KDE/KAboutData>
#include <KDE/KToolBar>
#include <KDE/KFileDialog>
#include <KDE/KMessageBox>
#include <KDE/KIO/Job>
#include <KDE/KIO/NetAccess>
#include <KDE/KPushButton>
#include <KDE/KGuiItem>
#include <KDE/KInputDialog>
#include <KDE/KIconLoader>
#include <KDE/KProgressDialog>
#include <KDE/KZip>
#include <KDE/KTempDir>
#include <KDE/KTemporaryFile>
#include <KDE/KIcon>
#include <KDE/KActionMenu>
#include <KDE/KToggleAction>
#include <KDE/KStandardDirs>
#include <KDE/KMenu>
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>

#define CFG_GROUP                  "Main Settings"
#define CFG_PREVIEW_SPLITTER_SIZES "PreviewSplitterSizes"
#define CFG_GROUP_SPLITTER_SIZES   "GroupSplitterSizes"
#define CFG_FONT_SIZE              "FontSize"
#define CFG_FONT_MGT_MODE          "MgtMode"
#define CFG_SHOW_PREVIEW           "ShowPreview"

K_PLUGIN_FACTORY(FontInstallFactory,
        registerPlugin<KFI::CKCmFontInst>();
        )
K_EXPORT_PLUGIN(FontInstallFactory("fontinst"))

namespace KFI
{

static int constModeList[]=
    {CGroupListItem::ALL, CGroupListItem::PERSONAL, CGroupListItem::SYSTEM, -1};

inline CGroupListItem::EType modeToGrp(int mode)
{
    return (CGroupListItem::EType)(constModeList[mode]);
}

static int grpToMode(CGroupListItem::EType type)
{
    for(int i=0; i<3; ++i)
        if(constModeList[i]==type)
            return i;
    return 0;
}

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
        else if(text().isEmpty())
            sh.setWidth(theirHeight);
        return sh;
    }

    private:

    static int theirHeight;
};

class CToolBar : public KToolBar
{
    public:

    CToolBar(QWidget *parent)
        : KToolBar(parent)
    {
        theirHeight=qMax(theirHeight, height());
        setMovable(false);
        setFloatable(false);
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setFont(QApplication::font());
    }

    void addSeparator()
    {
        addWidget(new QLabel(" ", this));
    }

    void link(CToolBar *l)
    {
        itsLinked=l;
    }

    QSize sizeHint() const
    {
        QSize sh(KToolBar::sizeHint());

        sh.setHeight(theirHeight);
        return sh;
    }

    void paintEvent(QPaintEvent *)
    {
        QColor col(palette().color(backgroundRole()));

        col.setAlphaF(0.0);
        QPainter(this).fillRect(rect(), col);
    }

    void resizeEvent(QResizeEvent *ev)
    {
        KToolBar::resizeEvent(ev);
        if(height()>theirHeight)
        {
            theirHeight=height();
            itsLinked->resize(itsLinked->width(), height());
        }
    }

    private:

    CToolBar  *itsLinked;
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
int CToolBar::theirHeight=0;

CKCmFontInst::CKCmFontInst(QWidget *parent, const QVariantList&)
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
    setButtons(Help);

    CFcEngine::instance()->readConfig(itsConfig);
    CFcEngine::setBgndCol(QApplication::palette().color(QPalette::Active, QPalette::Base));
    CFcEngine::setTextCol(QApplication::palette().color(QPalette::Active, QPalette::Text));
    KGlobal::locale()->insertCatalog(KFI_CATALOGUE);
    KIconLoader::global()->addAppDir(KFI_NAME);

    KAboutData *about = new KAboutData("fontinst", 0, ki18n("KDE Font Installer"), 0, KLocalizedString(),
                                       KAboutData::License_GPL, ki18n("GUI front-end to the fonts:/ ioslave.\n"
                                                                      "(c) Craig Drummond, 2000 - 2007"));
    about->addAuthor(ki18n("Craig Drummond"), ki18n("Developer and maintainer"), "craig@kde.org");
    setAboutData(about);

    KConfigGroup cg(&itsConfig, CFG_GROUP);

    itsRunner=new CJobRunner(this);

    itsPreviewSplitter=new QSplitter(this);
    itsPreviewSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QWidget     *controls=new QWidget(itsPreviewSplitter);

    itsGroupSplitter=new QSplitter(controls);
    itsGroupSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QStringList items;
    QBoxLayout  *mainLayout=new QBoxLayout(QBoxLayout::TopToBottom, this),
                *controlLayout=new QBoxLayout(QBoxLayout::TopToBottom, controls);

    itsGroupsWidget=new QWidget(itsGroupSplitter);
    itsFontsWidget=new QWidget(itsGroupSplitter);
    itsPreviewWidget=new QWidget(itsPreviewSplitter);

    CToolBar    *toolbar=new CToolBar(this),
                *previewToolbar=new CToolBar(itsPreviewWidget);
    QGridLayout *groupsLayout=new QGridLayout(itsGroupsWidget),
                *fontsLayout=new QGridLayout(itsFontsWidget);
    QBoxLayout  *previewLayout=new QBoxLayout(QBoxLayout::TopToBottom, itsPreviewWidget);

    controlLayout->setMargin(0);
    controlLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    groupsLayout->setMargin(0);
    groupsLayout->setSpacing(KDialog::spacingHint());
    fontsLayout->setMargin(0);
    fontsLayout->setSpacing(KDialog::spacingHint());

    // Preview...
    itsPreviewControl=new CPreviewSelectAction(itsPreviewWidget);

    previewToolbar->addAction(itsPreviewControl);

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
    KAction     *changeTextAct=new KAction(KIcon("edit-rename"), i18n("Change Preview Text..."), this),
                *duplicateFontsAct=new KAction(KIcon("system-search"), i18n("Scan For Duplicate Fonts..."), this);
                //*validateFontsAct=new KAction(KIcon("checkmark"), i18n("Validate Fonts..."), this);
                //*downloadFontsAct=new KAction(KIcon("go-down"), i18n("Download Fonts..."), this);

    itsToolsMenu=new KActionMenu(KIcon("system-run"), i18n("Tools"), this);
    itsMgtMode=new KToggleAction(KIcon("preferences-desktop-font-installer"),
                                 i18n("Font Management Mode"), this),
    itsShowPreview=new KToggleAction(KIcon("view-preview"), i18n("Show Large Preview"), this);
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
    if(Misc::root())
        itsModeControl=NULL;
    else
    {
        itsModeControl=new QComboBox(toolbar);
        itsModeAct=toolbar->addWidget(itsModeControl);
        toolbar->addSeparator();
    }

    itsFilter=new CFontFilter(toolbar);
    toolbar->addWidget(itsFilter);

    // Details - Groups...
    itsGroupList=new CGroupList(itsGroupsWidget);
    itsGroupListView=new CGroupListView(itsGroupsWidget, itsGroupList);

    if(itsModeControl)
        for(int i=0; i<3; ++i)
            itsModeControl->addItem(itsGroupList->group(modeToGrp(i))->name());

    KPushButton *createGroup=new CPushButton(KGuiItem(QString(), "list-add",
                                                      i18n("Create a new group")),
                                             itsGroupsWidget);

    itsDeleteGroupControl=new CPushButton(KGuiItem(QString(), "edit-delete",
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

    itsAddFontControl=new CPushButton(KGuiItem(i18n("Add..."), "list-add",
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
    controlLayout->addWidget(toolbar);
    controlLayout->addWidget(itsGroupSplitter);
    mainLayout->addWidget(itsPreviewSplitter);
    mainLayout->addWidget(statusWidget);
    previewLayout->addWidget(previewToolbar);
    previewLayout->addWidget(previewFrame);

    // Set size of widgets...
    itsPreviewSplitter->setChildrenCollapsible(false);
    itsGroupSplitter->setChildrenCollapsible(false);
    itsGroupSplitter->setStretchFactor(0, 0);
    itsGroupSplitter->setStretchFactor(1, 1);
    itsPreviewSplitter->setStretchFactor(0, 1);
    itsPreviewSplitter->setStretchFactor(1, 0);

    toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    // Set sizes for 3 views...
    QList<int> defaultSizes;

    defaultSizes+=450;
    defaultSizes+=150;

    QList<int> sizes(cg.readEntry(CFG_PREVIEW_SPLITTER_SIZES, defaultSizes));

    if(2!=sizes.count())
        sizes=defaultSizes;

    itsPreviewSplitter->setSizes(sizes);

    defaultSizes=QList<int>();
    defaultSizes+=100;
    defaultSizes+=350;
    sizes=cg.readEntry(CFG_GROUP_SPLITTER_SIZES, defaultSizes);

    if(2!=sizes.count())
        sizes=defaultSizes;

    itsGroupSplitter->setSizes(sizes);

    // Connect signals...
    connect(itsFilter, SIGNAL(textChanged(const QString &)), itsFontListView, SLOT(filterText(const QString &)));
    connect(itsFilter, SIGNAL(criteriaChanged(int, qulonglong)), itsFontListView, SLOT(filterCriteria(int, qulonglong)));
    connect(itsGroupListView, SIGNAL(del()), SLOT(removeGroup()));
    connect(itsGroupListView, SIGNAL(print()), SLOT(printGroup()));
    connect(itsGroupListView, SIGNAL(enable()), SLOT(enableGroup()));
    connect(itsGroupListView, SIGNAL(disable()), SLOT(disableGroup()));
    connect(itsGroupListView, SIGNAL(copyFonts()), SLOT(copyFonts()));
    connect(itsGroupListView, SIGNAL(moveFonts()), SLOT(moveFonts()));
    connect(itsGroupListView, SIGNAL(itemSelected(const QModelIndex &)),
           SLOT(groupSelected(const QModelIndex &)));
    connect(itsGroupListView, SIGNAL(info(const QString &)),
           SLOT(showInfo(const QString &)));
    connect(itsGroupList, SIGNAL(refresh()), SLOT(refreshFontList()));
    connect(itsFontList, SIGNAL(started()), SLOT(listingStarted()));
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
        connect(itsModeControl, SIGNAL(activated(int)), SLOT(selectMode(int)));

    itsMgtMode->setChecked(cg.readEntry(CFG_FONT_MGT_MODE, false));
    itsShowPreview->setChecked(cg.readEntry(CFG_SHOW_PREVIEW, false));
    showPreview(itsShowPreview->isChecked());
    itsPreviewWidget->setVisible(itsShowPreview->isChecked());
    toggleFontManagement(itsMgtMode->isChecked());
    selectMainGroup();
    itsFontList->scan();

    toolbar->link(previewToolbar);
    previewToolbar->link(toolbar);
}

CKCmFontInst::~CKCmFontInst()
{
    KConfigGroup cg(&itsConfig, CFG_GROUP);

    cg.writeEntry(CFG_PREVIEW_SPLITTER_SIZES, itsPreviewSplitter->sizes());
    cg.writeEntry(CFG_GROUP_SPLITTER_SIZES, itsGroupSplitter->sizes());
    cg.writeEntry(CFG_FONT_MGT_MODE, itsMgtMode->isChecked());
    cg.writeEntry(CFG_SHOW_PREVIEW, itsShowPreview->isChecked());
    itsFontListView->writeConfig(cg);
    delete itsTempDir;
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
            if(itsShowPreview->isChecked())
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
        listingStarted();
        itsFontList->scan();
        itsGroupList->rescan();
    }
}

void CKCmFontInst::addFonts()
{
    if(!working())
    {
        QString filter("application/x-font-ttf application/x-font-otf "
                       "application/x-font-type1");

        if(itsMgtMode->isChecked() && FC::bitmapsEnabled())
            filter+=" application/x-font-pcf application/x-font-bdf";

        filter+=" fonts/package";

        KUrl::List list=KFileDialog::getOpenUrls(KUrl(), filter, this, i18n("Add Fonts"));

        if(list.count())
        {
            QSet<KUrl>           urls;
            KUrl::List::Iterator it(list.begin()),
                                 end(list.end());

            for(; it!=end; ++it)
            {
                if(KFI_KIO_FONTS_PROTOCOL!=(*it).protocol()) // Do not try to install from fonts:/ !!!
                {
                    KUrl url(KIO::NetAccess::mostLocalUrl(*it, this));

                    if(url.isLocalFile())
                    {
                        QString file(url.path());

                        if(Misc::isPackage(file)) // If its a package we need to unzip 1st...
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

                                                QString name(entry->name());

                                                //
                                                // Cant install hidden fonts, therefore need to
                                                // unhide 1st!
                                                if(Misc::isHidden(name))
                                                {
                                                    ::rename(QFile::encodeName(itsTempDir->name()+name).data(),
                                                             QFile::encodeName(itsTempDir->name()+name.mid(1)).data());
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
    if(grp && grp->isCustom() && !grp->validated())
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
    if(!working() && (!itsPrintProc || QProcess::NotRunning==itsPrintProc->state()))
    {
        QString exe(KStandardDirs::findExe(QLatin1String(KFI_PRINTER), KStandardDirs::installPath("libexec")));

        if(exe.isEmpty())
            KMessageBox::error(this, i18n("Failed to locate font printer."));
        else
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
                    bool                             useFile(fonts.count()>16),
                                                    startProc(true);
                    QStringList                      args;

                    if(!itsPrintProc)
                        itsPrintProc=new QProcess(this);
                    else
                        itsPrintProc->kill();

                    //
                    // If we have lots of fonts to print, pass kfontinst a tempory groups file to print
                    // instead of passing font by font...
                    if(useFile)
                    {
                        if(tmpFile.open())
                        {
                            QTextStream str(&tmpFile);

                            for(; it!=end; ++it)
                                str << (*it).family << endl
                                    << (*it).styleInfo << endl;

                            args << "--embed" << QString().sprintf("0x%x", (unsigned int)window()->winId())
                                << "--caption" << KGlobal::caption().toUtf8()
                                << "--icon" << "preferences-desktop-font-installer"
                                << "--size" << QString().setNum(constSizes[dlg.chosenSize() < 6 ? dlg.chosenSize() : 2])
                                << "--listfile" << tmpFile.fileName()
                                << "--deletefile";
                        }
                        else
                        {
                            KMessageBox::error(this, i18n("Failed to save list of fonts to print."));
                            startProc=false;
                        }
                    }
                    else
                    {
                        args << "--embed" << QString().sprintf("0x%x", (unsigned int)window()->winId())
                            << "--caption" << KGlobal::caption().toUtf8()
                            << "--icon" << "preferences-desktop-font-installer"
                            << "--size" << QString().setNum(constSizes[dlg.chosenSize()<6 ? dlg.chosenSize() : 2]);

                        for(; it!=end; ++it)
                            args << "--pfont" << QString((*it).family.toUtf8()+','+QString().setNum((*it).styleInfo));
                    }

                    if(startProc)
                    {
                        itsPrintProc->start(exe, args);

                        if(itsPrintProc->waitForStarted(1000))
                        {
                            if(useFile)
                                tmpFile.setAutoRemove(false);
                        }
                        else
                            KMessageBox::error(this, i18n("Failed to start font printer."));
                    }
                    cg.writeEntry(CFG_FONT_SIZE, dlg.chosenSize());
                }
            }
            else
                KMessageBox::information(this, i18n("There are no printable fonts.\n"
                                                    "You can only print non-bitmap and enabled fonts."),
                                         i18n("Cannot Print"));
        }
    }
}

void CKCmFontInst::deleteFonts()
{
    if(!working())
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
            bool                             doIt=false;

            for(; it!=end; ++it)
                itsDeletedFonts.insert((*it).family);

            switch(fontNames.count())
            {
                case 0:
                    break;
                case 1:
                    doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                            i18n("<p>Do you really want to "
                                    "delete</p><p>\'<b>%1</b>\'?</p>", fontNames.first()),
                            i18n("Delete Font"), KStandardGuiItem::del());
                break;
                default:
                    doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                            i18np("Do you really want to delete this font?",
                                  "Do you really want to delete these %1 fonts?",
                                    fontNames.count()),
                            fontNames, i18n("Delete Fonts"), KStandardGuiItem::del());
            }

            if(doIt && (!hasSys || itsRunner->getAdminPasswd(this)))
            {
                itsStatusLabel->setText(i18n("Deleting font(s)..."));
                doCmd(CJobRunner::CMD_DELETE, urls, KUrl());
            }
        }
    }
}

void CKCmFontInst::copyFonts()
{
    if(!working())
    {
        CJobRunner::ItemList urls;
        QStringList          fontNames;
        bool                 fromSys(itsGroupListView->isSystem());

        itsDeletedFonts.clear();
        itsFontListView->getFonts(urls, fontNames, NULL, NULL, true);

        if(urls.isEmpty())
            KMessageBox::information(this, i18n("You did not select anything to copy."),
                                     i18n("Nothing to Copy"));
        else
        {
            bool doIt=false;

            switch(fontNames.count())
            {
                case 0:
                    break;
                case 1:
                    doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                            i18n("<p>Do you really want to "
                                    "copy</p><p>\'<b>%1</b>\'?</p>", fontNames.first()),
                            i18n("Copy Font"), KGuiItem(i18n("Copy")));
                break;
                default:
                    doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                            i18np("Do you really want to copy this font?",
                                  "Do you really want to copy these %1 fonts?",
                                    fontNames.count()),
                            fontNames, i18n("Copy Fonts"), KGuiItem(i18n("Copy")));
            }

            if(doIt && (fromSys || itsRunner->getAdminPasswd(this)))
            {
                itsStatusLabel->setText(i18n("Copying font(s)..."));
                doCmd(CJobRunner::CMD_COPY, urls, baseUrl(!fromSys));
            }
        }
    }
}

void CKCmFontInst::moveFonts()
{
    if(!working())
    {
        CJobRunner::ItemList urls;
        QStringList          fontNames;

        itsDeletedFonts.clear();
        itsFontListView->getFonts(urls, fontNames, NULL, NULL, true);

        if(urls.isEmpty())
            KMessageBox::information(this, i18n("You did not select anything to move."),
                                     i18n("Nothing to Move"));
        else
        {
            bool doIt=false;

            switch(fontNames.count())
            {
                case 0:
                    break;
                case 1:
                    doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                            i18n("<p>Do you really want to "
                                    "move</p><p>\'<b>%1</b>\'?</p>", fontNames.first()),
                            i18n("Move Font"), KGuiItem(i18n("Move")));
                break;
                default:
                    doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                            i18np("Do you really want to move this font?",
                                  "Do you really want to move these %1 fonts?",
                                    fontNames.count()),
                            fontNames, i18n("Move Fonts"), KGuiItem(i18n("Move")));
            }

            if(doIt && itsRunner->getAdminPasswd(this))
            {
                itsStatusLabel->setText(i18n("Moving font(s)..."));
                doCmd(CJobRunner::CMD_MOVE, urls, baseUrl(!itsGroupListView->isSystem()));
            }
        }
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

void CKCmFontInst::changeText()
{
    if(!working())
    {
        bool             status;
        QRegExpValidator validator(QRegExp(".*"), 0L);
        QString          oldStr(CFcEngine::instance()->getPreviewString()),
                         newStr(KInputDialog::getText(i18n("Preview Text"),
                                                      i18n("Please enter new text:"),
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

void CKCmFontInst::listingStarted()
{
    showInfo(i18n("Scanning font list..."));
    itsListingProgress->show();
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

    QSet<QString> foundries;

    itsFontList->getFoundries(foundries);
    itsFilter->setFoundries(foundries);
    refreshFamilies();
    itsListingProgress->hide();
    itsFontListView->selectFirstFont();
}

void CKCmFontInst::refreshFontList()
{
    itsFontListView->refreshFilter();
    refreshFamilies();
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

void CKCmFontInst::showInfo(const QString &info)
{
    if(info.isEmpty())
        if(itsLastStatusBarMsg.isEmpty())
            setStatusBar();
        else
        {
            itsStatusLabel->setText(itsLastStatusBarMsg);
            itsLastStatusBarMsg=QString();
        }
    else
    {
        if(itsLastStatusBarMsg.isEmpty())
            itsLastStatusBarMsg=itsStatusLabel->text();
        itsStatusLabel->setText(info);
    }
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

    CGroupListItem::EType type(itsGroupListView->getType());

    bool isStd(CGroupListItem::CUSTOM==type);

    itsAddFontControl->setEnabled(CGroupListItem::ALL==type||CGroupListItem::UNCLASSIFIED==type||
                                  CGroupListItem::PERSONAL==type||CGroupListItem::SYSTEM==type);
    itsDeleteGroupControl->setEnabled(isStd);
    itsEnableGroupControl->setEnabled(disabled||partial);
    itsDisableGroupControl->setEnabled(isStd && (enabled||partial));

    itsGroupListView->controlMenu(itsDeleteGroupControl->isEnabled(),
                                  itsEnableGroupControl->isEnabled(),
                                  itsDisableGroupControl->isEnabled(), enabled||partial);

    itsEnableFontControl->setEnabled(selectedDisabled);
    itsDisableFontControl->setEnabled(selectedEnabled);
    itsDeleteFontControl->setEnabled(selectedEnabled||selectedDisabled);
}

void CKCmFontInst::addFonts(const QSet<KUrl> &src)
{
    if(!working() && src.count() && !itsGroupListView->isCustom())
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
                                "(only available to you), or "
                                "system-wide (available to all users)?"),
                           i18n("Where to Install"), KGuiItem(i18n(KFI_KIO_FONTS_USER)),
                                KGuiItem(i18n(KFI_KIO_FONTS_SYS))))
                    {
                        case KMessageBox::Yes:
                            dest=baseUrl(false);
                            break;
                        case KMessageBox::No:
                            if(itsRunner->getAdminPasswd(this))
                                dest=baseUrl(true);
                            else if(KMessageBox::Yes==KMessageBox::questionYesNo(this,
                                    i18n("Would you like to install the font(s) for personal use?"),
                                    i18n("Authorization Failed")))
                                dest=baseUrl(false);
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
                    else if(KMessageBox::Yes==KMessageBox::questionYesNo(this,
                            i18n("Would you like to install the font(s) for personal use?"),
                            i18n("Authorization Failed")))
                        dest=baseUrl(false);
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
                                            i18n("Looking for additional files to install..."));
            itsProgress->setModal(true);
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

            itsProgress->setLabelText(i18n("Looking for files associated with %1", (*it).prettyUrl()));
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
        itsPreviewControl->setMode(on ? CPreviewSelectAction::ScriptsOnly : CPreviewSelectAction::Basic);
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

void CKCmFontInst::selectMode(int mode)
{
    selectGroup(modeToGrp(mode));
}

void CKCmFontInst::selectGroup(CGroupListItem::EType grp)
{
    QModelIndex current(itsGroupListView->currentIndex());

    if(current.isValid())
    {
        CGroupListItem *grpItem=static_cast<CGroupListItem *>(current.internalPointer());

        if(grpItem && grp==grpItem->type())
            return;
        else
            itsGroupListView->selectionModel()->select(current,
                                                       QItemSelectionModel::Deselect);
    }

    QModelIndex idx(itsGroupList->index(grp));

    itsGroupListView->selectionModel()->select(idx, QItemSelectionModel::Select);
    itsGroupListView->setCurrentIndex(idx);
    groupSelected(idx);
    itsFontListView->refreshFilter();
    setStatusBar();
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
                       enable ? KGuiItem(i18n("Enable"), "enablefont", i18n("Enable Font"))
                              : KGuiItem(i18n("Disable"), "disablefont", i18n("Disable Font")));
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
                       enable ? KGuiItem(i18n("Enable"), "enablefont", i18n("Enable Fonts"))
                              : KGuiItem(i18n("Disable"), "disablefont", i18n("Disable Fonts")));
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
            KMessageBox::error(this, i18n("The installer is currently busy. Please wait for the "
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
    selectGroup(/*Misc::root() || itsMgtMode->isChecked()
                    ? */CGroupListItem::ALL/* : CGroupListItem::PERSONAL*/);
    if(itsModeControl)
        itsModeControl->setCurrentIndex(grpToMode(/*Misc::root()
                                                    ? */CGroupListItem::ALL
                                                    /*: CGroupListItem::PERSONAL*/));
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
    if(itsMgtMode->isChecked())
        return itsGroupListView->getType();
    else if(itsModeControl && itsModeControl->currentIndex() >= 0)
        return modeToGrp(itsModeControl->currentIndex());

    return CGroupListItem::ALL;
}

}

#include "KCmFontInst.moc"
