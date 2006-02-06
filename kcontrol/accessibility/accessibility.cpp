/****************************************************************************
  accessibility.cpp
  KDE Control Accessibility module to control Bell, Keyboard and ?Mouse?
  -------------------
  Copyright : (c) 2000 Matthias Hölzer-Klüpfel
  -------------------
  Original Author: Matthias Hölzer-Klüpfel
  Contributors: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 ****************************************************************************/

/****************************************************************************
 *                                                                          *
 *   This program is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation; either version 2 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 ****************************************************************************/

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>

#include <kaboutdata.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <knuminput.h>
#include <kurlrequester.h>
#include <ktoolinvocation.h>

#include "accessibility.moc"

typedef KGenericFactory<AccessibilityConfig, QWidget> AccessibilityFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_accessibility, AccessibilityFactory("kcmaccessibility") )

/**
 * This function checks if the kaccess daemon needs to be run
 * This function will be deprecated since the kaccess daemon will be part of kded
 */
// static bool needToRunKAccessDaemon( KConfig *config ){
//    KConfigGroup group( config, "Bell" );
//
//    if(!group.readEntry("SystemBell", true)){
//       return true;
//    }
//    if(group.readEntry("ArtsBell", false)){
//       return true;
//    }
//    if(group.readEntry("VisibleBell", false)){
//       return true;
//    }
//    return false; // don't need it
// }

