/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
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
#include <qwidget.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlistview.h>                                                                     
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabwidget.h>
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
#include <kcolordlg.h>
#include <kcolorbtn.h>
#include <kbuttonbox.h>

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
    top->setColStretch(1, 1);

    // Use of Icon at (0,0)
    QGroupBox *gbox = new QGroupBox(i18n("Use of Icon"), this);
    top->addWidget(gbox, 0, 0);
    QBoxLayout *g_lay = new QVBoxLayout(gbox,
                                        KDialog::marginHint(),
                                        KDialog::spacingHint());
    g_lay->addSpacing(fontMetrics().lineSpacing());
    mpUsageList = new QListBox(gbox);
    connect(mpUsageList, SIGNAL(highlighted(int)), SLOT(slotUsage(int)));
    g_lay->addWidget(mpUsageList);

    // Preview at (0,1)
    gbox = new QGroupBox(i18n("Preview"), this);
    top->addWidget(gbox, 0, 1);
    g_lay = new QVBoxLayout(gbox, KDialog::marginHint(), 0);
    g_lay->addSpacing(fontMetrics().lineSpacing());
    mpPreview = new QLabel(gbox);
    mpPreview->setAlignment(AlignCenter);
    mpPreview->setMinimumSize(128, 128);
    g_lay->addWidget(mpPreview);

    // Tabwidget at (1,0) - (1,1)
    m_pTabWidget = new QTabWidget(this);
    top->addMultiCellWidget(m_pTabWidget, 1, 1, 0, 1);                                   

    // Icon settings at Tab 1
    m_pTab1 = new QWidget(0L, "Size Tab");
    m_pTabWidget->addTab(m_pTab1, i18n("&Size"));
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

    mpDPCheck = new QCheckBox(i18n("Double sized pixels"), m_pTab1);
    connect(mpDPCheck, SIGNAL(toggled(bool)), SLOT(slotDPCheck(bool)));
    grid->addWidget(mpDPCheck, 1, 0, Qt::AlignLeft);

    // Iconeffects at Tab 2
    m_pTab2 = new QWidget(0L, "Iconeffect Tab");
    m_pTabWidget->addTab(m_pTab2, i18n("&Effects"));
    grid = new QGridLayout(m_pTab2, 4, 3, 10, 10);
    grid->setColStretch(1, 1);
    grid->setColStretch(2, 1);                                                                                                                                                                  


    // Effects
// I need to do a QListView here instead
    mpStateList=new QListBox(m_pTab2);
    // Try to set minimum size to 3 lines of text.
    mpStateList->setMinimumSize(QSize(0, 
	mpStateList->fontMetrics().lineSpacing()*4));
    connect(mpStateList, SIGNAL(highlighted(int)), SLOT(slotState(int)));
    grid->addMultiCellWidget(mpStateList, 0, 0, 0, 3);                                   
    lbl = new QLabel(i18n("&Effect:"), m_pTab2);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 1, 0, Qt::AlignLeft);
    mpEffectBox = new QComboBox(m_pTab2);
    connect(mpEffectBox, SIGNAL(activated(int)), SLOT(slotEffect(int)));
    grid->addWidget(mpEffectBox, 1,1, Qt::AlignLeft);
    lbl->setBuddy(mpEffectBox);

    mpESetupBut = new QPushButton( i18n("&Setup ..."),m_pTab2);
    connect(mpESetupBut, SIGNAL(clicked()), SLOT(slotEffectSetup()));
    grid->addWidget(mpESetupBut, 1,2, Qt::AlignLeft);

    mpSTCheck = new QCheckBox(i18n("Semi-transparent"), m_pTab2);
    connect(mpSTCheck, SIGNAL(toggled(bool)), SLOT(slotSTCheck(bool)));
    grid->addWidget(mpSTCheck, 2, 0, Qt::AlignLeft);


    top->activate(); // needs improvement

    top->activate();

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
    mpUsageList->insertItem(i18n("Desktop / FileManager"));
    mpUsageList->insertItem(i18n("Toolbar"));
    mpUsageList->insertItem(i18n("Main Toolbar"));
    mpUsageList->insertItem(i18n("Small Icons"));
    mpUsageList->insertItem(i18n("Panel"));

    mpStateList->insertItem(i18n("Default - Default icon look"));
    mpStateList->insertItem(i18n("Active - The mouse is over the icon"));
    mpStateList->insertItem(i18n("Disabled - The icon cannot be used"));

    mpEffectBox->insertItem(i18n("No Effect"));
    mpEffectBox->insertItem(i18n("To Gray"));
    mpEffectBox->insertItem(i18n("Colorize"));
    mpEffectBox->insertItem(i18n("Gamma"));
    mpEffectBox->insertItem(i18n("Desaturate"));

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

	// defaults
	mEffects[i][0] = KIconEffect::NoEffect;
	mEffects[i][1] = KIconEffect::NoEffect;
	mEffects[i][2] = KIconEffect::NoEffect;
	mEffectTrans[i][0] = false;
	mEffectTrans[i][1] = false;
	mEffectTrans[i][2] = true;
	mEffectValues[i][0] = 1.0;
	mEffectValues[i][1] = 1.0;
	mEffectValues[i][2] = 1.0;
	mEffectColors[i][0] = QColor(144,128,248);
	mEffectColors[i][1] = QColor(169,156,255);
	mEffectColors[i][2] = QColor(34,202,0);

	for (it2=mStates.begin(), j=0; it2!=mStates.end(); it2++, j++)
	{
	    QString tmp = mpConfig->readEntry(*it2 + "Effect");
	    if (tmp == "togray")
		effect = KIconEffect::ToGray;
	    else if (tmp == "colorize")
		effect = KIconEffect::Colorize;
	    else if (tmp == "togamma")
		effect = KIconEffect::ToGamma;
	    else if (tmp == "desaturate")
		effect = KIconEffect::DeSaturate;
	    else if (tmp == "none")
		effect = KIconEffect::NoEffect;
	    else continue;
	    mEffects[i][j] = effect;
	    mEffectValues[i][j] = mpConfig->readDoubleNumEntry(*it2 + "Value");
	    mEffectColors[i][j] = mpConfig->readColorEntry(*it2 + "Color");
	    mEffectTrans[i][j] = mpConfig->readBoolEntry(*it2 + "SemiTransparent");
	}
    }
}

