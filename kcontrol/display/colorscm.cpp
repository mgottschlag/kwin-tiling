//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//
// Converted to a kcc module by Matthias Hoelzer 1997
// Ported to Qt-2.0 by Matthias Ettrich 1999
// Ported to kcontrol2 by Geert Jansen 1999
// Made maintable by Waldo Bastian 2000

#include <assert.h>
#include <config.h>
#include <stdlib.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qcombobox.h>
#include <klistbox.h>
#include <qlayout.h>
#include <qcursor.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstddirs.h>
#include <kipc.h>
#include <kcolordlg.h>
#include <kcolorbtn.h>
#include <kprocess.h>
#include <kbuttonbox.h>

#include <X11/Xlib.h>

#include "colorscm.h"
#include "widgetcanvas.h"


/**** DLL Interface ****/

extern "C" {
    KCModule *create_colors(QWidget *parent, const char *name) {
	KGlobal::locale()->insertCatalogue("kcmdisplay");
	return new KColorScheme(parent, name);
    }
}

/**** KColorScheme ****/

KColorScheme::KColorScheme(QWidget *parent, const char *name)
	: KCModule(parent, name)
{
    m_bChanged = false;
    nSysSchemes = 2;

    KConfig *cfg = new KConfig("kcmdisplayrc");
    cfg->setGroup("X11");
    useRM = cfg->readBoolEntry("useResourceManager", true);
    delete cfg;

    cs = new WidgetCanvas( this );
    cs->setCursor( KCursor::handCursor() );

    // LAYOUT

    QGridLayout *topLayout = new QGridLayout( this, 2, 2, 10 );
    topLayout->setRowStretch(0,0);
    topLayout->setRowStretch(1,0);
    topLayout->setColStretch(0,1);
    topLayout->setColStretch(1,1);

    cs->setFixedHeight(160);
    cs->setMinimumWidth(440);

    QWhatsThis::add( cs, i18n("This is a preview of the color settings which"
       " will be applied if you click \"Apply\" or \"OK\". You can click on"
       " different parts of this preview image. The widget name in the"
       " \"Widget color\" box will change to reflect the part of the preview"
       " image you clicked.") );

    connect( cs, SIGNAL( widgetSelected( int ) ),
	     SLOT( slotWidgetColor( int ) ) );
    connect( cs, SIGNAL( colorDropped( int, const QColor&)),
	     SLOT( slotColorForWidget( int, const QColor&)));
    topLayout->addMultiCellWidget( cs, 0, 0, 0, 1 );

    QGroupBox *group = new QGroupBox( i18n("Color Scheme"), this );
    topLayout->addWidget( group, 1, 0 );
    QBoxLayout *groupLayout = new QVBoxLayout( group, 10 );
    groupLayout->addSpacing(10);

    sList = new KListBox( group );
    readSchemeNames();
    sList->setFixedHeight(sList->sizeHint().height()/2);
    sList->setCurrentItem( 0 );
    connect(sList, SIGNAL(highlighted(int)), SLOT(slotPreviewScheme(int)));
    groupLayout->addWidget(sList);

    QWhatsThis::add( sList, i18n("This is a list of predefined color schemes,"
       " including any that you may have created. You can preview an existing"
       " color scheme by selecting it from the list. The current scheme will"
       " be replaced by the selected color scheme.<p>"
       " Warning: if you have not yet applied any changes you may have made"
       " to the current scheme, those changes will be lost if you select"
       " another color scheme.") );

    QBoxLayout *pushLayout = new QHBoxLayout;
    groupLayout->addLayout( pushLayout );

    addBt = new QPushButton(i18n("&Add ..."), group);
    connect(addBt, SIGNAL(clicked()), SLOT(slotAdd()));
    pushLayout->addWidget(addBt, 10);

    QWhatsThis::add( addBt, i18n("Press this button if you want to save"
       " the current color settings as a new color scheme. You will be"
       " prompted for a name.") );

    removeBt = new QPushButton(i18n("&Remove"), group);
    removeBt->setEnabled(FALSE);
    connect(removeBt, SIGNAL(clicked()), SLOT(slotRemove()));

    QWhatsThis::add( removeBt, i18n("Press this button to remove the selected"
       " color scheme. Note that this button is disabled if you do not have"
       " permission to delete the color scheme.") );

    pushLayout->addWidget( removeBt, 10 );

    saveBt = new QPushButton(i18n("&Save changes"), group);
    saveBt->setEnabled(FALSE);
    connect(saveBt, SIGNAL(clicked()), SLOT(slotSave()));
    groupLayout->addWidget(saveBt);

    QWhatsThis::add(saveBt, i18n("Press this button to save changes made"
       " to the current color scheme. Note that this button is disabled if"
       " you do not have permission to modify the color scheme, but you can"
       " still save the changes by adding a new color scheme.") );

    QBoxLayout *stackLayout = new QVBoxLayout;
    topLayout->addLayout(stackLayout, 1, 1);

    group = new QGroupBox(i18n("Widget color"), this);
    stackLayout->addWidget(group);
    groupLayout = new QVBoxLayout(group, 10);
    groupLayout->addSpacing(10);

    wcCombo = new QComboBox(false, group);
    for(int i=0; i < CSM_LAST;i++)
    {
       wcCombo->insertItem(QString::null);
    }
    wcCombo->changeItem(i18n("Inactive title bar") , CSM_Inactive_title_bar);
    wcCombo->changeItem(i18n("Inactive title text"), CSM_Inactive_title_text);
    wcCombo->changeItem(i18n("Inactive title blend"), CSM_Inactive_title_blend);
    wcCombo->changeItem(i18n("Active title bar"), CSM_Active_title_bar);
    wcCombo->changeItem(i18n("Active title text"), CSM_Active_title_text);
    wcCombo->changeItem(i18n("Active title blend"), CSM_Active_title_blend);
    wcCombo->changeItem(i18n("Window background"), CSM_Background);
    wcCombo->changeItem(i18n("Window text"), CSM_Text);
    wcCombo->changeItem(i18n("Select background"), CSM_Select_background);
    wcCombo->changeItem(i18n("Select text"), CSM_Select_text);
    wcCombo->changeItem(i18n("Standard Background"), CSM_Standard_background);
    wcCombo->changeItem(i18n("Standard Text"), CSM_Standard_text);
    wcCombo->changeItem(i18n("Button background"), CSM_Button_background);
    wcCombo->changeItem(i18n("Button text"), CSM_Button_text);
    wcCombo->changeItem(i18n("Active title button"), CSM_Active_title_button);
    wcCombo->changeItem(i18n("Inactive title button"), CSM_Inactive_title_button);
    wcCombo->changeItem(i18n("Link"), CSM_Link);
    wcCombo->changeItem(i18n("Followed Link"), CSM_Followed_Link);

    wcCombo->adjustSize();
    connect(wcCombo, SIGNAL(activated(int)), SLOT(slotWidgetColor(int)));
    groupLayout->addWidget(wcCombo);

    QWhatsThis::add( wcCombo, i18n("Click here to select an element of"
       " the KDE desktop whose color you want to change. You may either"
       " choose the \"widget\" here, or click on the corresponding part"
       " of the preview image above.") );

    colorButton = new KColorButton( group );
    connect( colorButton, SIGNAL( changed(const QColor &)),
	    SLOT(slotSelectColor(const QColor &)));

    groupLayout->addWidget( colorButton );

    QWhatsThis::add( colorButton, i18n("Click here to bring up a dialog"
       " box where you can choose a color for the \"widget\" selected"
       " in the above list.") );

    group = new QGroupBox(  i18n("Contrast"), this );
    stackLayout->addWidget(group);

    QVBoxLayout *groupLayout2 = new QVBoxLayout(group, 10);
    groupLayout2->addSpacing(10);
    groupLayout = new QHBoxLayout;
    groupLayout2->addLayout(groupLayout);

    sb = new QSlider( QSlider::Horizontal,group,"Slider" );
    sb->setRange( 0, 10 );
    sb->setFocusPolicy( QWidget::StrongFocus );
    connect(sb, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)));

    QWhatsThis::add(sb, i18n("Use this slider to change the contrast level"
       " of the current color scheme. Contrast does not affect all of the"
       " colors, only the edges of 3D objects."));

    QLabel *label = new QLabel(sb, i18n("Low Contrast", "&Low"), group);
    groupLayout->addWidget(label);
    groupLayout->addWidget(sb, 10);
    label = new QLabel(group);
    label->setText(i18n("High Contrast", "High"));
    groupLayout->addWidget( label );

    load();
}


