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
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qiconview.h>
#include <qradiobutton.h>

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
#include <kcolordlg.h>
#include <kcolorbtn.h>

#include <icnlabel.h>

/**** KIconConfigLabel ****/

KIconConfigLabel::KIconConfigLabel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QGridLayout *top = new QGridLayout(this, 2, 2,
                                       KDialog::marginHint(),
                                       KDialog::spacingHint());
    top->setColStretch(0, 1);
    top->setColStretch(1, 1);


    // Desktop-Icons - Preview
    QGroupBox *gbox = new QGroupBox(i18n("Desktop-Icons"), this);
    top->addWidget(gbox, 0, 0);
    QBoxLayout *g_lay = new QVBoxLayout(gbox, KDialog::marginHint(), 0);
    g_lay->addSpacing(fontMetrics().lineSpacing());
    mpPreview = new QIconView(gbox);
    mpPreview->setGridX(70);
    mpPreview->setItemsMovable(false);
    mpPreview->setMinimumSize(128, 128);
    g_lay->addWidget(mpPreview);

    mpPreviewItem = new QIconViewItem(mpPreview,
                                      i18n("Very long filename.ext"));

    // Desktop-Icons - Fonts
    g_lay->addSpacing(fontMetrics().lineSpacing());

    QPushButton *mpDTfont = new QPushButton(i18n("Font..."), gbox);
    g_lay->addWidget(mpDTfont);

    g_lay->addSpacing(fontMetrics().lineSpacing());

    QBoxLayout *h_lay = new QHBoxLayout;
    g_lay->addLayout(h_lay);

    QLabel *lbl = new QLabel(i18n("Background Color:"), gbox);
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

    // Toolbar-Icons
    gbox = new QGroupBox(i18n("Toolbar-Icons"), this);
    top->addWidget(gbox, 0, 1);
    g_lay = new QVBoxLayout(gbox, KDialog::marginHint(), 0);
    g_lay->addSpacing(fontMetrics().lineSpacing());

    // Use of Icon

    mpUsageList = new QListBox(gbox);
    connect(mpUsageList, SIGNAL(highlighted(int)), SLOT(slotUsage(int)));
    g_lay->addSpacing(10);
    g_lay->addWidget(mpUsageList);

    //

    g_lay->addSpacing(fontMetrics().lineSpacing());

    QPushButton *mpTBfont = new QPushButton(i18n("Font..."), gbox);
    g_lay->addWidget(mpTBfont);

    g_lay->addSpacing(fontMetrics().lineSpacing());

    QRadioButton *tbIcon   = new QRadioButton( i18n( "&Icons only" ), gbox);
    QRadioButton *tbText   = new QRadioButton( i18n( "&Text only" ), gbox);
    QRadioButton *tbAside  = new QRadioButton( i18n( "Text a&side icons" ), gbox);
    QRadioButton *tbUnder  = new QRadioButton( i18n( "Text &under icons" ), gbox);

    connect( tbIcon , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbText , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbAside, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbUnder, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );

    g_lay->addWidget(tbIcon);
    g_lay->addWidget(tbText);
    g_lay->addWidget(tbAside);
    g_lay->addWidget(tbUnder);

    init();
    read();
    apply();
    preview();
}

void KIconConfigLabel::init()
{
    mpLoader = KGlobal::iconLoader();
    mpConfig = KGlobal::config();
    mpEffect = new KIconEffect;
    mpTheme = mpLoader->theme();

}

void KIconConfigLabel::read()
{
    mTheme = mpTheme->current();
    mExample = mpTheme->example();
    mpConfig->setGroup("Icon Labels");
    wrap = mpConfig->readBoolEntry("WordWrap", true);
    underline = mpConfig->readBoolEntry("Underline", true);
}


void KIconConfigLabel::apply()
{

    wordWrapCB->setChecked(wrap);
    underlineCB->setChecked(underline);

}

void KIconConfigLabel::preview()
{
    // Apply effects ourselves because we don't want to sync
    // the configuration every preview.
    QPixmap pm = mpLoader->loadIcon(mExample, KIcon::Desktop);
    mpPreviewItem->setPixmap(pm);

    QFont fnt = mpPreview->font();
    fnt.setUnderline(underline);
    mpPreview->setFont(fnt);
    mpPreview->setWordWrapIconText(wrap);

}

void KIconConfigLabel::load()
{
/*    read();
    apply();
    emit changed(false);
    for (int i=0; i<KIcon::LastGroup; i++)
        mbChanged[i] = false; */
}

void KIconConfigLabel::save()
{
/*
    read();
    apply();
    emit changed(false);
    for (int i=0; i<KIcon::LastGroup; i++)
        mbChanged[i] = false;
*/
}

void KIconConfigLabel::defaults()
{
/*
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

    }
    apply();
    preview();

    emit changed(true);
*/
}

void KIconConfigLabel::slotBgColorChanged(const QColor &) {}
void KIconConfigLabel::slotNormalColorChanged(const QColor &) {}
void KIconConfigLabel::slotHiliteColorChanged(const QColor &) {}

void KIconConfigLabel::slotUnderline(bool ul)
{
    underline = ul;
    preview();
    emit changed(true);
}


void KIconConfigLabel::slotWrap(bool w)
{
    wrap = w;
    preview();
    emit changed(true);
}

void KIconConfigLabel::slotUsage(int /*index*/)
{
/*    mUsage = index;
    mState = 0;
    apply();
    preview();
*/
}

void KIconConfigLabel::slotChangeTbStyle()
{
/*    if (tbIcon->isChecked() )
        tbUseText = 0;
    else if (tbText->isChecked() )
        tbUseText = 2;
    else if (tbAside->isChecked() )
        tbUseText = 1;
    else if (tbUnder->isChecked() )
        tbUseText = 3;
    else
        tbUseText = 0 ;

    m_bChanged = true;
    emit changed(true);
*/
}
#include "icnlabel.moc"
