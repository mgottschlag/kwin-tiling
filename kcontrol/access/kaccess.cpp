#include <unistd.h>


#include <qtimer.h>
#include <qpainter.h>

#include <kmessagebox.h>
#include <kdebug.h>
#include <kaudioplayer.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <netwm.h>
#include <kshortcut.h>
#include <kkeynative.h>

#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>


#include "kaccess.moc"


KAccessApp::KAccessApp(bool allowStyles, bool GUIenabled)
  : KUniqueApplication(allowStyles, GUIenabled), _artsBellBlocked(false),
                                                 overlay(0), wm(0, KWinModule::INFO_DESKTOP)
{
  // verify the Xlib has matching XKB extension
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  if (!XkbLibraryVersion(&major, &minor))
    {
      kdError() << "Xlib XKB extension does not match" << endl;
      exit(-1);
    }
  kdDebug() << "Xlib XKB extension major=" << major << " minor=" << minor << endl;

  // verify the X server has matching XKB extension
  // if yes, the XKB extension is initialized
  int opcode_rtrn;
  int error_rtrn;
  if (!XkbQueryExtension(qt_xdisplay(), &opcode_rtrn, &xkb_opcode, &error_rtrn,
			 &major, &minor))
    {
      kdError() << "X server has not matching XKB extension" << endl;
      exit(-1);
    }
  kdDebug() << "X server XKB extension major=" << major << " minor=" << minor << endl;

  _activeWindow = wm.activeWindow();
  connect(&wm, SIGNAL(activeWindowChanged(WId)), this, SLOT(activeWindowChanged(WId)));

  artsBellTimer = new QTimer( this );
  connect( artsBellTimer, SIGNAL( timeout() ), SLOT( slotArtsBellTimeout() ));

  features = 0;
}

int KAccessApp::newInstance()
{
  KGlobal::config()->reparseConfiguration();
  readSettings();
  return 0;
}

