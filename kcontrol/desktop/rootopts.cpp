//
//
// "Root Options" Tab for KFM configuration
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998
//
// Layouts
// (c) Christian Tibirna 1998
// Port to KControl, split from Misc Tab
// (c) David Faure 1998

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>//CT - 12Nov1998
#include <kapp.h>

#include "rootopts.h"

#include "defaults.h"
#include <klocale.h> // include default values directly from kfm

//-----------------------------------------------------------------------------

KRootOptions::KRootOptions( QWidget *parent, const char *name )
    : KConfigWidget( parent, name )
{
  QGridLayout *lay = new QGridLayout(this, 9, 2, 10);
  
  QLabel *label;
  label = new QLabel( i18n("Horizontal Root Grid Spacing:"), this );
  lay->addWidget(label, 0, 0, AlignLeft);
  
  hspin  = new QSpinBox(0, DEFAULT_GRID_MAX - DEFAULT_GRID_MIN, 1, this);
  lay->addWidget(hspin, 0, 1, AlignLeft);
 
  label = new QLabel( i18n("Vertical Root Grid Spacing:"), this );
  lay->addWidget(label, 1, 0);
  
  vspin  = new QSpinBox(0, DEFAULT_GRID_MAX - DEFAULT_GRID_MIN, 1, this);
  lay->addWidget(vspin, 1, 1, AlignLeft);

  QFrame *hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);
  lay->addMultiCellWidget(hLine, 2, 2, 0, 1);
  
  iconstylebox = new QCheckBox(i18n("&Transparent Text for Desktop Icons."),
			       this);
  lay->addMultiCellWidget(iconstylebox,3, 3, 0, 1);
  
  connect(iconstylebox,SIGNAL(toggled(bool)),this,SLOT(makeBgActive(bool)));
  
  label = new QLabel(i18n("Icon foreground color:"),this);
  lay->addWidget(label, 4, 0, AlignLeft);
  
  fgColorBtn = new KColorButton(icon_fg,this);
  lay->addWidget(fgColorBtn, 4, 1, AlignLeft);
  connect( fgColorBtn, SIGNAL( changed( const QColor & ) ),
	   SLOT( slotIconFgColorChanged( const QColor & ) ) );
  
  bgLabel = new QLabel(i18n("Icon background color:"),this);
  lay->addWidget(bgLabel, 5, 0, AlignLeft);
  
  bgColorBtn = new KColorButton(icon_bg,this);
  lay->addWidget(bgColorBtn, 5, 1, AlignLeft);
  connect( bgColorBtn, SIGNAL( changed( const QColor & ) ),
	   SLOT( slotIconBgColorChanged( const QColor & ) ) );
  
  hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);
  lay->addMultiCellWidget(hLine, 6, 6, 0, 1);

  showHiddenBox = new QCheckBox(i18n("Show &Hidden Files on Desktop"), this);
  lay->addMultiCellWidget(showHiddenBox, 7, 7, 0, 1);

  lay->setRowStretch(8, 2);
  lay->activate();
  
  loadSettings();
}

void KRootOptions::loadSettings()
{
    // *** load ***

    // Root Icons settings
    g_pConfig->setGroup( "KFM Root Icons" );
    bool bTransparent = (bool)g_pConfig->readNumEntry("Style", DEFAULT_ROOT_ICONS_STYLE);
    bool bShowHidden = g_pConfig->readBoolEntry("ShowHidden", DEFAULT_SHOW_HIDDEN_ROOT_ICONS);
    //CT 12Nov1998
    icon_fg = g_pConfig->readColorEntry("Foreground",&DEFAULT_ICON_FG);
    icon_bg = g_pConfig->readColorEntry("Background",&DEFAULT_ICON_BG);
    //CT
    g_pConfig->setGroup( "KFM Misc Defaults" );
    int gridwidth = g_pConfig->readNumEntry( "GridWidth", DEFAULT_GRID_WIDTH );
    int gridheight = g_pConfig->readNumEntry( "GridHeight", DEFAULT_GRID_HEIGHT );
    // *** apply to GUI ***

    // Root Icon Settings
    iconstylebox->setChecked(bTransparent);
    makeBgActive(bTransparent);

    fgColorBtn->setColor(icon_fg);
    bgColorBtn->setColor(icon_bg);

    if(gridwidth - DEFAULT_GRID_MIN < 0 )
        gridwidth = DEFAULT_GRID_MIN;
    hspin->setValue(gridwidth - DEFAULT_GRID_MIN);

    if(gridheight - DEFAULT_GRID_MIN < 0 )
        gridheight = DEFAULT_GRID_MIN;
    vspin->setValue(gridheight - DEFAULT_GRID_MIN);

    showHiddenBox->setChecked(bShowHidden);
}

void KRootOptions::defaultSettings()
{
    // Root Icons Settings
    iconstylebox->setChecked((bool)DEFAULT_ROOT_ICONS_STYLE);
    makeBgActive((bool)DEFAULT_ROOT_ICONS_STYLE);
    fgColorBtn->setColor(DEFAULT_ICON_FG);
    icon_fg=DEFAULT_ICON_FG;//CT
    bgColorBtn->setColor(DEFAULT_ICON_BG);
    icon_bg=DEFAULT_ICON_BG;//CT
    hspin->setValue(DEFAULT_GRID_WIDTH - DEFAULT_GRID_MIN);
    vspin->setValue(DEFAULT_GRID_HEIGHT - DEFAULT_GRID_MIN);
    showHiddenBox->setChecked(DEFAULT_SHOW_HIDDEN_ROOT_ICONS);
}

void KRootOptions::saveSettings()
{
    // Root Icons Settings
    g_pConfig->setGroup( "KFM Root Icons" );
    g_pConfig->writeEntry( "Style", iconstylebox->isChecked() ? 1 : 0);
    g_pConfig->writeEntry("ShowHidden", showHiddenBox->isChecked());

    //CT 12Nov1998
    g_pConfig->writeEntry( "Foreground", icon_fg);
    g_pConfig->writeEntry( "Background", icon_bg);
    //CT
    g_pConfig->setGroup( "KFM Misc Defaults" );
    g_pConfig->writeEntry( "GridWidth", hspin->value()+DEFAULT_GRID_MIN);
    g_pConfig->writeEntry( "GridHeight", vspin->value()+DEFAULT_GRID_MIN);

    g_pConfig->sync();
}

void KRootOptions::applySettings()
{
    saveSettings();
}

//CT 12Nov1998
void KRootOptions::slotIconFgColorChanged(const QColor &col) {
    if ( icon_fg != col )
        icon_fg = col;
}
 
void KRootOptions::slotIconBgColorChanged(const QColor &col) {
    if ( icon_bg != col )
        icon_bg = col;
}
 
void KRootOptions::makeBgActive(bool a) {
  bgColorBtn->setEnabled(!a);
  bgLabel->setEnabled(!a);
}
//CT

#include "rootopts.moc"
