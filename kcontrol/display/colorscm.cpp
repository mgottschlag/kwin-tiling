//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//
// Converted to a kcc module by Matthias Hoelzer 1997
// Ported to Qt-2.0 by Matthias Ettrich 1999
// Ported to kcontrol2 by Geert Jansen 1999

#include <config.h>
#include <stdlib.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qcursor.h>
#include <qstringlist.h>
#include <qfileinfo.h>

#include <kapp.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kglobal.h>
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

    m_pDirs = KGlobal::dirs();
    m_pDirs->addResourceType("colorscm", m_pDirs->kde_default("data") +
                             "kdisplay/color-schemes");
    KConfig *cfg = new KConfig("kcmdisplayrc");
    cfg->setGroup("X11");
    cfg->readBoolEntry("useResourceManager", true);
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

    connect( cs, SIGNAL( widgetSelected( int ) ),
	     SLOT( slotWidgetColor( int ) ) );
    connect( cs, SIGNAL( colorDropped( int, const QColor&)),
	     SLOT( slotColorForWidget( int, const QColor&)));
    topLayout->addMultiCellWidget( cs, 0, 0, 0, 1 );

    QGroupBox *group = new QGroupBox( i18n("Color Scheme"), this );
    topLayout->addWidget( group, 1, 0 );
    QBoxLayout *groupLayout = new QVBoxLayout( group, 10 );
    groupLayout->addSpacing(10);

    sFileList = new QStrList();
    sList = new QListBox( group );
    readSchemeNames();
    sList->setFixedHeight(sList->sizeHint().height()/2);
    sList->setCurrentItem( 0 );
    connect(sList, SIGNAL(highlighted(int)), SLOT(slotPreviewScheme(int)));
    groupLayout->addWidget(sList);

    QBoxLayout *pushLayout = new QHBoxLayout;
    groupLayout->addLayout( pushLayout );

    addBt = new QPushButton(i18n("&Add ..."), group);
    connect(addBt, SIGNAL(clicked()), SLOT(slotAdd()));
    pushLayout->addWidget(addBt, 10);

    removeBt = new QPushButton(i18n("&Remove"), group);
    removeBt->setEnabled(FALSE);
    connect(removeBt, SIGNAL(clicked()), SLOT(slotRemove()));

    pushLayout->addWidget( removeBt, 10 );

    saveBt = new QPushButton(i18n("&Save changes"), group);
    saveBt->setEnabled(FALSE);
    connect(saveBt, SIGNAL(clicked()), SLOT(slotSave()));
    groupLayout->addWidget(saveBt);

    QBoxLayout *stackLayout = new QVBoxLayout;
    topLayout->addLayout(stackLayout, 1, 1);

    group = new QGroupBox(i18n("Widget color"), this);
    stackLayout->addWidget(group);
    groupLayout = new QVBoxLayout(group, 10);
    groupLayout->addSpacing(10);

    wcCombo = new QComboBox(false, group);
    wcCombo->insertItem(i18n("Inactive title bar"));
    wcCombo->insertItem(i18n("Inactive title text"));
    wcCombo->insertItem(i18n("Inactive title blend"));
    wcCombo->insertItem(i18n("Active title bar"));
    wcCombo->insertItem(i18n("Active title text"));
    wcCombo->insertItem(i18n("Active title blend"));
    wcCombo->insertItem(i18n("Background"));
    wcCombo->insertItem(i18n("Text"));
    wcCombo->insertItem(i18n("Select background"));
    wcCombo->insertItem(i18n("Select text"));
    wcCombo->insertItem(i18n("Window background"));
    wcCombo->insertItem(i18n("Window text"));
    wcCombo->insertItem(i18n("Button background"));
    wcCombo->insertItem(i18n("Button text"));
    wcCombo->insertItem(i18n("Active title button"));
    wcCombo->insertItem(i18n("Inactive title button"));
    wcCombo->insertItem(i18n("Active title button background"));
    wcCombo->insertItem(i18n("Inactive title button background"));
    wcCombo->insertItem(i18n("Active title button blend"));
    wcCombo->insertItem(i18n("Inactive title button blend"));

    wcCombo->adjustSize();
    connect(wcCombo, SIGNAL(activated(int)), SLOT(slotWidgetColor(int)));
    groupLayout->addWidget(wcCombo);

    colorButton = new KColorButton( group );
    colorButton->setColor(cs->iaTitle);
    colorPushColor = cs->iaTitle;
    connect( colorButton, SIGNAL( changed(const QColor &)),
	    SLOT(slotSelectColor(const QColor &)));
	
    groupLayout->addWidget( colorButton );

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

    QLabel *label = new QLabel(sb, i18n("&Low"), group);
    groupLayout->addWidget(label);
    groupLayout->addWidget(sb, 10);
    label = new QLabel(group);
    label->setText(i18n("High"));
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
    wcCombo->setCurrentItem(0);
    colorButton->setColor(cs->iaTitle);
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

    cfg->setGroup( "WM" );
    cfg->writeEntry("activeForeground", cs->aTxt, true, true);
    cfg->writeEntry("inactiveBackground", cs->iaTitle, true, true);
    cfg->writeEntry("inactiveBlend", cs->iaBlend, true, true);
    cfg->writeEntry("activeBackground", cs->aTitle, true, true);
    cfg->writeEntry("activeBlend", cs->aBlend, true, true);
    cfg->writeEntry("inactiveForeground", cs->iaTxt, true, true);
    cfg->writeEntry("activeTitleBtnFg", cs->aTitleBtn, true, true);
    cfg->writeEntry("inactiveTitleBtnFg", cs->iTitleBtn, true, true);
    cfg->writeEntry("activeTitleBtnBg", cs->aTitleBtnBack, true, true);
    cfg->writeEntry("inactiveTitleBtnBg", cs->iTitleBtnBack, true, true);
    if(cs->aTitleBtnBlend != cs->aTitleBtnBack)
        cfg->writeEntry("activeTitleBtnBlend", cs->aTitleBtnBlend, true, true);
    else
        cfg->writeEntry("activeTitleBtnBlend", "", true, true);
    if(cs->iTitleBtnBlend != cs->iTitleBtnBack)
        cfg->writeEntry("inactiveTitleBtnBlend", cs->iTitleBtnBlend, true, true);
    else
        cfg->writeEntry("inactiveTitleBtnBlend", "", true, true);

    cfg->setGroup( "KDE" );
    cfg->writeEntry("contrast", cs->contrast, true, true);
    cfg->sync();

    // Notify applications

    // TODO: GJ 15/11/99: Must we do this with KIPC, DCOP or the
    // Qt desktop properties ???
    // For now: do KIPC and Qt prop.

    KIPC::sendMessageAll("KDEChangePalette");

    // Write some Qt root property.
    QByteArray properties;
    QDataStream d(properties, IO_WriteOnly);
    d << createPalette() << KGlobal::generalFont();
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
    wcCombo->setCurrentItem(0);
    colorButton->setColor(cs->iaTitle);
    sb->setValue(cs->contrast);

    m_bChanged = true;
    emit changed(true);
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
	new KSimpleConfig( sFileList->at( sList->currentItem() ) );
			
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
    config->writeEntry("activeTitleBtnFg", cs->aTitleBtn);
    config->writeEntry("inactiveTitleBtnFg", cs->iTitleBtn);
    config->writeEntry("activeTitleBtnBg", cs->aTitleBtnBack);
    config->writeEntry("inactiveTitleBtnBg", cs->iTitleBtnBack);
    if(cs->aTitleBtnBlend != cs->aTitleBtnBack)
        config->writeEntry("activeTitleBtnBlend", cs->aTitleBtnBlend);
    else
        config->writeEntry("activeTitleBtnBlend", "");
    if(cs->iTitleBtnBlend != cs->iTitleBtnBack)
        config->writeEntry("inactiveTitleBtnBlend", cs->iTitleBtnBlend);
    else
        config->writeEntry("inactiveTitleBtnBlend", "");


    config->sync();
    saveBt->setEnabled( FALSE );
}


