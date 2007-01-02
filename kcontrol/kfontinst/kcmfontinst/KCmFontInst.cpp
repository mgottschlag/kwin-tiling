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

#include "KCmFontInst.h"
#include "KfiConstants.h"
#include "PrintDialog.h"
#include "UpdateDialog.h"
#include "FcEngine.h"
#include "FontPreview.h"
#include "Misc.h"
#include "RenameJob.h"
#include "FontList.h"
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
#include <kstandardshortcut.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/jobuidelegate.h>
#include <kpassworddialog.h>
#include <kdesu/su.h>
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
#include <klineedit.h>

#define CFG_GROUP          "Main Settings"
#define CFG_SPLITTER_SIZES "SplitterSizes"
#define CFG_FONT_SIZE      "FontSize"
#define CFG_FONT_MGT_MODE  "MgtMode"

typedef KGenericFactory<KFI::CKCmFontInst, QWidget> FontInstallFactory;
K_EXPORT_COMPONENT_FACTORY(fontinst, FontInstallFactory("fontinst"))

namespace KFI
{

CKCmFontInst::CKCmFontInst(QWidget *parent, const QStringList&)
            : KCModule(FontInstallFactory::instance(), parent),
              itsModeControl(NULL),
              itsPreview(NULL),
              itsConfig(KFI_UI_CFG_FILE),
              itsJob(NULL),
              itsProgress(NULL),
              itsUpdateDialog(NULL),
              itsTempDir(NULL),
              itsMode(0),
              itsPrintProc(NULL),
              itsExportFile(NULL)
{
    if(!Misc::root())
        KPasswordDialog::disableCoreDumps();
    setButtons(0);

    CFcEngine::instance()->readConfig(itsConfig);
    CFcEngine::setBgndCol(QApplication::palette().color(QPalette::Active, QPalette::Base));
    CFcEngine::setTextCol(QApplication::palette().color(QPalette::Active, QPalette::Text));
    KGlobal::locale()->insertCatalog(KFI_CATALOGUE);
    kapp->iconLoader()->addAppDir(KFI_NAME);
    KAboutData* about = new KAboutData("fontinst",
         I18N_NOOP("KDE Font Installer"),
         0, 0,
         KAboutData::License_GPL,
         I18N_NOOP("GUI front end to the fonts:/ ioslave.\n"
         "(c) Craig Drummond, 2000 - 2006"));
    about->addAuthor("Craig Drummond", I18N_NOOP("Developer and maintainer"), "craig@kde.org");
    setAboutData(about);

    itsSplitter=new QSplitter(this);

    QWidget *detailsWidget=new QWidget(itsSplitter);

    itsStatusLabel = new QLabel(detailsWidget);
    itsStatusLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    itsLayout=new QGridLayout(detailsWidget);
    itsLayout->setMargin(0);
    itsLayout->setSpacing(KDialog::spacingHint());

    QStringList items;
    itsModeControl=new QComboBox(detailsWidget);

    if(Misc::root())
        itsModeControl->setEnabled(false);
    else
    {
        // TODO: Better text below!
        itsModeControl->setWhatsThis(i18n("<h3>Operational Category</h3><p>Fonts can be installed in two categories:</p>"
                                     "<ol><li><i>%1</i> The fonts will be accesible only to yourself.</li>"
                                     "<li><i>%2</i> The fonts will be available to all users.</li></ol>"
                                     "<p>Select the appropriate entry to control the category to which this installer "
                                     "will function.</p>", i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS)));
        items.append(i18n("Personal Fonts"));
    }
    items.append(i18n("System Fonts"));
    itsModeControl->addItems(items);
    itsModeControl->setCurrentItem(0);

    itsLayout->addWidget(itsModeControl, 0, 0);

    QWidget    *widget=new QWidget(detailsWidget);
    QBoxLayout *widgetLayout=new QBoxLayout(QBoxLayout::LeftToRight, widget);

    widgetLayout->setMargin(0);
    widgetLayout->setSpacing(KDialog::spacingHint());

