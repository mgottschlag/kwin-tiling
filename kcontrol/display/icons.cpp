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
#include <qpushbutton.h>
#include <qiconview.h>

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
#include <kipc.h>
#include <kdialog.h>
#include <kcolorbtn.h>

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
    QGridLayout *top = new QGridLayout(this, 2, 2, 
				       KDialog::marginHint(),
				       KDialog::spacingHint());
    top->setColStretch(0, 1);
    top->setColStretch(2, 1);

    // Use of Icon
    QGroupBox *gbox = new QGroupBox(i18n("Use of Icon"), this);
    top->addWidget(gbox, 0, 0);
    QBoxLayout *g_lay = new QVBoxLayout(gbox, 
					KDialog::marginHint(),
					KDialog::spacingHint());
    g_lay->addSpacing(fontMetrics().lineSpacing());
    mpUsageList = new QListBox(gbox);
    connect(mpUsageList, SIGNAL(highlighted(int)), SLOT(slotUsage(int)));
    g_lay->addWidget(mpUsageList);

    // Effects
    gbox = new QGroupBox(i18n("Effects for States"), this);
    top->addWidget(gbox, 1, 0);
    g_lay = new QVBoxLayout(gbox, KDialog::marginHint(),
			    KDialog::spacingHint());
    mpStateList = new QListBox(gbox);
    connect(mpStateList, SIGNAL(highlighted(int)), SLOT(slotState(int)));
    g_lay->addSpacing(fontMetrics().lineSpacing());
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
    g_lay = new QVBoxLayout(gbox, KDialog::marginHint(), 0);
    g_lay->addSpacing(fontMetrics().lineSpacing());
    mpPreview = new QIconView(gbox);
    mpPreview->setGridX(70);
    mpPreview->setItemsMovable(false);
    mpPreview->setMinimumSize(128, 128);
    g_lay->addWidget(mpPreview);

    mpPreviewItem = new QIconViewItem(mpPreview, 
				      i18n("Very long filename.ext"));

    gbox = new QGroupBox(i18n("Size"), this);
    top->addWidget(gbox, 1, 1);
    g_lay = new QVBoxLayout(gbox, KDialog::marginHint(),
			    KDialog::spacingHint());
    g_lay->addSpacing(fontMetrics().lineSpacing());
    QHBoxLayout *h_lay = new QHBoxLayout;
    g_lay->addLayout(h_lay);
    lbl = new QLabel(i18n("Size"), gbox);
    h_lay->addWidget(lbl);
    mpSizeBox = new QComboBox(gbox);
    connect(mpSizeBox, SIGNAL(activated(int)), SLOT(slotSize(int)));
    h_lay->addWidget(mpSizeBox);
    mpDPCheck = new QCheckBox(i18n("Double sized pixels"), gbox);
    connect(mpDPCheck, SIGNAL(toggled(bool)), SLOT(slotDPCheck(bool)));
    g_lay->addWidget(mpDPCheck);

    // Font
    gbox = new QGroupBox(i18n("Labels"), this);
    top->addWidget(gbox, 2, 0);
    g_lay = new QVBoxLayout(gbox, KDialog::marginHint(),
			    KDialog::spacingHint());
    g_lay->addSpacing(fontMetrics().lineSpacing());

    QPushButton *fontButton = new QPushButton(i18n("Font..."), gbox);
    g_lay->addWidget(fontButton);

    h_lay = new QHBoxLayout;
    g_lay->addLayout(h_lay);

    lbl = new QLabel(i18n("Background Color:"), gbox);
    h_lay->addWidget(lbl);
    KColorButton *colorButton = new KColorButton(bgColor, gbox);
    h_lay->addWidget(colorButton);
    connect(colorButton, SIGNAL(changed(const QColor &)),
	    SLOT(slotBgColorChanged(const QColor &)));

    h_lay = new QHBoxLayout;
    g_lay->addLayout(h_lay);

    lbl = new QLabel(i18n("Normal Color:"), gbox);
    h_lay->addWidget(lbl);
    colorButton = new KColorButton(normalColor, gbox);
    h_lay->addWidget(colorButton);
    connect(colorButton, SIGNAL(changed(const QColor &)),
	    SLOT(slotNormalColorChanged(const QColor &)));

    h_lay = new QHBoxLayout;
    g_lay->addLayout(h_lay);

    lbl = new QLabel(i18n("Highlighted Color:"), gbox);
    h_lay->addWidget(lbl);
    colorButton = new KColorButton(hiliteColor, gbox);
    h_lay->addWidget(colorButton);
    connect(colorButton, SIGNAL(changed(const QColor &)),
	    SLOT(slotHiliteColorChanged(const QColor &)));

    wordWrapCB = new QCheckBox(i18n("&Word-wrap Text"), gbox);
    g_lay->addWidget(wordWrapCB);
    connect(wordWrapCB, SIGNAL(toggled(bool)),
	    SLOT(slotWrap(bool)));

    underlineCB = new QCheckBox(i18n("&Underline Text"), gbox);
    g_lay->addWidget(underlineCB);
    connect(underlineCB, SIGNAL(toggled(bool)),
	    SLOT(slotUnderline(bool)));

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
    for (int i=0; i<KIcon::LastGroup; i++)
	mbChanged[i] = false;

    // Fill list/checkboxen
    mpUsageList->insertItem(i18n("Desktop / File Manager"));
    mpUsageList->insertItem(i18n("Toolbar"));
    mpUsageList->insertItem(i18n("Main Toolbar"));
    mpUsageList->insertItem(i18n("Small Icons"));

    mpStateList->insertItem(i18n("Default"));
    mpStateList->insertItem(i18n("Active"));
    mpStateList->insertItem(i18n("Disabled"));

    mpEffectBox->insertItem(i18n("No Effect"));
    mpEffectBox->insertItem(i18n("To Gray"));
    mpEffectBox->insertItem(i18n("Desaturate"));
    mpEffectBox->insertItem(i18n("SemiTransparent"));
    mpEffectBox->insertItem(i18n("SemiTransparent/To Gray"));

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
	mSizes[i] = mpConfig->readNumEntry("Size", mpTheme->defaultSize(i));
	mbDP[i] = mpConfig->readBoolEntry("DoublePixels");
	mEffects[i][0] = KIconEffect::NoEffect;
	mEffects[i][1] = KIconEffect::NoEffect;
	mEffects[i][2] = KIconEffect::SemiTransparent;
	for (it2=mStates.begin(), j=0; it2!=mStates.end(); it2++, j++)
	{
	    QString tmp = mpConfig->readEntry(*it2 + "Effect");
	    if (tmp == "togray")
		effect = KIconEffect::ToGray;
	    else if (tmp == "desaturate")
		effect = KIconEffect::DeSaturate;
	    else if (tmp == "semitransparent")
		effect = KIconEffect::SemiTransparent;
	    else if (tmp == "semigray")
		effect = KIconEffect::SemiGray;
	    else if (tmp == "none")
		effect = KIconEffect::NoEffect;
	    else
		continue;
	    mEffects[i][j] = effect;
	    mEffectValues[i][j] = mpConfig->readDoubleNumEntry(*it2 + "Value");
	}
    }

    mpConfig->setGroup("Icon Labels");
    wrap = mpConfig->readBoolEntry("WordWrap", true);
    underline = mpConfig->readBoolEntry("Underline", true);
    
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

    wordWrapCB->setChecked(wrap);
    underlineCB->setChecked(underline);
}

