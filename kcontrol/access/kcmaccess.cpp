/**
 *  kcmaccess.cpp
 *
 *  Copyright (c) 2000 Matthias Hölzer-Klüpfel
 *
 */


#include <stdlib.h>


#include <qtabwidget.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>


#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <krun.h>
#include <kurl.h>
#include <kinstance.h>
#include <kcolorbtn.h>
#include <kfiledialog.h>


#include "kcmaccess.moc"


KAccessConfig::KAccessConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QVBoxLayout *main = new QVBoxLayout(this, 0,0);
  QTabWidget *tab = new QTabWidget(this);
  main->addWidget(tab);

  // bell settings ---------------------------------------
  QWidget *bell = new QWidget(this);

  QVBoxLayout *vbox = new QVBoxLayout(bell, 6,6);
  
  QGroupBox *grp = new QGroupBox(i18n("Audible bell"), bell);
  vbox->addWidget(grp);

  QVBoxLayout *vvbox = new QVBoxLayout(grp, 6,6);
  vvbox->addSpacing(grp->fontMetrics().height());
  
  systemBell = new QCheckBox(i18n("Use &System bell"), grp);
  vvbox->addWidget(systemBell);
  customBell = new QCheckBox(i18n("Use &customized bell"), grp);
  vvbox->addWidget(customBell);

  QHBoxLayout *hbox = new QHBoxLayout(vvbox, 6);
  hbox->addSpacing(24);
  soundEdit = new QLineEdit(grp);
  soundLabel = new QLabel(soundEdit, i18n("Sound to play:"), grp);
  hbox->addWidget(soundLabel);
  hbox->addWidget(soundEdit);
  soundButton = new QPushButton(i18n("&Browse..."), grp);
  hbox->addWidget(soundButton);

  connect(soundButton, SIGNAL(clicked()), this, SLOT(selectSound()));

  connect(customBell, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(systemBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(customBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(soundEdit, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  
  tab->addTab(bell, i18n("&Bell"));
  // -----------------------------------------------------

  // visible bell ----------------------------------------
  grp = new QGroupBox(i18n("Visible bell"), bell);
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp, 6,6);
  vvbox->addSpacing(grp->fontMetrics().height());
  
  visibleBell = new QCheckBox(i18n("&Use visible bell"), grp);
  vvbox->addWidget(visibleBell);

  hbox = new QHBoxLayout(vvbox, 6);
  hbox->addSpacing(24);
  invertScreen = new QRadioButton(i18n("&Invert screen"), grp);
  hbox->addWidget(invertScreen);
  hbox = new QHBoxLayout(vvbox, 6);
  hbox->addSpacing(24);
  flashScreen = new QRadioButton(i18n("&Flash screen"), grp);
  hbox->addWidget(flashScreen);
  hbox->addSpacing(12);
  colorButton = new KColorButton(grp);
  colorButton->setFixedWidth(colorButton->sizeHint().height()*2);
  hbox->addWidget(colorButton);
  hbox->addStretch();

  hbox = new QHBoxLayout(vvbox, 6);
  hbox->addSpacing(24);

  durationSlider = new QSlider(100,2000,100,500,Qt::Horizontal,grp);
  durationLabel = new QLabel(durationSlider, i18n("&Duration: "), grp);
  durationMax= new QLabel(i18n("2000 ms"), grp);
  durationMin = new QLabel(i18n("100 ms"), grp);
  hbox->addWidget(durationLabel);
  hbox->addWidget(durationMin);
  hbox->addWidget(durationSlider);
  hbox->addWidget(durationMax);
  
  connect(invertScreen, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(flashScreen, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(visibleBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(visibleBell, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(colorButton, SIGNAL(clicked()), this, SLOT(configChanged()));
  
  connect(invertScreen, SIGNAL(clicked()), this, SLOT(invertClicked()));
  connect(flashScreen, SIGNAL(clicked()), this, SLOT(flashClicked()));  

  connect(durationSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));

  // -----------------------------------------------------

  vbox->addStretch();
  
  load();
}


KAccessConfig::~KAccessConfig() 
{
}


void KAccessConfig::selectSound()
{
  QStringList list = KGlobal::dirs()->findDirs("sound", "");
  QString start;
  if (list.count()>0)
    start = list[0];
  // TODO: Why only wav's? How can I find out what artsd supports?
  QString fname = KFileDialog::getOpenFileName(start, i18n("*.wav|WAV files"));
  if (!fname.isEmpty())
    soundEdit->setText(fname);
}


void KAccessConfig::configChanged()
{
  emit changed(true);
}


void KAccessConfig::load()
{
  KConfig *config = new KConfig("kaccessrc", true);
  
  config->setGroup("Bell");

  systemBell->setChecked(config->readBoolEntry("SystemBell", true));
  customBell->setChecked(config->readBoolEntry("ArtsBell", false));
  soundEdit->setText(config->readEntry("ArtsBellFile"));

  visibleBell->setChecked(config->readBoolEntry("VisibleBell", false));
  invertScreen->setChecked(config->readBoolEntry("VisibleBellInvert", true));
  flashScreen->setChecked(!invertScreen->isChecked());
  QColor def(Qt::red);
  colorButton->setColor(config->readColorEntry("VisibleBellColor", &def));
  
  durationSlider->setValue(config->readNumEntry("VisibleBellPause", 500));

  delete config;

  checkAccess();

  emit changed(false);
}


void KAccessConfig::save()
{
  KConfig *config= new KConfig("kaccessrc", false);

  config->setGroup("Bell");

  config->writeEntry("SystemBell", systemBell->isChecked());
  config->writeEntry("ArtsBell", customBell->isChecked());
  config->writeEntry("ArtsBellFile", soundEdit->text());

  config->writeEntry("VisibleBell", visibleBell->isChecked());
  config->writeEntry("VisibleBellInvert", invertScreen->isChecked());
  config->writeEntry("VisibleBellColor", colorButton->color());

  config->writeEntry("VisibleBellPause", durationSlider->value());

  config->sync();
  delete config;

  // restart kaccess
  // TODO: This is more than dirty
  system("killall kaccess; kaccess");

  emit changed(false);
}


void KAccessConfig::defaults()
{
  systemBell->setChecked(true);
  customBell->setChecked(false);
  soundEdit->setText("");

  visibleBell->setChecked(false);
  invertScreen->setChecked(true);
  flashScreen->setChecked(false);
  colorButton->setColor(QColor(Qt::red));
  
  durationSlider->setValue(500);

  checkAccess();

  emit changed(true);
}


void KAccessConfig::invertClicked()
{
  flashScreen->setChecked(false); 
}


void KAccessConfig::flashClicked()
{
  invertScreen->setChecked(false);
}


void KAccessConfig::checkAccess()
{
  bool custom = customBell->isChecked();
  soundEdit->setEnabled(custom);
  soundButton->setEnabled(custom);
  soundLabel->setEnabled(custom);

  bool visible = visibleBell->isChecked();
  invertScreen->setEnabled(visible);
  flashScreen->setEnabled(visible);
  colorButton->setEnabled(visible);

  durationSlider->setEnabled(visible);
  durationLabel->setEnabled(visible);
  durationMin->setEnabled(visible);
  durationMax->setEnabled(visible);
}


extern "C"
{
  KCModule *create_access(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmaccess");
    return new KAccessConfig(parent, name);
  };

  void init_access()
  {
    bool run=false;

    KConfig *config = new KConfig("kaccessrc");

    config->setGroup("Bell");
    
    if (!config->readBoolEntry("SystemBell", true))
      run = true;
    if (config->readBoolEntry("ArtsBell", false))
      run = true;
    if (config->readBoolEntry("VisibleBell", false))
      run = true;

    if (run)
      {
	// TODO: This is more than dirty
	system("kaccess");
      }
  }
}
