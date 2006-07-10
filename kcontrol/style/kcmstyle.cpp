/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 *
 * Portions Copyright (C) 2000 TrollTech AS.
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

#include <QCheckBox>
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <kcombobox.h>
#include <q3groupbox.h>
#include <QLabel>
#include <QLayout>
#include <QSlider>
#include <qstylefactory.h>
#include <QTabWidget>

#include <QFile>
#include <qsettings.h>
#include <QObject>
#include <qpixmapcache.h>

#include <QPushButton>
#include <QX11Info>
#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <klocale.h>
#include <kipc.h>
#include <kaboutdata.h>
#include <kdialog.h>
#include <klibloader.h>
#include <k3listview.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstyle.h>
#include <kstandarddirs.h>

#include "../krdb/krdb.h"

#include "kcmstyle.h"
#include "styleconfdialog.h"

#include <X11/Xlib.h>
#include <kvbox.h>
// X11 namespace cleanup
#undef Below
#undef KeyPress
#undef KeyRelease


/**** DLL Interface for kcontrol ****/


#include <kgenericfactory.h>

typedef KGenericFactory<KCMStyle, QWidget> KCMStyleFactory;
K_EXPORT_COMPONENT_FACTORY( style, KCMStyleFactory("kcmstyle") )

extern "C"
{
    KDE_EXPORT void kcminit_style()
    {
        uint flags = KRdbExportQtSettings | KRdbExportQtColors | KRdbExportXftSettings;
        KConfig config("kcmdisplayrc", true /*readonly*/, false /*don't read kdeglobals etc.*/);
        config.setGroup("X11");

        // This key is written by the "colors" module.
        bool exportKDEColors = config.readEntry("exportKDEColors", true);
        if (exportKDEColors)
            flags |= KRdbExportColors;
        runRdb( flags );

        // Write some Qt root property.
#ifndef __osf__      // this crashes under Tru64 randomly -- will fix later
        QByteArray properties;
        QDataStream d(&properties, QIODevice::WriteOnly);
        d.setVersion( 3 );      // Qt2 apps need this.
        d << kapp->palette() << KGlobalSettings::generalFont();
        Atom a = XInternAtom(QX11Info::display(), "_QT_DESKTOP_PROPERTIES", false);

        // do it for all root windows - multihead support
        int screen_count = ScreenCount(QX11Info::display());
        for (int i = 0; i < screen_count; i++)
            XChangeProperty(QX11Info::display(), RootWindow(QX11Info::display(), i),
                            a, a, 8, PropModeReplace,
                            (unsigned char*) properties.data(), properties.size());
#endif
    }
}

