/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>                                                *
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <stdlib.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qslider.h>
//Added by qt3to4:
#include <QPixmap>
#include <QGridLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <Q3ListBox>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kipc.h>
#include <klocale.h>
#include <kseparator.h>

#include "icons.h"

/**** KIconConfig ****/

KIconConfig::KIconConfig(KInstance *inst, QWidget *parent)
    : KCModule(inst, parent)
{

    QGridLayout *top = new QGridLayout(this, 2, 2,
                                       KDialog::marginHint(),
                                       KDialog::spacingHint());
    top->setColStretch(0, 1);
    top->setColStretch(1, 1);

    // Use of Icon at (0,0) - (1, 0)
    Q3GroupBox *gbox = new Q3GroupBox(i18n("Use of Icon"), this);
    top->addMultiCellWidget(gbox, 0, 1, 0, 0);
    QBoxLayout *g_vlay = new QVBoxLayout(gbox,
                                        KDialog::marginHint(),
                                        KDialog::spacingHint());
    g_vlay->addSpacing(fontMetrics().lineSpacing());
    mpUsageList = new Q3ListBox(gbox);
    connect(mpUsageList, SIGNAL(highlighted(int)), SLOT(slotUsage(int)));
    g_vlay->addWidget(mpUsageList);

    KSeparator *sep = new KSeparator( Qt::Horizontal, this );
    top->addWidget(sep, 1, 1);
    // Preview at (2,0) - (2, 1)
    QGridLayout *g_lay = new QGridLayout(4, 3, KDialog::marginHint(), 0);
    top->addMultiCellLayout(g_lay, 2, 2, 0, 1);
    g_lay->addRowSpacing(0, fontMetrics().lineSpacing());

    QPushButton *push;

    push = addPreviewIcon(0, i18n("Default"), this, g_lay);
    connect(push, SIGNAL(clicked()), SLOT(slotEffectSetup0()));
    push = addPreviewIcon(1, i18n("Active"), this, g_lay);
    connect(push, SIGNAL(clicked()), SLOT(slotEffectSetup1()));
    push = addPreviewIcon(2, i18n("Disabled"), this, g_lay);
    connect(push, SIGNAL(clicked()), SLOT(slotEffectSetup2()));

    m_pTab1 = new QWidget(this, "General Tab");
    top->addWidget(m_pTab1, 0, 1);

    QGridLayout *grid = new QGridLayout(m_pTab1, 4, 3, 10, 10);
    grid->setColStretch(1, 1);
    grid->setColStretch(2, 1);


    // Size
    QLabel *lbl = new QLabel(i18n("Size:"), m_pTab1);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 0, 0, Qt::AlignLeft);
    mpSizeBox = new QComboBox(m_pTab1);
    connect(mpSizeBox, SIGNAL(activated(int)), SLOT(slotSize(int)));
    lbl->setBuddy(mpSizeBox);
    grid->addWidget(mpSizeBox, 0, 1, Qt::AlignLeft);

    mpDPCheck = new QCheckBox(i18n("Double-sized pixels"), m_pTab1);
    connect(mpDPCheck, SIGNAL(toggled(bool)), SLOT(slotDPCheck(bool)));
    grid->addMultiCellWidget(mpDPCheck, 1, 1, 0, 1, Qt::AlignLeft);

    mpAnimatedCheck = new QCheckBox(i18n("Animate icons"), m_pTab1);
    connect(mpAnimatedCheck, SIGNAL(toggled(bool)), SLOT(slotAnimatedCheck(bool)));
    grid->addMultiCellWidget(mpAnimatedCheck, 2, 2, 0, 1, Qt::AlignLeft);

    top->activate();

    init();
    read();
    apply();
    preview();
}

KIconConfig::~KIconConfig()
{
  delete mpEffect;
}