void KColorScheme::slotRemove()
{
    uint ind = sList->currentItem();
    if (unlink(sFileList->at(ind))) {
	KMessageBox::error( 0,
	      i18n("This color scheme could not be removed.\n"
		   "Perhaps you do not have permission to alter the file\n"
		    "system where the color scheme is stored." ));
	return;
    }

    sList->removeItem(ind);
    sFileList->remove(ind);
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

    sFile = m_pDirs->saveLocation("colorscm") + sFile.latin1() + ".kcsrc";
    sFileList->append(sFile.latin1());

    KSimpleConfig *config = new KSimpleConfig(sFile);
    config->setGroup( "Color Scheme");
    config->writeEntry("name", sName);
    delete config;
    slotSave();

    connect(sList, SIGNAL(highlighted(int)), SLOT(slotPreviewScheme(int)));
    slotPreviewScheme(sList->currentItem());
}


void KColorScheme::slotSelectColor(const QColor &col)
{
    colorPushColor = col;

    int selection;
    selection = wcCombo->currentItem()+1;

    switch(selection) {
    case 1:
	cs->iaTitle = colorPushColor;
	break;
    case 2:
	cs->iaTxt = colorPushColor;
	break;
    case 3:
	cs->iaBlend = colorPushColor;
	break;
    case 4:
	cs->aTitle = colorPushColor;
	break;
    case 5:
	cs->aTxt = colorPushColor;
	break;
    case 6:
	cs->aBlend = colorPushColor;
	break;
    case 7:
	cs->back = colorPushColor;
	break;
    case 8:
	cs->txt = colorPushColor;
	break;
    case 9:
	cs->select = colorPushColor;
	break;
    case 10:
	cs->selectTxt = colorPushColor;
	break;
    case 11:
	cs->window = colorPushColor;
	break;
    case 12:
	cs->windowTxt = colorPushColor;
	break;
    case 13:
	cs->button = colorPushColor;
	break;
    case 14:
	cs->buttonTxt = colorPushColor;
	break;
    case 15:
	cs->aTitleBtn = colorPushColor;
	break;
    case 16:
	cs->iTitleBtn = colorPushColor;
	break;
    case 17:
	cs->aTitleBtnBack = colorPushColor;
	break;
    case 18:
	cs->iTitleBtnBack = colorPushColor;
	break;
    case 19:
	cs->aTitleBtnBlend = colorPushColor;
	break;
    case 20:
	cs->iTitleBtnBlend = colorPushColor;
	break;
    }
	
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
    int selection;
    QColor col;
    if (wcCombo->currentItem() != indx)
	wcCombo->setCurrentItem( indx );

    selection = indx + 1;

    switch (selection) {
    case 1:
	col = cs->iaTitle;
	break;
    case 2:
	col = cs->iaTxt;
	break;
    case 3:
	col = cs->iaBlend;
	break;
    case 4:
	col = cs->aTitle;
	break;
    case 5:
	col = cs->aTxt;
	break;	
    case 6:
	col = cs->aBlend;
	break;
    case 7:
	col = cs->back;
	break;
    case 8:
	col = cs->txt;
	break;
    case 9:
	col = cs->select;
	break;
    case 10:
	col = cs->selectTxt;
	break;
    case 11:
	col = cs->window;
	break;
    case 12:
	col = cs->windowTxt;
	break;
    case 13:
	col = cs->button;
	break;
    case 14:
	col = cs->buttonTxt;
	break;
    case 15:
	col = cs->aTitleBtn;
	break;
    case 16:
	col = cs->iTitleBtn;
	break;
    case 17:
	col = cs->aTitleBtnBack;
	break;
    case 18:
	col = cs->iTitleBtnBack;
	break;
    case 19:
	col = cs->aTitleBtnBlend;
    	break;
    case 20:
	col = cs->iTitleBtnBlend;
	break;
    }

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
	cs->aTitleBtnBack = lightGray;
	cs->iTitleBtnBack = lightGray;
	cs->aTitleBtnBlend = lightGray;
	cs->iTitleBtnBlend = lightGray;
	cs->contrast = 7;
	return;
    }

    if (index == 0) {
	// Current scheme
	config = kapp->config();
	config->setGroup("General");
    } else {
	// Open scheme file
	config = new KSimpleConfig(sFileList->at(index), true);
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

    if (index == 0)
	config->setGroup( "WM" );

    cs->iaTitle = config->readColorEntry("inactiveBackground", &darkGray);
    cs->iaTxt = config->readColorEntry("inactiveForeground", &lightGray);
    cs->iaBlend = config->readColorEntry("inactiveBlend", &lightGray);
    cs->aTitle = config->readColorEntry("activeBackground", &darkBlue);
    cs->aTxt = config->readColorEntry("activeForeground", &white);
    cs->aBlend = config->readColorEntry("activeBlend", &black);
    cs->aTitleBtn = config->readColorEntry("activeTitleBtnFg", &cs->back);
    cs->iTitleBtn = config->readColorEntry("inactiveTitleBtnFg", &cs->back);
    cs->aTitleBtnBack = config->readColorEntry("activeTitleBtnBg", &cs->back);
    cs->iTitleBtnBack = config->readColorEntry("inactiveTitleBtnBg", &cs->back);
    cs->aTitleBtnBlend = config->readColorEntry("activeTitleBtnBlend", &cs->back);
    cs->iTitleBtnBlend = config->readColorEntry("inactiveTitleBtnBlend", &cs->back);

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
    sFileList->append( "Not a  kcsrc file" );
    sList->insertItem( i18n("KDE default"), 1 );
    sFileList->append( "Not a kcsrc file" );
    nSysSchemes = 2;

    // Global + local schemes
    QStringList list = m_pDirs->findAllResources("colorscm", "*.kcsrc",
	    false, true);

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
	sFileList->append((*it).ascii());
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
	sFileList->append((*it).ascii());
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
