/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <stdlib.h>

#include <qwidget.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <kapp.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kwin.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconeffect.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <klocale.h>

#include <icons.h>

/**** DLL Interface ****/

extern "C" {
    KCModule *create_icons(QWidget *parent, const char *name) {
	KGlobal::locale()->insertCatalogue("kcmicons");
	return new KIconConfig(parent, name);
    }
}


/**** KIconConfig ****/

KIconConfig::KIconConfig(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    QGridLayout *top = new QGridLayout(this, 2, 2, 10, 10);
    top->setColStretch(0, 1);
    top->setColStretch(1, 1);

    // Use of Icon
    QGroupBox *gbox = new QGroupBox(i18n("Use of Icon"), this);
    top->addWidget(gbox, 0, 0);
    QBoxLayout *g_lay = new QVBoxLayout(gbox, 10, 10);
    mpUsageList = new QListBox(gbox);
    connect(mpUsageList, SIGNAL(highlighted(int)), SLOT(slotUsage(int)));
    g_lay->addSpacing(10);
    g_lay->addWidget(mpUsageList);

    // Effects
    gbox = new QGroupBox(i18n("Effects for States"), this);
    top->addWidget(gbox, 1, 0);
    g_lay = new QVBoxLayout(gbox, 10, 10);
    mpStateList = new QListBox(gbox);
    connect(mpStateList, SIGNAL(highlighted(int)), SLOT(slotState(int)));
    g_lay->addSpacing(10);
    g_lay->addWidget(mpStateList);

    QGridLayout *grid = new QGridLayout;
    grid->setColStretch(0, 2);
    grid->setColStretch(0, 1);
    g_lay->addLayout(grid);
    QLabel *lbl = new QLabel(i18n("&Effect"), gbox);
    grid->addWidget(lbl, 0, 0);
    mpEffectBox = new QComboBox(gbox);
    connect(mpEffectBox, SIGNAL(activated(int)), SLOT(slotEffect(int)));
    grid->addWidget(mpEffectBox, 0, 1);
    lbl->setBuddy(mpEffectBox);
    lbl = new QLabel(i18n("Value"), gbox);
    grid->addWidget(lbl, 1, 0);
    mpEffectSlider = new QSlider(0, 100, 5, 10, QSlider::Horizontal, gbox);
    connect(mpEffectSlider, SIGNAL(valueChanged(int)), SLOT(slotEffectValue(int)));
    grid->addWidget(mpEffectSlider, 1, 1);

    // Preview
    gbox = new QGroupBox(i18n("Preview"), this);
    top->addWidget(gbox, 0, 1);
    g_lay = new QVBoxLayout(gbox, 10, 10);
    mpPreview = new QLabel(gbox);
    mpPreview->setAlignment(AlignCenter);
    mpPreview->setMinimumSize(128, 128);
    g_lay->addSpacing(10);
    g_lay->addWidget(mpPreview);

    // Size
    gbox = new QGroupBox(i18n("Size"), this);
    top->addWidget(gbox, 1, 1);
    g_lay = new QVBoxLayout(gbox, 10, 10);
    QHBoxLayout *h_lay = new QHBoxLayout;
    g_lay->addSpacing(10);
    g_lay->addLayout(h_lay);
    lbl = new QLabel(i18n("Size"), gbox);
    h_lay->addWidget(lbl);
    mpSizeBox = new QComboBox(gbox);
    connect(mpSizeBox, SIGNAL(activated(int)), SLOT(slotSize(int)));
    h_lay->addWidget(mpSizeBox);
    mpDPCheck = new QCheckBox(i18n("Double sized pixels"), gbox);
    connect(mpDPCheck, SIGNAL(toggled(bool)), SLOT(slotDPCheck(bool)));
    g_lay->addWidget(mpDPCheck);

    init();
    read();
    apply();
    preview();
}

    
void KIconConfig::init()
{
    mpLoader = KGlobal::iconLoader();
    mpConfig = KGlobal::config();
    mpEffect = new KIconEffect;
    mpTheme = mpLoader->theme();
    mUsage = 0; mState = 0;

    // Fill list/checkboxen
    mpUsageList->insertItem(i18n("Desktop"));
    mpUsageList->insertItem(i18n("Toolbar"));
    mpUsageList->insertItem(i18n("Main Toolbar"));
    mpUsageList->insertItem(i18n("Small Icons"));

    mpStateList->insertItem(i18n("Default"));
    mpStateList->insertItem(i18n("Active"));
    mpStateList->insertItem(i18n("Disabled"));

    mpEffectBox->insertItem(i18n("No Effect"));
    mpEffectBox->insertItem(i18n("To Gray"));
    mpEffectBox->insertItem(i18n("Desaturate"));
    mpEffectBox->insertItem(i18n("Emboss"));

    // For reading the configuration
    mGroups += "Desktop";
    mGroups += "Toolbar";
    mGroups += "MainToolbar";
    mGroups += "Small";

    mStates += "Default";
    mStates += "Active";
    mStates += "Disabled";


}