QPushButton *KIconConfig::addPreviewIcon(int i, const QString &str, QWidget *parent, QGridLayout *lay)
{
    QLabel *lab = new QLabel(str, parent);
    lay->addWidget(lab, 1, i, Qt::AlignCenter);
    mpPreview[i] = new QLabel(parent);
    mpPreview[i]->setAlignment(Qt::AlignCenter);
    mpPreview[i]->setMinimumSize(105, 105);
    lay->addWidget(mpPreview[i], 2, i);
    QPushButton *push = new QPushButton(i18n("Set Effect..."), parent);
    lay->addWidget(push, 3, i, Qt::AlignCenter);
    return push;
}

void KIconConfig::init()
{
    mpLoader = KGlobal::iconLoader();
    mpConfig = KGlobal::config();
    mpEffect = new KIconEffect;
    mpTheme = mpLoader->theme();
    mUsage = 0;
    for (int i=0; i<KIcon::LastGroup; i++)
	mbChanged[i] = false;

    // Fill list/checkboxen
    mpUsageList->insertItem(i18n("Desktop/File Manager"));
    mpUsageList->insertItem(i18n("Toolbar"));
    mpUsageList->insertItem(i18n("Main Toolbar"));
    mpUsageList->insertItem(i18n("Small Icons"));
    mpUsageList->insertItem(i18n("Panel"));
    mpUsageList->insertItem(i18n("All Icons"));

    // For reading the configuration
    mGroups += "Desktop";
    mGroups += "Toolbar";
    mGroups += "MainToolbar";
    mGroups += "Small";
    mGroups += "Panel";

    mStates += "Default";
    mStates += "Active";
    mStates += "Disabled";
}

void KIconConfig::initDefaults()
{
    mDefaultEffect[0].type = KIconEffect::NoEffect;
    mDefaultEffect[1].type = KIconEffect::NoEffect;
    mDefaultEffect[2].type = KIconEffect::ToGray;
    mDefaultEffect[0].transparant = false;
    mDefaultEffect[1].transparant = false;
    mDefaultEffect[2].transparant = true;
    mDefaultEffect[0].value = 1.0;
    mDefaultEffect[1].value = 1.0;
    mDefaultEffect[2].value = 1.0;
    mDefaultEffect[0].color = QColor(144,128,248);
    mDefaultEffect[1].color = QColor(169,156,255);
    mDefaultEffect[2].color = QColor(34,202,0);
    mDefaultEffect[0].color2 = QColor(0,0,0);
    mDefaultEffect[1].color2 = QColor(0,0,0);
    mDefaultEffect[2].color2 = QColor(0,0,0);

    const int defDefSizes[] = { 32, 22, 22, 16, 32 };

    KIcon::Group i;
    QStringList::ConstIterator it;
    for(it=mGroups.begin(), i=KIcon::FirstGroup; it!=mGroups.end(); ++it, i++)
    {
	mbDP[i] = false;
	mbChanged[i] = true;
	mbAnimated[i] = false;
	if (mpTheme)
	    mSizes[i] = mpTheme->defaultSize(i);
	else
	    mSizes[i] = defDefSizes[i];

	mEffects[i][0] = mDefaultEffect[0];
	mEffects[i][1] = mDefaultEffect[1];
	mEffects[i][2] = mDefaultEffect[2];
    }
    // Animate desktop icons by default
    int group = mGroups.findIndex( "Desktop" );
    if ( group != -1 )
        mbAnimated[group] = true;

    // This is the new default in KDE 2.2, in sync with the kiconeffect of kdelibs Nolden 2001/06/11
    int activeState = mStates.findIndex( "Active" );
    if ( activeState != -1 )
    {
        int group = mGroups.findIndex( "Desktop" );
        if ( group != -1 )
        {
            mEffects[ group ][ activeState ].type = KIconEffect::ToGamma;
            mEffects[ group ][ activeState ].value = 0.7;
        }

        group = mGroups.findIndex( "Panel" );
        if ( group != -1 )
        {
            mEffects[ group ][ activeState ].type = KIconEffect::ToGamma;
            mEffects[ group ][ activeState ].value = 0.7;
        }
    }
}

