// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//
// Converted to a kcc module by Matthias Hoelzer 1997
// Ported to Qt-2.0 by Matthias Ettrich 1999
// Ported to kcontrol2 by Geert Jansen 1999
// Made maintainable by Waldo Bastian 2000

#include <assert.h>
#include <config.h>
#include <stdlib.h>
#include <unistd.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QSlider>
#include <QSplitter>
#include <q3groupbox.h>
#include <Q3PtrList>

#include <kcolorbutton.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kglobalsettings.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <k3process.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <klistwidget.h>
#include <kvbox.h>
#include "../../../runtime/kcontrol/krdb/krdb.h"

#include "colorscm.h"
#include "kcolortreewidget.h"
#include <QX11Info>

#if defined Q_WS_X11 && !defined K_WS_QTONLY
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

/**** DLL Interface ****/
typedef KGenericFactory<KColorScheme , QWidget> KolorFactory;
K_EXPORT_COMPONENT_FACTORY( colors, KolorFactory("kcmcolors") )

class KColorSchemeEntry {
public:
    KColorSchemeEntry(const QString &_path, const QString &_name, bool _local)
    	: path(_path), name(_name), local(_local) { }

    QString path;
    QString name;
    bool local;
};

class KColorSchemeList : public Q3PtrList<KColorSchemeEntry> {
public:
    KColorSchemeList()
        { setAutoDelete(true); }

    int compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2)
        {
           KColorSchemeEntry *i1 = (KColorSchemeEntry*)item1;
           KColorSchemeEntry *i2 = (KColorSchemeEntry*)item2;
           if (i1->local != i2->local)
              return i1->local ? -1 : 1;
           return i1->name.localeAwareCompare(i2->name);
        }
};

#define SIZE 8

// make a 24 * 8 pixmap with the main colors in a scheme
QPixmap mkColorPreview(const WidgetCanvas *cs)
{
   QPixmap group(SIZE*3,SIZE);
   QPixmap block(SIZE,SIZE);
   group.fill(QColor(0,0,0));
   block.fill(cs->back);   bitBlt(&group,0*SIZE,0,&block,0,0,SIZE,SIZE);
   block.fill(cs->window); bitBlt(&group,1*SIZE,0,&block,0,0,SIZE,SIZE);
   block.fill(cs->aTitle); bitBlt(&group,2*SIZE,0,&block,0,0,SIZE,SIZE);
   QPainter p(&group);
   p.drawRect(0,0,3*SIZE,SIZE);
   return group;
}

/**** KColorScheme ****/

