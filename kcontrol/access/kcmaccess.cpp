/**
 *  kcmaccess.cpp
 *
 *  Copyright (c) 2000 Matthias H�zer-Klpfel
 *
 */


#include <stdlib.h>

#include <dcopref.h>

#include <qtabwidget.h>
#include <qlayout.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>


#include <kstandarddirs.h>
#include <kcolorbutton.h>
#include <kfiledialog.h>
#include <knuminput.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kshortcut.h>
#include <kkeynative.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#include "kcmaccess.moc"

static bool needToRunKAccessDaemon( KConfig * )
{
   // Since we have the gesture settings we always have to start
   // the KAccess daemon as we do not know whether the gestures
   // are activated by default in the X configuration or not.
   return true;
}

QString mouseKeysShortcut (Display *display) {
  // Calculate the keycode
  KeySym sym = XK_MouseKeys_Enable;
  KeyCode code = XKeysymToKeycode(display, sym);
  if (code == 0) {
     sym = XK_Pointer_EnableKeys;
     code = XKeysymToKeycode(display, sym);
     if (code == 0)
        return ""; // No shortcut available?
  }

  // Calculate the modifiers by searching the keysym in the X keyboard mapping
  XkbDescPtr xkbdesc = XkbGetMap(display, XkbKeyTypesMask | XkbKeySymsMask, XkbUseCoreKbd);
  if (!xkbdesc)
     return ""; // Failed to obtain the mapping from server

  bool found = false;
  unsigned char modifiers = 0;
  int groups = XkbKeyNumGroups(xkbdesc, code);
  for (int grp = 0; grp < groups && !found; grp++)
  {
     int levels = XkbKeyGroupWidth(xkbdesc, code, grp);
     for (int level = 0; level < levels && !found; level++)
     {
        if (sym == XkbKeySymEntry(xkbdesc, code, level, grp))
        {
           // keysym found => determine modifiers
           int typeIdx = xkbdesc->map->key_sym_map[code].kt_index[grp];
           XkbKeyTypePtr type = &(xkbdesc->map->types[typeIdx]);
           for (int i = 0; i < type->map_count && !found; i++)
           {
              if (type->map[i].active && (type->map[i].level == level))
              {
                 modifiers = type->map[i].mods.mask;
                 found = true;
              }
           }
        }
     }
  }
  XkbFreeClientMap (xkbdesc, 0, true);

  if (!found)
     return ""; // Somehow the keycode -> keysym mapping is flawed
  
  XEvent ev;
  ev.xkey.display = display;
  ev.xkey.keycode = code;
  ev.xkey.state = 0;
  KKey key = KKey(KKeyNative(&ev));
  QString keyname = key.toString();

  unsigned int AltMask   = KKeyNative::modX(KKey::ALT);
  unsigned int WinMask   = KKeyNative::modX(KKey::WIN);
  unsigned int NumMask   = KKeyNative::modXNumLock();
  unsigned int ScrollMask= KKeyNative::modXScrollLock();

  unsigned int MetaMask  = XkbKeysymToModifiers (display, XK_Meta_L);
  unsigned int SuperMask = XkbKeysymToModifiers (display, XK_Super_L);
  unsigned int HyperMask = XkbKeysymToModifiers (display, XK_Hyper_L);
  unsigned int AltGrMask = XkbKeysymToModifiers (display, XK_Mode_switch)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Shift)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Latch)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Lock);
  
  unsigned int mods = ShiftMask | ControlMask | AltMask | WinMask
                    | LockMask | NumMask | ScrollMask;

  AltGrMask &= ~mods;
  MetaMask  &= ~(mods | AltGrMask);
  SuperMask &= ~(mods | AltGrMask | MetaMask);
  HyperMask &= ~(mods | AltGrMask | MetaMask | SuperMask);

  if ((modifiers & AltGrMask) != 0)
    keyname = i18n("AltGraph") + "+" + keyname;
  if ((modifiers & HyperMask) != 0)
    keyname = i18n("Hyper") + "+" + keyname;
  if ((modifiers & SuperMask) != 0)
    keyname = i18n("Super") + "+" + keyname;
  if ((modifiers & WinMask) != 0)
    keyname = KKey::modFlagLabel(KKey::WIN) + "+" + keyname;
  if ((modifiers & AltMask) != 0)
    keyname = KKey::modFlagLabel(KKey::ALT) + "+" + keyname;
  if ((modifiers & ControlMask) != 0)
    keyname = KKey::modFlagLabel(KKey::CTRL) + "+" + keyname;
  if ((modifiers & ShiftMask) != 0)
    keyname = KKey::modFlagLabel(KKey::SHIFT) + "+" + keyname;

  QString result;
  if ((modifiers & ScrollMask) != 0)
    if ((modifiers & LockMask) != 0)
      if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock, CapsLock and ScrollLock are active");
      else
        result = i18n("Press %1 while CapsLock and ScrollLock are active");
    else if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock and ScrollLock are active");
      else
        result = i18n("Press %1 while ScrollLock is active");
  else if ((modifiers & LockMask) != 0)
      if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock and CapsLock are active");
      else
        result = i18n("Press %1 while CapsLock is active");
    else if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock is active");
      else
        result = i18n("Press %1");
  
  return result.arg(keyname);
}