void KIconConfig::read()
{
    if (mpTheme)
    {
        for (KIcon::Group i=KIcon::FirstGroup; i<KIcon::LastGroup; i++)
            mAvSizes[i] = mpTheme->querySizes(i);

        mTheme = mpTheme->current();
        mExample = mpTheme->example();
    }
    else
    {
        for (KIcon::Group i=KIcon::FirstGroup; i<KIcon::LastGroup; i++)
            mAvSizes[i] = QList<int>();

        mTheme.clear();
        mExample.clear();
    }

    initDefaults();

    int i, j, effect;
    QStringList::ConstIterator it, it2;
    for (it=mGroups.begin(), i=0; it!=mGroups.end(); ++it, i++)
    {
        mbChanged[i] = false;

	mpConfig->setGroup(*it + "Icons");
	mSizes[i] = mpConfig->readEntry("Size", mSizes[i]);
	mbDP[i] = mpConfig->readEntry("DoublePixels", QVariant(mbDP[i])).toBool();
	mbAnimated[i] = mpConfig->readEntry("Animated", QVariant(mbAnimated[i])).toBool();

	for (it2=mStates.begin(), j=0; it2!=mStates.end(); ++it2, j++)
	{
	    QString tmp = mpConfig->readEntry(*it2 + "Effect", QString());
	    if (tmp == "togray")
		effect = KIconEffect::ToGray;
	    else if (tmp == "colorize")
		effect = KIconEffect::Colorize;
	    else if (tmp == "togamma")
		effect = KIconEffect::ToGamma;
	    else if (tmp == "desaturate")
		effect = KIconEffect::DeSaturate;
	    else if (tmp == "tomonochrome")
		effect = KIconEffect::ToMonochrome;
	    else if (tmp == "none")
		effect = KIconEffect::NoEffect;
	    else continue;
	    mEffects[i][j].type = effect;
	    mEffects[i][j].value = mpConfig->readDoubleNumEntry(*it2 + "Value");
	    mEffects[i][j].color = mpConfig->readEntry(*it2 + "Color",QColor());
	    mEffects[i][j].color2 = mpConfig->readEntry(*it2 + "Color2", QColor());
	    mEffects[i][j].transparant = mpConfig->readEntry(*it2 + "SemiTransparent", false);
	}
    }
}

void KIconConfig::apply()
{
    mpUsageList->setCurrentItem(mUsage);

    int delta = 1000, dw, index = -1, size = 0, i;
    QList<int>::Iterator it;
    mpSizeBox->clear();
    if (mUsage < KIcon::LastGroup) {
        for (it=mAvSizes[mUsage].begin(), i=0; it!=mAvSizes[mUsage].end(); ++it, i++)
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
        mpAnimatedCheck->setChecked(mbAnimated[mUsage]);
    }
}

void KIconConfig::preview(int i)
{
    // Apply effects ourselves because we don't want to sync
    // the configuration every preview.

    int viewedGroup = (mUsage == KIcon::LastGroup) ? KIcon::FirstGroup : mUsage;

    QPixmap pm = mpLoader->loadIcon(mExample, KIcon::NoGroup, mSizes[viewedGroup]);
    QImage img = pm.convertToImage();
    if (mbDP[viewedGroup])
    {
	int w = img.width() * 2;
	img = img.smoothScale(w, w);
    }

    Effect &effect = mEffects[viewedGroup][i];

    img = mpEffect->apply(img, effect.type,
	    effect.value, effect.color, effect.color2, effect.transparant);
    pm.convertFromImage(img);
    mpPreview[i]->setPixmap(pm);
}

void KIconConfig::preview()
{
    preview(0);
    preview(1);
    preview(2);
}

void KIconConfig::load()
{
    read();
    apply();
    emit changed(false);
    for (int i=0; i<KIcon::LastGroup; i++)
	mbChanged[i] = false;
    preview();
}


