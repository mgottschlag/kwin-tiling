//#include <services/system/daemons.h>
//using namespace COAS::System::Daemons;

#include <qlayout.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <klistbox.h>
#include <kcombobox.h>
#include <ktextbrowser.h>
#include <kiconloader.h>

#include "daemons.h"

ButtonWidget::ButtonWidget(QWidget *parent, const char *name)
  :QWidget(parent, name)
{
  _iconlbl = new QLabel(this);
  _iconlbl->setFixedSize(48,48);
  _startbtn = new QPushButton(i18n("&Start"), this); 
  _stopbtn = new QPushButton(i18n("Sto&p"), this);
  _restartbtn = new QPushButton(i18n("&Restart"), this);

  connect(_startbtn, SIGNAL(clicked()), SIGNAL(startClicked()));
  connect(_stopbtn, SIGNAL(clicked()), SIGNAL(stopClicked()));
  connect(_restartbtn, SIGNAL(clicked()), SIGNAL(restartClicked()));

  setMinimumWidth(52);
  setMinimumHeight(124);
}

void ButtonWidget::resizeEvent(QResizeEvent*)
{
  _restartbtn->setGeometry(2, height()-24, width()-4, 22);
  _stopbtn->setGeometry(2, height()-48, width()-4, 22);
  _startbtn->setGeometry(2, height()-72, width()-4, 22);

  _iconlbl->move(width() - 50, 2);
}

void ButtonWidget::setIcon(const QPixmap& icon)
{
  _iconlbl->setPixmap(icon);
}

QSize ButtonWidget::sizeHint () const
{
  return QSize(_startbtn->width() + 4, 140);
}

DaemonsConfig::DaemonsConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  // main layout
  QVBoxLayout *mainlayout = new QVBoxLayout(this);

  // group hbox
  QHBox *groupBox = new QHBox(this);
  groupBox->setSpacing(5);

  // group combo label
  QLabel *grpLabel = new QLabel(i18n("&Available daemons:"), groupBox);

  // group combo box
  KComboBox *_groupCombo = new KComboBox(groupBox);
  grpLabel->setBuddy(_groupCombo);
  
  // give the group combo as much space as possible
  groupBox->setStretchFactor(_groupCombo, 2);

  // daemon listbox
  _daemonList = new KListBox(this);
  _daemonList->setMinimumHeight(150);
  _daemonList->setMaximumHeight(200);

  QHBox *subBox = new QHBox(this);
  subBox->setSpacing(5);
  
  // setup descriptuion text browser
  _textBrowser = new KTextBrowser(subBox);
  QColorGroup clgrp = colorGroup();
  clgrp.setColor( QColorGroup::Base, QColor( 255, 255, 224 ) );
  _textBrowser->setPaperColorGroup( clgrp );
  _textBrowser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  _textBrowser->setFocusPolicy( NoFocus );
  _textBrowser->setHScrollBarMode( QScrollView::AlwaysOff );
  _textBrowser->setNotifyClick(true);

  // add reasonable stretch factor for the text browser
  subBox->setStretchFactor(_textBrowser, 2);

  // widget containing the start/stop/restart buttons and the daemons icon
  _btnWidget = new ButtonWidget(subBox);
  _btnWidget->setIcon(KGlobal::iconLoader()->loadIcon("unknown", KIconLoader::Large));
  connect(_btnWidget, SIGNAL(startClicked()), SLOT(startDaemon()));
  connect(_btnWidget, SIGNAL(stopClicked()), SLOT(stopDaemon()));
  connect(_btnWidget, SIGNAL(restartClicked()), SLOT(restartDaemon()));

  // give the mainlayout something to manage
  mainlayout->addWidget(groupBox);
  mainlayout->addSpacing(5);
  mainlayout->addWidget(_daemonList);
  mainlayout->addSpacing(5);
  mainlayout->addWidget(subBox);
  mainlayout->setStretchFactor(subBox,2);

  load();
}

DaemonsConfig::~DaemonsConfig() {}

QSize DaemonsConfig::sizeHint () const
{
  return QSize(400,400);
}

void DaemonsConfig::startDaemon()
{
  // TODO : start current daemon
  qDebug("DaemonsConfig::startDaemon()");
}
void DaemonsConfig::stopDaemon()
{
  // TODO : stop current daemon
  qDebug("DaemonsConfig::stopDaemon()");
}

void DaemonsConfig::restartDaemon()
{
  // TODO : restart current daemon
  qDebug("DaemonsConfig::restartDaemon()");
}

void DaemonsConfig::load()
{
  // TODO : get data from COAS++ backend
  
  emit changed(false);
}

void DaemonsConfig::save()
{
  // TODO : set data

  emit changed(false);
}

void DaemonsConfig::defaults()
{
  // TODO

  emit changed(true);
}

extern "C"
{
  KCModule *create_daemons(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmdaemons");
    return new DaemonsConfig(parent, name);
  };
}