KAccessConfig::KAccessConfig(QWidget *parent, const char *)
  : KCModule(parent, "kcmaccess")
{

  KAboutData *about =
  new KAboutData(I18N_NOOP("kaccess"), I18N_NOOP("KDE Accessibility Tool"),
                 0, 0, KAboutData::License_GPL,
                 I18N_NOOP("(c) 2000, Matthias Hoelzer-Kluepfel"));

  about->addAuthor("Matthias Hoelzer-Kluepfel", I18N_NOOP("Author") , "hoelzer@kde.org");

  setAboutData( about );

  QVBoxLayout *main = new QVBoxLayout(this, 0, KDialogBase::spacingHint());
  QTabWidget *tab = new QTabWidget(this);
  main->addWidget(tab);

  // bell settings ---------------------------------------
  QWidget *bell = new QWidget(this);

  QVBoxLayout *vbox = new QVBoxLayout(bell, KDialogBase::marginHint(), 
      KDialogBase::spacingHint());

  Q3GroupBox *grp = new Q3GroupBox(i18n("Audible Bell"), bell);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  QVBoxLayout *vvbox = new QVBoxLayout(grp->layout(), 
      KDialogBase::spacingHint());

  systemBell = new QCheckBox(i18n("Use &system bell"), grp);
  vvbox->addWidget(systemBell);
  customBell = new QCheckBox(i18n("Us&e customized bell"), grp);
  vvbox->addWidget(customBell);
  systemBell->setWhatsThis( i18n("If this option is checked, the default system bell will be used. See the"
    " \"System Bell\" control module for how to customize the system bell."
    " Normally, this is just a \"beep\".") );
  customBell->setWhatsThis( i18n("Check this option if you want to use a customized bell, playing"
    " a sound file. If you do this, you will probably want to turn off the system bell.<p> Please note"
    " that on slow machines this may cause a \"lag\" between the event causing the bell and the sound being played.") );

  QHBoxLayout *hbox = new QHBoxLayout(vvbox, KDialogBase::spacingHint());
  hbox->addSpacing(24);
  soundEdit = new QLineEdit(grp);
  soundLabel = new QLabel(soundEdit, i18n("Sound &to play:"), grp);
  hbox->addWidget(soundLabel);
  hbox->addWidget(soundEdit);
  soundButton = new QPushButton(i18n("Browse..."), grp);
  hbox->addWidget(soundButton);
  QString wtstr = i18n("If the option \"Use customized bell\" is enabled, you can choose a sound file here."
    " Click \"Browse...\" to choose a sound file using the file dialog.");
  soundEdit->setWhatsThis( wtstr );
  soundLabel->setWhatsThis( wtstr );
  soundButton->setWhatsThis( wtstr );

  connect(soundButton, SIGNAL(clicked()), this, SLOT(selectSound()));

  connect(customBell, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(systemBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(customBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(soundEdit, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));

  // -----------------------------------------------------

  // visible bell ----------------------------------------
  grp = new Q3GroupBox(i18n("Visible Bell"), bell);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  visibleBell = new QCheckBox(i18n("&Use visible bell"), grp);
  vvbox->addWidget(visibleBell);
  visibleBell->setWhatsThis( i18n("This option will turn on the \"visible bell\", i.e. a visible"
    " notification shown every time that normally just a bell would occur. This is especially useful"
    " for deaf people.") );

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  invertScreen = new QRadioButton(i18n("I&nvert screen"), grp);
  hbox->addWidget(invertScreen);
  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  invertScreen->setWhatsThis( i18n("All screen colors will be inverted for the amount of time specified below.") );
  hbox->addSpacing(24);
  flashScreen = new QRadioButton(i18n("F&lash screen"), grp);
  hbox->addWidget(flashScreen);
  flashScreen->setWhatsThis( i18n("The screen will turn to a custom color for the amount of time specified below.") );
  hbox->addSpacing(12);
  colorButton = new KColorButton(grp);
  colorButton->setFixedWidth(colorButton->sizeHint().height()*2);
  hbox->addWidget(colorButton);
  hbox->addStretch();
  colorButton->setWhatsThis( i18n("Click here to choose the color used for the \"flash screen\" visible bell.") );

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);

  durationSlider = new KIntNumInput(grp);
  durationSlider->setRange(100, 2000, 100);
  durationSlider->setLabel(i18n("Duration:"));
  durationSlider->setSuffix(i18n(" msec"));
  hbox->addWidget(durationSlider);
  durationSlider->setWhatsThis( i18n("Here you can customize the duration of the \"visible bell\" effect being shown.") );

  connect(invertScreen, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(flashScreen, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(visibleBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(visibleBell, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(colorButton, SIGNAL(clicked()), this, SLOT(changeFlashScreenColor()));

  connect(invertScreen, SIGNAL(clicked()), this, SLOT(invertClicked()));
  connect(flashScreen, SIGNAL(clicked()), this, SLOT(flashClicked()));

  connect(durationSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));

  vbox->addStretch();

  // -----------------------------------------------------

  tab->addTab(bell, i18n("&Bell"));


  // keys settings ---------------------------------------
  QWidget *keys = new QWidget(this);

  vbox = new QVBoxLayout(keys, KDialog::marginHint(), KDialog::spacingHint());

  grp = new Q3GroupBox(i18n("S&ticky Keys"), keys);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  stickyKeys = new QCheckBox(i18n("Use &sticky keys"), grp);
  vvbox->addWidget(stickyKeys);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  stickyKeysLock = new QCheckBox(i18n("&Lock sticky keys"), grp);
  hbox->addWidget(stickyKeysLock);

  grp = new Q3GroupBox(i18n("Slo&w Keys"), keys);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  slowKeys = new QCheckBox(i18n("&Use slow keys"), grp);
  vvbox->addWidget(slowKeys);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  slowKeysDelay = new KIntNumInput(grp);
  slowKeysDelay->setSuffix(i18n(" msec"));
  slowKeysDelay->setRange(100, 2000, 100);
  slowKeysDelay->setLabel(i18n("Dela&y:"));
  hbox->addWidget(slowKeysDelay);


  grp = new Q3GroupBox(i18n("Bounce Keys"), keys);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  bounceKeys = new QCheckBox(i18n("Use bou&nce keys"), grp);
  vvbox->addWidget(bounceKeys);

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  bounceKeysDelay = new KIntNumInput(grp);
  bounceKeysDelay->setSuffix(i18n(" msec"));
  bounceKeysDelay->setRange(100, 2000, 100);
  bounceKeysDelay->setLabel(i18n("D&elay:"));
  hbox->addWidget(bounceKeysDelay);

  grp = new Q3GroupBox(i18n("Activation Gestures"), keys);
  grp->setColumnLayout( 0, Qt::Horizontal );
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout(grp->layout(), KDialog::spacingHint());

  gestures = new QCheckBox(i18n("Use gestures for activating the above features"), grp);
  vvbox->addWidget(gestures);
  QString shortcut = mouseKeysShortcut(this->x11Display());
  if (shortcut.isEmpty())
    gestures->setWhatsThis( i18n("Here you can activate keyboard gestures that turn on the following features: \n"
    "Sticky keys: Press Shift key 5 consecutive times\n"
    "Slow keys: Hold down Shift for 8 seconds"));
  else
    gestures->setWhatsThis( i18n("Here you can activate keyboard gestures that turn on the following features: \n"
    "Mouse Keys: %1\n"
    "Sticky keys: Press Shift key 5 consecutive times\n"
    "Slow keys: Hold down Shift for 8 seconds").arg(shortcut));

  hbox = new QHBoxLayout(vvbox, KDialog::spacingHint());
  hbox->addSpacing(24);
  gestureConfirmation = new QCheckBox(i18n("Show a confirmation dialog whenever a gesture is used"), grp);
  hbox->addWidget(gestureConfirmation);
  gestureConfirmation->setWhatsThis( i18n("KDE will show a confirmation dialog whenever a gesture is used if this option is checked.\nBe carefull do know what yo do if you uncheck it as then the AccessX settings will always be applied without confirmation.") );

//XK_MouseKeys_Enable,XK_Pointer_EnableKeys
  connect(stickyKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(stickyKeysLock, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(slowKeysDelay, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
  connect(slowKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(slowKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(stickyKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(bounceKeysDelay, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
  connect(bounceKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(bounceKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(gestures, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(gestures, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(gestureConfirmation, SIGNAL(clicked()), this, SLOT(configChanged()));

  vbox->addStretch();

  tab->addTab(keys, i18n("&Keyboard"));

  load();
}


KAccessConfig::~KAccessConfig()
{
}

void KAccessConfig::changeFlashScreenColor()
{
    invertScreen->setChecked(false);
    flashScreen->setChecked(true);
    configChanged();
}

void KAccessConfig::selectSound()
{
  QStringList list = KGlobal::dirs()->findDirs("sound", "");
  QString start;
  if (list.count()>0)
    start = list[0];
  // TODO: Why only wav's? How can I find out what artsd supports?
  QString fname = KFileDialog::getOpenFileName(start, i18n("*.wav|WAV Files"));
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
  soundEdit->setText(config->readPathEntry("ArtsBellFile"));

  visibleBell->setChecked(config->readBoolEntry("VisibleBell", false));
  invertScreen->setChecked(config->readBoolEntry("VisibleBellInvert", true));
  flashScreen->setChecked(!invertScreen->isChecked());
  QColor def(Qt::red);
  colorButton->setColor(config->readColorEntry("VisibleBellColor", &def));

  durationSlider->setValue(config->readNumEntry("VisibleBellPause", 500));


  config->setGroup("Keyboard");

  stickyKeys->setChecked(config->readBoolEntry("StickyKeys", false));
  stickyKeysLock->setChecked(config->readBoolEntry("StickyKeysLatch", true));
  slowKeys->setChecked(config->readBoolEntry("SlowKeys", false));
  slowKeysDelay->setValue(config->readNumEntry("SlowKeysDelay", 500));
  bounceKeys->setChecked(config->readBoolEntry("BounceKeys", false));
  bounceKeysDelay->setValue(config->readNumEntry("BounceKeysDelay", 500));
  gestures->setChecked(config->readBoolEntry("Gestures", true));
  gestureConfirmation->setChecked(config->readBoolEntry("GestureConfirmation", true));


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
  config->writePathEntry("ArtsBellFile", soundEdit->text());

  config->writeEntry("VisibleBell", visibleBell->isChecked());
  config->writeEntry("VisibleBellInvert", invertScreen->isChecked());
  config->writeEntry("VisibleBellColor", colorButton->color());

  config->writeEntry("VisibleBellPause", durationSlider->value());


  config->setGroup("Keyboard");

  config->writeEntry("StickyKeys", stickyKeys->isChecked());
  config->writeEntry("StickyKeysLatch", stickyKeysLock->isChecked());

  config->writeEntry("SlowKeys", slowKeys->isChecked());
  config->writeEntry("SlowKeysDelay", slowKeysDelay->value());

  config->writeEntry("BounceKeys", bounceKeys->isChecked());
  config->writeEntry("BounceKeysDelay", bounceKeysDelay->value());
  
  config->writeEntry("Gestures", gestures->isChecked());
  config->writeEntry("GestureConfirmation", gestureConfirmation->isChecked());


  config->sync();

  if (systemBell->isChecked() ||
      customBell->isChecked() ||
      visibleBell->isChecked())
  {
    KConfig cfg("kdeglobals", false, false);
    cfg.setGroup("General");
    cfg.writeEntry("UseSystemBell", true);
    cfg.sync();
  }

  // make kaccess reread the configuration
  if ( needToRunKAccessDaemon( config ) )
      kapp->startServiceByDesktopName("kaccess");

  else // don't need it -> kill it
  {
      DCOPRef kaccess( "kaccess", "qt/kaccess" );
      kaccess.send( "quit" );
  }

  delete config;
  
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

  slowKeys->setChecked(false);
  slowKeysDelay->setValue(500);

  bounceKeys->setChecked(false);
  bounceKeysDelay->setValue(500);

  stickyKeys->setChecked(false);
  stickyKeysLock->setChecked(true);

  gestures->setChecked(true);
  gestureConfirmation->setChecked(true);

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

  stickyKeysLock->setEnabled(stickyKeys->isChecked());

  slowKeysDelay->setEnabled(slowKeys->isChecked());

  bounceKeysDelay->setEnabled(bounceKeys->isChecked());

  gestureConfirmation->setEnabled(gestures->isChecked());
}

extern "C"
{
  KDE_EXPORT KCModule *create_access(QWidget *parent, const char *name)
  {
    return new KAccessConfig(parent, name);
  }

  /* This one gets called by kcminit

   */
  KDE_EXPORT void init_access()
  {
    KConfig *config = new KConfig("kaccessrc", true, false);
    bool run = needToRunKAccessDaemon( config );

    delete config;
    if (run)
      kapp->startServiceByDesktopName("kaccess");
  }
}