void KAccessApp::readSettings()
{
  KConfig *config = KGlobal::config();

  // bell ---------------------------------------------------------------

  config->setGroup("Bell");
  _systemBell = config->readBoolEntry("SystemBell", true);
  _artsBell = config->readBoolEntry("ArtsBell", false);
  _artsBellFile = config->readPathEntry("ArtsBellFile");
  _visibleBell = config->readBoolEntry("VisibleBell", false);
  _visibleBellInvert = config->readBoolEntry("VisibleBellInvert", false);
  QColor def(Qt::red);
  _visibleBellColor = config->readColorEntry("VisibleBellColor", &def);
  _visibleBellPause = config->readNumEntry("VisibleBellPause", 500);

  // select bell events if we need them
  int state = (_artsBell || _visibleBell) ? XkbBellNotifyMask : 0;
  XkbSelectEvents(qt_xdisplay(), XkbUseCoreKbd, XkbBellNotifyMask, state);

  // deactivate system bell if not needed
  if (!_systemBell)
    XkbChangeEnabledControls(qt_xdisplay(), XkbUseCoreKbd, XkbAudibleBellMask, 0);
  else
    XkbChangeEnabledControls(qt_xdisplay(), XkbUseCoreKbd, XkbAudibleBellMask, XkbAudibleBellMask);

  // keyboard -------------------------------------------------------------

  config->setGroup("Keyboard");

  // get keyboard state
  XkbDescPtr xkb = XkbGetMap(qt_xdisplay(), 0, XkbUseCoreKbd);
  if (!xkb)
    return;
  if (XkbGetControls(qt_xdisplay(), XkbAllControlsMask, xkb) != Success)
    return;

  // sticky keys
  if (config->readBoolEntry("StickyKeys", false))
    {
      if (config->readBoolEntry("StickyKeysLatch", true))
	xkb->ctrls->ax_options |= XkbAX_LatchToLockMask;
      else
	xkb->ctrls->ax_options &= ~XkbAX_LatchToLockMask;
      xkb->ctrls->enabled_ctrls |= XkbStickyKeysMask;
    }
  else
    xkb->ctrls->enabled_ctrls &= ~XkbStickyKeysMask;

  // turn off two-keys-disable sticky keys option
  xkb->ctrls->ax_options &= ~XkbAX_TwoKeysMask;

  // turn off timeout
  xkb->ctrls->enabled_ctrls &= ~XkbAccessXTimeoutMask;

  // slow keys
  if (config->readBoolEntry("SlowKeys", false))
      xkb->ctrls->enabled_ctrls |= XkbSlowKeysMask;
  else
      xkb->ctrls->enabled_ctrls &= ~XkbSlowKeysMask;
  xkb->ctrls->slow_keys_delay = config->readNumEntry("SlowKeysDelay", 500);

  // bounce keys
  if (config->readBoolEntry("BounceKeys", false))
      xkb->ctrls->enabled_ctrls |= XkbBounceKeysMask;
  else
      xkb->ctrls->enabled_ctrls &= ~XkbBounceKeysMask;
  xkb->ctrls->debounce_delay = config->readNumEntry("BounceKeysDelay", 500);

  // gestures for enabling the other features
  if (config->readBoolEntry("Gestures", true))
      xkb->ctrls->enabled_ctrls |= XkbAccessXKeysMask;
  else
      xkb->ctrls->enabled_ctrls &= ~XkbAccessXKeysMask;

  // mouse-by-keyboard ----------------------------------------------

  config->setGroup("Mouse");

  if (config->readBoolEntry("MouseKeys", false))
    {
      xkb->ctrls->mk_delay = config->readNumEntry("MKDelay", 160);
      xkb->ctrls->mk_interval = config->readNumEntry("MKInterval", 5);
      xkb->ctrls->mk_time_to_max = config->readNumEntry("MKTimeToMax", 1000);
      xkb->ctrls->mk_max_speed = config->readNumEntry("MKMaxSpeed", 500);
      xkb->ctrls->mk_curve = config->readNumEntry("MKCurve", 0);
      xkb->ctrls->mk_dflt_btn = config->readNumEntry("MKDefaultButton", 0);

      xkb->ctrls->enabled_ctrls |= XkbMouseKeysMask;
    }
  else
    xkb->ctrls->enabled_ctrls &= ~XkbMouseKeysMask;

   features = xkb->ctrls->enabled_ctrls & (XkbSlowKeysMask | XkbBounceKeysMask | XkbStickyKeysMask | XkbMouseKeysMask);
  // set state
  XkbSetControls(qt_xdisplay(), XkbControlsEnabledMask | XkbMouseKeysAccelMask | XkbStickyKeysMask | XkbAccessXKeysMask, xkb);

  // select AccessX events if we need them
  config->setGroup("Keyboard");
  bool gestures = config->readBoolEntry("Gestures", true);
  state = gestures ? XkbControlsNotifyMask : 0;
  XkbSelectEvents(qt_xdisplay(), XkbUseCoreKbd, XkbControlsNotifyMask, state);

  // reset them after program exit
  uint ctrls = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbMouseKeysMask | XkbAudibleBellMask | XkbControlsNotifyMask;
  uint values = XkbAudibleBellMask;
  XkbSetAutoResetControls(qt_xdisplay(), ctrls, &ctrls, &values);

  delete overlay;
  overlay = 0;
}


bool KAccessApp::x11EventFilter(XEvent *event)
{
  // handle XKB events
  if (event->type == xkb_opcode)
    {
      XkbAnyEvent *ev = (XkbAnyEvent*) event;

      if (ev->xkb_type == XkbBellNotify)
	xkbBellNotify((XkbBellNotifyEvent*)event);
      else if (ev->xkb_type == XkbControlsNotify)
	xkbControlsNotify((XkbControlsNotifyEvent*)event);

      return true;
    }

  // process other events as usual
  return KApplication::x11EventFilter(event);
}


void VisualBell::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);
  QTimer::singleShot(_pause, this, SLOT(hide()));
}


void KAccessApp::activeWindowChanged(WId wid)
{
  _activeWindow = wid;
}


