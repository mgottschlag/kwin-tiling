//-----------------------------------------------------------------------------
//
// KDE xscreensaver configuration dialog
//
// Copyright (c)  Martin R. Jones 1999
//

#include <qlabel.h>
#include <qslider.h>
#include <qlayout.h>
#include "kxscontrol.h"

//===========================================================================
KXSRangeControl::KXSRangeControl(QWidget *parent, const QString &name,
                                  KConfig &config)
  : QWidget(parent), KXSRangeItem(name, config)
{
  QVBoxLayout *l = new QVBoxLayout(this);
  QLabel *label = new QLabel(mLabel, this);
  l->add(label);
  mSlider = new QSlider(mMinimum, mMaximum, 10, mValue, Qt::Horizontal, this);
  connect(mSlider, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)));
  l->add(mSlider);
}

void KXSRangeControl::slotValueChanged(int value)
{
  mValue = value;
  emit changed();
}

//===========================================================================
KXSCheckBoxControl::KXSCheckBoxControl(QWidget *parent, const QString &name,
                                      KConfig &config)
  : QCheckBox(parent), KXSBoolItem(name, config)
{
  setText(mLabel);
  setChecked(mValue);
  connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
}

void KXSCheckBoxControl::slotToggled(bool state)
{
  mValue = state;
  emit changed();
}

//===========================================================================
KXSDropListControl::KXSDropListControl(QWidget *parent, const QString &name,
                                      KConfig &config)
  : QWidget(parent), KXSSelectItem(name, config)
{
  QVBoxLayout *l = new QVBoxLayout(this);
  QLabel *label = new QLabel(mLabel, this);
  l->add(label);
  mCombo = new QComboBox(this);
  mCombo->insertStringList(mOptions);
  mCombo->setCurrentItem(mValue);
  connect(mCombo, SIGNAL(activated(int)), SLOT(slotActivated(int)));
  l->add(mCombo);
}

void KXSDropListControl::slotActivated(int indx)
{
  mValue = indx;
  emit changed();
}