void KIconConfig::read()
{
    for (int i=0; i<KIcon::LastGroup; i++)
	mAvSizes[i] = mpTheme->querySizes(i);

    mTheme = mpTheme->current();
    mExample = mpTheme->example();

    int i, j, effect;
    QStringList::ConstIterator it, it2;
    for (it=mGroups.begin(), i=0; it!=mGroups.end(); it++, i++)
    {
	mpConfig->setGroup(*it + "Icons");
	mSizes[i] = mpConfig->readNumEntry("Size");
	mbDP[i] = mpConfig->readBoolEntry("DoublePixels");
	for (it2=mStates.begin(), j=0; it2!=mStates.end(); it2++, j++)
	{
	    QString tmp = mpConfig->readEntry(*it2 + "Effect");
	    if (tmp == "togray")
		effect = KIconEffect::ToGray;
	    else if (tmp == "desaturate")
		effect = KIconEffect::DeSaturate;
	    else if (tmp == "emboss")
		effect = KIconEffect::Emboss;
	    else
		effect = KIconEffect::NoEffect;
	    mEffects[i][j] = effect;
	    mEffectValues[i][j] = mpConfig->readDoubleNumEntry(*it2 + "Value");
	}
    }

}

void KIconConfig::apply()
{
    mpUsageList->setCurrentItem(mUsage);
    mpStateList->setCurrentItem(mState);
    mpEffectBox->setCurrentItem(mEffects[mUsage][mState]);
    if (mEffects[mUsage][mState] == KIconEffect::DeSaturate)
    {
	mpEffectSlider->setEnabled(true);
	mpEffectSlider->setValue((int) (100.0 * mEffectValues[mUsage][mState] + 0.5));
    } else
    {
	mpEffectSlider->setEnabled(false);
	mpEffectSlider->setValue(0);
    }

    int delta = 1000, dw, index = -1, size, i;
    QValueList<int>::Iterator it;
    mpSizeBox->clear();
    for (it=mAvSizes[mUsage].begin(), i=0; it!=mAvSizes[mUsage].end(); it++, i++)
    {
	mpSizeBox->insertItem(QString().setNum(*it));
	dw = abs(mSizes[mUsage] - *it);
	if (dw < delta)
	{
	    delta = dw;
	    index = i;
	    size = *it;
	}
	    
    }
    if (index != -1)
    {
	mpSizeBox->setCurrentItem(index);
	mSizes[mUsage] = size; // best or exact match
    }
    mpDPCheck->setChecked(mbDP[mUsage]);
}

void KIconConfig::preview()
{
    // Apply effects ourselves because we don't want to sync 
    // the configuratio every preview.
    QPixmap pm = mpLoader->loadIcon(mExample, KIcon::NoGroup, mSizes[mUsage]);
    QImage img = pm.convertToImage();
    img = mpEffect->apply(img, mEffects[mUsage][mState],
	    mEffectValues[mUsage][mState]);
    if (mbDP[mUsage])
    {
	int w = img.width() * 2;
	img = img.smoothScale(w, w);
    }
    pm.convertFromImage(img);
    mpPreview->setPixmap(pm);
}

void KIconConfig::load()
{
    read();
    apply();
    emit changed(false);
}


void KIconConfig::save()
{
    int i, j;
    QStringList::ConstIterator it, it2;
    for (it=mGroups.begin(), i=0; it!=mGroups.end(); it++, i++)
    {
	mpConfig->setGroup(*it + "Icons");
	mpConfig->writeEntry("Size", mSizes[i], true, true);
	mpConfig->writeEntry("DoublePixels", mbDP[i], true, true);
	for (it2=mStates.begin(), j=0; it2!=mStates.end(); it2++, j++)
	{
	    QString tmp;
	    switch (mEffects[i][j])
	    {
	    case KIconEffect::ToGray:
		tmp = "togray";
		break;
	    case KIconEffect::DeSaturate:
		tmp = "desaturate";
		break;
	    case KIconEffect::Emboss:
		tmp = "emboss";
		break;
	    default:
		tmp = "none";
		break;
	    }
	    mpConfig->writeEntry(*it2 + "Effect", tmp, true, true);
	    mpConfig->writeEntry(*it2 + "Value", mEffectValues[i][j], true, true);
	}
    }
    mpConfig->sync();
    emit changed(false);
}


void KIconConfig::defaults()
{
    for (int i=0; i<KIcon::LastGroup; i++)
    {
	mSizes[i] = mpTheme->defaultSize(i);
	mbDP[i] = false;
	for (int j=0; j<KIcon::LastState; j++)
	{
	    mEffects[i][j] = KIconEffect::NoEffect;
	    mEffectValues[i][j] = 0.0;
	}
    }
    apply();
    preview();
    emit changed(true);
}

void KIconConfig::slotUsage(int index)
{
    mUsage = index;
    mState = 0;
    apply();
    preview();
    emit changed(true);
}

void KIconConfig::slotState(int index)
{
    mState = index;
    mpEffectBox->setCurrentItem(mEffects[mUsage][mState]);
    mpEffectSlider->setValue((int) (100.0 * mEffectValues[mUsage][mState] + 0.5));
    preview();
    emit changed(true);
}

void KIconConfig::slotEffect(int index)
{
    mEffects[mUsage][mState] = index;
    if (mEffects[mUsage][mState] == KIconEffect::DeSaturate)
    {
	mpEffectSlider->setValue((int) (100.0 * mEffectValues[mUsage][mState] + 0.5));
	mpEffectSlider->setEnabled(true);
    } else 
    {
	mpEffectSlider->setEnabled(false);
	mpEffectSlider->setValue(0);
    }
    preview();
    emit changed(true);
}

void KIconConfig::slotEffectValue(int value)
{
    mEffectValues[mUsage][mState] = 0.01 * value;
    preview();
    emit changed(true);
}

void KIconConfig::slotSize(int index)
{
    kdDebug(264) << "Index: " << index << "\n";
    mSizes[mUsage] = mAvSizes[mUsage][index];
    preview();
    emit changed(true);
}

void KIconConfig::slotDPCheck(bool check)
{
    mbDP[mUsage] = check;
    preview();
    emit changed(true);
}

#include "icons.moc"
