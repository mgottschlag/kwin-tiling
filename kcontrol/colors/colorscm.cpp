/* KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

// FIXME QFrame included only for placeholders

#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QSlider>
#include <QtGui/QLabel>

#include <KColorButton>
#include <KGenericFactory>
#include <KGlobalSettings>
#include <KAboutData>
#include <KListWidget>

#include "colorscm.h"

K_PLUGIN_FACTORY( KolorFactory, registerPlugin<KColorCm>(); )
K_EXPORT_PLUGIN( KolorFactory("kcmcolors") )

KColorCm::KColorCm(QWidget *parent, const QVariantList &)
    : KCModule( KolorFactory::componentData(), parent )
{
    KAboutData* about = new KAboutData(
        "kcmcolors", 0, ki18n("Colors"), 0, KLocalizedString(),
        KAboutData::License_GPL,
        ki18n("(c) 2007 Matthew Woehlke")
    );
    about->addAuthor( ki18n("Matthew Woehlke"), KLocalizedString(),
                     "mw_triad@users.sourceforge.net" );
    setAboutData( about );

    // root tab widget
    QVBoxLayout *mainLayout = new QVBoxLayout( this );
    mainLayout->setMargin(0);
    QTabWidget *tabWidget = new QTabWidget( this );
    mainLayout->addWidget( tabWidget );

    // scheme page
    QWidget *schemePage = new QWidget;
    QGridLayout *schemePageLayout = new QGridLayout( schemePage );
    QFrame *preview = new QFrame; // TODO
    schemePageLayout->addWidget( preview, 0, 0, 1, 2 );

    QGroupBox *schemeBox = new QGroupBox( "Color Scheme" );
    QGridLayout *schemeLayout = new QGridLayout();
    schemeBox->setLayout( schemeLayout );
    KListWidget *schemeList = new KListWidget();
    schemeLayout->addWidget( schemeList, 0, 0, 1, 3 );
    QPushButton *schemeSaveButton = new QPushButton( "&Save..." );
    QPushButton *schemeRemoveButton = new QPushButton( "Rem&ove" );
    QPushButton *schemeImportButton = new QPushButton( "I&mport..." );
    schemeLayout->addWidget( schemeSaveButton, 1, 0 );
    schemeLayout->addWidget( schemeRemoveButton, 1, 1 );
    schemeLayout->addWidget( schemeImportButton, 1, 2 );
    schemePageLayout->addWidget( schemeBox, 1, 0, 3, 1 );

    QGroupBox *colorsBox = new QGroupBox( "Colors" );
    QVBoxLayout *colorsLayout = new QVBoxLayout();
    colorsBox->setLayout( colorsLayout );
    QComboBox *colorSet = new QComboBox;
    colorSet->addItem( "Common Colors" );
    colorSet->addItem( "  View" );
    colorSet->addItem( "  Window" );
    colorSet->addItem( "  Button" );
    colorSet->addItem( "  Selection" );
    colorSet->addItem( "  Tooltip" );
    colorsLayout->addWidget( colorSet );
    KListWidget *colorList = new KListWidget();
    colorsLayout->addWidget( colorList );
    schemePageLayout->addWidget( colorsBox, 1, 1 );

    QCheckBox *shadeSortedColumn = new QCheckBox( "Shade sorted &column in lists" );
    schemePageLayout->addWidget( shadeSortedColumn, 2, 1 );

    QGroupBox *contrastBox = new QGroupBox( "Con&trast" );
    QHBoxLayout *contrastLayout = new QHBoxLayout();
    contrastBox->setLayout( contrastLayout );
    QSlider *contrastSlider = new QSlider( Qt::Horizontal );
    contrastSlider->setRange( 0, 10 );
    contrastSlider->setSingleStep( 1 );
    contrastSlider->setPageStep( 5 );
    contrastLayout->addWidget( contrastSlider );
    schemePageLayout->addWidget( contrastBox, 3, 1 );

    QCheckBox *applyToAlien = new QCheckBox( "Apply colors to &non-KDE4 applications" );
    schemePageLayout->addWidget( applyToAlien, 4, 0, 1, 2 );

    // effects page
    QWidget *effectsPage = new QWidget;
    QHBoxLayout *effectsPageLayout = new QHBoxLayout( effectsPage );
    // TODO
    QGroupBox *inactiveBox = new QGroupBox( i18n( "Inactive" ) );
    QGridLayout *inactiveLayout = new QGridLayout(inactiveBox);
    QLabel *intensityLabel = new QLabel( i18n( "Intensity" ) );
    QComboBox *intensityComboBox = new QComboBox;
    intensityComboBox->addItem( i18n( "Shade" ) );
    intensityComboBox->addItem( i18n( "Darken" ) );
    intensityComboBox->addItem( i18n( "Lighten" ) );
    QSlider *intensitySlider = new QSlider( Qt::Horizontal );
    inactiveLayout->addWidget(intensityLabel, 0, 0);
    inactiveLayout->addWidget(intensityComboBox, 0, 1);
    inactiveLayout->addWidget(intensitySlider, 1, 0, 1, 3);
    QLabel *colorLabel = new QLabel( i18n( "Color" ) );
    QComboBox *colorComboBox = new QComboBox;
    colorComboBox->addItem( i18n( "Desaturate" ) );
    colorComboBox->addItem( i18n( "Fade" ) );
    colorComboBox->addItem( i18n( "Tint in" ) );
    colorComboBox->addItem( i18n( "Tint out" ) );
    KColorButton *colorButton = new KColorButton;
    QSlider *colorSlider = new QSlider( Qt::Horizontal );
    inactiveLayout->addWidget(colorLabel, 2, 0);
    inactiveLayout->addWidget(colorComboBox, 2, 1);
    inactiveLayout->addWidget(colorButton, 2, 2);
    inactiveLayout->addWidget(colorSlider, 3, 0, 1, 3);
    QLabel *inactiveContrastLabel = new QLabel( i18n( "Contrast" ) );
    QComboBox *inactiveContrastComboBox = new QComboBox;
    inactiveContrastComboBox->addItem( i18n( "Fade" ) );
    inactiveContrastComboBox->addItem( i18n( "Tint in" ) );
    inactiveContrastComboBox->addItem( i18n( "Tint out" ) );
    QSlider *inactiveContrastSlider = new QSlider( Qt::Horizontal );
    inactiveLayout->addWidget(inactiveContrastLabel, 4, 0);
    inactiveLayout->addWidget(inactiveContrastComboBox, 4, 1);
    inactiveLayout->addWidget(inactiveContrastSlider, 5, 0, 1, 3);
    QFrame *inactivePreview = new QFrame;
    inactiveLayout->addWidget(inactivePreview, 6, 0, 1, 3);

    QGroupBox *disabledBox = new QGroupBox( i18n( "Disabled" ) );
    QGridLayout *disabledLayout = new QGridLayout(disabledBox);
    QLabel *intensityLabel2 = new QLabel( i18n( "Intensity" ) );
    QComboBox *intensityComboBox2 = new QComboBox;
    intensityComboBox2->addItem( i18n( "Shade" ) );
    intensityComboBox2->addItem( i18n( "Darken" ) );
    intensityComboBox2->addItem( i18n( "Lighten" ) );
    QSlider *intensitySlider2 = new QSlider( Qt::Horizontal );
    disabledLayout->addWidget(intensityLabel2, 0, 0);
    disabledLayout->addWidget(intensityComboBox2, 0, 1);
    disabledLayout->addWidget(intensitySlider2, 1, 0, 1, 3);
    QLabel *colorLabel2 = new QLabel( i18n( "Color" ) );
    QComboBox *colorComboBox2 = new QComboBox;
    colorComboBox2->addItem( i18n( "Desaturate" ) );
    colorComboBox2->addItem( i18n( "Fade" ) );
    colorComboBox2->addItem( i18n( "Tint in" ) );
    colorComboBox2->addItem( i18n( "Tint out" ) );
    KColorButton *colorButton2 = new KColorButton;
    QSlider *colorSlider2 = new QSlider( Qt::Horizontal );
    disabledLayout->addWidget(colorLabel2, 2, 0);
    disabledLayout->addWidget(colorComboBox2, 2, 1);
    disabledLayout->addWidget(colorButton2, 2, 2);
    disabledLayout->addWidget(colorSlider2, 3, 0, 1, 3);
    QLabel *inactiveContrastLabel2 = new QLabel( i18n( "Contrast" ) );
    QComboBox *inactiveContrastComboBox2 = new QComboBox;
    inactiveContrastComboBox2->addItem( i18n( "Fade" ) );
    inactiveContrastComboBox2->addItem( i18n( "Tint in" ) );
    inactiveContrastComboBox2->addItem( i18n( "Tint out" ) );
    QSlider *inactiveContrastSlider2 = new QSlider( Qt::Horizontal );
    disabledLayout->addWidget(inactiveContrastLabel2, 4, 0);
    disabledLayout->addWidget(inactiveContrastComboBox2, 4, 1);
    disabledLayout->addWidget(inactiveContrastSlider2, 5, 0, 1, 3);
    QFrame *inactivePreview2 = new QFrame;
    disabledLayout->addWidget(inactivePreview2, 6, 0, 1, 3);

    effectsPageLayout->addWidget(inactiveBox);
    effectsPageLayout->addWidget(disabledBox);

    // connect signals/slots
    // TODO

    // finally, add UI's to tab widget
    tabWidget->addTab( schemePage, i18n("&Scheme"));
    tabWidget->addTab( effectsPage, i18n("&Effects"));
}

KColorCm::~KColorCm()
{
}

#include "colorscm.moc"