    KLineEdit *lineed=new KLineEdit(widget);
    lineed->setClickMessage(i18n("Filter"));
    lineed->setClearButtonShown(true);
    lineed->setTrapReturnKey(true);
    itsMgtMode=new KPushButton(KGuiItem(QString(), "font_cfg_update1",
                                        i18n("Enable Font Management mode")), widget);
    itsMgtMode->setCheckable(true);
    itsMgtMode->setChecked(true);
    itsMgtMode->setWhatsThis(i18n("<p><h2>Font Management</h2></p>"
                                  "<p>Enabling this item will place the installer into "
                                  "its <i>Font Management</i> Mode. Using this mode will allow you to:"
                                  "<ul>"
                                  "<li>Enable/disable fonts. Disabled fonts will not appear in "
                                  "your applications but are not actually removed from your "
                                  " computer.</li>"
                                  "<li>Create font groups. This allows you to group fonts "
                                  "arbitrarily, these can then be enabled/disabled together.</li>"
                                  "</ul></p>"));
    widgetLayout->addWidget(itsMgtMode);
    widgetLayout->addWidget(lineed);

    itsLayout->addWidget(widget, 0, 1);

    itsGroupWidget=new QWidget(detailsWidget);
    QBoxLayout *groupLayout=new QBoxLayout(QBoxLayout::LeftToRight, itsGroupWidget);

    groupLayout->setMargin(0);
    groupLayout->setSpacing(KDialog::spacingHint());
    itsLayout->addWidget(itsGroupWidget, 2, 0);

    itsGroupList=new CGroupList(detailsWidget);
    itsGroupListView=new CGroupListView(detailsWidget, itsGroupList);

    itsGroupListView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    itsLayout->addWidget(itsGroupListView, 1, 0);

    KPushButton *createGroup=new KPushButton(KGuiItem(QString(), "filenew",
                                        i18n("Create a new group")),
                                        itsGroupWidget);
    createGroup->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //createGroup->setFlat(true);
    groupLayout->addWidget(createGroup);

    itsDeleteGroupControl=new KPushButton(KGuiItem(QString(), "remove",
                                         i18n("Remove group")), itsGroupWidget);
    itsDeleteGroupControl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //itsDeleteGroupControl->setFlat(true);
    groupLayout->addWidget(itsDeleteGroupControl);

    itsEnableGroupControl=new KPushButton(KGuiItem(QString(), "enablefont",
                                         i18n("Enable all disabled fonts in the current group")),
                                         itsGroupWidget);
    itsEnableGroupControl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //itsEnableGroupControl->setFlat(true);
    groupLayout->addWidget(itsEnableGroupControl);

