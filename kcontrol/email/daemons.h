#ifndef __daemons_h__
#define __daemons_h__

#include <qpixmap.h>

#include <kcmodule.h>

class QPushButton;
class QLabel;

class KComboBox;
class KListBox;
class KTextBrowser;

class ButtonWidget : public QWidget
{
  Q_OBJECT;

 public:
  ButtonWidget(QWidget *parent = 0L, const char *name = 0L);

  QSize sizeHint () const;

  void setIcon(const QPixmap& icon);

 signals:
  void startClicked();
  void stopClicked();
  void restartClicked();

 protected:
  void resizeEvent(QResizeEvent*);

 private:
  QPushButton *_startbtn, *_stopbtn, *_restartbtn;
  QLabel *_iconlbl;
};

class DaemonsConfig : public KCModule
{
  Q_OBJECT;
  
 public:
  DaemonsConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~DaemonsConfig();
  
  void load();
  void save();
  void defaults();
  
  int buttons();

  QSize sizeHint() const;

 protected slots:
  void startDaemon();
  void stopDaemon();
  void restartDaemon();

 private:
  KComboBox *_groupCombo;
  KListBox  *_daemonList;
  KTextBrowser *_textBrowser;
  ButtonWidget *_btnWidget;
};

#endif