KColorScheme::KColorScheme(QWidget *parent, const QStringList &)
    : KCModule(KolorFactory::componentData(), parent)
{
    nSysSchemes = 2;

    setQuickHelp( i18n("<h1>Colors</h1> This module allows you to choose"
       " the color scheme used for the KDE desktop. The different"
       " elements of the desktop, such as title bars, menu text, etc.,"
       " are called \"widgets\". You can choose the widget whose"
       " color you want to change by selecting it from a list, or by"
       " clicking on a graphical representation of the desktop.<p>"
       " You can save color settings as complete color schemes,"
       " which can also be modified or deleted. KDE comes with several"
       " predefined color schemes on which you can base your own.<p>"
       " All KDE applications will obey the selected color scheme."
       " Non-KDE applications may also obey some or all of the color"
       " settings, if this option is enabled."));

    KConfigGroup cfg(KSharedConfig::openConfig("kcmdisplayrc"), "X11");
    useRM = cfg.readEntry("useResourceManager", true);

    cs = new WidgetCanvas( this );
    cs->setCursor( KCursor::handCursor() );


    cs->setFixedHeight(160);
    cs->setMinimumWidth(440);
    cs->setWhatsThis( i18n("This is a preview of the color settings which"
       " will be applied if you click \"Apply\" or \"OK\". You can click on"
       " different parts of this preview image. The widget name in the"
       " \"Widget color\" box will change to reflect the part of the preview"
       " image you clicked.") );

    connect( cs, SIGNAL( colorDropped( int, const QColor&)),
         SLOT( slotColorForWidget( int, const QColor&)));

    QSplitter *splitter = new QSplitter(this);

    Q3GroupBox *group = new Q3GroupBox( i18n("Color Scheme"), splitter );
    group->setOrientation( Qt::Horizontal );
    group->setColumns( 1 );


    sList = new KListWidget( group );
    mSchemeList = new KColorSchemeList();
    connect(sList, SIGNAL(highlighted(int)), SLOT(slotPreviewScheme(int)));

    sList->setWhatsThis( i18n("This is a list of predefined color schemes,"
       " including any that you may have created. You can preview an existing"
       " color scheme by selecting it from the list. The current scheme will"
       " be replaced by the selected color scheme.<p>"
       " Warning: if you have not yet applied any changes you may have made"
       " to the current scheme, those changes will be lost if you select"
       " another color scheme.") );

    addBt = new QPushButton(i18n("&Save Scheme..."), group);
    connect(addBt, SIGNAL(clicked()), SLOT(slotAdd()));

    addBt->setWhatsThis( i18n("Press this button if you want to save"
       " the current color settings as a color scheme. You will be"
       " prompted for a name.") );

    removeBt = new QPushButton(i18n("R&emove Scheme"), group);
    removeBt->setEnabled(false);
    connect(removeBt, SIGNAL(clicked()), SLOT(slotRemove()));

    removeBt->setWhatsThis( i18n("Press this button to remove the selected"
       " color scheme. Note that this button is disabled if you do not have"
       " permission to delete the color scheme.") );

	importBt = new QPushButton(i18n("I&mport Scheme..."), group);
	connect(importBt, SIGNAL(clicked()),SLOT(slotImport()));

	importBt->setWhatsThis( i18n("Press this button to import a new color"
		" scheme. Note that the color scheme will only be available for the"
		" current user." ));



    KVBox *vb=new KVBox(splitter);

        // LAYOUT
    splitter->addWidget(group);
    splitter->addWidget(vb);
    QVBoxLayout *toplayout = new QVBoxLayout(this);
    toplayout->addWidget(cs);
    toplayout->addWidget(splitter);
    setLayout(toplayout);

    mColorTreeWidget = new KColorTreeWidget(vb);
    connect(mColorTreeWidget , SIGNAL(colorChanged(int, const QColor& )) ,
            this , SLOT(slotColorChanged( int, const QColor& ) ));

    setColorName(i18n("Inactive Title Bar") , CSM_Inactive_title_text , CSM_Inactive_title_bar);
    setColorName(i18n("Inactive Title Blend"), -1 , CSM_Inactive_title_blend);
    setColorName(i18n("Active Title Bar"), CSM_Active_title_text, CSM_Active_title_bar);
    setColorName(i18n("Active Title Blend"), -1, CSM_Active_title_blend);
    setColorName(i18n("Window"), CSM_Text , CSM_Background);
    setColorName(i18n("Selected"), CSM_Select_text , CSM_Select_background);
    setColorName(i18n("Standard"), CSM_Standard_text,  CSM_Standard_background);
    setColorName(i18n("Button"), CSM_Button_text , CSM_Button_background);
    setColorName(i18n("Active Title Button"), CSM_Active_title_button , -1 );
    setColorName(i18n("Inactive Title Button"), CSM_Inactive_title_button , -1);
    setColorName(i18n("Active Window Frame"), CSM_Active_frame , -1);
    setColorName(i18n("Active Window Handle"), CSM_Active_handle , -1);
    setColorName(i18n("Inactive Window Frame"), CSM_Inactive_frame , -1);
    setColorName(i18n("Inactive Window Handle"), CSM_Inactive_handle , -1);
    setColorName(i18n("Link"), CSM_Link , -1);
    setColorName(i18n("Followed Link"), CSM_Followed_Link , -1 );
    setColorName(i18n("Alternate Background in Lists"), -1 , CSM_Alternate_background);


    cbShadeList = new QCheckBox(i18n("Shade sorted column in lists"), vb);
    connect(cbShadeList, SIGNAL(toggled(bool)), this, SLOT(slotShadeSortColumnChanged(bool)));

    cbShadeList->setWhatsThis(
       i18n("Check this box to show the sorted column in a list with a shaded background"));

    group = new Q3GroupBox(  i18n("Con&trast"), vb );

    QVBoxLayout *groupLayout2 = new QVBoxLayout(group);
    QHBoxLayout *groupLayout = new QHBoxLayout;
    groupLayout2->addLayout(groupLayout);

    sb = new QSlider( Qt::Horizontal,group,"Slider" );
    sb->setRange( 0, 10 );
    sb->setFocusPolicy( Qt::StrongFocus );
    connect(sb, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)));

    sb->setWhatsThis( i18n("Use this slider to change the contrast level"
       " of the current color scheme. Contrast does not affect all of the"
       " colors, only the edges of 3D objects."));

    QLabel *label = new QLabel(i18nc("Low Contrast", "Low"), group);
    label->setBuddy( sb );
    groupLayout->addWidget(label);
    groupLayout->addWidget(sb, 10);
    label = new QLabel(group);
    label->setText(i18nc("High Contrast", "High"));
    groupLayout->addWidget( label );

    cbExportColors = new QCheckBox(i18n("Apply colors to &non-KDE applications"), this);
    toplayout->addWidget( cbExportColors );
    connect(cbExportColors, SIGNAL(toggled(bool)), this, SLOT(changed()));

    cbExportColors->setWhatsThis( i18n("Check this box to apply the"
       " current color scheme to non-KDE applications."));

    readSchemeNames();
    sList->setCurrentItem( 0 );

    load();

    KAboutData* about = new KAboutData("kcmcolors", I18N_NOOP("Colors"), 0, 0,
        KAboutData::License_GPL,
        I18N_NOOP("(c) 1997-2005 Colors Developers"), 0, 0);
    about->addAuthor("Mark Donohoe", 0, 0);
    about->addAuthor("Matthias Hoelzer", 0, 0);
    about->addAuthor("Matthias Ettrich", 0, 0);
    about->addAuthor("Geert Jansen", 0, 0);
    about->addAuthor("Waldo Bastian", 0, 0);
    setAboutData( about );
}


