#ifndef _CURSOR_
#define _CURSOR_

#include <kapp.h>
#include <kcontrol.h>
#include <klocale.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

const int numcursors=14;

class KCursorConfig : public KConfigWidget
{
  Q_OBJECT
public:
  QButtonGroup * qbg;
  KCursorConfig( QWidget *parent=0, const char *name=0, bool init=FALSE );
  ~KCursorConfig();
  void loadSettings();
  void applySettings();
  void setCustom(QString);
  void GetSettings();
  void saveParams();
  void initCheck();

public slots:
  void wastoggled(int);
  void browseSelected();

protected:
  virtual void resizeEvent(QResizeEvent *);

private:
  const QCursor * cursors[numcursors];
  QRadioButton * cnames[numcursors];
  QPushButton * browse;
  QPoint calcpos(int);
  KConfig * config;
  int mysel;
  QString custom;

};

class KCursorApplication : public KControlApplication
{
public:

  KCursorApplication(int &argc, char **arg, const char *name);
  ~KCursorApplication();
  void init();
  void apply();

private:

  KCursorConfig * CursorCfg;

};

#endif