void KAccessApp::xkbBellNotify(XkbBellNotifyEvent *event)
{
  // bail out if we should not really ring
  if (event->event_only)
    return;

  // flash the visible bell
  if (_visibleBell)
    {
      // create overlay widget
      if (!overlay)
        overlay = new VisualBell(_visibleBellPause);

      WId id = _activeWindow;

      NETRect frame, window;
      NETWinInfo net(qt_xdisplay(), id, desktop()->winId(), 0);

      net.kdeGeometry(frame, window);

      overlay->setGeometry(window.pos.x, window.pos.y, window.size.width, window.size.height);

      if (_visibleBellInvert)
        {
	  QPixmap screen = QPixmap::grabWindow(id, 0, 0, window.size.width, window.size.height);
	  QPixmap invert(window.size.width, window.size.height);
	  QPainter p(&invert);
	  p.setRasterOp(QPainter::NotCopyROP);
	  p.drawPixmap(0, 0, screen);
	  overlay->setBackgroundPixmap(invert);
	}
      else
	overlay->setBackgroundColor(_visibleBellColor);

      // flash the overlay widget
      overlay->raise();
      overlay->show();
      flushX();
    }

  // ask artsd to ring a nice bell
  if (_artsBell && !_artsBellBlocked ) {
    KAudioPlayer::play(_artsBellFile);
    _artsBellBlocked = true;
    artsBellTimer->start( 300, true );
  }
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
    keyname = i18n("Meta") + "+" + keyname;
  if ((modifiers & WinMask) != 0)
    keyname = KKey::modFlagLabel(KKey::WIN) + "+" + keyname;
  if ((modifiers & AltMask) != 0)
    keyname = KKey::modFlagLabel(KKey::ALT) + "+" + keyname;
  if ((modifiers & ControlMask) != 0)
    keyname = KKey::modFlagLabel(KKey::CTRL) + "+" + keyname;
  if ((modifiers & ShiftMask) != 0)
    keyname = KKey::modFlagLabel(KKey::SHIFT) + "+" + keyname;

  return keyname;
}