void KIconConfig::save()
{
    int i, j;
    QStringList::ConstIterator it, it2;
    for (it=mGroups.begin(), i=0; it!=mGroups.end(); ++it, i++)
    {
	mpConfig->setGroup(*it + "Icons");
	mpConfig->writeEntry("Size", mSizes[i], KConfigBase::Normal|KConfigBase::Global);
	mpConfig->writeEntry("DoublePixels", mbDP[i], KConfigBase::Normal|KConfigBase::Global);
	mpConfig->writeEntry("Animated", mbAnimated[i], KConfigBase::Normal|KConfigBase::Global);
	for (it2=mStates.begin(), j=0; it2!=mStates.end(); ++it2, j++)
	{
	    QString tmp;
	    switch (mEffects[i][j].type)
	    {
	    case KIconEffect::ToGray:
		tmp = "togray";
		break;
	    case KIconEffect::ToGamma:
		tmp = "togamma";
		break;
	    case KIconEffect::Colorize:
		tmp = "colorize";
		break;
	    case KIconEffect::DeSaturate:
		tmp = "desaturate";
		break;
	    case KIconEffect::ToMonochrome:
		tmp = "tomonochrome";
		break;
	    default:
		tmp = "none";
		break;
	    }
	    mpConfig->writeEntry(*it2 + "Effect", tmp, KConfigBase::Normal|KConfigBase::Global);
	    mpConfig->writeEntry(*it2 + "Value", int(mEffects[i][j].value), KConfigBase::Normal|KConfigBase::Global);
            mpConfig->writeEntry(*it2 + "Color", mEffects[i][j].color, KConfigBase::Normal|KConfigBase::Global);
            mpConfig->writeEntry(*it2 + "Color2", mEffects[i][j].color2, KConfigBase::Normal|KConfigBase::Global);
            mpConfig->writeEntry(*it2 + "SemiTransparent", mEffects[i][j].transparant, KConfigBase::Normal|KConfigBase::Global);
	}
    }

    mpConfig->sync();

    emit changed(false);

    // Emit KIPC change message.
    for (int i=0; i<KIcon::LastGroup; i++)
    {
	if (mbChanged[i])
	{
	    KIPC::sendMessageAll(KIPC::IconChanged, i);
	    mbChanged[i] = false;
	}
    }
}

void KIconConfig::defaults()
{
    initDefaults();
    apply();
    preview();
    emit changed(true);
}

void KIconConfig::slotUsage(int index)
{
    mUsage = index;
    if ( mUsage == KIcon::Panel || mUsage == KIcon::LastGroup )
    {
        mpSizeBox->setEnabled(false);
        mpDPCheck->setEnabled(false);
	mpAnimatedCheck->setEnabled( mUsage == KIcon::Panel );
    }
    else
    {
        mpSizeBox->setEnabled(true);
        mpDPCheck->setEnabled(true);
	mpAnimatedCheck->setEnabled( mUsage == KIcon::Desktop );
    }

    apply();
    preview();
}

void KIconConfig::EffectSetup(int state)
{
    int viewedGroup = (mUsage == KIcon::LastGroup) ? KIcon::FirstGroup : mUsage;

    QPixmap pm = mpLoader->loadIcon(mExample, KIcon::NoGroup, mSizes[viewedGroup]);
    QImage img = pm.convertToImage();
    if (mbDP[viewedGroup])
    {
	int w = img.width() * 2;
	img = img.smoothScale(w, w);
    }

    QString caption;
    switch (state)
    {
    case 0 : caption = i18n("Setup Default Icon Effect"); break;
    case 1 : caption = i18n("Setup Active Icon Effect"); break;
    case 2 : caption = i18n("Setup Disabled Icon Effect"); break;
    }

    KIconEffectSetupDialog dlg(mEffects[viewedGroup][state], mDefaultEffect[state], caption, img);

    if (dlg.exec() == QDialog::Accepted)
    {
        if (mUsage == KIcon::LastGroup) {
            for (int i=0; i<KIcon::LastGroup; i++)
                mEffects[i][state] = dlg.effect();
        } else {
            mEffects[mUsage][state] = dlg.effect();
        }

        // AK - can this call be moved therefore removing
        //      code duplication?

        emit changed(true);

        if (mUsage == KIcon::LastGroup) {
            for (int i=0; i<KIcon::LastGroup; i++)
                mbChanged[i] = true;
        } else {
            mbChanged[mUsage] = true;
        }
    }
    preview(state);
}

