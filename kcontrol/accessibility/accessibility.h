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

#ifndef __kcmaccess_h__
#define __kcmaccess_h__


#include <kcmodule.h>


class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class KColorButton;
class QSlider;
class KNumInput;
class KAboutData;

class KAccessConfig : public KCModule
{
  Q_OBJECT

public:

  KAccessConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KAccessConfig();
  
  void load();
  void save();
  void defaults();
  const KAboutData* aboutData() const;
  

protected slots:

  void configChanged();
  void checkAccess();
  void invertClicked();
  void flashClicked();
  void selectSound();
  void changeFlashScreenColor();

      
private:

  QCheckBox *systemBell, *customBell, *visibleBell;
  QRadioButton *invertScreen, *flashScreen;
  QLabel    *soundLabel, *colorLabel;
  QLineEdit *soundEdit;
  QPushButton *soundButton;
  KColorButton *colorButton;
  KIntNumInput *durationSlider;

  QCheckBox *stickyKeys, *stickyKeysLock;

  QCheckBox *slowKeys, *bounceKeys;    
  KIntNumInput *slowKeysDelay, *bounceKeysDelay;

};


#endif