void KIconConfig::preview()
{
    // Apply effects ourselves because we don't want to sync 
    // the configuration every preview.
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
    mpPreviewItem->setPixmap(pm);

    QFont fnt = mpPreview->font();
    fnt.setUnderline(underline);
    mpPreview->setFont(fnt);
    mpPreview->setWordWrapIconText(wrap);
}

void KIconConfig::load()
{
    read();
    apply();
    emit changed(false);
    for (int i=0; i<KIcon::LastGroup; i++)
	mbChanged[i] = false;
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
	    case KIconEffect::SemiTransparent:
		tmp = "semitransparent";
		break;
	    case KIconEffect::SemiGray:
		tmp = "semigray";
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

    // Send two KIPC messages. This way repainting is easier to implement in
    // clients. On the first message, all icons are updated, while on the
    // second one, the view is repainted.
    for (int i=0; i<KIcon::LastGroup; i++)
    {
	if (mbChanged[i])
	{
	    KIPC::sendMessageAll(KIPC::IconChanged, i);
	    mbChanged[i] = false;   
	}
    }
    KIPC::sendMessageAll(KIPC::IconChanged, KIcon::LastGroup);
}

void KIconConfig::defaults()
{
    for (int i=0; i<KIcon::LastGroup; i++)
    {
	mbDP[i] = false;
	mbChanged[i] = true;
	mSizes[i] = mpTheme->defaultSize(i);
	mEffects[i][0] = KIconEffect::NoEffect;
	mEffects[i][1] = KIconEffect::NoEffect;
	mEffects[i][2] = KIconEffect::SemiTransparent;
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
}

void KIconConfig::slotState(int index)
{
    mState = index;
    mpEffectBox->setCurrentItem(mEffects[mUsage][mState]);
    mpEffectSlider->setValue((int) (100.0 * mEffectValues[mUsage][mState] + 0.5));
    preview();
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
    mbChanged[mUsage] = true;
}

void KIconConfig::slotEffectValue(int value)
{
    mEffectValues[mUsage][mState] = 0.01 * value;
    preview();
    emit changed(true);
    mbChanged[mUsage] = true;
}

void KIconConfig::slotSize(int index)
{
    kdDebug(264) << "Index: " << index << "\n";
    mSizes[mUsage] = mAvSizes[mUsage][index];
    preview();
    emit changed(true);
    mbChanged[mUsage] = true;
}

void KIconConfig::slotDPCheck(bool check)
{
    mbDP[mUsage] = check;
    preview();
    emit changed(true);
    mbChanged[mUsage] = true;
}

void KIconConfig::slotBgColorChanged(const QColor &newColor) {}
void KIconConfig::slotNormalColorChanged(const QColor &newColor) {}
void KIconConfig::slotHiliteColorChanged(const QColor &newColor) {}

void KIconConfig::slotUnderline(bool ul)
{
    underline = ul;
    preview();
    emit changed(true);
}

void KIconConfig::slotWrap(bool w)
{
    wrap = w;
    preview();
    emit changed(true);
}

#include "icons.moc"
