/***************************************************************************
                          kospage.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qmap.h>

#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kipc.h>
#include <krun.h>
#include <kkeynative.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <ktoolinvocation.h>

#include "kospage.h"

KOSPage::KOSPage(QWidget *parent, const char *name ) : KOSPageDlg(parent,name) {
	px_osSidebar->setPixmap(UserIcon("step2.png"));
	// initialize the textview with the default description - KDE of course
	slotKDEDescription();
	// Set the configfiles
	cglobal = new KConfig("kdeglobals");
	claunch = new KConfig("klaunchrc", false, false);
	cwin = new KConfig("kwinrc");
	cdesktop = new KConfig("kdesktoprc");
	ckcminput = new KConfig("kcminputrc");
	ckcmdisplay = new KConfig("kcmdisplayrc");
	ckonqueror = new KConfig("konquerorrc");
	cklipper = new KConfig("klipperrc", false, false);
	ckaccess = new KConfig("kaccessrc");
	// Save the current user defaults
	getUserDefaults();
	// set default-selections for this page
	setDefaults();
}

KOSPage::~KOSPage(){
	delete cglobal;
	delete claunch;
	delete cwin;
	delete cdesktop;
	delete ckcmdisplay;
	delete ckcminput;
	delete ckonqueror;
	delete cklipper;
	delete ckaccess;
}


void KOSPage::save(bool currSettings){
	kDebug() << "KOSPage::save()" << endl;
	// save like we want. Just set the Radiobutton to either how it is set in the dialog (currSettings=true, default)
	// or, if false, take the settings we got in getUserDefaults()
	saveCheckState(currSettings);
	// sync all configs
	cglobal->sync();
	claunch->sync();
	cwin->sync();
	cdesktop->sync();
	ckcmdisplay->sync();
	ckcminput->sync();
	ckonqueror->sync();
	cklipper->sync();
	ckaccess->sync();
	///////////////////////////////////////////
	// kcmdisplay changes
	KIPC::sendMessageAll(KIPC::SettingsChanged);
	QApplication::syncX();
	// enable/disable the mac menu, call dcop
	// Tell kdesktop about the new config file
	kapp->dcopClient()->send("kdesktop", "KDesktopIface", "configure()", QByteArray());
	///////////////////////////////////////////
	/// restart kwin  for window effects
	kapp->dcopClient()->send("kwin*", "", "reconfigure()", QByteArray(""));
	///////////////////////////////////////////
	
	// Make the kaccess daemon read the changed config file
	KToolInvocation::startServiceByDesktopName("kaccess");
}


	/** called by save() -- currSettings= true -> take the radiobutton, otherwise take user values set */
void KOSPage::saveCheckState(bool currSettings){
	if(currSettings){
		// Set the path for the keysscheme resource files
		KGlobal::dirs()->addResourceType("keys", KStandardDirs::kde_default("data")+"kcmkeys");
		// write the settings to the configfiles, depending on wich radiobutton is checked
		if(rb_kde->isChecked()){
			writeKDE();
			emit selectedOS("KDE");	// send a signal to be caught by the KStylePage to set the according style by default depending on the OS selection
		}
		else if(rb_unix->isChecked()){
			writeUNIX();
			emit selectedOS("CDE");	// send a signal to be caught by the KStylePage to set the according style by default depending on the OS selection
		}
		else if(rb_windows->isChecked()){
			writeWindows();
			emit selectedOS("win");	// send a signal to be caught by the KStylePage to set the according style by default depending on the OS selection
		}
		else if(rb_mac->isChecked()){
			writeMacOS();
			emit selectedOS("mac");	// send a signal to be caught by the KStylePage to set the according style by default depending on the OS selection
		}

		ckaccess->setGroup("Keyboard");
		ckaccess->writeEntry("Gestures", cb_gestures->isChecked(), KConfigBase::Persistent|KConfigBase::Global);

	}
	else {  // User has pressed "cancel & dismiss", so his old settings are written back
		writeUserDefaults();
	}
}


	/** write the settings for KDE-Behavior (called by saveCheckStatus) */