void KIconConfig::apply()
{
    mpUsageList->setCurrentItem(mUsage);
    mpStateList->setCurrentItem(mState);
    mpEffectBox->setCurrentItem(mEffects[mUsage][mState]);

    switch (mEffects[mUsage][mState])
    {
    case KIconEffect::Colorize:
    case KIconEffect::DeSaturate:
    case KIconEffect::ToGamma:
    case KIconEffect::ToGray:
	mpESetupBut -> setEnabled(true);
	break;
    default:
        mpESetupBut -> setEnabled(false);
	break;
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
    mpSTCheck->setChecked(mEffectTrans[mUsage][mState]);
    mpDPCheck->setChecked(mbDP[mUsage]);
}

void KIconConfig::preview()
{
    // Apply effects ourselves because we don't want to sync
    // the configuration every preview.
    QPixmap pm = mpLoader->loadIcon(mExample, KIcon::NoGroup, mSizes[mUsage]);
    QImage img = pm.convertToImage();
    if (mbDP[mUsage])
    {
	int w = img.width() * 2;
	img = img.smoothScale(w, w);
    }
    img = mpEffect->apply(img, mEffects[mUsage][mState],
	    mEffectValues[mUsage][mState], mEffectColors[mUsage][mState],
            mEffectTrans[mUsage][mState]);
    pm.convertFromImage(img);
    mpPreview->setPixmap(pm);
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
	    case KIconEffect::ToGamma:
		tmp = "togamma";
		break;
	    case KIconEffect::Colorize:
		tmp = "colorize";
		break;
	    case KIconEffect::DeSaturate:
		tmp = "desaturate";
		break;
	    default:
		tmp = "none";
		break;
	    }
	    mpConfig->writeEntry(*it2 + "Effect", tmp, true, true);
	    mpConfig->writeEntry(*it2 + "Value", mEffectValues[i][j], true, true);
            mpConfig->writeEntry(*it2 + "Color", mEffectColors[i][j], true, true);
            mpConfig->writeEntry(*it2 + "SemiTransparent", mEffectTrans[i][j], true, true);
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
    for (int i=0; i<KIcon::LastGroup; i++)
    {
	mbDP[i] = false;
	mbChanged[i] = true;
	mSizes[i] = mpTheme->defaultSize(i);
	mEffects[i][0] = KIconEffect::NoEffect;
	mEffects[i][1] = KIconEffect::NoEffect;
	mEffects[i][2] = KIconEffect::NoEffect;
	mEffectTrans[i][0] = false;
	mEffectTrans[i][1] = false;
	mEffectTrans[i][2] = true;
	mEffectValues[i][0] = 1.0;
	mEffectValues[i][1] = 1.0;
	mEffectValues[i][2] = 1.0;
	mEffectColors[i][0] = QColor(144,128,248);
        mEffectColors[i][1] = QColor(169,156,255);
        mEffectColors[i][2] = QColor(34,202,0);
    }
    apply();
    preview();

    emit changed(true);
}

void KIconConfig::slotUsage(int index)
{
    mUsage = index;
    mState = 0;
    if (mUsage == KIcon::Panel || mUsage == KIcon::Small)
    {
	m_pTabWidget->setTabEnabled(m_pTab1, false);
	m_pTabWidget->setCurrentPage(1);
    }
    else
    {
	m_pTabWidget->setTabEnabled(m_pTab1, true);
    }
    apply();
    preview();
}