AccessibilityConfig::AccessibilityConfig(QWidget *parent, const char *name, const QStringList &)
  : AccessibilityConfigWidget( parent, name){

   KAboutData *about =
   new KAboutData(I18N_NOOP("kcmaccessiblity"), I18N_NOOP("KDE Accessibility Tool"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 2000, Matthias Hoelzer-Kluepfel"));

   about->addAuthor("Matthias Hoelzer-Kluepfel", I18N_NOOP("Author") , "hoelzer@kde.org");
   about->addAuthor("José Pablo Ezequiel Fernández", I18N_NOOP("Author") , "pupeno@kde.org");
   setAboutData( about );

   kDebug() << "Running: AccessibilityConfig::AccessibilityConfig(QWidget *parent, const char *name, const QStringList &)" << endl;
   // TODO: set the KUrl Dialog to open just audio files
   connect( mainTab, SIGNAL(currentChanged(QWidget*)), this, SIGNAL(quickHelpChanged()) );
   load();
}


AccessibilityConfig::~AccessibilityConfig(){
   kDebug() << "Running: AccessibilityConfig::~AccessibilityConfig()" << endl;
}

void AccessibilityConfig::load(){
   kDebug() << "Running: AccessibilityConfig::load()" << endl;
   
   KConfig *bell = new KConfig("bellrc", true);
   
   bell->setGroup("General");
   systemBell->setChecked(bell->readEntry("SystemBell", false));
   customBell->setChecked(bell->readEntry("CustomBell", false));
   visibleBell->setChecked(bell->readEntry("VisibleBell", false));
   
   bell->setGroup("CustomBell");
   soundToPlay->setURL(bell->readPathEntry("Sound", ""));

   bell->setGroup("Visible");
   invertScreen->setChecked(bell->readEntry("Invert", true));
   flashScreen->setChecked(bell->readEntry("Flash", false));
   flashScreenColor->setColor(bell->readEntry("FlashColor", Qt::red));
   visibleBellDuration->setValue(bell->readEntry("Duration", 500));
  
   delete bell;   
//    KConfig *config = new KConfig("kaccessrc", true);
//
//    config->setGroup("Bell");
//
//    systemBell->setChecked(config->readEntry("SystemBell", true));
//    customBell->setChecked(config->readEntry("ArtsBell", false));
//    soundEdit->setText(config->readPathEntry("ArtsBellFile"));
//
//    visibleBell->setChecked(config->readEntry("VisibleBell", false));
//    invertScreen->setChecked(config->readEntry("VisibleBellInvert", true));
//    flashScreen->setChecked(!invertScreen->isChecked());
//    QColor def(Qt::red);
//    colorButton->setColor(config->readEntry("VisibleBellColor", &def));
//
//    durationSlider->setValue(config->readEntry("VisibleBellPause", 500));
//
//
//    config->setGroup("Keyboard");
//
//    stickyKeys->setChecked(config->readEntry("StickyKeys", false));
//    stickyKeysLock->setChecked(config->readEntry("StickyKeysLatch", true));
//    slowKeys->setChecked(config->readEntry("SlowKeys", false));
//    slowKeysDelay->setValue(config->readEntry("SlowKeysDelay", 500));
//    bounceKeys->setChecked(config->readEntry("BounceKeys", false));
//    bounceKeysDelay->setValue(config->readEntry("BounceKeysDelay", 500));
//
//
//    delete config;
//
//    checkAccess();
//
//    emit changed(false);
}


void AccessibilityConfig::save(){
   kDebug() << "Running: AccessibilityConfig::save()" << endl;
   
   KConfig *bell = new KConfig("bellrc");
   
   bell->setGroup("General");
   bell->writeEntry("SystemBell", systemBell->isChecked());
   bell->writeEntry("CustomBell", customBell->isChecked());
   bell->writeEntry("VisibleBell", visibleBell->isChecked());
   
   bell->setGroup("CustomBell");
   bell->writePathEntry("Sound", soundToPlay->url());

   bell->setGroup("Visible");
   bell->writeEntry("Invert", invertScreen->isChecked());
   bell->writeEntry("Flash", flashScreen->isChecked());
   bell->writeEntry("FlashColor", flashScreenColor->color());
   bell->writeEntry("Duration", visibleBellDuration->value());
   
   bell->sync();
   delete bell;
// 
// 
//    config->setGroup("Keyboard");
//
//    config->writeEntry("StickyKeys", stickyKeys->isChecked());
//    config->writeEntry("StickyKeysLatch", stickyKeysLock->isChecked());
//
//    config->writeEntry("SlowKeys", slowKeys->isChecked());
//    config->writeEntry("SlowKeysDelay", slowKeysDelay->value());
//
//    config->writeEntry("BounceKeys", bounceKeys->isChecked());
//    config->writeEntry("BounceKeysDelay", bounceKeysDelay->value());
//
//
//    config->sync();
//
//    if(systemBell->isChecked() || customBell->isChecked() || visibleBell->isChecked()){
//       KConfig cfg("kdeglobals", false, false);
//       cfg.setGroup("General");
//       cfg.writeEntry("UseSystemBell", true);
//       cfg.sync();
//    }
//
//
//    if( needToRunKAccessDaemon( config ) ){
//       KToolInvocation::startServiceByDesktopName("kaccess");
//    }else{
//    // don't need it -> kill it
//       DCOPRef kaccess( "kaccess", "qt/kaccess" );
//       kaccess.send( "quit" );
//    }
//
//    delete config;
//
//    emit changed(false);
}


void AccessibilityConfig::defaults(){
   kDebug() << "Running: AccessibilityConfig::defaults()" << endl;

   // Bell
   // Audibe Bell
   systemBell->setChecked(false);
   customBell->setChecked(false);
   soundToPlay->clear();

   // Visible Bell
   visibleBell->setChecked(false);
   invertScreen->setChecked(true);
   flashScreen->setChecked(false);
   flashScreenColor->setColor(QColor(Qt::red));
   visibleBellDuration->setValue(500);

   // Keyboard
   // Sticky Keys
   stickyKeys->setChecked(false);
   lockWithStickyKeys->setChecked(true);

   // Slow Keys
   slowKeys->setChecked(false);
   slowKeysDelay->setValue(500);

   // Bounce Keys
   bounceKeys->setChecked(false);
   bounceKeysDelay->setValue(500);

   // Mouse
   // Navigation
   accelerationDelay->setValue(160);
   repeatInterval->setValue(5);
   accelerationTime->setValue(1000);
   maximumSpeed->setValue(500);
   accelerationProfile->setValue(0);
}