void KOSPage::writeKDE(){
	kDebug() << "KOSPage::writeKDE()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", false, KConfigBase::Persistent|KConfigBase::Global);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", true, KConfigBase::Persistent|KConfigBase::Global);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", true);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", "Shade");
	cwin->writeEntry("FocusPolicy", "ClickToFocus");
	cwin->writeEntry("AltTabStyle", "KDE");
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", "Lower");
	cwin->writeEntry("CommandActiveTitlebar3", "Operations menu");

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", false);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", "WindowListMenu");
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", true);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", true);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", true, KConfigBase::Persistent|KConfigBase::Global);

	cklipper->setGroup("General");
	cklipper->writeEntry("SynchronizeClipboards", false);

	writeKeyEntrys(locate("keys", "kde3.kksrc"));
}


	/** write the settings for fvwm-like-behavior (called by saveCheckStatus) */
void KOSPage::writeUNIX(){
	kDebug() << "KOSPage::writeUNIX()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", false, KConfigBase::Persistent|KConfigBase::Global);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", true, KConfigBase::Persistent|KConfigBase::Global);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", false);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", "Shade");
	cwin->writeEntry("FocusPolicy", "FocusStrictlyUnderMouse");
	cwin->writeEntry("AltTabStyle", "CDE");
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", "Operations menu");
	cwin->writeEntry("CommandActiveTitlebar3", "Lower");

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", false);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", "AppMenu");
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", false);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", false);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", false, KConfigBase::Persistent|KConfigBase::Global);

	cklipper->setGroup("General");
	cklipper->writeEntry("SynchronizeClipboards", true);

	writeKeyEntrys(locate("keys", "unix3.kksrc"));
}


	/** write the settings for windows-like-behavior (called by saveCheckStatus) */
void KOSPage::writeWindows(){
	kDebug() << "KOSPage::writeWindows()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", false, KConfigBase::Persistent|KConfigBase::Global);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", false, KConfigBase::Persistent|KConfigBase::Global);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", true);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", "Maximize");
	cwin->writeEntry("FocusPolicy", "ClickToFocus");
	cwin->writeEntry("AltTabStyle", "KDE");
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", "Lower");
	cwin->writeEntry("CommandActiveTitlebar3", "Operations menu");

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", false);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", "WindowListMenu");
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", false);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", false);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", false, KConfigBase::Persistent|KConfigBase::Global);

	cklipper->setGroup("General");
	cklipper->writeEntry("SynchronizeClipboards", false);

	// set the schemefile depending on whether or not the keyboard has got Metakeys.
	if( KKeyNative::keyboardHasWinKey() ) {
		writeKeyEntrys(locate("keys", "win4.kksrc"));
	}
	else {
		writeKeyEntrys(locate("keys", "win3.kksrc"));
	}
}


	/** write the settings for MacOS-like-behavior (called by saveCheckStatus) */
void KOSPage::writeMacOS(){
	kDebug() << "KOSPage::writeMacOS()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", true, KConfigBase::Persistent|KConfigBase::Global);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", true, KConfigBase::Persistent|KConfigBase::Global);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", false);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", "Shade");
	cwin->writeEntry("FocusPolicy", "ClickToFocus");
	cwin->writeEntry("AltTabStyle", "KDE");
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", "Lower");
	cwin->writeEntry("CommandActiveTitlebar3", "Operations menu");

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", true);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", "WindowListMenu");
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", false);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", false);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", true, KConfigBase::Persistent|KConfigBase::Global);

	cklipper->setGroup("General");
	cklipper->writeEntry("SynchronizeClipboards", false);

	writeKeyEntrys(locate("keys", "mac4.kksrc"));
}


	/** write Keyscheme to kdeglobals (called by saveCheckState) */
