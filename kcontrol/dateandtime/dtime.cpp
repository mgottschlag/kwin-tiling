/*
 *  dtime.cpp
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <QComboBox>
#include <q3groupbox.h>
#include <QPushButton>
#include <QPainter>
#include <QLayout>
#include <QLabel>

#include <QCheckBox>
#include <QRegExp>
//Added by qt3to4:
#include <QPaintEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
//Added by qt3to4:
#include <QApplication>
#include <QPolygon>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kconfig.h>

#include "dtime.h"
#include "dtime.moc"

HMSTimeWidget::HMSTimeWidget(QWidget *parent) :
	KIntSpinBox(parent)
{
}

QString HMSTimeWidget::mapValueToText(int value)
{
  QString s = QString::number(value);
  if( value < 10 ) {
    s = '0' + s;
  }
  return s;
}

Dtime::Dtime(QWidget * parent, const char *name)
  : QWidget(parent, name)
{
  // *************************************************************
  // Start Dialog
  // *************************************************************

  // Time Server

  privateLayoutWidget = new QWidget( this, "layout1" );
  QHBoxLayout *layout1 = new QHBoxLayout( privateLayoutWidget );
  layout1->setObjectName( "ntplayout" );
  layout1->setSpacing( 0 );
  layout1->setMargin( 0 );

  setDateTimeAuto = new QCheckBox( privateLayoutWidget, "setDateTimeAuto" );
  setDateTimeAuto->setText(i18n("Set date and time &automatically:"));
  connect(setDateTimeAuto, SIGNAL(toggled(bool)), this, SLOT(serverTimeCheck()));
  connect(setDateTimeAuto, SIGNAL(toggled(bool)), SLOT(configChanged()));
  layout1->addWidget( setDateTimeAuto );

  timeServerList = new QComboBox( privateLayoutWidget );
  timeServerList->setObjectName( "timeServerList" );
  timeServerList->setEditable(false);
  connect(timeServerList, SIGNAL(activated(int)), SLOT(configChanged()));
  connect(timeServerList, SIGNAL(textChanged(const QString &)), SLOT(configChanged()));
  connect(setDateTimeAuto, SIGNAL(toggled(bool)), timeServerList, SLOT(setEnabled(bool)));
  timeServerList->setEnabled(false);
  timeServerList->setEditable(true);
  layout1->addWidget( timeServerList );
  findNTPutility();

  // Date box
  Q3GroupBox* dateBox = new Q3GroupBox( this, "dateBox" );

  QVBoxLayout *l1 = new QVBoxLayout( dateBox );
  l1->setMargin( 0 );
  l1->setSpacing( KDialog::spacingHint() );

  cal = new KDatePicker( dateBox );
  cal->setMinimumSize(cal->sizeHint());
  l1->addWidget( cal );
  cal->setWhatsThis( i18n("Here you can change the system date's day of the month, month and year.") );

  // Time frame
  Q3GroupBox* timeBox = new Q3GroupBox( this, "timeBox" );

  QVBoxLayout *v2 = new QVBoxLayout( timeBox );
  v2->setMargin( 0 );
  v2->setSpacing( KDialog::spacingHint() );

  kclock = new Kclock( timeBox, "kclock" );
  kclock->setMinimumSize(150,150);
  v2->addWidget( kclock );

  QGridLayout *v3 = new QGridLayout( );
  v2->addItem( v3 );

  // Even if the module's widgets are reversed (usually when using RTL
  // languages), the placing of the time fields must always be H:M:S, from
  // left to right.
  bool isRTL = QApplication::isRightToLeft();

  QSpacerItem *spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  v3->addItem(spacer1, 0, 1, 2, 1);

  hour = new HMSTimeWidget( timeBox );
  hour->setWrapping(true);
  hour->setMaximum(23);
  #warning fixme hour->setValidator(new KStrictIntValidator(0, 23, hour));
  v3->addWidget(hour, 0, isRTL ? 6 : 2, 2, 1);

  QLabel *dots1 = new QLabel(":", timeBox);
  dots1->setMinimumWidth( 7 );
  dots1->setAlignment( Qt::AlignCenter );
  v3->addWidget(dots1, 0, 3, 2, 1);

  minute = new HMSTimeWidget( timeBox );
  minute->setWrapping(true);
  minute->setMinimum(0);
  minute->setMaximum(59);
  #warning fixme minute->setValidator(new KStrictIntValidator(0, 59, minute));
  v3->addWidget(minute, 0, 4, 2, 1);

  QLabel *dots2 = new QLabel(":", timeBox);
  dots2->setMinimumWidth( 7 );
  dots2->setAlignment( Qt::AlignCenter );
  v3->addWidget(dots2, 0, 5, 2, 1);

  second = new HMSTimeWidget( timeBox );
  second->setWrapping(true);
  second->setMinimum(0);
  second->setMaximum(59);
  #warning fixme second->setValidator(new KStrictIntValidator(0, 59, second));
  v3->addWidget(second, 0, isRTL ? 2 : 6, 2, 1);

  v3->addColSpacing(7, 7);

  QString wtstr = i18n("Here you can change the system time. Click into the"
    " hours, minutes or seconds field to change the relevant value, either"
    " using the up and down buttons to the right or by entering a new value.");
  hour->setWhatsThis( wtstr );
  minute->setWhatsThis( wtstr );
  second->setWhatsThis( wtstr );

  QSpacerItem *spacer3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  v3->addItem(spacer3, 0, 9, 2, 1);

  QGridLayout *top = new QGridLayout( this );
  top->setSpacing( KDialog::spacingHint() );

  top->addWidget(dateBox, 1,0);
  top->addWidget(timeBox, 1,1);
  top->addWidget(privateLayoutWidget, 0, 0, 1, 2 );

  // *************************************************************
  // End Dialog
  // *************************************************************

  connect( hour, SIGNAL(valueChanged(int)), SLOT(set_time()) );
  connect( minute, SIGNAL(valueChanged(int)), SLOT(set_time()) );
  connect( second, SIGNAL(valueChanged(int)), SLOT(set_time()) );
  connect( cal, SIGNAL(dateChanged(QDate)), SLOT(changeDate(QDate)));

  connect( &internalTimer, SIGNAL(timeout()), SLOT(timeout()) );

  load();

  if (getuid() != 0)
    {
      cal->setEnabled(false);
      hour->setEnabled(false);
      minute->setEnabled(false);
      second->setEnabled(false);
      timeServerList->setEnabled(false);
      setDateTimeAuto->setEnabled(false);
    }
  kclock->setEnabled(false);
}

void Dtime::serverTimeCheck() {
  bool enabled = !setDateTimeAuto->isChecked();
  cal->setEnabled(enabled);
  hour->setEnabled(enabled);
  minute->setEnabled(enabled);
  second->setEnabled(enabled);
  //kclock->setEnabled(enabled);
}

void Dtime::findNTPutility(){
  KProcess proc;
  proc << "which" << "ntpdate";
  proc.start(KProcess::Block);
  if(proc.exitStatus() == 0) {
    ntpUtility = "ntpdate";
    kDebug() << "ntpUtility = " << ntpUtility.toLatin1() << endl;
    return;
  }
  proc.clearArguments();
  proc << "which" << "rdate";
  proc.start(KProcess::Block);
  if(proc.exitStatus() == 0) {
    ntpUtility = "rdate";
    kDebug() << "ntpUtility = " << ntpUtility.toLatin1() << endl;
    return;
  }
  privateLayoutWidget->hide();
  kDebug() << "ntpUtility not found!" << endl;
}

void Dtime::set_time()
{
  if( ontimeout )
    return;

  internalTimer.stop();

  time.setHMS( hour->value(), minute->value(), second->value() );
  kclock->setTime( time );

  emit timeChanged( true );
}

void Dtime::changeDate(QDate d)
{
  date = d;
  emit timeChanged( true );
}

void Dtime::configChanged(){
  emit timeChanged( true );
}

void Dtime::load()
{
  KConfig config("kcmclockrc", true, false);
  config.setGroup("NTP");
  timeServerList->addItems(QStringList::split(',', config.readEntry("servers",
    i18n("Public Time Server (pool.ntp.org),\
asia.pool.ntp.org,\
europe.pool.ntp.org,\
north-america.pool.ntp.org,\
oceania.pool.ntp.org"))));
  setDateTimeAuto->setChecked(config.readEntry("enabled", false));

  // Reset to the current date and time
  time = QTime::currentTime();
  date = QDate::currentDate();
  cal->setDate(date);

  // start internal timer
  internalTimer.start( 1000 );

  timeout();
}

void Dtime::save()
{
  KConfig config("kcmclockrc", false, false);
  config.setGroup("NTP");

  // Save the order, but don't duplicate!
  QStringList list;
  if( timeServerList->count() != 0)
    list.append(timeServerList->currentText());
  for ( int i=0; i<timeServerList->count();i++ ) {
    QString text = timeServerList->text(i);
    if( !list.contains(text) )
      list.append(text);
    // Limit so errors can go away and not stored forever
    if( list.count() == 10)
      break;
  }
  config.writeEntry("servers", list);
  config.writeEntry("enabled", setDateTimeAuto->isChecked());

  if(setDateTimeAuto->isChecked() && !ntpUtility.isEmpty()){
    // NTP Time setting
    QString timeServer = timeServerList->currentText();
    if( timeServer.indexOf( QRegExp(".*\\(.*\\)$") ) != -1 ) {
      timeServer.replace( QRegExp(".*\\("), "" );
      timeServer.replace( QRegExp("\\).*"), "" );
      // Would this be better?: s/^.*\(([^)]*)\).*$/\1/
    }
    KProcess proc;
    proc << ntpUtility << timeServer;
    proc.start( KProcess::Block );
    if( proc.exitStatus() != 0 ){
      KMessageBox::error( this, i18n(QString("Unable to contact time server: %1.").arg(timeServer).toLatin1()));
      setDateTimeAuto->setChecked( false );
    }
    else {
        // success
        kDebug() << "Set date from time server " << timeServer.toLatin1() << " success!" << endl;
    }
  }
  else {
    // User time setting
    KProcess c_proc;

  // BSD systems reverse year compared to Susv3
#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
    BufS.sprintf("%04d%02d%02d%02d%02d.%02d",
               date.year(),
               date.month(), date.day(),
               hour->value(), minute->value(), second->value());
#else
    BufS.sprintf("%02d%02d%02d%02d%04d.%02d",
               date.month(), date.day(),
               hour->value(), minute->value(),
               date.year(), second->value());
#endif

    kDebug() << "Set date " << BufS << endl;

    c_proc << "date" << BufS;
    c_proc.start( KProcess::Block );
    int result = c_proc.exitStatus();
    if (result != 0
#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
  	  && result != 2	// can only set local date, which is okay
#endif
      ) {
      KMessageBox::error( this, i18n("Can not set date."));
      return;
    }

    // try to set hardware clock. We do not care if it fails
    KProcess hwc_proc;
    hwc_proc << "hwclock" << "--systohc";
    hwc_proc.start(KProcess::Block);
  }

  // restart time
  internalTimer.start( 1000 );
}

void Dtime::timeout()
{
  // get current time
  time = QTime::currentTime();

  ontimeout = true;
  second->setValue(time.second());
  minute->setValue(time.minute());
  hour->setValue(time.hour());
  ontimeout = false;

  kclock->setTime( time );
}

QString Dtime::quickHelp() const
{
  return i18n("<h1>Date & Time</h1> This control module can be used to set the system date and"
    " time. As these settings do not only affect you as a user, but rather the whole system, you"
    " can only change these settings when you start the Control Center as root. If you do not have"
    " the root password, but feel the system time should be corrected, please contact your system"
    " administrator.");
}

void Kclock::setTime(const QTime &time)
{
  this->time = time;
  repaint();
}

void Kclock::paintEvent( QPaintEvent * )
{
  if ( !isVisible() )
    return;

  QPainter paint;
  paint.begin( this );

  QPolygon pts;
  QPoint cp = rect().center();
  int d = qMin(width(),height());
  QColor hands =  palette().color(QPalette::Dark );
  QColor shadow =  palette().color( QPalette::Text );
  paint.setPen( shadow );
  paint.setBrush( shadow );
  paint.setViewport(4,4,width(),height());

  for ( int c=0 ; c < 2 ; c++ )
    {
      QMatrix matrix;
      matrix.translate( cp.x(), cp.y() );
      matrix.scale( d/1000.0F, d/1000.0F );

      // lancetta delle ore
      float h_angle = 30*(time.hour()%12-3) + time.minute()/2;
      matrix.rotate( h_angle );
      paint.setMatrix( matrix );
      pts.setPoints( 4, -20,0, 0,-20, 300,0, 0,20 );
      paint.drawPolygon( pts );
      matrix.rotate( -h_angle );

      // lancetta dei minuti
      float m_angle = (time.minute()-15)*6;
      matrix.rotate( m_angle );
      paint.setMatrix( matrix );
      pts.setPoints( 4, -10,0, 0,-10, 400,0, 0,10 );
      paint.drawPolygon( pts );
      matrix.rotate( -m_angle );

      // lancetta dei secondi
      float s_angle = (time.second()-15)*6;
      matrix.rotate( s_angle );
      paint.setMatrix( matrix );
      pts.setPoints(4,0,0,0,0,400,0,0,0);
      paint.drawPolygon( pts );
      matrix.rotate( -s_angle );

      // quadrante
      for ( int i=0 ; i < 60 ; i++ )
        {
          paint.setMatrix( matrix );
          if ( (i % 5) == 0 )
            paint.drawLine( 450,0, 500,0 ); // draw hour lines
          else  paint.drawPoint( 480,0 );   // draw second lines
          matrix.rotate( 6 );
        }

      paint.setPen( hands );
      paint.setBrush( hands );
      paint.setViewport(0,0,width(),height());
    }
  paint.end();
}

QValidator::State KStrictIntValidator::validate( QString & input, int & d ) const
{
  if( input.isEmpty() )
    return Valid;

  State st = QIntValidator::validate( input, d );

  if( st == Intermediate )
    return Invalid;

  return st;
}
