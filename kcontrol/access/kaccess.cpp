#include <unistd.h>
#include <stream.h>


#include <qtimer.h>
#include <qpainter.h>


#include <kdebug.h>
#include <kaudioplayer.h>
#include <kconfig.h>
#include <kglobal.h>


#include "kaccess.moc"


KAccessApp::KAccessApp(bool allowStyles, bool GUIenabled)
  : KUniqueApplication(allowStyles, GUIenabled), overlay(0)
{
  // verify the Xlib has matching XKB extension
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  if (!XkbLibraryVersion(&major, &minor))
    {
      cerr << "Xlib XKB extension does not match" << endl;
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
      cerr << "X server has not matching XKB extension" << endl;
      exit(-1);
    }
  kdDebug() << "X server XKB extension major=" << major << " minor=" << minor << endl;

  readSettings();
}


void KAccessApp::readSettings()
{
  KConfig *config = KGlobal::config();

  // bell ---------------------------------------------------------------

  config->setGroup("Bell");
  _systemBell = config->readBoolEntry("SystemBell", true);
  _artsBell = config->readBoolEntry("ArtsBell", false);
  _artsBellFile = config->readEntry("ArtsBellFile");
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
  
  // do not forget to turn it back on if kaccess exits!
  uint ctrls=XkbAudibleBellMask;
  uint values=XkbAudibleBellMask;
  XkbSetAutoResetControls(qt_xdisplay(), XkbAudibleBellMask, &ctrls, &values);

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

  // slow keys
  if (config->readBoolEntry("SlowKeys", false))
    {
      xkb->ctrls->slow_keys_delay = config->readNumEntry("SlowKeysDelay", 500);
      xkb->ctrls->enabled_ctrls |= XkbSlowKeysMask;
    }
  else
      xkb->ctrls->enabled_ctrls &= ~XkbSlowKeysMask;

  // bounce keys
  if (config->readBoolEntry("BounceKeys", false))
    {
      xkb->ctrls->debounce_delay = config->readNumEntry("BounceKeysDelay", 500);
      xkb->ctrls->enabled_ctrls |= XkbBounceKeysMask;
    }
  else
      xkb->ctrls->enabled_ctrls &= ~XkbBounceKeysMask;

  // set keyboard state
  XkbSetControls(qt_xdisplay(), XkbControlsEnabledMask, xkb);

  // reset them after program exit
  ctrls = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask;
  values = 0;
  XkbSetAutoResetControls(qt_xdisplay(), ctrls, &ctrls, &values);
}


bool KAccessApp::x11EventFilter(XEvent *event)
{
  // handle XKB events
  if (event->type == xkb_opcode)
    {
      XkbAnyEvent *ev = (XkbAnyEvent*) event;

      if (ev->xkb_type == XkbBellNotify)
	xkbBellNotify((XkbBellNotifyEvent*)event);

      return true;
    }

  // process other events as usual
  return false;
}


void VisualBell::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);
  QTimer::singleShot(_pause, this, SLOT(hide()));
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
	{
	  overlay = new VisualBell(_visibleBellPause);
	  overlay->setGeometry(0,0,desktop()->width(),desktop()->height());
	}

      // if requested, invert the screen
      if (_visibleBellInvert)
	{
	  QPixmap screen = QPixmap::grabWindow(desktop()->winId(),
					       0, 0, desktop()->width(), desktop()->height());
	  QPixmap invert(desktop()->width(), desktop()->height());
	  QPainter p(&invert);
	  p.setRasterOp(QPainter::NotCopyROP);
	  p.drawPixmap(0, 0, screen);
	  overlay->setBackgroundPixmap(invert);
	}
      else
	overlay->setBackgroundColor(_visibleBellColor);
      
      // flash the overlay widget
      overlay->show();
      flushX();
    }      

  // ask artsd to ring a nice bell
  if (_artsBell)
    KAudioPlayer::play(_artsBellFile);
}



