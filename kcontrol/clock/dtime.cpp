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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <qcombobox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kdialog.h>

#include "dtime.h"
#include "dtime.moc"

HMSTimeWidget::HMSTimeWidget(QWidget *parent, const char *name) :
	KIntSpinBox(parent, name)
{
}

QString HMSTimeWidget::mapValueToText(int value)
{
  QString s = QString::number(value);
  if( value < 10 ) {
    s = "0" + s;
  }
  return s;
}

Dtime::Dtime(QWidget * parent, const char *name)
  : QWidget(parent, name)
{
  // *************************************************************
  // Start Dialog
  // *************************************************************

  // Date box
  QGroupBox* dateBox = new QGroupBox( this, "dateBox" );

  QVBoxLayout *l1 = new QVBoxLayout( dateBox, KDialog::spacingHint() );

  cal = new KDatePicker( dateBox );
  cal->setMinimumSize(cal->sizeHint());
  l1->addWidget( cal );
  QWhatsThis::add( cal, i18n("Here you can change the system date's day of the month, month and year.") );

  // Time frame
  QGroupBox* timeBox = new QGroupBox( this, "timeBox" );

  QVBoxLayout *v2 = new QVBoxLayout( timeBox, KDialog::spacingHint() );

  kclock = new Kclock( timeBox, "kclock" );
  kclock->setMinimumSize(150,150);
  v2->addWidget( kclock );

  QGridLayout *v3 = new QGridLayout( v2, 2, 9 );

  // Even if the module's widgets are reversed (usually when using RTL
  // languages), the placing of the time fields must always be H:M:S, from
  // left to right.
  bool isRTL = QApplication::reverseLayout();

  QSpacerItem *spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  v3->addMultiCell(spacer1, 0, 1, 1, 1);

  hour = new HMSTimeWidget( timeBox );
  hour->setWrapping(true);
  hour->setMaxValue(23);
  hour->setValidator(new KStrictIntValidator(0, 23, hour));
  v3->addMultiCellWidget(hour, 0, 1, isRTL ? 6 : 2, isRTL ? 6 : 2 );

  QLabel *dots1 = new QLabel(":", timeBox);
  dots1->setMinimumWidth( 7 );
  dots1->setAlignment( QLabel::AlignCenter );
  v3->addMultiCellWidget(dots1, 0, 1, 3, 3 );

  minute = new HMSTimeWidget( timeBox );
  minute->setWrapping(true);
  minute->setMinValue(0);
  minute->setMaxValue(59);
  minute->setValidator(new KStrictIntValidator(0, 59, minute));
  v3->addMultiCellWidget(minute, 0, 1, 4, 4 );

  QLabel *dots2 = new QLabel(":", timeBox);
  dots2->setMinimumWidth( 7 );
  dots2->setAlignment( QLabel::AlignCenter );
  v3->addMultiCellWidget(dots2, 0, 1, 5, 5 );

  second = new HMSTimeWidget( timeBox );
  second->setWrapping(true);
  second->setMinValue(0);
  second->setMaxValue(59);
  second->setValidator(new KStrictIntValidator(0, 59, second));
  v3->addMultiCellWidget(second, 0, 1, isRTL ? 2 : 6, isRTL ? 2 : 6 );

  v3->addColSpacing(7, 7);

  QString wtstr = i18n("Here you can change the system time. Click into the"
    " hours, minutes or seconds field to change the relevant value, either"
    " using the up and down buttons to the right or by entering a new value.");
  QWhatsThis::add( hour, wtstr );
  QWhatsThis::add( minute, wtstr );
  QWhatsThis::add( second, wtstr );

  QSpacerItem *spacer3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  v3->addMultiCell(spacer3, 0, 1, 9, 9);

  QHBoxLayout *top = new QHBoxLayout( this, 0, KDialog::spacingHint() );
  top->addWidget(dateBox, 1);
  top->addWidget(timeBox, 1);

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
      kclock->setEnabled(false);
    }

}

void Dtime::set_time()
{
  if( ontimeout )
    return;

  internalTimer.stop();

  time.setHMS( hour->value(), minute->value(), second->value() );
  kclock->setTime( time );

  emit timeChanged( TRUE );
}

void Dtime::changeDate(QDate d)
{
  date = d;
  emit timeChanged( TRUE );
}

void Dtime::load()
{
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

  kdDebug() << "Set date " << BufS << endl;

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

  // restart time
  internalTimer.start( 1000 );
}

void Dtime::timeout()
{
  // get current time
  time = QTime::currentTime();

  ontimeout = TRUE;
  second->setValue(time.second());
  minute->setValue(time.minute());
  hour->setValue(time.hour());
  ontimeout = FALSE;

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

  QPointArray pts;
  QPoint cp = rect().center();
  int d = QMIN(width(),height());
  QColor hands =  colorGroup().dark();
  QColor shadow =  colorGroup().text();
  paint.setPen( shadow );
  paint.setBrush( shadow );
  paint.setViewport(4,4,width(),height());

  for ( int c=0 ; c < 2 ; c++ )
    {
      QWMatrix matrix;
      matrix.translate( cp.x(), cp.y() );
      matrix.scale( d/1000.0F, d/1000.0F );

      // lancetta delle ore
      float h_angle = 30*(time.hour()%12-3) + time.minute()/2;
      matrix.rotate( h_angle );
      paint.setWorldMatrix( matrix );
      pts.setPoints( 4, -20,0,  0,-20, 300,0, 0,20 );
      paint.drawPolygon( pts );
      matrix.rotate( -h_angle );

      // lancetta dei minuti
      float m_angle = (time.minute()-15)*6;
      matrix.rotate( m_angle );
      paint.setWorldMatrix( matrix );
      pts.setPoints( 4, -10,0, 0,-10, 400,0, 0,10 );
      paint.drawPolygon( pts );
      matrix.rotate( -m_angle );

      // lancetta dei secondi
      float s_angle = (time.second()-15)*6;
      matrix.rotate( s_angle );
      paint.setWorldMatrix( matrix );
      pts.setPoints(4,0,0,0,0,400,0,0,0);
      paint.drawPolygon( pts );
      matrix.rotate( -s_angle );

      // quadrante
      for ( int i=0 ; i < 60 ; i++ )
        {
          paint.setWorldMatrix( matrix );
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
