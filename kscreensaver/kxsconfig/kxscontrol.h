//-----------------------------------------------------------------------------
//
// KDE xscreensaver configuration dialog
//
// Copyright (c)  Martin R. Jones 1999
//

#ifndef __KXSCONTROL_H__
#define __KXSCONTROL_H__

#include <qwidget.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include "kxsitem.h"

class QLabel;
class QSlider;

//===========================================================================
class KXSRangeControl : public QWidget, public KXSRangeItem
{
  Q_OBJECT
public:
  KXSRangeControl(QWidget *parent, const QString &name, KConfig &config);

signals:
  void changed();

protected slots:
  void slotValueChanged(int value);

protected:
  QSlider *mSlider;
};

//===========================================================================
class KXSCheckBoxControl : public QCheckBox, public KXSBoolItem
{
  Q_OBJECT
public:
  KXSCheckBoxControl(QWidget *parent, const QString &name, KConfig &config);

signals:
  void changed();

protected slots:
  void slotToggled(bool);
};

//===========================================================================
class KXSDropListControl : public QWidget, public KXSSelectItem
{
  Q_OBJECT
public:
  KXSDropListControl(QWidget *parent, const QString &name, KConfig &config);

signals:
  void changed();

protected slots:
  void slotActivated(int);

protected:
  QComboBox *mCombo;
};

#endif