void KIconConfig::slotSize(int index)
{
    Q_ASSERT(mUsage < KIcon::LastGroup);
    mSizes[mUsage] = mAvSizes[mUsage][index];
    preview();
    emit changed(true);
    mbChanged[mUsage] = true;
}

void KIconConfig::slotDPCheck(bool check)
{
    Q_ASSERT(mUsage < KIcon::LastGroup);
    if (mbDP[mUsage] != check)
    {
        mbDP[mUsage] = check;
        emit changed(true);
        mbChanged[mUsage] = true;
    }
    preview();

}

void KIconConfig::slotAnimatedCheck(bool check)
{
    Q_ASSERT(mUsage < KIcon::LastGroup);
    if (mbAnimated[mUsage] != check)
    {
        mbAnimated[mUsage] = check;
        emit changed(true);
        mbChanged[mUsage] = true;
    }
}

KIconEffectSetupDialog::KIconEffectSetupDialog(const Effect &effect,
    const Effect &defaultEffect,
    const QString &caption, const QImage &image,
    QWidget *parent, char *name)
    : KDialogBase(parent, name, true, caption,
	Default|Ok|Cancel, Ok, true),
      mEffect(effect),
      mDefaultEffect(defaultEffect),
      mExample(image)
{
    mpEffect = new KIconEffect;

    QLabel *lbl;
    Q3GroupBox *frame;
    QGridLayout *grid;

    QWidget *page = new QWidget(this);
    setMainWidget(page);

    QGridLayout *top = new QGridLayout(page, 4, 2, 0, spacingHint());
    top->setColStretch(0,1);
    top->addColSpacing(1,10);
    top->setColStretch(2,2);
    top->setRowStretch(1,1);

    lbl = new QLabel(i18n("&Effect:"), page);
    lbl->setFixedSize(lbl->sizeHint());
    top->addWidget(lbl, 0, 0, Qt::AlignLeft);
    mpEffectBox = new Q3ListBox(page);
    mpEffectBox->insertItem(i18n("No Effect"));
    mpEffectBox->insertItem(i18n("To Gray"));
    mpEffectBox->insertItem(i18n("Colorize"));
    mpEffectBox->insertItem(i18n("Gamma"));
    mpEffectBox->insertItem(i18n("Desaturate"));
    mpEffectBox->insertItem(i18n("To Monochrome"));
    mpEffectBox->setMinimumWidth( 100 );
    connect(mpEffectBox, SIGNAL(highlighted(int)), SLOT(slotEffectType(int)));
    top->addMultiCellWidget(mpEffectBox, 1, 2, 0, 0, Qt::AlignLeft);
    lbl->setBuddy(mpEffectBox);

    mpSTCheck = new QCheckBox(i18n("&Semi-transparent"), page);
    connect(mpSTCheck, SIGNAL(toggled(bool)), SLOT(slotSTCheck(bool)));
    top->addWidget(mpSTCheck, 3, 0, Qt::AlignLeft);

    frame = new Q3GroupBox(i18n("Preview"), page);
    top->addMultiCellWidget(frame, 0, 1, 1, 1);
    grid = new QGridLayout(frame, 2, 1, marginHint(), spacingHint());
    grid->addRowSpacing(0, fontMetrics().lineSpacing());
    grid->setRowStretch(1, 1);

    mpPreview = new QLabel(frame);
    mpPreview->setAlignment(Qt::AlignCenter);
    mpPreview->setMinimumSize(105, 105);
    grid->addWidget(mpPreview, 1, 0);

    mpEffectGroup = new Q3GroupBox(i18n("Effect Parameters"), page);
    top->addMultiCellWidget(mpEffectGroup, 2, 3, 1, 1);
    grid = new QGridLayout(mpEffectGroup, 3, 2, marginHint(), spacingHint());
    grid->addRowSpacing(0, fontMetrics().lineSpacing());

    mpEffectLabel = new QLabel(i18n("&Amount:"), mpEffectGroup);
    grid->addWidget(mpEffectLabel, 1, 0);
    mpEffectSlider = new QSlider(0, 100, 5, 10, Qt::Horizontal, mpEffectGroup);
    mpEffectLabel->setBuddy( mpEffectSlider );
    connect(mpEffectSlider, SIGNAL(valueChanged(int)), SLOT(slotEffectValue(int)));
    grid->addWidget(mpEffectSlider, 1, 1);

    mpEffectColor = new QLabel(i18n("Co&lor:"), mpEffectGroup);
    grid->addWidget(mpEffectColor, 2, 0);
    mpEColButton = new KColorButton(mpEffectGroup);
    mpEffectColor->setBuddy( mpEColButton );
    connect(mpEColButton, SIGNAL(changed(const QColor &)),
		SLOT(slotEffectColor(const QColor &)));
    grid->addWidget(mpEColButton, 2, 1);

    mpEffectColor2 = new QLabel(i18n("&Second color:"), mpEffectGroup);
    grid->addWidget(mpEffectColor2, 3, 0);
    mpECol2Button = new KColorButton(mpEffectGroup);
    mpEffectColor2->setBuddy( mpECol2Button );
    connect(mpECol2Button, SIGNAL(changed(const QColor &)),
		SLOT(slotEffectColor2(const QColor &)));
    grid->addWidget(mpECol2Button, 3, 1);

    init();
    preview();
}