void KOSPage::writeKeyEntrys(QString keyfile){
	kDebug() << "KOSPage::writeKeyEntrys()" << endl;

	// load the given .kksrc - file
	KSimpleConfig* scheme = new KSimpleConfig(keyfile, true);
	// load the default .kksrc - file
	KSimpleConfig* defScheme = new KSimpleConfig(locate("keys", "kde3.kksrc"), true);

	// we need the entries from the default - file, so we can compare with them
	QMap<QString, QString> defMap = defScheme->entryMap("Global Shortcuts");
	// first delete the group in kdeglobals, then write the non-default entries from the global .kksrc - file
	cglobal->deleteGroup("Global Shortcuts", KConfigBase::Global);
	// get the Global - Shortcuts and write them to kdeglobals
	cglobal->setGroup("Global Shortcuts");
	QMap<QString, QString> givenMap = scheme->entryMap("Global Shortcuts");
	for ( QMap<QString, QString>::Iterator it = givenMap.begin(); it != givenMap.end(); ++it ) {
		if ( (defMap[it.key()] == it.value()) && (it.value() != "none") ) {
			cglobal->writeEntry(it.key(), "default("+it.value()+")", KConfigBase::Persistent|KConfigBase::Global);
		} else {
			cglobal->writeEntry(it.key(), it.value(), KConfigBase::Persistent|KConfigBase::Global);
		}
	}

	// we need the entries from the default - file, so we can compare with them
	defMap = defScheme->entryMap("Shortcuts");
	// first delete the group in kdeglobals, then write the non-default entries from the global .kksrc - file
	cglobal->deleteGroup("Shortcuts", KConfigBase::Global);
	cglobal->setGroup("Shortcuts");
	givenMap = scheme->entryMap("Shortcuts");
	for ( QMap<QString, QString>::Iterator it = givenMap.begin(); it != givenMap.end(); ++it ) {
		// only write the entry, if it defers from kde3.kksrc
		if ( defMap[it.key()] != it.value() ) {
			cglobal->writeEntry(it.key(), it.value(), KConfigBase::Persistent|KConfigBase::Global);
		}
	}

	delete scheme;
	delete defScheme;
}

void KOSPage::slotKDEDescription(){
	kDebug() << "slotKDEDescription()" << endl;
	textview_ospage->setText("");
	textview_ospage->setText(i18n(
	"<b>Window activation:</b> <i>Focus on click</i><br>"
	"<b>Titlebar double-click:</b> <i>Shade window</i><br>"
	"<b>Mouse selection:</b> <i>Single click</i><br>"
	"<b>Application startup notification:</b> <i>busy cursor</i><br>"
	"<b>Keyboard scheme:</b> <i>KDE default</i><br>"
	));
}

void KOSPage::slotUnixDescription(){
	kDebug() << "slotUnixDescription()" << endl;
	textview_ospage->setText("" );
	textview_ospage->setText(i18n(
	"<b>Window activation:</b> <i>Focus follows mouse</i><br>"
	"<b>Titlebar double-click:</b> <i>Shade window</i><br>"
	"<b>Mouse selection:</b> <i>Single click</i><br>"
	"<b>Application startup notification:</b> <i>none</i><br>"
	"<b>Keyboard scheme:</b> <i>UNIX</i><br>"
	));
}

void KOSPage::slotWindowsDescription(){
	kDebug() << "slotWindowsDescription()" << endl;
	textview_ospage->setText("");
	textview_ospage->setText(i18n(
	"<b>Window activation:</b> <i>Focus on click</i><br>"
	"<b>Titlebar double-click:</b> <i>Maximize window</i><br>"
	"<b>Mouse selection:</b> <i>Double click</i><br>"
	"<b>Application startup notification:</b> <i>busy cursor</i><br>"
	"<b>Keyboard scheme:</b> <i>Windows</i><br>"
	));
}

void KOSPage::slotMacDescription(){
	kDebug() << "slotMacDescription()" << endl;
	textview_ospage->setText("");
	textview_ospage->setText(i18n(
	"<b>Window activation:</b> <i>Focus on click</i><br>"
	"<b>Titlebar double-click:</b> <i>Shade window</i><br>"
	"<b>Mouse selection:</b> <i>Single click</i><br>"
	"<b>Application startup notification:</b> <i>none</i><br>"
	"<b>Keyboard scheme:</b> <i>Mac</i><br>"
	));
}