KColorScheme::~KColorScheme()
{
}


void KColorScheme::load()
{
    sList->setCurrentItem(0);
    readScheme(0);

    cs->drawSampleWidgets();
    slotWidgetColor(0);
    sb->setValue(cs->contrast);

    m_bChanged = false;
    emit changed(false);
}


void KColorScheme::save()
{
    if (!m_bChanged)
	return;

    KConfig *cfg = KGlobal::config();
    cfg->setGroup( "General" );
    cfg->writeEntry("background", cs->back, true, true);
    cfg->writeEntry("selectBackground", cs->select, true, true);
    cfg->writeEntry("foreground", cs->txt, true, true);
    cfg->writeEntry("windowForeground", cs->windowTxt, true, true);
    cfg->writeEntry("windowBackground", cs->window, true, true);
    cfg->writeEntry("selectForeground", cs->selectTxt, true, true);
    cfg->writeEntry("buttonBackground", cs->button, true, true);
    cfg->writeEntry("buttonForeground", cs->buttonTxt, true, true);
    cfg->writeEntry("linkColor", cs->link, true, true);
    cfg->writeEntry("visitedLinkColor", cs->visitedLink, true, true);

    cfg->setGroup( "WM" );
    cfg->writeEntry("activeForeground", cs->aTxt, true, true);
    cfg->writeEntry("inactiveBackground", cs->iaTitle, true, true);
    cfg->writeEntry("inactiveBlend", cs->iaBlend, true, true);
    cfg->writeEntry("activeBackground", cs->aTitle, true, true);
    cfg->writeEntry("activeBlend", cs->aBlend, true, true);
    cfg->writeEntry("inactiveForeground", cs->iaTxt, true, true);
    cfg->writeEntry("activeTitleBtnBg", cs->aTitleBtn, true, true);
    cfg->writeEntry("inactiveTitleBtnBg", cs->iTitleBtn, true, true);

    cfg->setGroup( "KDE" );
    cfg->writeEntry("contrast", cs->contrast, true, true);
    cfg->sync();

    // Notify all KDE applications

    KIPC::sendMessageAll(KIPC::PaletteChanged);

    // Write some Qt root property.
    QByteArray properties;
    QDataStream d(properties, IO_WriteOnly);
    d << createPalette() << KGlobalSettings::generalFont();
    Atom a = XInternAtom(qt_xdisplay(), "_QT_DESKTOP_PROPERTIES", false);
    XChangeProperty(qt_xdisplay(),  qt_xrootwin(), a, a, 8, PropModeReplace,
		    (unsigned char*) properties.data(), properties.size());
    QApplication::flushX();
    // Notify non Qt apps ?
    if (useRM) {
	QApplication::setOverrideCursor( waitCursor );
	KProcess proc;
	proc.setExecutable("krdb");
	proc.start(KProcess::Block);
	QApplication::restoreOverrideCursor();
    }

    m_bChanged = false;
    emit changed(false);
}


