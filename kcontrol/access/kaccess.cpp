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

  // read in bell settings
  config->setGroup("Bell");
  _systemBell = config->readBoolEntry("SystemBell", true);
  _artsBell = config->readBoolEntry("ArtsBell", false);
  _artsBellFile = config->readEntry("ArtsBellFile");
  _visibleBell = config->readBoolEntry("VisibleBell", true);
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