/** retrieves the user's local values. In case he doesn't have these set, use the default values of KDE */
void KOSPage::getUserDefaults(){
	ckcmdisplay->setGroup("KDE");
	b_MacMenuBar = ckcmdisplay->readEntry("macStyle", QVariant(false)).toBool();

	cglobal->setGroup("KDE");
	b_SingleClick = cglobal->readEntry("SingleClick", QVariant(true)).toBool();

	claunch->setGroup("FeedbackStyle");
	b_BusyCursor = claunch->readEntry("BusyCursor", QVariant(true)).toBool();

	cwin->setGroup("Windows");
	s_TitlebarDCC = cwin->readEntry("TitlebarDoubleClickCommand", "Shade");
	s_FocusPolicy = cwin->readEntry("FocusPolicy", "ClickToFocus");
	s_AltTabStyle = cwin->readEntry("AltTabStyle", "KDE");
	cwin->setGroup( "MouseBindings");
	s_TitlebarMMB = cwin->readEntry("CommandActiveTitlebar2", "Lower");
	s_TitlebarRMB = cwin->readEntry("CommandActiveTitlebar3", "Operations menu");

	cdesktop->setGroup( "Menubar" );
	b_ShowMenuBar = cdesktop->readEntry("ShowMenubar", QVariant(false)).toBool();
	cdesktop->setGroup( "Mouse Buttons" );
	s_MMB = cdesktop->readEntry("Middle", "WindowListMenu");
	cdesktop->setGroup( "FMSettings" );
	b_DesktopUnderline = cdesktop->readEntry("UnderlineLinks", QVariant(true)).toBool();

	ckonqueror->setGroup( "FMSettings" );
	b_KonqUnderline = ckonqueror->readEntry("UnderlineLinks", QVariant(true)).toBool();

	ckcminput->setGroup("KDE");
	b_ChangeCursor = ckcminput->readEntry("ChangeCursor", QVariant(true)).toBool();

	cklipper->setGroup("General");
	b_syncClipboards = cklipper->readEntry("SynchronizeClipboards", QVariant(false)).toBool();

	map_GlobalUserKeys = cglobal->entryMap("Global Shortcuts");
	map_AppUserKeys = cglobal->entryMap("Shortcuts");

	ckaccess->setGroup("Keyboard");
	b_Gestures = ckaccess->readEntry("Gestures", QVariant(true)).toBool();
}


	/** writes the user-defaults back */
void KOSPage::writeUserDefaults(){
	kDebug() << "KOSPage::writeUserDefaults()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", b_MacMenuBar, KConfigBase::Persistent|KConfigBase::Global);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", b_SingleClick, KConfigBase::Persistent|KConfigBase::Global);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", b_BusyCursor);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", s_TitlebarDCC);
	cwin->writeEntry("FocusPolicy", s_FocusPolicy);
	cwin->writeEntry("AltTabStyle", s_AltTabStyle);
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", s_TitlebarMMB);
	cwin->writeEntry("CommandActiveTitlebar3", s_TitlebarRMB);

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", b_ShowMenuBar);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", s_MMB);
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", b_DesktopUnderline);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", b_KonqUnderline);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", b_ChangeCursor, KConfigBase::Persistent|KConfigBase::Global);

	cklipper->setGroup("General");
	cklipper->writeEntry("SynchronizeClipboards", b_syncClipboards);

	ckaccess->setGroup("Keyboard");
	ckaccess->writeEntry("Gestures", b_Gestures, KConfigBase::Persistent|KConfigBase::Global);

	writeUserKeys();
}

	/** called by writeUserDefaults() */
void KOSPage::writeUserKeys(){
	kDebug() << "KOSPage::writeUserKeys()" << endl;

	cglobal->setGroup("Global Shortcuts");
	QMap<QString, QString>::Iterator it;	
	for ( it = map_GlobalUserKeys.begin(); it != map_GlobalUserKeys.end(); ++it ) {
		cglobal->writeEntry(it.key(), it.value(), KConfigBase::Persistent|KConfigBase::Global);
	}

	cglobal->deleteGroup("Shortcuts", KConfigBase::Global);
	cglobal->setGroup("Shortcuts");
	for ( it = map_AppUserKeys.begin(); it != map_AppUserKeys.end(); ++it ) {
		cglobal->writeEntry(it.key(), it.value(), KConfigBase::Normal|KConfigBase::Global);
	}
}


	/** resets the radio button selected to kde */
void KOSPage::setDefaults(){
    rb_kde->setChecked(true);
    cb_gestures->setChecked(false);
}

#include "kospage.moc"