void KColorScheme::defaults()
{
    readScheme(1);
    sList->setCurrentItem(1);

    cs->drawSampleWidgets();
    slotWidgetColor(0);
    sb->setValue(cs->contrast);

    m_bChanged = true;
    emit changed(true);
}

QString KColorScheme::quickHelp()
{
    return i18n("<h1>Colors</h1> This module allows you to choose"
       " the color scheme used for the KDE desktop. The different"
       " elements of the desktop, such as title bars, menu text, etc.,"
       " are called \"widgets\". You can choose the widget whose"
       " color you want to change by selecting it from a list, or by"
       " clicking on a graphic representation of the desktop.<p>"
       " You can save color settings as complete color schemes,"
       " which can also be modified or deleted. KDE comes with several"
       " predefined color schemes on which you can base your own.<p>"
       " All KDE applications will obey the selected color scheme."
       " Non-KDE applications may also obey some or all of the color"
       " settings. See the \"Style\" control module under \"Themes\""
       " for more details.");
}


void KColorScheme::sliderValueChanged( int val )
{
    cs->contrast = val;
    cs->drawSampleWidgets();

    m_bChanged = true;
    emit changed(true);
}


void KColorScheme::slotSave( )
{
    KSimpleConfig *config =
	new KSimpleConfig(sFileList[ sList->currentItem() ] );
			
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
    config->writeEntry("linkColor", cs->link);
    config->writeEntry("visitedLinkColor", cs->visitedLink);

    config->sync();
    saveBt->setEnabled( FALSE );
}