void KAccessApp::xkbControlsNotify(XkbControlsNotifyEvent *event)
{
   unsigned int newFeatures = event->enabled_ctrls & (XkbSlowKeysMask | XkbBounceKeysMask | XkbStickyKeysMask | XkbMouseKeysMask);
   if (newFeatures != features) {
     unsigned int enabled  = newFeatures & ~features;
     unsigned int disabled = features & ~newFeatures;
     features = newFeatures;

     QString message;
     QString explanation;
     if (disabled == 0) {
        switch (enabled) {
        case 0:
           return;
        case XkbSlowKeysMask:
           message = i18n("\"Slow keys \"is now enabled.");
           explanation = i18n("This may be because you held down the Shift key for 8 seconds "
           "or because an application has enabled this feature.\n"
           "You can configure \"slow keys\" and other accessibility features in the "
           "Regional & Accessibility->Accessibility module of the "
           "KDE Control Center.\n"
           "To quickly disable slow keys, press the Shift key for another 8 seconds.");
           break;
        case XkbBounceKeysMask:
           message = i18n("\"Bounce keys\" is now enabled.");
           explanation = i18n("This may be because an application has enabled this feature.\n"
           "You can configure \"bounce keys\" and other accessibility features in the "
           "Regional & Accessibility->Accessibility module of the "
           "KDE Control Center.");
           break;
        case XkbStickyKeysMask:
           message = i18n("\"Sticky keys\" is now enabled.");
           explanation = i18n("This may be because you pressed the Shift key 5 consecutive "
           "times or because an application has enabled this feature.\n"
           "You can configure \"sticky keys\" and other accessibility features in the "
           "Regional & Accessibility->Accessibility module of the "
           "KDE Control Center.\n"
           "To quickly disable sticky keys, press the Shift key for another 5 times.");
           break;
        case XkbMouseKeysMask: {
           message = i18n("\"Mouse keys\" is now enabled.");
           QString shortcut = mouseKeysShortcut(qt_xdisplay());
           if (shortcut.isEmpty())
              explanation = i18n("This may be because an application has enabled this feature.\n"
              "You can configure \"mouse keys\" in the Peripherals->Mouse "
              "module of the KDE Control Center.");
           else
              explanation = i18n("This may be because you pressed %1 "
              "or because an application has enabled this feature.\n"
              "You can configure \"mouse keys\" in the Peripherals->Mouse "
              "module of the KDE Control Center.\n"
              "To quickly disable mouse keys, press %2 again.")
              .arg(shortcut).arg(shortcut);
           }
           break;
        default:
           message = i18n("Multiple AccessX features have been enabled.");
           explanation = i18n("This may be because an application has enabled these features.\n"
           "You can configure them in the "
           "Regional & Accessibility->Accessibility module of the "
           "KDE Control Center.");
        }
     }
     else if (enabled == 0) {
        switch (disabled) {
        case XkbSlowKeysMask:
           message = i18n("\"Slow keys\" is now disabled.");
           explanation = i18n("This may be because you held down the Shift key for 8 seconds "
           "or because an application has disabled this feature.\n"
           "You can configure \"slow keys\" and other accessibility features in the "
           "Regional & Accessibility->Accessibility module of the "
           "KDE Control Center.");
           break;
        case XkbBounceKeysMask:
           message = i18n("\"Bounce keys\" is now disabled.");
           explanation = i18n("This may be because an application has disabled this feature.\n"
           "You can configure bounce keys and other accessibility features in the "
           "Regional & Accessibility->Accessibility module of the "
           "KDE Control Center.");
           break;
        case XkbStickyKeysMask:
           message = i18n("\"Sticky keys\" is now disabled.");
           explanation = i18n("This may be because you pressed the Shift key 5 consecutive "
           "times or because an application has disabled this feature.\n"
           "You can configure sticky keys and other accessibility features in the "
           "Regional & Accessibility->Accessibility module of the "
           "KDE Control Center.");
           break;
        case XkbMouseKeysMask: {
           message = i18n("\"Mouse keys\" is now disabled.");
           QString shortcut = mouseKeysShortcut(qt_xdisplay());
           if (shortcut.isEmpty())
              explanation = i18n("This may be because an application has disabled this feature.\n"
              "You can configure \"mouse keys\" in the Peripherals->Mouse "
              "module of the KDE Control Center.");
           else
              explanation = i18n("This may be because you pressed %1 "
              "or because an application has disabled this feature.\n"
              "You can configure \"mouse keys\" in the Peripherals->Mouse "
              "module of the KDE Control Center.").arg(shortcut);
           }
           break;
        default:
           message = i18n("Multiple AccessX features have been disabled.");
           explanation = i18n("This may be because an application has disabled these features.\n"
           "You can configure them in the "
           "Regional & Accessibility->Accessibility module of the "
           "KDE Control Center.");
        }
     }
     else {
        message = i18n("Multiple AccessX features have been changed.");
        explanation = i18n("This may be because an application has changed these features.\n"
        "You can configure them in the "
        "Regional & Accessibility->Accessibility module of the "
        "KDE Control Center.");
     }

     KMessageBox::information (0, message+"\n"+explanation);
     
     KConfig *config = KGlobal::config();

     config->setGroup("Keyboard");

     if (enabled & XkbSlowKeysMask)
        config->writeEntry("SlowKeys", true);
     else if (disabled & XkbSlowKeysMask)
        config->writeEntry("SlowKeys", false);

     if (enabled & XkbBounceKeysMask)
        config->writeEntry("BounceKeys", true);
     else if (disabled & XkbBounceKeysMask)
        config->writeEntry("BounceKeys", false);

     if (enabled & XkbStickyKeysMask)
        config->writeEntry("StickyKeys", true);
     else if (disabled & XkbStickyKeysMask)
        config->writeEntry("StickyKeys", false);

     config->setGroup("Mouse");

     if (enabled & XkbMouseKeysMask)
        config->writeEntry("MouseKeys", true);
     else if (disabled & XkbMouseKeysMask)
        config->writeEntry("MouseKeys", false);

     config->sync();
  }
}

void KAccessApp::slotArtsBellTimeout()
{
  _artsBellBlocked = false;
}

