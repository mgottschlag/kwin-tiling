/****************************************************************************
  accessibility.h
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

// $Id$

#ifndef _ACCESSIBILITY_H_
#define _ACCESSIBILITY_H_

// #include <kcmodule.h>
#include <kdebug.h>

#include "accessibilityconfigwidget.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class KColorButton;
class QSlider;
class KNumInput;
class KAboutData;

class AccessibilityConfig : public AccessibilityConfigWidget {
   Q_OBJECT

   public:
      /**
       * Constructor
       */   
      AccessibilityConfig(QWidget *parent = 0L, const char *name = 0L, const QStringList &foo = QStringList());

      /**
       * Destructor
       */            
      virtual ~AccessibilityConfig();
  
      /**
       * This method is invoked whenever the module should read its 
       * configuration (most of the times from a config file) and update the 
       * user interface. This happens when the user clicks the "Reset" button in 
       * the control center, to undo all of his changes and restore the currently 
       * valid settings. NOTE that this is not called after the modules is loaded,
       * so you probably want to call this method in the constructor.
       */
      void load();

      /**
       * This function gets called when the user wants to save the settings in 
       * the user interface, updating the config files or wherever the 
       * configuration is stored. The method is called when the user clicks "Apply" 
       * or "Ok".
       */
      void save();

      /**
       * This function is called to set the settings in the module to sensible
       * default values. It gets called when hitting the "Default" button. The 
       * default values should probably be the same as the ones the application 
       * uses when started without a config file.
       */
      void defaults();

      /**
       * This function returns the small quickhelp.
       * That is displayed in the sidebar in the KControl
       */
      QString quickHelp() const;            

      const KAboutData* aboutData() const;

   protected slots:
      void checkAccess();
      void invertClicked();
      void flashClicked();
      void selectSound();
      void changeFlashScreenColor();

   private:
//       QCheckBox *systemBell, *customBell, *visibleBell;
//       QRadioButton *invertScreen, *flashScreen;
//       QLabel    *soundLabel, *colorLabel;
//       QLineEdit *soundEdit;
//       QPushButton *soundButton;
//       KColorButton *colorButton;
//       KIntNumInput *durationSlider;
// 
//       QCheckBox *stickyKeys, *stickyKeysLock;
// 
//       QCheckBox *slowKeys, *bounceKeys;    
//       KIntNumInput *slowKeysDelay, *bounceKeysDelay;

   private slots:
      /**
       * This slot is used to emit the signal changed when any widget changes the configuration 
       */
      void configChanged(){
         kdDebug() << "Running: AccessibilityConfig::configChanged()"<< endl;
         emit changed(true);
      };
};

#endif // _ACCESSIBILITY_H_