KColorScheme::~KColorScheme()
{
    delete mSchemeList;
}


void KColorScheme::load()
{
    KConfigGroup config(KGlobal::config(), "KDE");
    sCurrentScheme = config.readEntry("colorScheme");

    sList->setCurrentRow(findSchemeByName(sCurrentScheme));
    readScheme(0);

    cbShadeList->setChecked(cs->shadeSortColumn);

    cs->drawSampleWidgets();
    sb->blockSignals(true);
    sb->setValue(cs->contrast);
    sb->blockSignals(false);

    KConfig _cfg( "kcmdisplayrc", KConfig::NoGlobals  );
    KConfigGroup cfg(&_cfg, "X11");
    bool exportColors = cfg.readEntry("exportKDEColors", true);
    cbExportColors->setChecked(exportColors);

    emit changed(false);
}


void KColorScheme::save()
{
    KConfigGroup cfg(KGlobal::config(), "General");
    cfg.writeEntry("background", cs->back, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("selectBackground", cs->select, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("foreground", cs->txt, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("windowForeground", cs->windowTxt, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("windowBackground", cs->window, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("selectForeground", cs->selectTxt, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("buttonBackground", cs->button, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("buttonForeground", cs->buttonTxt, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("linkColor", cs->link, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("visitedLinkColor", cs->visitedLink, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("alternateBackground", cs->alternateBackground, KConfigBase::Normal|KConfigBase::Global);

    cfg.writeEntry("shadeSortColumn", cs->shadeSortColumn, KConfigBase::Normal|KConfigBase::Global);

    cfg.changeGroup( "WM" );
    cfg.writeEntry("activeForeground", cs->aTxt, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("inactiveBackground", cs->iaTitle, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("inactiveBlend", cs->iaBlend, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("activeBackground", cs->aTitle, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("activeBlend", cs->aBlend, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("inactiveForeground", cs->iaTxt, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("activeTitleBtnBg", cs->aTitleBtn, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("inactiveTitleBtnBg", cs->iTitleBtn, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("frame", cs->aFrame, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("inactiveFrame", cs->iaFrame, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("handle", cs->aHandle, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("inactiveHandle", cs->iaHandle, KConfigBase::Normal|KConfigBase::Global);

    cfg.changeGroup( "KDE" );
    cfg.writeEntry("contrast", cs->contrast, KConfigBase::Normal|KConfigBase::Global);
    cfg.writeEntry("colorScheme", sCurrentScheme, KConfigBase::Normal|KConfigBase::Global);
    cfg.sync();

    // KDE-1.x support
    KConfigGroup config(KSharedConfig::openConfig( QDir::homePath() + "/.kderc" ), "General" );
    config.writeEntry("background", cs->back );
    config.writeEntry("selectBackground", cs->select );
    config.writeEntry("foreground", cs->txt, KConfigBase::Normal|KConfigBase::Global);
    config.writeEntry("windowForeground", cs->windowTxt );
    config.writeEntry("windowBackground", cs->window );
    config.writeEntry("selectForeground", cs->selectTxt );
    config.sync();

    KConfig _cfg2("kcmdisplayrc", KConfig::NoGlobals);
    KConfigGroup cfg2(&_cfg2, "X11");
    bool exportColors = cbExportColors->isChecked();
    cfg2.writeEntry("exportKDEColors", exportColors);
    cfg2.sync();
    QApplication::syncX();

    // Notify all qt-only apps of the KDE palette changes
    uint flags = KRdbExportQtColors;
    if ( exportColors )
        flags |= KRdbExportColors;
    else
    {
#if defined Q_WS_X11 && !defined K_WS_QTONLY
        // Undo the property xrdb has placed on the root window (if any),
        // i.e. remove all entries, including ours
        XDeleteProperty( QX11Info::display(), QX11Info::appRootWindow(), XA_RESOURCE_MANAGER );
#endif
    }
    runRdb( flags );	// Save the palette to qtrc for KStyles

    // Notify all KDE applications
    KGlobalSettings::self()->emitChange(KGlobalSettings::PaletteChanged);

    // Update the "Current Scheme"
    int current = findSchemeByName(sCurrentScheme);
    sList->setCurrentItem(0);
    readScheme(0);
    QPixmap preview = mkColorPreview(cs);
    sList->item(0)->setIcon(preview);
    sList->setCurrentRow(current);
    readScheme(current);
    preview = mkColorPreview(cs);
    sList->item(current)->setIcon(preview);

    emit changed(false);
}


void KColorScheme::defaults()
{
    readScheme(1);
    sList->setCurrentRow(1);

    cbShadeList->setChecked(cs->shadeSortColumn);

    cs->drawSampleWidgets();
    sb->blockSignals(true);
    sb->setValue(cs->contrast);
    sb->blockSignals(false);

    cbExportColors->setChecked(true);

    emit changed(true);
}

void KColorScheme::sliderValueChanged( int val )
{
    cs->contrast = val;
    cs->drawSampleWidgets();

    sCurrentScheme.clear();

    emit changed(true);
}


void KColorScheme::slotSave( )
{
    KColorSchemeEntry *entry = mSchemeList->at(sList->currentRow()-nSysSchemes);
    if (!entry) return;
    sCurrentScheme = entry->path;
    KConfig *config = new KConfig(sCurrentScheme, KConfig::OnlyLocal);
    int i = sCurrentScheme.lastIndexOf('/');
    if (i >= 0)
      sCurrentScheme = sCurrentScheme.mid(i+1);

    config->setGroup("Color Scheme" );
    config->writeEntry("background", cs->back );
    config->writeEntry("selectBackground", cs->select );
    config->writeEntry("foreground", cs->txt );
    config->writeEntry("activeForeground", cs->aTxt );
    config->writeEntry("inactiveBackground", cs->iaTitle );
    config->writeEntry("inactiveBlend", cs->iaBlend );
    config->writeEntry("activeBackground", cs->aTitle );
    config->writeEntry("activeBlend", cs->aBlend );
    config->writeEntry("inactiveForeground", cs->iaTxt );
    config->writeEntry("windowForeground", cs->windowTxt );
    config->writeEntry("windowBackground", cs->window );
    config->writeEntry("selectForeground", cs->selectTxt );
    config->writeEntry("contrast", cs->contrast );
    config->writeEntry("buttonForeground", cs->buttonTxt );
    config->writeEntry("buttonBackground", cs->button );
    config->writeEntry("activeTitleBtnBg", cs->aTitleBtn);
    config->writeEntry("inactiveTitleBtnBg", cs->iTitleBtn);
    config->writeEntry("frame", cs->aFrame);
    config->writeEntry("inactiveFrame", cs->iaFrame);
    config->writeEntry("handle", cs->aHandle);
    config->writeEntry("inactiveHandle", cs->iaHandle);
    config->writeEntry("linkColor", cs->link);
    config->writeEntry("visitedLinkColor", cs->visitedLink);
    config->writeEntry("alternateBackground", cs->alternateBackground);
    config->writeEntry("shadeSortColumn", cs->shadeSortColumn);

    delete config;
}


void KColorScheme::slotRemove()
{
    int ind = sList->currentRow();
    KColorSchemeEntry *entry = mSchemeList->at(ind-nSysSchemes);
    if (!entry) return;

    if (unlink(QFile::encodeName(entry->path).data())) {
        KMessageBox::error( 0,
          i18n("This color scheme could not be removed.\n"
           "Perhaps you do not have permission to alter the file"
           "system where the color scheme is stored." ));
        return;
    }

    delete sList->takeItem(ind);
    mSchemeList->remove(entry);

    ind = sList->currentRow();
    entry = mSchemeList->at(ind-nSysSchemes);
    if (!entry) return;
    removeBt->setEnabled(entry ? entry->local : false);
}


/*
 * Add a local color scheme.
 */
void KColorScheme::slotAdd()
{
    QString sName;
    if (sList->currentRow() >= nSysSchemes)
       sName = sList->currentItem()->text();

    QString sFile;

    bool valid = false;
    bool ok;
    int exists = -1;

    while (!valid)
    {
        sName = KInputDialog::getText( i18n( "Save Color Scheme" ),
            i18n( "Enter a name for the color scheme:" ), sName, &ok, this );
        if (!ok)
            return;

        sName = sName.simplified();
        sFile = sName;

        int i = 0;

        exists = -1;
        // Check if it's already there
        for (i=0; i < (int) sList->count(); i++)
        {
            if (sName == sList->item(i)->text())
            {
                exists = i;
                int result = KMessageBox::warningContinueCancel( this,
                   i18n("A color scheme with the name '%1' already exists.\n"
                        "Do you want to overwrite it?\n", sName),
                   i18n("Save Color Scheme"),
                   KGuiItem(i18n("Overwrite")));
                if (result == KMessageBox::Cancel)
                    break;
            }
        }
        if (i == (int) sList->count())
            valid = true;
    }

    disconnect(sList, SIGNAL(highlighted(int)), this,
               SLOT(slotPreviewScheme(int)));

    if (exists != -1)
    {
       sList->setFocus();
       sList->setCurrentRow(exists);
    }
    else
    {
       sFile = KGlobal::dirs()->saveLocation("data", "kdisplay/color-schemes/") + sFile + ".kcsrc";
       KConfigGroup config(KSharedConfig::openConfig(sFile, KConfig::OnlyLocal), "Color Scheme");
       config.writeEntry("Name", sName);

       insertEntry(sFile, sName);

    }
    slotSave();

    QPixmap preview = mkColorPreview(cs);
    int current = sList->currentRow();
    sList->item(current)->setIcon(preview);
    connect(sList, SIGNAL(highlighted(int)), SLOT(slotPreviewScheme(int)));
    slotPreviewScheme(current);
}

void KColorScheme::slotImport()
{
	QString location = KStandardDirs::locateLocal( "data", "kdisplay/color-schemes/" );

	KUrl file ( KFileDialog::getOpenFileName(QString(), "*.kcsrc", this) );
	if ( file.isEmpty() )
		return;

	//kDebug() << "Location: " << location << endl;
	if (!KIO::NetAccess::file_copy(file, KUrl( location+file.fileName( KUrl::ObeyTrailingSlash ) ) ) )
	{
		KMessageBox::error(this, KIO::NetAccess::lastErrorString(),i18n("Import failed."));
		return;
	}
	else
	{
		QString sFile = location + file.fileName( false );
		KConfigGroup config(KSharedConfig::openConfig(sFile, KConfig::OnlyLocal), "Color Scheme");
		QString sName = config.readEntry("Name", i18n("Untitled Theme"));

		insertEntry(sFile, sName);
		QPixmap preview = mkColorPreview(cs);
		int current = sList->currentRow();
		sList->item(current)->setIcon(preview);
		connect(sList, SIGNAL(highlighted(int)), SLOT(slotPreviewScheme(int)));
		slotPreviewScheme(current);
	}
}

QColor &KColorScheme::color(int index)
{
    switch(index) {
    case CSM_Inactive_title_bar:
    return cs->iaTitle;
    case CSM_Inactive_title_text:
    return cs->iaTxt;
    case CSM_Inactive_title_blend:
    return cs->iaBlend;
    case CSM_Active_title_bar:
    return cs->aTitle;
    case CSM_Active_title_text:
    return cs->aTxt;
    case CSM_Active_title_blend:
    return cs->aBlend;
    case CSM_Background:
    return cs->back;
    case CSM_Text:
    return cs->txt;
    case CSM_Select_background:
    return cs->select;
    case CSM_Select_text:
    return cs->selectTxt;
    case CSM_Standard_background:
    return cs->window;
    case CSM_Standard_text:
    return cs->windowTxt;
    case CSM_Button_background:
    return cs->button;
    case CSM_Button_text:
    return cs->buttonTxt;
    case CSM_Active_title_button:
    return cs->aTitleBtn;
    case CSM_Inactive_title_button:
    return cs->iTitleBtn;
    case CSM_Active_frame:
    return cs->aFrame;
    case CSM_Active_handle:
    return cs->aHandle;
    case CSM_Inactive_frame:
    return cs->iaFrame;
    case CSM_Inactive_handle:
    return cs->iaHandle;
    case CSM_Link:
    return cs->link;
    case CSM_Followed_Link:
    return cs->visitedLink;
    case CSM_Alternate_background:
    return cs->alternateBackground;
    }

    assert(0); // Should never be here!
    return cs->iaTxt; // Silence compiler
}

void KColorScheme::slotColorForWidget(int indx, const QColor& col)
{
    mColorTreeWidget->setColor( indx , col );
}

void KColorScheme::slotShadeSortColumnChanged(bool b)
{
    cs->shadeSortColumn = b;
    sCurrentScheme.clear();

    emit changed(true);
}

/*
 * Read a color scheme into "cs".
 *
 * KEEP IN SYNC with thememgr!
 */
void KColorScheme::readScheme( int index )
{
    QColor widget(239, 239, 239);
    QColor kde34Blue(103,141,178);
    QColor inactiveBackground(157,170,186);
    QColor activeBackground(65,142,220);
    QColor inactiveForeground(221,221,221);
    QColor activeBlend(107,145,184);
    QColor inactiveBlend(157,170,186);
    QColor activeTitleBtnBg(220,220,220);
    QColor inactiveTitleBtnBg(220,220,220);
    QColor alternateBackground(237,244,249);

    QColor button;
    if (QPixmap::defaultDepth() > 8)
      button.setRgb(221, 223, 228 );
    else
      button.setRgb(220, 220, 220);

    QColor link(0, 0, 238);
    QColor visitedLink(82, 24,139);

    // note: keep default color scheme in sync with default Current Scheme
    if (index == 1) {
      sCurrentScheme  = "<default>";
      cs->txt         = Qt::black;
      cs->back        = widget;
      cs->select      = kde34Blue;
      cs->selectTxt   = Qt::white;
      cs->window      = Qt::white;
      cs->windowTxt   = Qt::black;
      cs->iaTitle     = inactiveBackground;
      cs->iaTxt       = inactiveForeground;
      cs->iaBlend     = inactiveBlend;
      cs->aTitle      = activeBackground;
      cs->aTxt        = Qt::white;
      cs->aBlend      = activeBlend;
      cs->button      = button;
      cs->buttonTxt   = Qt::black;
      cs->aTitleBtn   = activeTitleBtnBg;
      cs->iTitleBtn   = inactiveTitleBtnBg;
      cs->aFrame      = cs->back;
      cs->aHandle     = cs->back;
      cs->iaFrame     = cs->back;
      cs->iaHandle    = cs->back;
      cs->link        = link;
      cs->visitedLink = visitedLink;
      cs->alternateBackground = alternateBackground;

      cs->contrast    = 7;
      cs->shadeSortColumn = KDE_DEFAULT_SHADE_SORT_COLUMN;

      return;
    }

    KConfigBase *config;
    if (index == 0) {
      // Current scheme
      config = KGlobal::config().data();
      config->setGroup("General");
    } else {
      // Open scheme file
      KColorSchemeEntry *entry = mSchemeList->at(sList->currentRow()-nSysSchemes);
      if (!entry) return;
      sCurrentScheme = entry->path;
      config = new KConfig(sCurrentScheme, KConfig::OnlyLocal);
      config->setGroup("Color Scheme");
      int i = sCurrentScheme.lastIndexOf('/');
      if (i >= 0)
        sCurrentScheme = sCurrentScheme.mid(i+1);
    }

    cs->shadeSortColumn = config->readEntry( "shadeSortColumn", KDE_DEFAULT_SHADE_SORT_COLUMN );

    // note: defaults should be the same as the KDE default
    QColor auxBlack, auxWhite;
    auxBlack = Qt::black;
    auxWhite = Qt::white;
    cs->txt = config->readEntry( "foreground", auxBlack );
    cs->back = config->readEntry( "background", widget );
    cs->select = config->readEntry( "selectBackground", kde34Blue );
    cs->selectTxt = config->readEntry( "selectForeground", auxWhite );
    cs->window = config->readEntry( "windowBackground", auxWhite );
    cs->windowTxt = config->readEntry( "windowForeground", auxBlack );
    cs->button = config->readEntry( "buttonBackground", button );
    cs->buttonTxt = config->readEntry( "buttonForeground", auxBlack );
    cs->link = config->readEntry( "linkColor", link );
    cs->visitedLink = config->readEntry( "visitedLinkColor", visitedLink );
    QColor alternate = KGlobalSettings::calculateAlternateBackgroundColor(cs->window);
    cs->alternateBackground = config->readEntry( "alternateBackground", alternate );

    if (index == 0)
      config->setGroup( "WM" );

    cs->iaTitle = config->readEntry("inactiveBackground", inactiveBackground);
    cs->iaTxt = config->readEntry("inactiveForeground", inactiveForeground);
    cs->iaBlend = config->readEntry("inactiveBlend", inactiveBackground);
    cs->iaFrame = config->readEntry("inactiveFrame", cs->back);
    cs->iaHandle = config->readEntry("inactiveHandle", cs->back);
    cs->aTitle = config->readEntry("activeBackground", activeBackground);
    cs->aTxt = config->readEntry("activeForeground", auxWhite);
    cs->aBlend = config->readEntry("activeBlend", activeBlend);
    cs->aFrame = config->readEntry("frame", cs->back);
    cs->aHandle = config->readEntry("handle", cs->back);
    // hack - this is all going away. For now just set all to button bg
    cs->aTitleBtn = config->readEntry("activeTitleBtnBg", activeTitleBtnBg);
    cs->iTitleBtn = config->readEntry("inactiveTitleBtnBg", inactiveTitleBtnBg);

    if (index == 0)
      config->setGroup( "KDE" );

    cs->contrast = config->readEntry( "contrast", 7 );
    if (index != 0)
      delete config;

    mColorTreeWidget->blockSignals(true);   //block the colorChanged signal
    for(int f=0; f< CSM_LAST ; f++)
        mColorTreeWidget->setColor( f , color(f) );
    mColorTreeWidget->blockSignals(false);

}


/*
 * Get all installed color schemes.
 */
void KColorScheme::readSchemeNames()
{
    mSchemeList->clear();
    sList->clear();
    // Always a current and a default scheme
    sList->insertItem( 0 , i18n("Current Scheme") );
    sList->insertItem( 1 , i18n("KDE Default") );
    nSysSchemes = 2;

    // Global + local schemes
    QStringList list = KGlobal::dirs()->findAllResources("data",
            "kdisplay/color-schemes/*.kcsrc", KStandardDirs::NoDuplicates);

    // And add them
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
       KConfigGroup config(KSharedConfig::openConfig(*it, KConfig::OnlyLocal), "Color Scheme");
       QString str = config.readEntry("Name");
       if (str.isEmpty()) {
          str =  config.readEntry("name");
          if (str.isEmpty())
             continue;
       }
       mSchemeList->append(new KColorSchemeEntry(*it, str, !config.isImmutable()));
    }

    mSchemeList->sort();

    for(KColorSchemeEntry *entry = mSchemeList->first(); entry; entry = mSchemeList->next())
    {
       sList->addItem(entry->name);
    }

    for (int i = 0; i < (nSysSchemes + mSchemeList->count()); i++)
    {
       sList->setCurrentRow(i);
       readScheme(i);
       QPixmap preview = mkColorPreview(cs);
       sList->item(i)->setIcon(preview);
    }

}

/*
 * Find scheme based on filename
 */
int KColorScheme::findSchemeByName(const QString &scheme)
{
   if (scheme.isEmpty())
      return 0;
   if (scheme == "<default>")
      return 1;

   QString search = scheme;
   int i = search.lastIndexOf('/');
   if (i >= 0)
      search = search.mid(i+1);

   i = 0;

   for(KColorSchemeEntry *entry = mSchemeList->first(); entry; entry = mSchemeList->next())
   {
      KUrl url;
      url.setPath(entry->path);
      if (url.fileName() == search)
         return i+nSysSchemes;
      i++;
   }

   return 0;
}


void KColorScheme::slotPreviewScheme(int indx)
{
    readScheme(indx);

    // Set various appropriate for the scheme

    cbShadeList->setChecked(cs->shadeSortColumn);

    cs->drawSampleWidgets();
    sb->blockSignals(true);
    sb->setValue(cs->contrast);
    sb->blockSignals(false);
    if (indx < nSysSchemes)
       removeBt->setEnabled(false);
    else
    {
       KColorSchemeEntry *entry = mSchemeList->at(indx-nSysSchemes);
       removeBt->setEnabled(entry ? entry->local : false);
    }

    emit changed((indx != 0));
}


/* this function should dissappear: colorscm should work directly on a Qt palette, since
   this will give us much more cusomization with qt-2.0.
   */
QPalette KColorScheme::createPalette()
{
    QColorGroup disabledgrp(cs->windowTxt, cs->back, cs->back.light(150),
                cs->back.dark(), cs->back.dark(120), cs->back.dark(120),
                cs->window);

    QColorGroup colgrp(cs->windowTxt, cs->back, cs->back.light(150),
               cs->back.dark(), cs->back.dark(120), cs->txt, cs->window);

    colgrp.setColor(QPalette::Highlight, cs->select);
    colgrp.setColor(QPalette::HighlightedText, cs->selectTxt);
    colgrp.setColor(QPalette::Button, cs->button);
    colgrp.setColor(QPalette::ButtonText, cs->buttonTxt);
    return QPalette( colgrp, disabledgrp, colgrp);
}

void KColorScheme::insertEntry(const QString &sFile, const QString &sName)
{
       KColorSchemeEntry *newEntry = new KColorSchemeEntry(sFile, sName, true);
       mSchemeList->inSort(newEntry);
       int newIndex = mSchemeList->findRef(newEntry)+nSysSchemes;
       sList->insertItem(newIndex , sName);
       sList->setCurrentRow(newIndex);
}

void KColorScheme::setColorName( const QString & name, int id , int id2 )
{
    mColorTreeWidget->addRole( id , id2, name );
}

void KColorScheme::slotColorChanged( int selection , const QColor & col)
{
    // Adjust the alternate background color if the standard color changes
    // Only if the previous alternate color was not a user-configured one
    // of course
    if ( selection == CSM_Standard_background &&
         color(CSM_Alternate_background) ==
         KGlobalSettings::calculateAlternateBackgroundColor(
             color(CSM_Standard_background) ) )
    {
        color(CSM_Alternate_background) =
            KGlobalSettings::calculateAlternateBackgroundColor( col );
    }

    color(selection) = col;

    cs->drawSampleWidgets();

    sCurrentScheme.clear();

    emit changed(true);
}



#include "colorscm.moc"
