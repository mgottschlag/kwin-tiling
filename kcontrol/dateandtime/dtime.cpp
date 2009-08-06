/*
 *  dtime.cpp
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
 *
 *  Plasma analog-clock drawing code:
 *
 *  Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *  Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
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
#include "dtime.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

#include <QComboBox>
#include <QtGui/QGroupBox>
#include <QPushButton>
#include <QPainter>
#include <QTimeEdit>

#include <QCheckBox>
#include <QPaintEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kcolorscheme.h>

#include <Plasma/Svg>

#include "dtime.moc"

#include "helper.h"

Dtime::Dtime(QWidget * parent)
  : QWidget(parent)
{
  // *************************************************************
  // Start Dialog
  // *************************************************************

  // Time Server

  privateLayoutWidget = new QWidget( this );
  QHBoxLayout *layout1 = new QHBoxLayout( privateLayoutWidget );
  layout1->setObjectName( "ntplayout" );
  layout1->setSpacing( 0 );
  layout1->setMargin( 0 );

  setDateTimeAuto = new QCheckBox( privateLayoutWidget );
  setDateTimeAuto->setObjectName( "setDateTimeAuto" );
  setDateTimeAuto->setText(i18n("Set date and time &automatically:"));
  connect(setDateTimeAuto, SIGNAL(toggled(bool)), this, SLOT(serverTimeCheck()));
  connect(setDateTimeAuto, SIGNAL(toggled(bool)), SLOT(configChanged()));
  layout1->addWidget( setDateTimeAuto );

  timeServerList = new QComboBox( privateLayoutWidget );
  timeServerList->setObjectName( "timeServerList" );
  timeServerList->setEditable(false);
  connect(timeServerList, SIGNAL(activated(int)), SLOT(configChanged()));
  connect(timeServerList, SIGNAL(editTextChanged(const QString &)), SLOT(configChanged()));
  connect(setDateTimeAuto, SIGNAL(toggled(bool)), timeServerList, SLOT(setEnabled(bool)));
  timeServerList->setEnabled(false);
  timeServerList->setEditable(true);
  layout1->addWidget( timeServerList );
  findNTPutility();

  // Date box
  QGroupBox* dateBox = new QGroupBox( this );
  dateBox->setObjectName( QLatin1String( "dateBox" ) );

  QVBoxLayout *l1 = new QVBoxLayout( dateBox );
  l1->setMargin( 0 );

  cal = new KDatePicker( dateBox );
  cal->setMinimumSize(cal->sizeHint());
  l1->addWidget( cal );
  cal->setWhatsThis( i18n("Here you can change the system date's day of the month, month and year.") );

  // Time frame
  QGroupBox* timeBox = new QGroupBox( this );
  timeBox->setObjectName( QLatin1String( "timeBox" ) );

  QVBoxLayout *v2 = new QVBoxLayout( timeBox );
  v2->setMargin( 0 );

  kclock = new Kclock( timeBox );
  kclock->setObjectName("Kclock");
  kclock->setMinimumSize(150,150);
  v2->addWidget( kclock );

  v2->addSpacing( KDialog::spacingHint() );

  QHBoxLayout *v3 = new QHBoxLayout( );
  v2->addLayout( v3 );

  v3->addStretch();

  timeEdit = new QTimeEdit( timeBox );
  timeEdit->setWrapping(true);
  timeEdit->setDisplayFormat("HH:mm:ss");
  v3->addWidget(timeEdit);

  v3->addStretch();

  QString wtstr = i18n("Here you can change the system time. Click into the"
    " hours, minutes or seconds field to change the relevant value, either"
    " using the up and down buttons to the right or by entering a new value.");
  timeEdit->setWhatsThis( wtstr );

  QGridLayout *top = new QGridLayout( this );
  top->setMargin( 0 );

  top->addWidget(dateBox, 1,0);
  top->addWidget(timeBox, 1,1);
  top->addWidget(privateLayoutWidget, 0, 0, 1, 2 );

  // *************************************************************
  // End Dialog
  // *************************************************************

  connect( timeEdit, SIGNAL(timeChanged(QTime)), SLOT(set_time()) );
  connect( cal, SIGNAL(dateChanged(QDate)), SLOT(changeDate(QDate)));

  connect( &internalTimer, SIGNAL(timeout()), SLOT(timeout()) );

  kclock->setEnabled(false);
}

void Dtime::serverTimeCheck() {
  bool enabled = !setDateTimeAuto->isChecked();
  cal->setEnabled(enabled);
  timeEdit->setEnabled(enabled);
  //kclock->setEnabled(enabled);
}

void Dtime::findNTPutility(){
  QByteArray envpath = qgetenv("PATH");
  if (!envpath.isEmpty() && envpath[0] == ':')
    envpath = envpath.mid(1);
  QString path = "/sbin:/usr/sbin:";
  if (!envpath.isEmpty())
    path += QString::fromLocal8Bit(envpath);
  else
    path += QLatin1String("/bin:/usr/bin");
  if(!KStandardDirs::findExe("ntpdate", path).isEmpty()) {
    ntpUtility = "ntpdate";
    kDebug() << "ntpUtility = " << ntpUtility;
    return;
  }
  if(!KStandardDirs::findExe("rdate").isEmpty()) {
    ntpUtility = "rdate";
    kDebug() << "ntpUtility = " << ntpUtility;
    return;
  }
  privateLayoutWidget->hide();
  kDebug() << "ntpUtility not found!";
}

void Dtime::set_time()
{
  if( ontimeout )
    return;

  internalTimer.stop();

  time = timeEdit->time();
  kclock->setTime( time );

  emit timeChanged( true );
}

void Dtime::changeDate(const QDate &d)
{
  date = d;
  emit timeChanged( true );
}

void Dtime::configChanged(){
  emit timeChanged( true );
}

void Dtime::load()
{
  // The config is actually written to the system config, but the user does not have any local config,
  // since there is nothing writing it.
  KConfig _config( "kcmclockrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "NTP");
  timeServerList->clear();
  timeServerList->addItems(config.readEntry("servers",
    i18n("Public Time Server (pool.ntp.org),\
asia.pool.ntp.org,\
europe.pool.ntp.org,\
north-america.pool.ntp.org,\
oceania.pool.ntp.org")).split(',', QString::SkipEmptyParts));
  setDateTimeAuto->setChecked(config.readEntry("enabled", false));

  // Reset to the current date and time
  time = QTime::currentTime();
  date = QDate::currentDate();
  cal->setDate(date);

  // start internal timer
  internalTimer.start( 1000 );

  timeout();
}

void Dtime::save( QStringList& helperargs )
{
  // Save the order, but don't duplicate!
  QStringList list;
  if( timeServerList->count() != 0)
    list.append(timeServerList->currentText());
  for ( int i=0; i<timeServerList->count();i++ ) {
    QString text = timeServerList->itemText(i);
    if( !list.contains(text) )
      list.append(text);
    // Limit so errors can go away and not stored forever
    if( list.count() == 10)
      break;
  }
  helperargs << "ntp" << QString::number( list.count()) << list
      << ( setDateTimeAuto->isChecked() ? "enabled" : "disabled" );

  if(setDateTimeAuto->isChecked() && !ntpUtility.isEmpty()){
    // NTP Time setting - done in helper
    timeServer = timeServerList->currentText();
    kDebug() << "Setting date from time server " << timeServer;
  }
  else {
    // User time setting
    QDateTime dt(date, QTime(timeEdit->time()));

    kDebug() << "Set date " << dt;

    helperargs << "date" << QString::number(dt.toTime_t())
                         << QString::number(::time(0));
  }

  // restart time
  internalTimer.start( 1000 );
}

void Dtime::processHelperErrors( int code )
{
  if( code & ERROR_DTIME_NTP ) {
    KMessageBox::error( this, i18n("Unable to contact time server: %1.", timeServer) );
    setDateTimeAuto->setChecked( false );
  }
  if( code & ERROR_DTIME_DATE ) {
    KMessageBox::error( this, i18n("Can not set date."));
  }
}

void Dtime::timeout()
{
  // get current time
  time = QTime::currentTime();

  ontimeout = true;
  timeEdit->setTime(time);
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

Kclock::Kclock(QWidget *parent)
    : QWidget(parent)
{
    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/clock");
    m_theme->setContainsMultipleImages(true);
}

Kclock::~Kclock()
{
    delete m_theme;
}

void Kclock::showEvent( QShowEvent *event )
{
    setClockSize( size() );
    QWidget::showEvent( event );
}

void Kclock::resizeEvent( QResizeEvent * )
{
    setClockSize( size() );
}

void Kclock::setClockSize(const QSize &size)
{
    int dim = qMin(size.width(), size.height());
    QSize newSize = QSize(dim, dim);

    if (newSize != m_faceCache.size()) {
        m_faceCache = QPixmap(newSize);
        m_handsCache = QPixmap(newSize);
        m_glassCache = QPixmap(newSize);

        m_theme->resize(newSize);
        m_repaintCache = RepaintAll;
    }
}

void Kclock::setTime(const QTime &time)
{
    if (time.minute() != this->time.minute() || time.hour() != this->time.hour()) {
        if (m_repaintCache == RepaintNone) {
            m_repaintCache = RepaintHands;
        }
    }
    this->time = time;
    update();
}

void Kclock::drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName)
{
    // this code assumes the following conventions in the svg file:
    // - the _vertical_ position of the hands should be set with respect to the center of the face
    // - the _horizontal_ position of the hands does not matter
    // - the _shadow_ elements should have the same vertical position as their _hand_ element counterpart

    QRectF elementRect;
    QString name = handName + "HandShadow";
    if (m_theme->hasElement(name)) {
        p->save();

        elementRect = m_theme->elementRect(name);
        if( rect.height() < 64 )
            elementRect.setWidth( elementRect.width() * 2.5 );
        static const QPoint offset = QPoint(2, 3);

        p->translate(rect.x() + (rect.width() / 2) + offset.x(), rect.y() + (rect.height() / 2) + offset.y());
        p->rotate(rotation);
        p->translate(-elementRect.width()/2, elementRect.y()-verticalTranslation);
        m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

        p->restore();
    }

    p->save();

    name = handName + "Hand";
    elementRect = m_theme->elementRect(name);
    if (rect.height() < 64) {
        elementRect.setWidth(elementRect.width() * 2.5);
    }

    p->translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
    p->rotate(rotation);
    p->translate(-elementRect.width()/2, elementRect.y()-verticalTranslation);
    m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

    p->restore();
}

void Kclock::paintInterface(QPainter *p, const QRect &rect)
{
    const bool m_showSecondHand = true;

    // compute hand angles
    const qreal minutes = 6.0 * time.minute() - 180;
    const qreal hours = 30.0 * time.hour() - 180 +
            ((time.minute() / 59.0) * 30.0);
    qreal seconds = 0;
    if (m_showSecondHand) {
        static const double anglePerSec = 6;
        seconds = anglePerSec * time.second() - 180;
    }

    // paint face and glass cache
    QRect faceRect = m_faceCache.rect();
    if (m_repaintCache == RepaintAll) {
        m_faceCache.fill(Qt::transparent);
        m_glassCache.fill(Qt::transparent);

        QPainter facePainter(&m_faceCache);
        QPainter glassPainter(&m_glassCache);
        facePainter.setRenderHint(QPainter::SmoothPixmapTransform);
        glassPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        m_theme->paint(&facePainter, m_faceCache.rect(), "ClockFace");

        glassPainter.save();
        QRectF elementRect = QRectF(QPointF(0, 0), m_theme->elementSize("HandCenterScrew"));
        glassPainter.translate(faceRect.width() / 2 - elementRect.width() / 2, faceRect.height() / 2 - elementRect.height() / 2);
        m_theme->paint(&glassPainter, elementRect, "HandCenterScrew");
        glassPainter.restore();

        m_theme->paint(&glassPainter, faceRect, "Glass");

        // get vertical translation, see drawHand() for more details
        m_verticalTranslation = m_theme->elementRect("ClockFace").center().y();
    }

    // paint hour and minute hands cache
    if (m_repaintCache == RepaintHands || m_repaintCache == RepaintAll) {
        m_handsCache.fill(Qt::transparent);

        QPainter handsPainter(&m_handsCache);
        handsPainter.drawPixmap(faceRect, m_faceCache, faceRect);
        handsPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        drawHand(&handsPainter, faceRect, m_verticalTranslation, hours, "Hour");
        drawHand(&handsPainter, faceRect, m_verticalTranslation, minutes, "Minute");
    }

    // reset repaint cache flag
    m_repaintCache = RepaintNone;

    // paint caches and second hand
    QRect targetRect = faceRect;
    if (targetRect.width() < rect.width()) {
        targetRect.moveLeft((rect.width() - targetRect.width()) / 2);
    }

    p->drawPixmap(targetRect, m_handsCache, faceRect);
    if (m_showSecondHand) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        drawHand(p, targetRect, m_verticalTranslation, seconds, "Second");
    }
    p->drawPixmap(targetRect, m_glassCache, faceRect);
}

void Kclock::paintEvent( QPaintEvent * )
{
  QPainter paint(this);

  paint.setRenderHint(QPainter::Antialiasing);
  paintInterface(&paint, rect());
}