void KIconConfig::slotPreview(float &m_pEfColor)
{
    float tmp;
    tmp = mEffectValues[mUsage][mState];
    mEffectValues[mUsage][mState] = m_pEfColor;
    preview();
    mEffectValues[mUsage][mState] = tmp;
}

void KIconConfig::slotEffectSetup()
{
    QColor r;
    r = mEffectColors[mUsage][mState];
    float s;
    s = mEffectValues[mUsage][mState];
    int t;
    t = mEffects[mUsage][mState];

    KIconEffectSetupDialog dlg(r,s,t);
    connect(&dlg, SIGNAL(changeView(float &)), SLOT(slotPreview(float &)));

    if (dlg.exec() == QDialog::Accepted)
    {
        mEffectColors[mUsage][mState] = dlg.fxcolor();
        mEffectValues[mUsage][mState] = dlg.fxvalue();
        emit changed(true);
        mbChanged[mUsage] = true;
    }
    preview();
}

void KIconConfig::slotState(int index)
{
    mState = index;
    switch (mEffects[mUsage][mState])
    {
    case KIconEffect::Colorize:
    case KIconEffect::DeSaturate:
    case KIconEffect::ToGamma:
    case KIconEffect::ToGray:
	mpESetupBut -> setEnabled(true);
	break;
    default:
        mpESetupBut -> setEnabled(false);
	break;
    }
    mpEffectBox->setCurrentItem(mEffects[mUsage][mState]);
    mpSTCheck->setChecked(mEffectTrans[mUsage][mState]);
    preview();
}

void KIconConfig::slotEffect(int index)
{
    mEffects[mUsage][mState] = index;

    switch (mEffects[mUsage][mState])
    {
    case KIconEffect::Colorize:
    case KIconEffect::DeSaturate:
    case KIconEffect::ToGamma:
    case KIconEffect::ToGray:
	mpESetupBut -> setEnabled(true);
	break;
    default:
        mpESetupBut -> setEnabled(false);
	break;
    }

    preview();
    emit changed(true);
    mbChanged[mUsage] = true;
}

void KIconConfig::slotSTCheck(bool tag)
{
    mEffectTrans[mUsage][mState] = tag;
    preview();
    emit changed(true);
    mbChanged[mUsage] = true;
}


void KIconConfig::slotSize(int index)
{
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

KIconEffectSetupDialog::KIconEffectSetupDialog(QColor ecolor, float m_pEfValue,
	int m_pEfTyp, QWidget *parent, char *name)
    : QDialog(parent, name, true)
{
    QPushButton *pbut;
    QLabel *lbl;

    m_pEfColor=ecolor;
    setCaption( i18n("Settings for Iconeffects"));

    QVBoxLayout*top = new QVBoxLayout(this);
    top->setSpacing(10);
    top->setMargin(10);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    top->addWidget(frame);

    QGridLayout *grid = new QGridLayout(frame, 2, 1, 10, 10);
    grid->setSpacing(10);
    grid->setMargin(10);

    if (m_pEfTyp == KIconEffect::Colorize)
    {
	lbl = new QLabel(i18n("Color"), frame);
	grid->addWidget(lbl, 0, 0);
	mpEColButton = new KColorButton(frame);
	connect(mpEColButton, SIGNAL(changed(const QColor &)),
		SLOT(slotEffectColor(const QColor &)));
	grid->addWidget(mpEColButton, 0, 1);
	mpEColButton->setColor(m_pEfColor);
    }

    lbl = new QLabel(i18n("Amount"), frame);
    grid->addWidget(lbl, 1, 0);
    mpEffectSlider = new QSlider(0, 100, 5, 10, QSlider::Horizontal, frame);
    connect(mpEffectSlider, SIGNAL(valueChanged(int)), SLOT(slotEffectValue(int)));
    grid->addWidget(mpEffectSlider, 1, 1);
    mpEffectSlider->setValue((int) (100.0 * m_pEfValue + 0.5));

    KButtonBox *bbox = new KButtonBox(this);
    pbut = bbox->addButton(i18n("&Help"));
    connect(pbut, SIGNAL(clicked()), SLOT(slotHelp()));
    bbox->addStretch();
    pbut = bbox->addButton(i18n("&OK"));
    connect(pbut, SIGNAL(clicked()), SLOT(slotOK()));
    pbut = bbox->addButton(i18n("&Cancel"));
    connect(pbut, SIGNAL(clicked()), SLOT(reject()));
    top->addWidget(bbox,2,0);
    emit changeView(m_pEfValue);
}

void KIconEffectSetupDialog::slotEffectValue(int value)
{
//    preview();

     m_pEfValue = 0.01 * value;
     emit changeView(m_pEfValue);
}

void KIconEffectSetupDialog::slotEffectColor(const QColor &col)
{
//    preview();

     m_pEfColor = col;
}


void KIconEffectSetupDialog::slotHelp()
{
}

void KIconEffectSetupDialog::slotOK()
{
    accept();
}

#include "icons.moc"