    itsDisableGroupControl=new KPushButton(KGuiItem(QString(), "disablefont",
                                          i18n("Disable all enabled fonts in the current group")),
                                          itsGroupWidget);
    itsDisableGroupControl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //itsDisableGroupControl->setFlat(true);
    groupLayout->addWidget(itsDisableGroupControl);
    groupLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                         QSizePolicy::Fixed, QSizePolicy::Fixed));

    itsSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QWidget     *previewWidget=new QWidget(itsSplitter);
    QGridLayout *previewWidgetLayout=new QGridLayout(previewWidget);

    previewWidgetLayout->setMargin(0);
    previewWidgetLayout->setSpacing(KDialog::spacingHint());

    KPushButton *changeTextButton=new KPushButton(KGuiItem(QString(), "text",
                                                  i18n("Change text used for previews")), previewWidget);
    changeTextButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    itsPreviewType=new QComboBox(previewWidget);
    itsPreviewType->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    items.clear();
    items.append(i18n("Standard preview"));
    items.append(i18n("All characters"));

    itsPreviewType->addItems(items);
    itsPreviewType->setCurrentItem(0);

    QFrame     *previewFrame=new QFrame(previewWidget);
    QBoxLayout *previewLayout=new QBoxLayout(QBoxLayout::LeftToRight, previewFrame);

    previewLayout->setMargin(0);
    previewLayout->setSpacing(0);
    previewFrame->setFrameShape(QFrame::StyledPanel);
    previewFrame->setFrameShadow(QFrame::Sunken);
    previewFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    itsPreview=new CFontPreview(previewFrame);
    previewLayout->addWidget(itsPreview);

    previewWidgetLayout->addWidget(changeTextButton, 0, 0);
    previewWidgetLayout->addWidget(itsPreviewType, 0, 1);
    previewWidgetLayout->addWidget(previewFrame, 1, 0, 1, 2);

    itsSplitter->setStretchFactor(0, 1);
    itsSplitter->setStretchFactor(1, 100);
    itsSplitter->setStretchFactor(2, 1);

    itsConfig.setGroup(CFG_GROUP);

    QList<int> defaultSizes;

    defaultSizes+=350;
    defaultSizes+=250;

    QList<int> sizes(itsConfig.readEntry(CFG_SPLITTER_SIZES, defaultSizes));

    if(2!=sizes.count())
        sizes=defaultSizes;

    itsSplitter->setSizes(sizes);

    itsFontWidget=new QWidget(detailsWidget);
    QBoxLayout *fontLayout=new QBoxLayout(QBoxLayout::LeftToRight, itsFontWidget);

    fontLayout->setMargin(0);
    fontLayout->setSpacing(KDialog::spacingHint());
    itsLayout->addWidget(itsFontWidget, 2, 1);

    itsFontList=new CFontList(itsFontWidget);
    itsFontListView=new CFontListView(detailsWidget, itsFontList);
    itsFontListView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    itsFontListView->readConfig(itsConfig);

    itsLayout->addWidget(itsFontListView, 1, 1);

    itsAddFontControl=new KPushButton(KGuiItem(i18n("Add..."), "newfont",
                                         i18n("Install fonts")),
                                         itsFontWidget);
    itsAddFontControl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //itsAddFontControl->setFlat(true);
    fontLayout->addWidget(itsAddFontControl);

    itsDeleteFontControl=new KPushButton(KGuiItem(i18n("Delete..."), "editdelete",
                                          i18n("Delete all selected fonts")),
                                          itsFontWidget);
    itsDeleteFontControl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //itsDeleteFontControl->setFlat(true);
    fontLayout->addWidget(itsDeleteFontControl);

    itsEnableFontControl=new KPushButton(KGuiItem(i18n("Enable"), "enablefont",
                                         i18n("Enable all selected fonts")),
                                         itsFontWidget);
    itsEnableFontControl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //itsEnableFontControl->setFlat(true);
    fontLayout->addWidget(itsEnableFontControl);

    itsDisableFontControl=new KPushButton(KGuiItem(i18n("Disable"), "disablefont",
                                          i18n("Disable all selected fonts")),
                                          itsFontWidget);
    itsDisableFontControl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    //itsDisableFontControl->setFlat(true);
    fontLayout->addWidget(itsDisableFontControl);
    fontLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                        QSizePolicy::Expanding, QSizePolicy::Fixed));

    itsPreview->setWhatsThis(i18n("This displays a preview of the selected font."));

    QBoxLayout *layout=new QBoxLayout(QBoxLayout::TopToBottom, this);

    layout->addWidget(itsSplitter);
    itsLayout->addWidget(itsStatusLabel, 3, 0, 1, 2);
    itsEnableGroupControl->resize(QSize(itsEnableGroupControl->width(), itsAddFontControl->height()));
    if(!Misc::root())
        connect(itsModeControl, SIGNAL(activated(int)), SLOT(setMode(int)));
    connect(itsPreviewType, SIGNAL(activated(int)), SLOT(displayType(int)));
    connect(lineed, SIGNAL(textChanged(const QString &)), itsFontListView, SLOT(filterText(const QString &)));
    connect(itsGroupListView, SIGNAL(del()), SLOT(removeGroup()));
    connect(itsGroupListView, SIGNAL(print()), SLOT(printGroup()));
    connect(itsGroupListView, SIGNAL(enable()), SLOT(enableGroup()));
    connect(itsGroupListView, SIGNAL(disable()), SLOT(disableGroup()));
    connect(itsGroupListView, SIGNAL(exportGroup()), SLOT(exportGroup()));
    connect(itsGroupListView, SIGNAL(itemSelected(const QModelIndex &)),
           SLOT(groupSelected(const QModelIndex &)));
    connect(itsGroupListView, SIGNAL(addFonts(const QModelIndex &, const QList<Misc::TFont> &)),
            itsGroupList, SLOT(addToGroup(const QModelIndex &, const QList<Misc::TFont> &)));
    connect(itsGroupListView, SIGNAL(removeFonts(const QModelIndex &, const QList<Misc::TFont> &)),
            itsGroupList, SLOT(removeFromGroup(const QModelIndex &, const QList<Misc::TFont> &)));
    connect(itsGroupListView, SIGNAL(unclassifiedChanged()), SLOT(setStatusBar()));
    connect(itsGroupList, SIGNAL(refresh()), SLOT(setStatusBar()));
    connect(itsFontList, SIGNAL(finished()), SLOT(listingCompleted()));
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
    connect(itsGroupListView, SIGNAL(removeFonts(const QModelIndex &, const QList<Misc::TFont> &)),
            itsFontListView, SLOT(refreshFilter()));
    connect(createGroup, SIGNAL(clicked()), SLOT(addGroup()));
    connect(itsDeleteGroupControl, SIGNAL(clicked()), SLOT(removeGroup()));
    connect(itsEnableGroupControl, SIGNAL(clicked()), SLOT(enableGroup()));
    connect(itsDisableGroupControl, SIGNAL(clicked()), SLOT(disableGroup()));
    connect(itsAddFontControl, SIGNAL(clicked()), SLOT(addFonts()));
    connect(itsDeleteFontControl, SIGNAL(clicked()), SLOT(deleteFonts()));
    connect(itsEnableFontControl, SIGNAL(clicked()), SLOT(enableFonts()));
    connect(itsDisableFontControl, SIGNAL(clicked()), SLOT(disableFonts()));
    connect(itsMgtMode, SIGNAL(toggled(bool)), SLOT(toggleFontManagement(bool)));
    connect(changeTextButton, SIGNAL(clicked()), SLOT(changeText()));
    itsMgtMode->setChecked(itsConfig.readEntry(CFG_FONT_MGT_MODE, false));
    itsGroupList->setSysMode(Misc::root()||itsMode);
    itsFontList->setUrl(baseUrl());
    reload();
    selectAllGroup();
}