KIconEffectSetupDialog::~KIconEffectSetupDialog()
{
  delete mpEffect;
}

void KIconEffectSetupDialog::init()
{
    mpEffectBox->setCurrentItem(mEffect.type);
    mpEffectSlider->setEnabled(mEffect.type != KIconEffect::NoEffect);
    mpEColButton->setEnabled(mEffect.type == KIconEffect::Colorize || mEffect.type == KIconEffect::ToMonochrome);
    mpECol2Button->setEnabled(mEffect.type == KIconEffect::ToMonochrome);
    mpEffectSlider->setValue((int) (100.0 * mEffect.value + 0.5));
    mpEColButton->setColor(mEffect.color);
    mpECol2Button->setColor(mEffect.color2);
    mpSTCheck->setChecked(mEffect.transparant);
}

void KIconEffectSetupDialog::slotEffectValue(int value)
{
     mEffect.value = 0.01 * value;
     preview();
}

void KIconEffectSetupDialog::slotEffectColor(const QColor &col)
{
     mEffect.color = col;
     preview();
}

void KIconEffectSetupDialog::slotEffectColor2(const QColor &col)
{
     mEffect.color2 = col;
     preview();
}

void KIconEffectSetupDialog::slotEffectType(int type)
{
    mEffect.type = type;
    mpEffectGroup->setEnabled(mEffect.type != KIconEffect::NoEffect);
    mpEffectSlider->setEnabled(mEffect.type != KIconEffect::NoEffect);
    mpEffectColor->setEnabled(mEffect.type == KIconEffect::Colorize || mEffect.type == KIconEffect::ToMonochrome);
    mpEColButton->setEnabled(mEffect.type == KIconEffect::Colorize || mEffect.type == KIconEffect::ToMonochrome);
    mpEffectColor2->setEnabled(mEffect.type == KIconEffect::ToMonochrome);
    mpECol2Button->setEnabled(mEffect.type == KIconEffect::ToMonochrome);
    preview();
}

void KIconEffectSetupDialog::slotSTCheck(bool b)
{
     mEffect.transparant = b;
     preview();
}

void KIconEffectSetupDialog::slotDefault()
{
     mEffect = mDefaultEffect;
     init();
     preview();
}

void KIconEffectSetupDialog::preview()
{
    QPixmap pm;
    QImage img = mExample.copy();
    img = mpEffect->apply(img, mEffect.type,
          mEffect.value, mEffect.color, mEffect.color2, mEffect.transparant);
    pm.convertFromImage(img);
    mpPreview->setPixmap(pm);
}

#include "icons.moc"