void KColorScheme::slotRemove()
{
    uint ind = sList->currentItem();
    if (unlink(QFile::encodeName(sFileList[ind]).data())) {
	KMessageBox::error( 0,
	      i18n("This color scheme could not be removed.\n"
		   "Perhaps you do not have permission to alter the file\n"
		    "system where the color scheme is stored." ));
	return;
    }

    sList->removeItem(ind);
    sFileList.remove(sFileList.at(ind));
}


/*
 * Add a local color scheme.
 */
void KColorScheme::slotAdd()
{
    SaveScm *ss = new SaveScm( 0,  "save scheme" );

    QString sName, sFile;

    bool valid = false;

    while (!valid) {

	if (ss->exec() != QDialog::Accepted)
	    return;

	sName = ss->nameLine->text();
	sName = sName.simplifyWhiteSpace();
	if (sName.isEmpty())
	    return;
	sFile = sName;
	
	int i = 0;

	// Capitalise each word
	sName.at(0) = sName.at(0).upper();
	while (1) {
	    i = sName.find(" ", i);
	    if (i == -1)
		break;
	    if (++i >= (int) sName.length())
		break;
	    sName.at(i) = sName.at(i).upper();
	}

	// Check if it's already there
	for (i=0; i < (int) sList->count(); i++)
	    if (sName == sList->text(i)) {
		KMessageBox::error( 0,
			i18n("Please choose a unique name for the new color\n"\
			"scheme. The one you entered already appears\n"\
			"in the color scheme list." ));
		break;
	    }
	if (i == (int) sList->count())
	    valid = true;
    }

    disconnect(sList, SIGNAL(highlighted(int)), this,
	    SLOT(slotPreviewScheme(int)));

    sList->insertItem(sName);
    sList->setFocus();
    sList->setCurrentItem(sList->count() - 1);

    sFile = KGlobal::dirs()->saveLocation("data", "kdisplay/color-schemes/") + sFile + ".kcsrc";
    sFileList.append(sFile);

    KSimpleConfig *config = new KSimpleConfig(sFile);
    config->setGroup( "Color Scheme");
    config->writeEntry("name", sName);
    delete config;
    slotSave();

    connect(sList, SIGNAL(highlighted(int)), SLOT(slotPreviewScheme(int)));
    slotPreviewScheme(sList->currentItem());
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
    case CSM_Link:
	return cs->link;
    case CSM_Followed_Link:
	return cs->visitedLink;
    }
    
    assert(0); // Should never be here!
    return cs->iaTxt; // Silence compiler
}


