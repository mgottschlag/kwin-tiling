/**
 * kcmaccess.h
 *
 * Copyright (c) 2000 Matthias H�zer-Klpfel <hoelzer@kde.org>
 *
 */

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
  
  QCheckBox *gestures;
  QCheckBox *gestureConfirmation;
};


#endif