CKCmFontInst::~CKCmFontInst()
{
    itsConfig.setGroup(CFG_GROUP);
    itsConfig.writeEntry(CFG_SPLITTER_SIZES, itsSplitter->sizes());
    itsConfig.writeEntry(CFG_FONT_MGT_MODE, itsMgtMode->isChecked());
    itsFontListView->writeConfig(itsConfig);
    delete itsTempDir;
    delete itsPrintProc;
    delete itsExportFile;
}

void CKCmFontInst::setMode(int m)
{
    if(itsMode!=m)
        if(working(false))
        {
            if(itsModeControl)
                itsModeControl->setCurrentItem(itsMode);
        }
        else
        {
            itsMode=m;
            itsFontList->setUrl(baseUrl());
            itsGroupList->setSysMode(Misc::root()||itsMode);
            reload();
            selectAllGroup();
        }
}

void CKCmFontInst::displayType(int)
{
    switch(itsPreviewType->currentItem())
    {
        case 0:
            itsPreview->setUnicodeStart(CFcEngine::STD_PREVIEW);
            break;
        case 1:
            itsPreview->setUnicodeStart(CFcEngine::ALL_CHARS);
            break;
        default:
            itsPreview->setUnicodeStart((itsPreviewType->currentItem()-2)*256);
    }

    itsPreview->showFont();
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
        itsFontList->rescan();
        itsGroupList->rescan();
        itsStatusLabel->setText(i18n("Loading list of fonts..."));
    }
}