void KColorScheme::slotSelectColor(const QColor &col)
{
    int selection;
    selection = wcCombo->currentItem();

    color(selection) = col;
	
    cs->drawSampleWidgets();

    if (removeBt->isEnabled())
	saveBt->setEnabled(true);
    else
	saveBt->setEnabled(false);
	
    m_bChanged = true;
    emit changed(true);
}


void KColorScheme::slotWidgetColor(int indx)
{
    if (wcCombo->currentItem() != indx)
	wcCombo->setCurrentItem( indx );

    QColor col = color(indx);
    colorButton->setColor( col );
    colorPushColor = col;	
}



void KColorScheme::slotColorForWidget(int indx, const QColor& col)
{
    slotWidgetColor(indx);
    slotSelectColor(col);
}


/*
 * Read a color scheme into "cs".
 */
void KColorScheme::readScheme( int index )
{
    KConfigBase* config;

    if (index == 1) {
	cs->back = lightGray;
	cs->txt = black;
	cs->select = darkBlue;
	cs->selectTxt = white;
	cs->window = white;
	cs->windowTxt = black;
	cs->iaTitle = darkGray;
	cs->iaTxt = lightGray;
	cs->iaBlend = lightGray;
	cs->aTitle = darkBlue;
	cs->aTxt = white;
	cs->aBlend = black;
	cs->button = cs->back;
	cs->buttonTxt = cs->txt;
	cs->aTitleBtn = lightGray;
	cs->iTitleBtn = lightGray;
        cs->link = blue;
        cs->visitedLink = magenta;
	cs->contrast = 7;
	return;
    }

    if (index == 0) {
	// Current scheme
	config = KGlobal::config();
	config->setGroup("General");
    } else {
	// Open scheme file
	config = new KSimpleConfig(sFileList[index], true);
	config->setGroup("Color Scheme");
    }

    cs->txt = config->readColorEntry( "foreground", &black );
    cs->back = config->readColorEntry( "background", &lightGray );
    cs->select = config->readColorEntry( "selectBackground", &darkBlue);
    cs->selectTxt = config->readColorEntry( "selectForeground", &white );
    cs->window = config->readColorEntry( "windowBackground", &white );
    cs->windowTxt = config->readColorEntry( "windowForeground", &black );
    cs->button = config->readColorEntry( "buttonBackground", &cs->back );
    cs->buttonTxt = config->readColorEntry( "buttonForeground", &cs->txt );
    cs->link = config->readColorEntry( "linkColor", &blue);
    cs->visitedLink = config->readColorEntry( "visitedLinkColor", &magenta );

    if (index == 0)
	config->setGroup( "WM" );

    cs->iaTitle = config->readColorEntry("inactiveBackground", &darkGray);
    cs->iaTxt = config->readColorEntry("inactiveForeground", &lightGray);
    cs->iaBlend = config->readColorEntry("inactiveBlend", &lightGray);
    cs->aTitle = config->readColorEntry("activeBackground", &darkBlue);
    cs->aTxt = config->readColorEntry("activeForeground", &white);
    cs->aBlend = config->readColorEntry("activeBlend", &black);
    // hack - this is all going away. For now just set all to button bg
    cs->aTitleBtn = config->readColorEntry("activeTitleBtnBg", &cs->back);
    cs->iTitleBtn = config->readColorEntry("inactiveTitleBtnBg", &cs->back);

    if (index == 0)
	config->setGroup( "KDE" );

    cs->contrast = config->readNumEntry( "contrast", 7 );
}


/*
 * Get all installed color schemes.
 */
