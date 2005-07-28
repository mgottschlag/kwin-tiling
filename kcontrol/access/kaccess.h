#ifndef __K_ACCESS_H__
#define __K_ACCESS_H__


#include <qwidget.h>
#include <qcolor.h>
//Added by qt3to4:
#include <QLabel>
#include <QPaintEvent>


#include <kuniqueapplication.h>
#include <kwinmodule.h>


#include <X11/Xlib.h>
#define explicit int_explicit        // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#undef explicit

class KDialogBase;
class QLabel;
class KComboBox;

class KAccessApp : public KUniqueApplication
{
  Q_OBJECT

public:

  KAccessApp(bool allowStyles=true, bool GUIenabled=true);

  bool x11EventFilter(XEvent *event);
  
  int newInstance();


protected:

  void readSettings();

  void xkbBellNotify(XkbBellNotifyEvent *event);
  void xkbControlsNotify(XkbControlsNotifyEvent *event);


private slots:

  void activeWindowChanged(WId wid);
  void slotArtsBellTimeout();
  void applyChanges();
  void yesClicked();
  void noClicked();
  void dialogClosed();


private:
   void  createDialogContents();

  int xkb_opcode;
  unsigned int features;
  unsigned int requestedFeatures;

  bool    _systemBell, _artsBell, _visibleBell, _visibleBellInvert;
  bool    _artsBellBlocked;
  QString _artsBellFile;
  QColor  _visibleBellColor;
  int     _visibleBellPause;

  bool    _gestures, _gestureConfirmation;

  QWidget *overlay;

  QTimer *artsBellTimer;

  KWinModule wm;

  WId _activeWindow;

  KDialogBase *dialog;
  QLabel *featuresLabel;
  KComboBox *showModeCombobox;
};


class VisualBell : public QWidget
{
  Q_OBJECT

public:

  VisualBell(int pause) 
    : QWidget(0, 0, Qt::WX11BypassWM), _pause(pause)
    {};

  
protected:
  
  void paintEvent(QPaintEvent *);


private:

  int _pause;

};




#endif
