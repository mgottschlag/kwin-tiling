//-----------------------------------------------------------------------------
//
// KDE Display background setup module
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __BACKGND_H__
#define __BACKGND_H__

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qbuttongroup.h>

#include <knuminput.h>
#include <kcolordlg.h>
#include <kcolorbtn.h>
#include <kcontrol.h>

#include "display.h"

class KBackground;
class KIntNumInput;
struct PatternEntry;

class KItem : public QObject
{
  
public:

  KItem() : QObject() {}

  KItem ( const KItem &itm ): QObject() // deep copy
    {
	*this = itm;
    }

  KItem &operator=( const KItem &itm )	// deep copy
    {
      color1 = itm.color1;
      color2 = itm.color2;
      wpMode = itm.wpMode;
      ncMode = itm.ncMode;
      stMode = itm.stMode;
      orMode = itm.orMode;
      bUseWallpaper = itm.bUseWallpaper;
      wallpaper = itm.wallpaper.data();
      pattern = itm.pattern;
      return *this;
    }

  QColor color1;
  QColor color2;
  int wpMode;
  int ncMode;
  int stMode; 
  int orMode;
  QString pattern;

  bool bUseWallpaper;
  QString wallpaper;
};

class KBGMonitor : public QWidget
{
  Q_OBJECT
public:
  KBGMonitor( QWidget *parent ) : QWidget( parent ) {
    setAcceptDrops( true);
  };

  // we don't want no steenking palette change
  virtual void setPalette( const QPalette & ) {};
signals:
  void imageDropped( QDropEvent *);
protected:
  virtual void dropEvent( QDropEvent *);
  virtual void dragEnterEvent( QDragEnterEvent *);
};

class KRenameDeskDlg : public QDialog
{
  Q_OBJECT
public:
  KRenameDeskDlg( const QString& t, QWidget *parent );
  virtual ~KRenameDeskDlg() {}

  QString title()
    {  return edit->text(); }

private:
  QLineEdit *edit;
};

class KBPatternDlg : public QDialog
{
  Q_OBJECT

public:
  KBPatternDlg( QColor color1, QColor color2, QString *name, int *orient,
		int *type, QWidget *parent = 0, char *name = 0 );
  ~KBPatternDlg();
    
protected slots:
  void selected(int index);
  virtual void done ( int r );
  void slotMode( int );
    
private:
  QLabel *preview;
  QLabel *lPreview;
  QLabel *lName;
  QListBox *listBox;
  QList<PatternEntry> *list;
  QCheckBox *orientCB;
  QRadioButton *rbVert;
  QRadioButton *rbHoriz;
  QRadioButton *rbPattern;
  QButtonGroup *suGroup;
	
  enum { Portrait = 1, Landscape, GreyMap };
	
  bool changed;
  QString current;
  QString *pattern;
  int *orMode;
  int *tpMode;
  int mode;
    
  QColor color1, color2;
};


class KRandomDlg : public QDialog
{
  Q_OBJECT

public:
  KRandomDlg(int _desktop, KBackground *_kb, char *name = 0 );
  ~KRandomDlg() {}

  friend class KBackground;

protected:
  void addToPicList( QString pic );
  void readSettings();
  void copyCurrent();
  virtual void dropEvent(QDropEvent *event);
  virtual void dragEnterEvent( QDragEnterEvent *);

protected slots:
  void selected( int index );
  void slotDelete();
  void slotAdd();
  void changeDir();
  void slotBrowse ();

  virtual void done ( int r );

private:
  QLabel *desktopLabel;
  QListBox *listBox;
  QList<QString> list;
  QCheckBox *orderButton;

  QCheckBox *dirCheckBox;
  QLineEdit *dirLined;
  QPushButton *dirPushButton;

  KIntNumInput* timerNumInput;

  KBackground *kb;
  QList<KItem> ItemList;

  bool useDir;
  QString picDir;

  int count;
  int delay;
  bool inorder;

  bool changed;
  int desktop;
  int item;
};


class KBackground : public KDisplayModule
{
  Q_OBJECT
public:
	
  enum { Portrait = 1, Landscape };
  enum { Flat = 1, Gradient, Pattern, GreyMap };

  KBackground(QWidget *parent, Mode mode);

  virtual void readSettings( int deskNum = 0 );
  virtual void apply( bool force = FALSE );
  virtual void loadSettings();
  virtual void applySettings();
  virtual void defaultSettings();

  friend class KItem;
  friend class KRandomDlg;

protected slots:

  void slotSelectColor1( const QColor &col );
  void slotSelectColor2( const QColor &col );
  void slotBrowse();
  void slotWallpaper( const QString & );
  void slotWallpaperMode( int );
  void slotColorMode( int );
  void slotStyleMode( int );
  void slotSwitchDesk( int );
  void slotRenameDesk();
  void slotSetup2Color();
  void slotSetupRandom();
  void slotToggleRandom();
  void slotToggleOneDesktop();
  void slotToggleDock();
  void slotDropped(QDropEvent *event);

protected:
  void getDeskNameList();
  void setDesktop( int );
  void showSettings();
  void writeSettings( int deskNum );
  void setMonitor();
  bool loadWallpaper( const QString& filename, bool useContext = TRUE );
  void retainResources();
  void setDefaults();

  void resizeEvent( QResizeEvent * );

  bool setNew( QString pic, int item );

protected:
  enum { Tiled = 1,
	 Mirrored,
	 CenterTiled,
	 Centred,
	 CentredBrick,
	 CentredWarp,
	 CentredMaxpect,
	 SymmetricalTiled,
	 SymmetricalMirrored,
	 Scaled };

  enum { OneColor = 1, TwoColor };

  QListBox     *deskListBox;

  QCheckBox *oneDesktopButton;
  QCheckBox *dockButton;
  QCheckBox *randomButton;

  QRadioButton *rbPattern;
  QRadioButton *rbGradient;

  KColorButton *colButton1;
  KColorButton *colButton2;

  QButtonGroup *ncGroup;

  QPushButton  *renameButton;
  QPushButton  *changeButton;
  QPushButton  *randomSetupButton;
  QPushButton  *browseButton;

  QComboBox *wpModeCombo;
  QComboBox *wpCombo;

  KIntNumInput* cacheSlider;

  QLabel *monitorLabel;
  KBGMonitor* monitor;

  QPixmap wpPixmap;
  QString deskName;
  QStrList deskNames;

  int deskNum;
  int random;
  int maxDesks;

  KItem currentItem;
  KRandomDlg *rnddlg;

  bool changed;

  bool randomMode;
  bool oneDesktopMode;
  bool interactive;
  bool docking;

};


#endif