void CKCmFontInst::addFonts()
{
    if(!working())
    {
        QString filter("application/x-font-ttf application/x-font-otf "
                       "application/x-font-ttc application/x-font-type1");

        if(itsMgtMode->isChecked())
            filter+=" application/x-font-pcf application/x-font-bdf fonts/group";
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
                                                    itsTempDir=new KTempDir;
                                                    itsTempDir->setAutoRemove(true);
                                                }

                                                ((KArchiveFile *)entry)->copyTo(itsTempDir->name());

                                                if(group && KFI_FONT_INFO==entry->name())
                                                {
                                                    CFontGroups grp(itsTempDir->name()+entry->name(),
                                                                    true);

                                                    itsGroupList->merge(grp);
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
        CFontInfo::TFontList           remList;
        CFontInfo::TFontList::Iterator it((*(grp->item())).fonts.begin()),
                                       end((*(grp->item())).fonts.end());

        for(; it!=end; ++it)
            if(!itsFontList->hasFont(*it))
                remList.insert(*it);
        it=remList.begin();
        end=remList.end();
        for(; it!=end; ++it)
            itsGroupList->removeFontFromGroup(grp->item(), *it);
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

            itsConfig.setGroup(CFG_GROUP);
            if(dlg.exec(itsConfig.readEntry(CFG_FONT_SIZE, 1)))
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
                        CFontGroups                     grps(tmpFile.fileName(), true, false);
                        CFontInfo::TGroupList::Iterator grp(grps.create(KFI_PRINT_GROUP));

                        for(; it!=end; ++it)
                            grps.addTo(grp, (*it).family, (*it).styleInfo);

                        if(grps.save())
                        {
                            *itsPrintProc << "-p"
                                          << QString().sprintf("0x%x", (unsigned int)topLevelWidget()->winId())
                                          << QString().setNum(constSizes[dlg.chosenSize()<6 ? dlg.chosenSize() : 2])
                                          << tmpFile.fileName()
                                          << "y"; // y => implies kfontinst will remove our tmp xml file
                        }
                        else
                        {
                            KMessageBox::error(this, i18n("Failed to save list of fonts to print."));
                            startProc=false;
                        }
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
                itsConfig.writeEntry(CFG_FONT_SIZE, dlg.chosenSize());
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
    QStringList files;
    KUrl::List  urls;

    itsDeletedFonts.clear();
    itsFontListView->getFonts(urls, files, &itsDeletedFonts, true);

    if(urls.isEmpty())
        KMessageBox::information(this, i18n("You did not select anything to delete."),
                                       i18n("Nothing to Delete"));
    else
        deleteFonts(files, urls);
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
        selectAllGroup();
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
                QStringList files;
                KUrl::List  urls;

                itsFontListView->getFonts(urls, files, NULL, false, true, true);

                if(urls.count())
                {
                    QString name(grp->isStandard() ? grp->name()
                                                   : Misc::root()
                                                       ? i18n("%1 (Exported)", grp->name())
                                                       : itsMode
                                                            ? i18n("%1 (System, Exported)", grp->name())
                                                            : i18n("%1 (Personal, Exported)", grp->name()));
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
                            itsTempDir=new KTempDir;

                            if(itsTempDir && !itsTempDir->name().isEmpty())
                            {
                                itsTempDir->setAutoRemove(true);
                                itsExportFile=new KZip(file);

                                if(itsExportFile && itsExportFile->open(IO_WriteOnly))
                                {
                                    CFontGroups group(itsTempDir->name()+KFI_FONT_INFO, true, false);

                                    if(grp->isStandard())
                                        group.add(*(grp->item()));
                                    else
                                    {
                                        CFontInfo::TGroupList::Iterator entry=group.create(name);

                                        if(entry!=group.items().end())
                                        {
                                            QModelIndexList                all(itsFontListView->allFonts());
                                            QModelIndexList::ConstIterator it(all.begin()),
                                                                           end(all.end());

                                            for(; it!=end; ++it)
                                                if((*it).isValid())
                                                {
                                                    CFontItem *f(static_cast<CFontItem *>((*it).internalPointer()));

                                                    group.addTo(entry, CFontInfo::TFont(f->family(), f->styleInfo()));
                                                }
                                        }
                                    }

                                    if(group.save())
                                    {
                                        itsExportFile->addLocalFile(itsTempDir->name()+KFI_FONT_INFO,
                                                                    KFI_FONT_INFO);
                                        ::unlink(QFile::encodeName(itsTempDir->name()+KFI_FONT_INFO));
                                        itsJob=KIO::copy(urls, itsTempDir->name(), true);
                                        connect(itsJob, SIGNAL(result(KJob *)),
                                                SLOT(exportJobResult(KJob *)));
                                        connect(itsJob, SIGNAL(copyingDone(KIO::Job *, const KUrl &,
                                                               const KUrl &, bool, bool)),
                                                SLOT(exported(KIO::Job *, const KUrl &, const KUrl &,
                                                           bool, bool)));
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

// TODO: I assmume KIO::Job below should be KJob
void CKCmFontInst::exported(KIO::Job *, const KUrl &, const KUrl &to, bool, bool)
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
                    KTempDir    tmpDir;

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

void CKCmFontInst::print()
{
    print(false);
}

void CKCmFontInst::printGroup()
{
    print(true);
}

void CKCmFontInst::initialJobResult(KJob *job)
{
    bool ok=job && !job->error();

    if(ok)
    {
        //
        // Installation, deletion, enabling, disabling, completed - so now reconfgigure...
        QByteArray  packedArgs;
        QDataStream stream(&packedArgs, QIODevice::WriteOnly);

        stream << KFI::SPECIAL_CONFIGURE << baseUrl();

        itsJob=KIO::special(baseUrl(), packedArgs, false);
        setMetaData(itsJob);
        connect(itsJob, SIGNAL(result(KJob *)), SLOT(jobResult(KJob *)));
        connect(itsJob, SIGNAL(infoMessage(KJob *, const QString &)),
                SLOT(infoMessage(KJob *, const QString &)));
        itsJob->ui()->setWindow(this);
        //CPD TODO itsJob->setAutoErrorHandlingEnabled(true, this);

        if(!itsUpdateDialog)
            itsUpdateDialog=new CUpdateDialog(this);
        itsUpdateDialog->start();
        delete itsTempDir;
        itsTempDir=NULL;
    }
    else
        jobResult(job);
}

void CKCmFontInst::jobResult(KJob *job)
{
    bool ok=job && 0==job->error();

    if(itsUpdateDialog)
        itsUpdateDialog->stop();

    CFcEngine::instance()->setDirty();
    itsJob=NULL;

    setStatusBar();

    if(ok)
        KMessageBox::information(this,
                                 i18n("<p>Please note that any open applications will need to be "
                                      "restarted in order for any changes to be noticed.</p>"
                                      "<p>(You will also have to restart this application in order "
                                      "to use its print function on any newly installed fonts.)</p>"),
                                 i18n("Success"), "NewFontsAndOpenApps");
}

void CKCmFontInst::infoMessage(KJob *, const QString &msg)
{
    if(msg.isEmpty())
        itsStatusLabel->setText(itsLastStatusBarMsg);
    else
    {
        itsLastStatusBarMsg=itsStatusLabel->text();
        itsStatusLabel->setText(msg);
    }
}

void CKCmFontInst::listingCompleted()
{
    if(itsDeletedFonts.count())
    {
        QSet<Misc::TFont>::Iterator it(itsDeletedFonts.begin()),
                                    end(itsDeletedFonts.end());

        for(; it!=end; ++it)
            if(!itsFontList->hasFont(*it))
                itsGroupList->removeFont(*it);

        itsDeletedFonts.clear();
    }

    itsFontListView->selectFirstFont();
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

        QString text(i18np("1 Font", "%n Fonts", enabled+disabled+partial));

        if(disabled||partial)
            text+=i18n(" (%1 Enabled, %2 Disabled, %3 Partial)", enabled, disabled, partial);

        itsStatusLabel->setText(text);
    }

    bool isStd(itsGroupListView->isStandard());

    itsAddFontControl->setEnabled(!isStd);
    itsDeleteGroupControl->setEnabled(isStd);
    itsEnableGroupControl->setEnabled(disabled||partial);
    itsDisableGroupControl->setEnabled(isStd && (enabled||partial));

    itsGroupListView->controlMenu(itsDeleteGroupControl->isEnabled(), itsEnableGroupControl->isEnabled(),
                                  itsDisableGroupControl->isEnabled(), enabled||partial,
                                  enabled||partial||disabled);

    itsEnableFontControl->setEnabled(selectedDisabled);
    itsDisableFontControl->setEnabled(selectedEnabled);
    itsDeleteFontControl->setEnabled(selectedEnabled||selectedDisabled);
}

void CKCmFontInst::addFonts(const QSet<KUrl> &src)
{
    if(!working() && src.count() && getPasswd())
    {
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
                if(dialogVisible && !itsProgress->isVisible()) // Use closed dialog! re-open!!!
                    itsProgress->show();
            }

            Misc::getAssociatedUrls(*it, associatedUrls, false, this);
            copy.insert(*it);

            KUrl::List::Iterator aIt(associatedUrls.begin()),
                                 aEnd(associatedUrls.end());

            for(; aIt!=aEnd; ++aIt)
                copy.insert(*aIt);
        }
        itsProgress->close();

        KUrl::List installUrls;

        end=copy.end();
        for(it=copy.begin(); it!=end; ++it)
            installUrls.append(*it);

        itsStatusLabel->setText(i18n("Installing font(s)..."));
        itsJob=KIO::copy(installUrls, baseUrl(), true);
        setMetaData(itsJob);
        connect(itsJob, SIGNAL(result(KJob *)), SLOT(initialJobResult(KJob *)));
        connect(itsJob, SIGNAL(infoMessage(KJob *, const QString &)),
                SLOT(infoMessage(KJob *, const QString &)));
        itsJob->ui()->setWindow(this);
        //CPD TODO itsJob->setAutoErrorHandlingEnabled(true, this);
    }
}

void CKCmFontInst::toggleFontManagement(bool on)
{
    if(working())
        itsMgtMode->setChecked(!on);
    else
    {
        itsFontListView->setMgtMode(on);
        if(on)
        {
            if(2==itsPreviewType->count())
            {
                QStringList items;

                for(int i=0; i<256;++i)
                    items.append(i18n("Characters %1 to %2", i*256, (i*256)+255));
                itsPreviewType->addItems(items);
            }
            itsLayout->removeWidget(itsGroupListView);
            itsLayout->removeWidget(itsFontListView);
            itsLayout->addWidget(itsGroupListView, 1, 0);
            itsLayout->addWidget(itsGroupWidget, 2, 0);
            itsLayout->addWidget(itsFontListView, 1, 1);
            itsLayout->addWidget(itsFontWidget, 2, 1);
            itsGroupListView->show();
            itsGroupWidget->show();
            itsEnableFontControl->show();
            itsDisableFontControl->show();
        }
        else
        {
            if(itsPreviewType->count()>2)
            {
                if(itsPreviewType->currentItem()>1)
                {
                    itsPreviewType->setCurrentItem(0);
                    displayType(0);
                }

                for(int i=itsPreviewType->count()-1; i>1; --i)
                    itsPreviewType->removeItem(i);
            }

            selectAllGroup();

            itsLayout->removeWidget(itsGroupListView);
            itsLayout->removeWidget(itsFontListView);
            itsLayout->removeWidget(itsGroupWidget);
            itsLayout->removeWidget(itsFontWidget);
            itsLayout->addWidget(itsFontListView, 1, 0, 1, 2);
            itsLayout->addWidget(itsFontWidget, 2, 0, 1, 2);
            itsGroupListView->hide();
            itsGroupWidget->hide();
            itsEnableFontControl->hide();
            itsDisableFontControl->hide();
        }
        setStatusBar();
    }
}

void CKCmFontInst::deleteFonts(QStringList &files, KUrl::List &urls)
{
    if(!working() && files.count() && urls.count())
    {
        bool doIt=false;

        switch(files.count())
        {
            case 0:
                break;
            case 1:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                           i18n("<p>Do you really want to "
                                "delete</p></p>\'<b>%1</b>\'?</p>", files.first()),
                           i18n("Delete Font"), KStandardGuiItem::del());
            break;
            default:
                doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                           i18nc("translators: not called for n == 1",
                                "Do you really want to delete these %n fonts?",
                                files.count()),
                           files, i18n("Delete Fonts"), KStandardGuiItem::del());
        }

        if(doIt && getPasswd())
        {
            itsStatusLabel->setText(i18n("Deleting font(s)..."));
            itsJob=KIO::del(urls, false, true);
            setMetaData(itsJob);
            connect(itsJob, SIGNAL(result(KJob *)), SLOT(initialJobResult(KJob *)));
            connect(itsJob, SIGNAL(infoMessage(KJob *, const QString &)),
                    SLOT(infoMessage(KJob *, const QString &)));
            itsJob->ui()->setWindow(this);
            //CPD TODO itsJob->setAutoErrorHandlingEnabled(true, this);
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
        QStringList files;
        KUrl::List  urls;

        itsFontListView->getFonts(urls, files, NULL, grp.isEmpty(), !enable, enable);

        if(urls.isEmpty())
            KMessageBox::information(this,
                                     enable ? i18n("You did not select anything to enable.")
                                            : i18n("You did not select anything to disable."),
                                     enable ? i18n("Nothing to Enable") : i18n("Nothing to Disable"));
        else
            toggleFonts(files, urls, enable, grp);
    }
}

void CKCmFontInst::toggleFonts(QStringList &files, KUrl::List &urls, bool enable, const QString &grp)
{
    bool doIt=false;

    switch(files.count())
    {
        case 0:
            break;
        case 1:
            doIt = KMessageBox::Yes==KMessageBox::warningYesNo(this,
                       grp.isEmpty()
                            ? enable ? i18n("<p>Do you really want to "
                                            "enable</p><p>\'<b>%1</b>\'?</p>", files.first())
                                     : i18n("<p>Do you really want to "
                                            "disable</p><p>\'<b>%1</b>\'?</p>", files.first())
                            : enable ? i18n("<p>Do you really want to "
                                            "enable</p><p>\'<b>%1</b>\', "
                                            "contained within group \'<b>%2</b>\'?</p>",
                                            files.first(), grp)
                                     : i18n("<p>Do you really want to "
                                            "disable</p><p>\'<b>%1</b>\', "
                                            "contained within group \'<b>%2</b>\'?</p>",
                                            files.first(), grp),
                       enable ? i18n("Enable Font") : i18n("Disable Font"),
                       enable ? KGuiItem(i18n("Enable"), "enablefont", i18n("Enable font"))
                              : KGuiItem(i18n("Disable"), "disablefont", i18n("Disable font")));
            break;
        default:
            doIt = KMessageBox::Yes==KMessageBox::warningYesNoList(this,
                       grp.isEmpty()
                            ? enable ? i18nc("translators: not called for n == 1",
                                             "Do you really want to enable these %n fonts?",
                                             files.count())
                                     : i18nc("translators: not called for n == 1",
                                             "Do you really want to disable these %n fonts?",
                                             files.count())
                            : enable ? i18nc("translators: not called for n == 1",
                                             "<p>Do you really want to enable these %n fonts "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             files.count(), grp)
                                     : i18nc("translators: not called for n == 1",
                                             "<p>Do you really want to disable these %n fonts "
                                             "contained within group \'<b>%2</b>\'?</p>",
                                             files.count(), grp),
                       files,
                       enable ? i18n("Enable Fonts") : i18n("Disable Fonts"),
                       enable ? KGuiItem(i18n("Enable"), "enablefont", i18n("Enable fonts"))
                              : KGuiItem(i18n("Disable"), "disablefont", i18n("Disable fonts")));
    }

    if(doIt && getPasswd())
    {
        KUrl::List::Iterator    it;
        KUrl::List              copy(urls);
        CRenameJob::Entry::List renameList;

        for(it=copy.begin(); it!=copy.end(); ++it)
        {
            KUrl url(*it);

            url.setFileName(enable
                         ? Misc::getFile((*it).path()).mid(1)
                         : QChar('.')+Misc::getFile((*it).path()));
            renameList.append(CRenameJob::Entry(*it, url));
        }

        if(enable)
            itsStatusLabel->setText(i18n("Enabling font(s)..."));
        else
            itsStatusLabel->setText(i18n("Disabling font(s)..."));

        itsJob=new CRenameJob(renameList, true);
        setMetaData(itsJob);
        connect(itsJob, SIGNAL(result(KJob *)), SLOT(initialJobResult(KJob *)));
        connect(itsJob, SIGNAL(infoMessage(KJob *, const QString &)),
                SLOT(infoMessage(KJob *, const QString &)));
        itsJob->ui()->setWindow(this);
        //CPD TODO  itsJob->setAutoErrorHandlingEnabled(true, this);
    }
}

bool CKCmFontInst::working(bool displayMsg)
{
    if(NULL!=itsJob || itsFontList->active())
    {
        if(displayMsg)
            KMessageBox::error(this, i18n("Sorry, the installer is currently busy. Please wait for the "
                                          "current operation to terminate."));
        return true;
    }

    return false;
}

KUrl CKCmFontInst::baseUrl()
{
    if(Misc::root())
        return KUrl(KFI_KIO_FONTS_PROTOCOL":/");
    else if(itsMode)
        return KUrl(KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS"/");
    else
        return KUrl(KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_USER"/");
}

void CKCmFontInst::selectAllGroup()
{
    QModelIndex current(itsGroupListView->currentIndex());

    if(current.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(current.internalPointer());

        if(grp && CGroupListItem::ALL==grp->type())
            return;
        else
            itsGroupListView->selectionModel()->select(current,
                                                       QItemSelectionModel::Deselect);
    }

    itsGroupListView->selectionModel()->select(itsGroupList->allIndex(),
                                               QItemSelectionModel::Select);
    itsGroupListView->setCurrentIndex(itsGroupList->allIndex());
}

bool CKCmFontInst::getPasswd()
{
    if(!Misc::root() && itsMode)
    {
        // Prompt user for password, if dont already have...
        if(itsPasswd.isEmpty())
        {
            QByteArray passwd;
            SuProcess  proc(KFI_SYS_USER);
            int        attempts(0);

            do
            {
                if(QDialog::Rejected==KPasswordDialog::getPassword(this, passwd,
                                                                   i18n("Authorisation Required"),
                                                                   i18n("The requested action "
                                                                        "requires administrator "
                                                                        "privilleges.\nIf you have "
                                                                        "these privilleges, then "
                                                                        "please enter your password. "
                                                                        "Otherwise enter the system "
                                                                        "administrator's password."), NULL))
                    return false;

                if(0==proc.checkInstall(passwd))
                {
                    itsPasswd=passwd;
                    break;
                }
                if(KMessageBox::No==KMessageBox::warningYesNo(this,
                                                i18n("<p><b>Incorrect password.</b></p><p>Try again?</p>")))
                    return false;
                if(++attempts>4)
                    return false;
            }
            while(itsPasswd.isEmpty());

            // TODO: If keep, then need to store password into kwallet!
        }
    }

    return true;
}

//
// Tell the io-slave not to clear, and re-read, the list of fonts. And also, tell it to not
// automatically recreate the config files - we want that to happen after all fonts are installed,
// deleted, etc.
void CKCmFontInst::setMetaData(KIO::Job *job)
{
    job->addMetaData(KFI_KIO_TIMEOUT, "0");
    job->addMetaData(KFI_KIO_NO_CLEAR, "1");

    if(!Misc::root() && itsMode && !itsPasswd.isEmpty())
        job->addMetaData(KFI_KIO_PASS, itsPasswd);
}

}

#include "KCmFontInst.moc"