void KColorScheme::readSchemeNames()
{
    // Always a current and a default scheme
    sList->insertItem( i18n("Current scheme"), 0 );
    sFileList.append( "Not a  kcsrc file" );
    sList->insertItem( i18n("KDE default"), 1 );
    sFileList.append( "Not a kcsrc file" );
    nSysSchemes = 2;

    // Global + local schemes
    QStringList list = KGlobal::dirs()->findAllResources("data", 
            "kdisplay/color-schemes/*.kcsrc", false, true);

    // Put local schemes into localList
    QStringList localList;
    QStringList::Iterator it;
    for (it = list.begin(); it != list.end(); it++) {
	QFileInfo fi(*it);
	if (fi.isWritable()) {
	    localList.append(*it);
	    it = list.remove(it);
	    it--;
	}
    }

    // And add them
    for (it = list.begin(); it != list.end(); it++) {
	KSimpleConfig *config = new KSimpleConfig(*it, true);
	config->setGroup("Color Scheme");
	QString str = config->readEntry("name");
	if (str.isEmpty())
	    continue;
	sList->insertItem(str);
	sFileList.append(*it);
	nSysSchemes++;
	delete config;
    }

    // Now repeat for local files
    for (it = localList.begin(); it != localList.end(); it++) {
	KSimpleConfig *config = new KSimpleConfig((*it), true);
	config->setGroup("Color Scheme");
	QString str = config->readEntry("name");
	if (str.isEmpty())
	    continue;
	sList->insertItem(str);
	sFileList.append(*it);
	delete config;
    }
}


void KColorScheme::slotPreviewScheme(int indx)
{
    readScheme(indx);

    // Set various appropriate for the scheme

    cs->drawSampleWidgets();
    sb->setValue(cs->contrast);
    slotWidgetColor(0);
    if (indx < nSysSchemes) {
	removeBt->setEnabled(false);
	saveBt->setEnabled(false);
    } else
	removeBt->setEnabled(true);

    m_bChanged = true;
    emit changed(true);
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

    colgrp.setColor(QColorGroup::Highlight, cs->select);
    colgrp.setColor(QColorGroup::HighlightedText, cs->selectTxt);
    colgrp.setColor(QColorGroup::Button, cs->button);
    colgrp.setColor(QColorGroup::ButtonText, cs->buttonTxt);
    return QPalette( colgrp, disabledgrp, colgrp);
}


/**** SaveScm ****/

SaveScm::SaveScm( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
    setFocusPolicy(QWidget::StrongFocus);
    setCaption( i18n("Add a color scheme"));

    QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
    QBoxLayout *stackLayout = new QVBoxLayout( 3 );
    topLayout->addLayout( stackLayout );

    nameLine = new QLineEdit( this );
    nameLine->setFocus();
    nameLine->setMaxLength(18);
    nameLine->setFixedHeight( nameLine->sizeHint().height() );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( nameLine,
	 i18n( "Enter a name for the new color scheme\n"\
	    "to be added to your personal list.\n\n"\
	    "The colors currently used in the preview will\n"\
	    "be copied into this scheme to begin with." ), this );
    tmpQLabel->setAlignment( AlignLeft | AlignBottom | ShowPrefix );
    tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    tmpQLabel->setMinimumWidth( tmpQLabel->sizeHint().width() );

    stackLayout->addStretch( 10 );
    stackLayout->addWidget( tmpQLabel );
    stackLayout->addWidget( nameLine );

    QFrame* tmpQFrame;
    tmpQFrame = new QFrame( this );
    tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    tmpQFrame->setMinimumHeight( tmpQFrame->sizeHint().height() );

    topLayout->addWidget( tmpQFrame );

    KButtonBox *bbox = new KButtonBox( this );
    bbox->addStretch( 10 );

    QPushButton *ok = bbox->addButton( i18n( "&OK" ) );
    ok->setDefault( true );
    connect( ok, SIGNAL( clicked() ), SLOT( accept() ) );

    QPushButton *cancel = bbox->addButton( i18n( "&Cancel" ) );
    connect( cancel, SIGNAL( clicked() ), SLOT( reject() ) );

    bbox->layout();
    topLayout->addWidget( bbox );
    resize( 250, 0 );
}


#include "colorscm.moc"