KCMStyle::KCMStyle( QWidget* parent, const QStringList& )
	: KCModule( KCMStyleFactory::instance(), parent ), appliedStyle(NULL)
{
    setQuickHelp( i18n("<h1>Style</h1>"
			"This module allows you to modify the visual appearance "
			"of user interface elements, such as the widget style "
			"and effects."));

	m_bEffectsDirty = false;
	m_bStyleDirty= false;
	m_bToolbarsDirty = false;

	KGlobal::dirs()->addResourceType("themes",
		KStandardDirs::kde_default("data") + "kstyle/themes");

	KAboutData *about =
		new KAboutData( I18N_NOOP("kcmstyle"),
						I18N_NOOP("KDE Style Module"),
						0, 0, KAboutData::License_GPL,
						I18N_NOOP("(c) 2002 Karol Szwed, Daniel Molkentin"));

	about->addAuthor("Karol Szwed", 0, "gallium@kde.org");
	about->addAuthor("Daniel Molkentin", 0, "molkentin@kde.org");
	about->addAuthor("Ralf Nolden", 0, "nolden@kde.org");
	setAboutData( about );

	// Setup pages and mainLayout
	mainLayout = new QVBoxLayout( this );
	tabWidget  = new QTabWidget( this );
	mainLayout->addWidget( tabWidget );

	page1 = new QWidget( tabWidget );
	page1Layout = new QVBoxLayout( page1 );
	page1Layout->setMargin( KDialog::marginHint() );
	page1Layout->setSpacing( KDialog::spacingHint() );
	page2 = new QWidget( tabWidget );
	page2Layout = new QVBoxLayout( page2 );
	page2Layout->setMargin( KDialog::marginHint() );
	page2Layout->setSpacing( KDialog::spacingHint() );
	page3 = new QWidget( tabWidget );
	page3Layout = new QVBoxLayout( page3 );
	page3Layout->setMargin( KDialog::marginHint() );
	page3Layout->setSpacing( KDialog::spacingHint() );

	// Add Page1 (Style)
	// -----------------
	gbWidgetStyle = new Q3GroupBox( i18n("Widget Style"), page1, "gbWidgetStyle" );
	gbWidgetStyle->setColumnLayout( 0, Qt::Vertical );
	gbWidgetStyle->layout()->setMargin( KDialog::marginHint() );
	gbWidgetStyle->layout()->setSpacing( KDialog::spacingHint() );

	gbWidgetStyleLayout = new QVBoxLayout( );
        gbWidgetStyle->layout()->addItem( gbWidgetStyleLayout );
	gbWidgetStyleLayout->setAlignment( Qt::AlignTop );
	hbLayout = new QHBoxLayout( );
        hbLayout->setSpacing( KDialog::spacingHint() );
	hbLayout->setObjectName( "hbLayout" );

	cbStyle = new KComboBox( gbWidgetStyle );
        cbStyle->setObjectName( "cbStyle" );
	cbStyle->setEditable( false );
	hbLayout->addWidget( cbStyle );

	pbConfigStyle = new QPushButton( i18n("Con&figure..."), gbWidgetStyle );
	pbConfigStyle->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Minimum );
	pbConfigStyle->setEnabled( false );
	hbLayout->addWidget( pbConfigStyle );

	gbWidgetStyleLayout->addLayout( hbLayout );

	lblStyleDesc = new QLabel( gbWidgetStyle );
	lblStyleDesc->setTextFormat(Qt::RichText);
	gbWidgetStyleLayout->addWidget( lblStyleDesc );

	cbIconsOnButtons = new QCheckBox( i18n("Sho&w icons on buttons"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbIconsOnButtons );
	cbEnableTooltips = new QCheckBox( i18n("E&nable tooltips"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbEnableTooltips );
	cbTearOffHandles = new QCheckBox( i18n("Show tear-off handles in &popup menus"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbTearOffHandles );
	cbTearOffHandles->hide(); // reenable when the corresponding Qt method is virtual and properly reimplemented

	Q3GroupBox *gbPreview = new Q3GroupBox( i18n( "Preview" ), page1 );
	gbPreview->setColumnLayout( 0, Qt::Vertical );
	gbPreview->layout()->setMargin( 0 );
	gbPreview->layout()->setSpacing( KDialog::spacingHint() );
	gbPreview->setFlat( true );
	stylePreview = new StylePreview( gbPreview );
	gbPreview->layout()->addWidget( stylePreview );

	page1Layout->addWidget( gbWidgetStyle );
	page1Layout->addWidget( gbPreview );

	// Connect all required stuff
	connect( cbStyle, SIGNAL(activated(int)), this, SLOT(styleChanged()) );
	connect( cbStyle, SIGNAL(activated(int)), this, SLOT(updateConfigButton()));
	connect( pbConfigStyle, SIGNAL(clicked()), this, SLOT(styleSpecificConfig()));

	// Add Page2 (Effects)
	// -------------------
	cbEnableEffects = new QCheckBox( i18n("&Enable GUI effects"), page2 );
	containerFrame = new QFrame( page2 );
	containerFrame->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	//containerFrame->setMargin(0);
	containerLayout = new QGridLayout( containerFrame );
        containerLayout->setMargin( KDialog::marginHint() );
        containerLayout->setSpacing( KDialog::spacingHint() );

	comboComboEffect = new QComboBox( containerFrame );
	comboComboEffect->setEditable( false );
	comboComboEffect->addItem( i18n("Disable") );
	comboComboEffect->addItem( i18n("Animate") );
	lblComboEffect = new QLabel( i18n("Combobo&x effect:"), containerFrame );
	lblComboEffect->setBuddy( comboComboEffect );
	containerLayout->addWidget( lblComboEffect, 0, 0 );
	containerLayout->addWidget( comboComboEffect, 0, 1 );

	comboTooltipEffect = new QComboBox( containerFrame );
	comboTooltipEffect->setEditable( false );
	comboTooltipEffect->addItem( i18n("Disable") );
	comboTooltipEffect->addItem( i18n("Animate") );
	comboTooltipEffect->addItem( i18n("Fade") );
	lblTooltipEffect = new QLabel( i18n("&Tool tip effect:"), containerFrame );
	lblTooltipEffect->setBuddy( comboTooltipEffect );
	containerLayout->addWidget( lblTooltipEffect, 1, 0 );
	containerLayout->addWidget( comboTooltipEffect, 1, 1 );

	comboMenuEffect = new QComboBox( containerFrame );
	comboMenuEffect->setEditable( false );
	comboMenuEffect->addItem( i18n("Disable") );
	comboMenuEffect->addItem( i18n("Animate") );
	comboMenuEffect->addItem( i18n("Fade") );
	comboMenuEffect->addItem( i18n("Make Translucent") );
	lblMenuEffect = new QLabel( i18n("&Menu effect:"), containerFrame );
	lblMenuEffect->setBuddy( comboMenuEffect );
	containerLayout->addWidget( lblMenuEffect, 2, 0 );
	containerLayout->addWidget( comboMenuEffect, 2, 1 );

	comboMenuHandle = new QComboBox( containerFrame );
	comboMenuHandle->setEditable( false );
	comboMenuHandle->addItem( i18n("Disable") );
	comboMenuHandle->addItem( i18n("Application Level") );
//	comboMenuHandle->addItem( i18n("Enable") );
	lblMenuHandle = new QLabel( i18n("Me&nu tear-off handles:"), containerFrame );
	lblMenuHandle->setBuddy( comboMenuHandle );
	containerLayout->addWidget( lblMenuHandle, 3, 0 );
	containerLayout->addWidget( comboMenuHandle, 3, 1 );

	cbMenuShadow = new QCheckBox( i18n("Menu &drop shadow"), containerFrame );
	containerLayout->addWidget( cbMenuShadow, 4, 0 );

	// Push the [label combo] to the left.
	comboSpacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	containerLayout->addItem( comboSpacer, 1, 2 );

	// Separator.
	QFrame* hline = new QFrame ( page2 );
	hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );

	// Now implement the Menu Transparency container.
	menuContainer = new QFrame( page2 );
	menuContainer->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	//menuContainer->setMargin(0);
	menuContainerLayout = new QGridLayout( menuContainer );
        menuContainerLayout->setMargin( KDialog::marginHint() );
        menuContainerLayout->setSpacing( KDialog::spacingHint() );

	menuPreview = new MenuPreview( menuContainer, /* opacity */ 90, MenuPreview::Blend );

	comboMenuEffectType = new QComboBox( menuContainer );
	comboMenuEffectType->setEditable( false );
	comboMenuEffectType->addItem( i18n("Software Tint") );
	comboMenuEffectType->addItem( i18n("Software Blend") );
#ifdef HAVE_XRENDER
	comboMenuEffectType->addItem( i18n("XRender Blend") );
#endif

	// So much stuffing around for a simple slider..
	sliderBox = new KVBox( menuContainer );
	sliderBox->setSpacing( KDialog::spacingHint() );
#warning "KDE4: fix setMargin"
	//sliderBox->setMargin( 0 );
	slOpacity = new QSlider( Qt::Horizontal, sliderBox );
	slOpacity->setMinimum( 0 );
	slOpacity->setMaximum( 100 );
	slOpacity->setPageStep( 5 );
	slOpacity->setTickPosition( QSlider::TicksBelow );
	slOpacity->setTickInterval( 10 );
	KHBox* box1 = new KHBox( sliderBox );
	box1->setSpacing( KDialog::spacingHint() );
	//box1->setMargin( 0 );
	QLabel* lbl = new QLabel( i18n("0%"), box1 );
	lbl->setAlignment( Qt::AlignLeft );
	lbl = new QLabel( i18n("50%"), box1 );
	lbl->setAlignment( Qt::AlignHCenter );
	lbl = new QLabel( i18n("100%"), box1 );
	lbl->setAlignment( Qt::AlignRight );

	lblMenuEffectType = new QLabel( i18n("Menu trans&lucency type:"), menuContainer );
	lblMenuEffectType->setBuddy( comboMenuEffectType );
	lblMenuEffectType->setAlignment( Qt::AlignBottom | Qt::AlignLeft );
	lblMenuOpacity    = new QLabel( i18n("Menu &opacity:"), menuContainer );
	lblMenuOpacity->setBuddy( slOpacity );
	lblMenuOpacity->setAlignment( Qt::AlignBottom | Qt::AlignLeft );

	menuContainerLayout->addWidget( lblMenuEffectType, 0, 0 );
	menuContainerLayout->addWidget( comboMenuEffectType, 1, 0 );
	menuContainerLayout->addWidget( lblMenuOpacity, 2, 0 );
	menuContainerLayout->addWidget( sliderBox, 3, 0 );
	menuContainerLayout->addWidget( menuPreview, 0, 1, 4, 1);

	// Layout page2.
	page2Layout->addWidget( cbEnableEffects );
	page2Layout->addWidget( containerFrame );
	page2Layout->addWidget( hline );
	page2Layout->addWidget( menuContainer );

	QSpacerItem* sp1 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	page2Layout->addItem( sp1 );

	// Data flow stuff.
	connect( cbEnableEffects,    SIGNAL(toggled(bool)), containerFrame, SLOT(setEnabled(bool)) );
	connect( cbEnableEffects,    SIGNAL(toggled(bool)), this, SLOT(menuEffectChanged(bool)) );
	connect( slOpacity,          SIGNAL(valueChanged(int)),menuPreview, SLOT(setOpacity(int)) );
	connect( comboMenuEffect,    SIGNAL(activated(int)), this, SLOT(menuEffectChanged()) );
	connect( comboMenuEffect,    SIGNAL(highlighted(int)), this, SLOT(menuEffectChanged()) );
	connect( comboMenuEffectType, SIGNAL(activated(int)), this, SLOT(menuEffectTypeChanged()) );
	connect( comboMenuEffectType, SIGNAL(highlighted(int)), this, SLOT(menuEffectTypeChanged()) );

	// Add Page3 (Miscellaneous)
	// -------------------------
	cbHoverButtons = new QCheckBox( i18n("High&light buttons under mouse"), page3 );
	cbTransparentToolbars = new QCheckBox( i18n("Transparent tool&bars when moving"), page3 );

	QWidget * dummy = new QWidget( page3 );

	QHBoxLayout* box2 = new QHBoxLayout( dummy );
	box2->setSpacing( KDialog::spacingHint() );
	box2->setMargin( 0 );
	lbl = new QLabel( i18n("Text pos&ition:"), dummy );
	comboToolbarIcons = new QComboBox( dummy );
	comboToolbarIcons->setEditable( false );
	comboToolbarIcons->addItem( i18n("Icons Only") );
	comboToolbarIcons->addItem( i18n("Text Only") );
	comboToolbarIcons->addItem( i18n("Text Alongside Icons") );
	comboToolbarIcons->addItem( i18n("Text Under Icons") );
	lbl->setBuddy( comboToolbarIcons );

	box2->addWidget( lbl );
	box2->addWidget( comboToolbarIcons );
	QSpacerItem* sp2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	box2->addItem( sp2 );

	page3Layout->addWidget( cbHoverButtons );
	page3Layout->addWidget( cbTransparentToolbars );
	page3Layout->addWidget( dummy );

	// Layout page3.
	QSpacerItem* sp3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	page3Layout->addItem( sp3 );

	// Load settings
	load();

	// Do all the setDirty connections.
	connect(cbStyle, SIGNAL(activated(int)), this, SLOT(setStyleDirty()));
	// Page2
	connect( cbEnableEffects,    SIGNAL(toggled(bool)),   this, SLOT(setEffectsDirty()));
	connect( cbEnableEffects,    SIGNAL(toggled(bool)),   this, SLOT(setStyleDirty()));
	connect( comboTooltipEffect, SIGNAL(activated(int)), this, SLOT(setEffectsDirty()));
	connect( comboComboEffect,   SIGNAL(activated(int)), this, SLOT(setEffectsDirty()));
	connect( comboMenuEffect,    SIGNAL(activated(int)), this, SLOT(setStyleDirty()));
	connect( comboMenuHandle,    SIGNAL(activated(int)), this, SLOT(setStyleDirty()));
	connect( comboMenuEffectType, SIGNAL(activated(int)), this, SLOT(setStyleDirty()));
	connect( slOpacity,          SIGNAL(valueChanged(int)),this, SLOT(setStyleDirty()));
	connect( cbMenuShadow,       SIGNAL(toggled(bool)),   this, SLOT(setStyleDirty()));
	// Page3
	connect( cbHoverButtons,       SIGNAL(toggled(bool)),   this, SLOT(setToolbarsDirty()));
	connect( cbTransparentToolbars, SIGNAL(toggled(bool)),   this, SLOT(setToolbarsDirty()));
	connect( cbEnableTooltips,     SIGNAL(toggled(bool)),   this, SLOT(setEffectsDirty()));
	connect( cbIconsOnButtons,     SIGNAL(toggled(bool)),   this, SLOT(setEffectsDirty()));
	connect( cbTearOffHandles,     SIGNAL(toggled(bool)),   this, SLOT(setEffectsDirty()));
	connect( comboToolbarIcons,    SIGNAL(activated(int)), this, SLOT(setToolbarsDirty()));

	addWhatsThis();

	// Insert the pages into the tabWidget
	tabWidget->insertTab( page1, i18n("&Style"));
	tabWidget->insertTab( page2, i18n("&Effects"));
	tabWidget->insertTab( page3, i18n("&Toolbar"));

	//Enable/disable the button for the initial style
	updateConfigButton();
}


KCMStyle::~KCMStyle()
{
    qDeleteAll(styleEntries);
	delete appliedStyle;
}

void KCMStyle::updateConfigButton()
{
	if (!styleEntries[currentStyle()] || styleEntries[currentStyle()]->configPage.isEmpty()) {
		pbConfigStyle->setEnabled(false);
		return;
	}

	// We don't check whether it's loadable here -
	// lets us report an error and not waste time
	// loading things if the user doesn't click the button
	pbConfigStyle->setEnabled( true );
}

void KCMStyle::styleSpecificConfig()
{
	QString libname = styleEntries[currentStyle()]->configPage;

	// Use KLibLoader to get the library, handling
	// any errors that arise
	KLibLoader* loader = KLibLoader::self();

	KLibrary* library = loader->library( QFile::encodeName(libname) );
	if (!library)
	{
		KMessageBox::detailedError(this,
			i18n("There was an error loading the configuration dialog for this style."),
			loader->lastErrorMessage(),
			i18n("Unable to Load Dialog"));
		return;
	}

	void* allocPtr = library->symbol("allocate_kstyle_config");

	if (!allocPtr)
	{
		KMessageBox::detailedError(this,
			i18n("There was an error loading the configuration dialog for this style."),
			loader->lastErrorMessage(),
			i18n("Unable to Load Dialog"));
		return;
	}

	//Create the container dialog
	StyleConfigDialog* dial = new StyleConfigDialog(this, styleEntries[currentStyle()]->name);
	dial->showButtonSeparator(true);

	typedef QWidget*(* factoryRoutine)( QWidget* parent );

	//Get the factory, and make the widget.
	factoryRoutine factory      = (factoryRoutine)(allocPtr); //Grmbl. So here I am on my
	//"never use C casts" moralizing streak, and I find that one can't go void* -> function ptr
	//even with a reinterpret_cast.

	QWidget*       pluginConfig = factory( dial );

	//Insert it in...
	dial->setMainWidget( pluginConfig );

	//..and connect it to the wrapper
	connect(pluginConfig, SIGNAL(changed(bool)), dial, SLOT(setDirty(bool)));
	connect(dial, SIGNAL(defaults()), pluginConfig, SLOT(defaults()));
	connect(dial, SIGNAL(save()), pluginConfig, SLOT(save()));

	if (dial->exec() == QDialog::Accepted  && dial->isDirty() ) {
		// Force re-rendering of the preview, to apply settings
		switchStyle(currentStyle(), true);

		//For now, ask all KDE apps to recreate their styles to apply the setitngs
		KIPC::sendMessageAll(KIPC::StyleChanged);

		// We call setStyleDirty here to make sure we force style re-creation
		setStyleDirty();
	}

	delete dial;
}

void KCMStyle::load()
{
	KConfig config( "kdeglobals", true, false );
	// Page1 - Build up the Style ListBox
	loadStyle( config );

	// Page2 - Effects
	loadEffects( config );

	// Page3 - Misc.
	loadMisc( config );

	m_bEffectsDirty = false;
	m_bStyleDirty= false;
	m_bToolbarsDirty = false;

	emit changed( false );
}


void KCMStyle::save()
{
	// Don't do anything if we don't need to.
	if ( !(m_bToolbarsDirty | m_bEffectsDirty | m_bStyleDirty ) )
		return;

	bool allowMenuTransparency = false;
	bool allowMenuDropShadow   = false;

	// Read the KStyle flags to see if the style writer
	// has enabled menu translucency in the style.
	if (appliedStyle && appliedStyle->inherits("KStyle"))
	{
		allowMenuDropShadow = true;
		KStyle* style = dynamic_cast<KStyle*>(appliedStyle);
/*		if (style) {
			KStyle::KStyleFlags flags = style->styleFlags();
			if (flags & KStyle::AllowMenuTransparency)
				allowMenuTransparency = true;
		}*/
	}

	//### KDE4: not at all clear whether this will come back
	allowMenuTransparency = false;

	QString warn_string( i18n("<qt>Selected style: <b>%1</b><br><br>"
		"One or more effects that you have chosen could not be applied because the selected "
		"style does not support them; they have therefore been disabled.<br>"
		"<br>", cbStyle->currentText()) );
	bool show_warning = false;

	// Warn the user if they're applying a style that doesn't support
	// menu translucency and they enabled it.
    if ( (!allowMenuTransparency) &&
		(cbEnableEffects->isChecked()) &&
		(comboMenuEffect->currentIndex() == 3) )	// Make Translucent
    {
		warn_string += i18n("Menu translucency is not available.<br>");
		comboMenuEffect->setCurrentIndex(0);    // Disable menu effect.
		show_warning = true;
	}

	if (!allowMenuDropShadow && cbMenuShadow->isChecked())
	{
		warn_string += i18n("Menu drop-shadows are not available.");
		cbMenuShadow->setChecked(false);
		show_warning = true;
	}

	// Tell the user what features we could not apply on their behalf.
	if (show_warning)
		KMessageBox::information(this, warn_string);


	// Save effects.
	KConfig config( "kdeglobals" );
	config.setGroup("KDE");

	config.writeEntry( "EffectsEnabled", cbEnableEffects->isChecked());
	int item = comboComboEffect->currentIndex();
	config.writeEntry( "EffectAnimateCombo", item == 1 );
	item = comboTooltipEffect->currentIndex();
	config.writeEntry( "EffectAnimateTooltip", item == 1);
	config.writeEntry( "EffectFadeTooltip", item == 2 );
	item = comboMenuHandle->currentIndex();
	config.writeEntry( "InsertTearOffHandle", item );
	item = comboMenuEffect->currentIndex();
	config.writeEntry( "EffectAnimateMenu", item == 1 );
	config.writeEntry( "EffectFadeMenu", item == 2 );

	// Handle KStyle's menu effects
	QString engine("Disabled");
	if (item == 3 && cbEnableEffects->isChecked())	// Make Translucent
		switch( comboMenuEffectType->currentIndex())
		{
			case 1: engine = "SoftwareBlend"; break;
			case 2: engine = "XRender"; break;
			default:
			case 0: engine = "SoftwareTint"; break;
		}

	{	// Braces force a QSettings::sync()
		QSettings settings;	// Only for KStyle stuff
		settings.setValue("/KStyle/Settings/MenuTransparencyEngine", engine);
		settings.setValue("/KStyle/Settings/MenuOpacity", slOpacity->value()/100.0);
 		settings.setValue("/KStyle/Settings/MenuDropShadow",
					cbEnableEffects->isChecked() && cbMenuShadow->isChecked() );
	}

	// Misc page
	config.writeEntry( "ShowIconsOnPushButtons", cbIconsOnButtons->isChecked(), KConfigBase::Normal|KConfigBase::Global);
	config.writeEntry( "EffectNoTooltip", !cbEnableTooltips->isChecked(), KConfigBase::Normal|KConfigBase::Global);

	config.setGroup("General");
	config.writeEntry( "widgetStyle", currentStyle() );

	config.setGroup("Toolbar style");
	config.writeEntry( "Highlighting", cbHoverButtons->isChecked(), KConfigBase::Normal|KConfigBase::Global);
	config.writeEntry( "TransparentMoving", cbTransparentToolbars->isChecked(), KConfigBase::Normal|KConfigBase::Global);
	QString tbIcon;
	switch( comboToolbarIcons->currentIndex() )
	{
		case 1: tbIcon = "TextOnly"; break;
		case 2: tbIcon = "IconTextRight"; break;
		case 3: tbIcon = "IconTextBottom"; break;
		case 0:
		default: tbIcon = "IconOnly"; break;
	}
	config.writeEntry( "IconText", tbIcon, KConfigBase::Normal|KConfigBase::Global);
	config.sync();

	// Export the changes we made to qtrc, and update all qt-only
	// applications on the fly, ensuring that we still follow the user's
	// export fonts/colors settings.
	if (m_bStyleDirty | m_bEffectsDirty)	// Export only if necessary
	{
		uint flags = KRdbExportQtSettings;
		KConfig kconfig("kcmdisplayrc", true /*readonly*/, false /*no globals*/);
		kconfig.setGroup("X11");
		bool exportKDEColors = kconfig.readEntry("exportKDEColors", true);
		if (exportKDEColors)
			flags |= KRdbExportColors;
		runRdb( flags );
	}

	// Now allow KDE apps to reconfigure themselves.
	if ( m_bStyleDirty )
		KIPC::sendMessageAll(KIPC::StyleChanged);

	if ( m_bToolbarsDirty )
		// ##### FIXME - Doesn't apply all settings correctly due to bugs in
		// KApplication/KToolbar
		KIPC::sendMessageAll(KIPC::ToolbarStyleChanged);

	if (m_bEffectsDirty) {
		KIPC::sendMessageAll(KIPC::SettingsChanged);
#warning "kde4: port it ! Need to fix kwin"		
		//kapp->dcopClient()->send("kwin*", "", "reconfigure()", QByteArray());
	}

	//update kicker to re-used tooltips kicker parameter otherwise, it overwritted
	//by style tooltips parameters.
	QByteArray data;
#warning "kde4: who is org.kde.kicker?"
	QDBusInterface kicker( "org.kde.kicker", "/kicker");
	kicker.call("configure");

	// Clean up
	m_bEffectsDirty  = false;
	m_bToolbarsDirty = false;
	m_bStyleDirty    = false;
	emit changed( false );
}


bool KCMStyle::findStyle( const QString& str, int& combobox_item )
{
	StyleEntry* se   = styleEntries[str.toLower()];

	QString     name = se ? se->name : str;

	combobox_item = 0;

	//look up name
	for( int i = 0; i < cbStyle->count(); i++ )
	{
		if ( cbStyle->itemText(i) == name )
		{
			combobox_item = i;
			return true;
		}
	}

	return false;
}


void KCMStyle::defaults()
{
	// Select default style
	int item = 0;
	bool found;

	found = findStyle( "plastique", item ); //### KStyle::defaultStyle()
	if (!found)
		found = findStyle( "highcolor", item );
	if (!found)
		found = findStyle( "default", item );
	if (!found)
		found = findStyle( "windows", item );
	if (!found)
		found = findStyle( "platinum", item );
	if (!found)
		found = findStyle( "motif", item );

	cbStyle->setCurrentIndex( item );

	m_bStyleDirty = true;
	switchStyle( currentStyle() );	// make resets visible

	// Effects..
	cbEnableEffects->setChecked(false);
	comboTooltipEffect->setCurrentIndex(0);
	comboComboEffect->setCurrentIndex(0);
	comboMenuEffect->setCurrentIndex(0);
	comboMenuHandle->setCurrentIndex(0);
	comboMenuEffectType->setCurrentIndex(0);
	slOpacity->setValue(90);
	cbMenuShadow->setChecked(false);

	// Miscellaneous
	cbHoverButtons->setChecked(true);
	cbTransparentToolbars->setChecked(true);
	cbEnableTooltips->setChecked(true);
	comboToolbarIcons->setCurrentIndex(0);
	cbIconsOnButtons->setChecked(false);
	cbTearOffHandles->setChecked(false);
}

void KCMStyle::setEffectsDirty()
{
	m_bEffectsDirty = true;
	emit changed(true);
}

void KCMStyle::setToolbarsDirty()
{
	m_bToolbarsDirty = true;
	emit changed(true);
}

void KCMStyle::setStyleDirty()
{
	m_bStyleDirty = true;
	emit changed(true);
}

// ----------------------------------------------------------------
// All the Style Switching / Preview stuff
// ----------------------------------------------------------------

void KCMStyle::loadStyle( KConfig& config )
{
	cbStyle->clear();

	// Create a dictionary of WidgetStyle to Name and Desc. mappings,
	// as well as the config page info
	qDeleteAll(styleEntries);
	styleEntries.clear();

	QString strWidgetStyle;
	QStringList list = KGlobal::dirs()->findAllResources("themes", "*.themerc", true, true);
	for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
	{
		KSimpleConfig config( *it, true );
		if ( !(config.hasGroup("KDE") && config.hasGroup("Misc")) )
			continue;

		config.setGroup("KDE");

		strWidgetStyle = config.readEntry("WidgetStyle");
		if (strWidgetStyle.isNull())
			continue;

		// We have a widgetstyle, so lets read the i18n entries for it...
		StyleEntry* entry = new StyleEntry;
		config.setGroup("Misc");
		entry->name = config.readEntry("Name");
		entry->desc = config.readEntry("Comment", i18n("No description available."));
		entry->configPage = config.readEntry("ConfigPage", QString());

		// Check if this style should be shown
		config.setGroup("Desktop Entry");
		entry->hidden = config.readEntry("Hidden", false);

		// Insert the entry into our dictionary.
		styleEntries.insert(strWidgetStyle.toLower(), entry);
	}

	// Obtain all style names
	QStringList allStyles = QStyleFactory::keys();

	// Get translated names, remove all hidden style entries.
	QStringList styles;
	StyleEntry* entry;
	for (QStringList::iterator it = allStyles.begin(); it != allStyles.end(); it++)
	{
		QString id = (*it).toLower();
		// Find the entry.
		if ( (entry = styleEntries[id]) != 0 )
		{
			// Do not add hidden entries
			if (entry->hidden)
				continue;

			styles += entry->name;

			nameToStyleKey[entry->name] = id;
		}
		else
		{
			styles += (*it); //Fall back to the key (but in original case)
			nameToStyleKey[*it] = id;
		}
	}

	// Sort the style list, and add it to the combobox
	styles.sort();
	cbStyle->addItems( styles );

	// Find out which style is currently being used
	config.setGroup( "General" );
	QString defaultStyle = "plastique"; //### KDE4: FIXME KStyle::defaultStyle();
	QString cfgStyle = config.readEntry( "widgetStyle", defaultStyle );

	// Select the current style
	// Do not use cbStyle->listBox() as this may be NULL for some styles when
	// they use QPopupMenus for the drop-down list!

	// ##### Since Trolltech likes to seemingly copy & paste code,
	// QStringList::findItem() doesn't have a Qt::StringComparisonMode field.
	// We roll our own (yuck)
	cfgStyle = cfgStyle.toLower();
	int item = 0;
	for( int i = 0; i < cbStyle->count(); i++ )
	{
		QString id = nameToStyleKey[cbStyle->itemText(i)];
		item = i;
		if ( id == cfgStyle )	// ExactMatch
			break;
		else if ( id.contains( cfgStyle ) )
			break;
		else if ( id.contains( QApplication::style()->className() ) )
			break;
		item = 0;
	}
	cbStyle->setCurrentIndex( item );

	m_bStyleDirty = false;

	switchStyle( currentStyle() );	// make resets visible
}

QString KCMStyle::currentStyle()
{
	return nameToStyleKey[cbStyle->currentText()];
}


void KCMStyle::styleChanged()
{
	switchStyle( currentStyle() );
}


void KCMStyle::switchStyle(const QString& styleName, bool force)
{
	// Don't flicker the preview if the same style is chosen in the cb
	if (!force && appliedStyle && appliedStyle->objectName() == styleName)
		return;

	// Create an instance of the new style...
	QStyle* style = QStyleFactory::create(styleName);
	if (!style)
		return;

	// Prevent Qt from wrongly caching radio button images
	QPixmapCache::clear();

	setStyleRecursive( stylePreview, style );

	// this flickers, but reliably draws the widgets correctly.
	stylePreview->resize( stylePreview->sizeHint() );

	delete appliedStyle;
	appliedStyle = style;

	// Set the correct style description
	StyleEntry* entry = styleEntries[ styleName ];
	QString desc;
	desc = i18n("Description: %1", entry ? entry->desc : i18n("No description available.") );
	lblStyleDesc->setText( desc );
}

void KCMStyle::setStyleRecursive(QWidget* w, QStyle* s)
{
	// Don't let broken styles kill the palette
	// for other styles being previewed. (e.g SGI style)
	w->unsetPalette();

	QPalette newPalette(KApplication::createApplicationPalette());
	s->polish( newPalette );
	w->setPalette(newPalette);

	// Apply the new style.
	w->setStyle(s);

	// Recursively update all children.
	const QObjectList children = w->children();

	// Apply the style to each child widget.
    foreach (QObject* child, children)
	{
		if (child->isWidgetType())
			setStyleRecursive((QWidget *) child, s);
	}
}


// ----------------------------------------------------------------
// All the Effects stuff
// ----------------------------------------------------------------

void KCMStyle::loadEffects( KConfig& config )
{
	// Load effects.
	config.setGroup("KDE");

	cbEnableEffects->setChecked( config.readEntry( "EffectsEnabled", false) );

	if ( config.readEntry( "EffectAnimateCombo", false) )
		comboComboEffect->setCurrentIndex( 1 );
	else
		comboComboEffect->setCurrentIndex( 0 );

	if ( config.readEntry( "EffectAnimateTooltip", false) )
		comboTooltipEffect->setCurrentIndex( 1 );
	else if ( config.readEntry( "EffectFadeTooltip", false) )
		comboTooltipEffect->setCurrentIndex( 2 );
	else
		comboTooltipEffect->setCurrentIndex( 0 );

	if ( config.readEntry( "EffectAnimateMenu", false) )
		comboMenuEffect->setCurrentIndex( 1 );
	else if ( config.readEntry( "EffectFadeMenu", false) )
		comboMenuEffect->setCurrentIndex( 2 );
	else
		comboMenuEffect->setCurrentIndex( 0 );

	comboMenuHandle->setCurrentIndex(config.readEntry("InsertTearOffHandle", 0));

	// KStyle Menu transparency and drop-shadow options...
	QSettings settings;
	QString effectEngine = settings.readEntry("/KStyle/Settings/MenuTransparencyEngine", "Disabled" );

#ifdef HAVE_XRENDER
	if (effectEngine == "XRender") {
		comboMenuEffectType->setCurrentIndex(2);
		comboMenuEffect->setCurrentIndex(3);
	} else if (effectEngine == "SoftwareBlend") {
		comboMenuEffectType->setCurrentIndex(1);
		comboMenuEffect->setCurrentIndex(3);
#else
	if (effectEngine == "XRender" || effectEngine == "SoftwareBlend") {
		comboMenuEffectType->setCurrentIndex(1);	// Software Blend
		comboMenuEffect->setCurrentIndex(3);
#endif
	} else if (effectEngine == "SoftwareTint") {
		comboMenuEffectType->setCurrentIndex(0);
		comboMenuEffect->setCurrentIndex(3);
	} else
		comboMenuEffectType->setCurrentIndex(0);

	if (comboMenuEffect->currentIndex() != 3)	// If not translucency...
		menuPreview->setPreviewMode( MenuPreview::Tint );
	else if (comboMenuEffectType->currentIndex() == 0)
		menuPreview->setPreviewMode( MenuPreview::Tint );
	else
		menuPreview->setPreviewMode( MenuPreview::Blend );

	slOpacity->setValue( (int)(100 * settings.value("/KStyle/Settings/MenuOpacity", 0.90).toDouble()) );

	// Menu Drop-shadows...
	cbMenuShadow->setChecked( settings.value("/KStyle/Settings/MenuDropShadow", false).toBool() );

	if (cbEnableEffects->isChecked()) {
		containerFrame->setEnabled( true );
		menuContainer->setEnabled( comboMenuEffect->currentIndex() == 3 );
	} else {
		menuContainer->setEnabled( false );
		containerFrame->setEnabled( false );
	}

	m_bEffectsDirty = false;
}


void KCMStyle::menuEffectTypeChanged()
{
	MenuPreview::PreviewMode mode;

	if (comboMenuEffect->currentIndex() != 3)
		mode = MenuPreview::Tint;
	else if (comboMenuEffectType->currentIndex() == 0)
		mode = MenuPreview::Tint;
	else
		mode = MenuPreview::Blend;

	menuPreview->setPreviewMode(mode);

	m_bEffectsDirty = true;
}


void KCMStyle::menuEffectChanged()
{
	menuEffectChanged( cbEnableEffects->isChecked() );
	m_bEffectsDirty = true;
}


void KCMStyle::menuEffectChanged( bool enabled )
{
	if (enabled &&
		comboMenuEffect->currentIndex() == 3) {
		menuContainer->setEnabled(true);
	} else
		menuContainer->setEnabled(false);
	m_bEffectsDirty = true;
}


// ----------------------------------------------------------------
// All the Miscellaneous stuff
// ----------------------------------------------------------------

void KCMStyle::loadMisc( KConfig& config )
{
	// KDE's Part via KConfig
	config.setGroup("Toolbar style");
	cbHoverButtons->setChecked(config.readEntry("Highlighting", true));
	cbTransparentToolbars->setChecked(config.readEntry("TransparentMoving", true));

	QString tbIcon = config.readEntry("IconText", "IconOnly");
	if (tbIcon == "TextOnly")
		comboToolbarIcons->setCurrentIndex(1);
	else if (tbIcon == "IconTextRight")
		comboToolbarIcons->setCurrentIndex(2);
	else if (tbIcon == "IconTextBottom")
		comboToolbarIcons->setCurrentIndex(3);
	else
		comboToolbarIcons->setCurrentIndex(0);

	config.setGroup("KDE");
	cbIconsOnButtons->setChecked(config.readEntry("ShowIconsOnPushButtons", false));
	cbEnableTooltips->setChecked(!config.readEntry("EffectNoTooltip", false));
	cbTearOffHandles->setChecked(config.readEntry("InsertTearOffHandle", false));

	m_bToolbarsDirty = false;
}

void KCMStyle::addWhatsThis()
{
	// Page1
	cbStyle->setWhatsThis( i18n("Here you can choose from a list of"
							" predefined widget styles (e.g. the way buttons are drawn) which"
							" may or may not be combined with a theme (additional information"
							" like a marble texture or a gradient).") );
	stylePreview->setWhatsThis( i18n("This area shows a preview of the currently selected style "
							"without having to apply it to the whole desktop.") );

	// Page2
	page2->setWhatsThis( i18n("This page allows you to enable various widget style effects. "
							"For best performance, it is advisable to disable all effects.") );
	cbEnableEffects->setWhatsThis( i18n( "If you check this box, you can select several effects "
							"for different widgets like combo boxes, menus or tooltips.") );
	comboComboEffect->setWhatsThis( i18n( "<p><b>Disable: </b>do not use any combo box effects.</p>\n"
							"<b>Animate: </b>Do some animation.") );
	comboTooltipEffect->setWhatsThis( i18n( "<p><b>Disable: </b>do not use any tooltip effects.</p>\n"
							"<p><b>Animate: </b>Do some animation.</p>\n"
							"<b>Fade: </b>Fade in tooltips using alpha-blending.") );
	comboMenuEffect->setWhatsThis( i18n( "<p><b>Disable: </b>do not use any menu effects.</p>\n"
							"<p><b>Animate: </b>Do some animation.</p>\n"
							"<p><b>Fade: </b>Fade in menus using alpha-blending.</p>\n"
							"<b>Make Translucent: </b>Alpha-blend menus for a see-through effect. (KDE styles only)") );
	cbMenuShadow->setWhatsThis( i18n( "When enabled, all popup menus will have a drop-shadow, otherwise "
							"drop-shadows will not be displayed. At present, only KDE styles can have this "
							"effect enabled.") );
	comboMenuEffectType->setWhatsThis( i18n( "<p><b>Software Tint: </b>Alpha-blend using a flat color.</p>\n"
							"<p><b>Software Blend: </b>Alpha-blend using an image.</p>\n"
							"<b>XRender Blend: </b>Use the XFree RENDER extension for image blending (if available). "
							"This method may be slower than the Software routines on non-accelerated displays, "
							"but may however improve performance on remote displays.</p>\n") );
	slOpacity->setWhatsThis( i18n("By adjusting this slider you can control the menu effect opacity.") );

	// Page3
	page3->setWhatsThis( i18n("<b>Note:</b> that all widgets in this combobox "
							"do not apply to Qt-only applications.") );
	cbHoverButtons->setWhatsThis( i18n("If this option is selected, toolbar buttons will change "
							"their color when the mouse cursor is moved over them." ) );
	cbTransparentToolbars->setWhatsThis( i18n("If you check this box, the toolbars will be "
							"transparent when moving them around.") );
	cbEnableTooltips->setWhatsThis( i18n( "If you check this option, the KDE application "
							"will offer tooltips when the cursor remains over items in the toolbar." ) );
	comboToolbarIcons->setWhatsThis( i18n( "<p><b>Icons only:</b> Shows only icons on toolbar buttons. "
							"Best option for low resolutions.</p>"
							"<p><b>Text only: </b>Shows only text on toolbar buttons.</p>"
							"<p><b>Text alongside icons: </b> Shows icons and text on toolbar buttons. "
							"Text is aligned alongside the icon.</p>"
							"<b>Text under icons: </b> Shows icons and text on toolbar buttons. "
							"Text is aligned below the icon.") );
	cbIconsOnButtons->setWhatsThis( i18n( "If you enable this option, KDE Applications will "
							"show small icons alongside some important buttons.") );
	cbTearOffHandles->setWhatsThis( i18n( "If you enable this option some pop-up menus will "
							"show so called tear-off handles. If you click them, you get the menu "
							"inside a widget. This can be very helpful when performing "
							"the same action multiple times.") );
}

#include "kcmstyle.moc"

// vim: set noet ts=4:
