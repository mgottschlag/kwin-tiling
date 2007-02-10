/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003 <ravi@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

#include <QApplication>
#include <QCheckBox>
#include <qdesktopwidget.h>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QProgressBar>
#include <QVBoxLayout>

#include "objkstheme.h"
#include "themelegacy.h"
#include "themelegacy.moc"

DefaultConfig::DefaultConfig( QWidget *parent, KConfig *config )
    :ThemeEngineConfig( parent, config )
{
  mConfig->setGroup( QString("KSplash Theme: Default") );
  KVBox *hbox = new KVBox( this );
  mFlash = new QCheckBox( i18n("Icons flash while they are starting"), hbox );
  mFlash->setChecked( mConfig->readEntry("Icons Flashing", true) );
  mAlwaysShow = new QCheckBox( i18n("Always show progress bar"), hbox );
  mAlwaysShow->setChecked( mConfig->readEntry("Always Show Progress", true) );
}

void DefaultConfig::save()
{
  kDebug() << "DefaultConfig::save()" << endl;
  mConfig->setGroup( QString("KSplash Theme: Default") );
  mConfig->writeEntry( "Icons Flashing", mFlash->isChecked() );
  mConfig->writeEntry( "Always Show Progress", mAlwaysShow->isChecked() );
  mConfig->sync();
}

#define BIDI 0

ThemeDefault::ThemeDefault( QWidget *parent, const QStringList &args )
    :ThemeEngine( parent, args )
{
  mActivePixmap = mInactivePixmap = 0L;
  mState = 0;

  _readSettings();
  _initUi();

  if( mIconsFlashing )
  {
    mFlashTimer = new QTimer( this );
    connect( mFlashTimer, SIGNAL(timeout()), this, SLOT(flash()) );
    mFlashPixmap1 = new QPixmap();
    mFlashPixmap2 = new QPixmap();

  }
  else
  {
    mFlashTimer = 0L;
    mFlashPixmap1 = 0L;
    mFlashPixmap2 = 0L;
  }
}

ThemeDefault::~ThemeDefault()
{
    delete mFlashPixmap1;
    delete mFlashPixmap2;
}

void ThemeDefault::_initUi()
{
    QString resource_prefix;

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setSpacing( 0 );
    layout->setMargin( 0 );
    setLayout( layout );

    //setAttribute(Qt::WA_NoSystemBackground, true);;

    QString activePix, inactivePix;
#if BIDI
    if ( QApplication::isRightToLeft() )
    {
        activePix = _findPicture(QString("splash_active_bar_bidi.png"));
        inactivePix = _findPicture(QString("splash_inactive_bar_bidi.png"));
    }
    else
#endif
    {
        activePix = _findPicture(QString("splash_active_bar.png"));
        inactivePix = _findPicture(QString("splash_inactive_bar.png"));
    }
    kDebug() << "Inactive pixmap: " << inactivePix << endl;
    kDebug() << "Active pixmap:   " <<   activePix << endl;

    mActivePixmap = new QPixmap( activePix );
    mInactivePixmap = new QPixmap( inactivePix );

    if (mActivePixmap->isNull())
    {
        *mActivePixmap = QPixmap(200,100);
        mActivePixmap->fill(Qt::blue);
    }
    if (mInactivePixmap->isNull())
    {
        *mInactivePixmap = QPixmap(200,100);
        mInactivePixmap->fill(Qt::black);
    }

    QPixmap tlimage( _findPicture(QString("splash_top.png")) );
    if (tlimage.isNull())
    {
        tlimage = QPixmap(200,100);
        tlimage.fill(Qt::blue);
    }
    QLabel *top_label = new QLabel( this );
    layout->addWidget( top_label );
    top_label->setPixmap( tlimage );
    top_label->setFixedSize( tlimage.width(), tlimage.height() );
    //top_label->setAttribute(Qt::WA_NoSystemBackground, true);

    mBarLabel = new QLabel( this );
    layout->addWidget( mBarLabel );
    mBarLabel->setPixmap(*mInactivePixmap);
    //mBarLabel->setAttribute(Qt::WA_NoSystemBackground, true);

    QPixmap blimage( _findPicture(QString("splash_bottom.png")) );
    if (blimage.isNull())
    {
        blimage = QPixmap(200,100);
        blimage.fill(Qt::black);
    }

    QFrame *bottom_label = new QFrame( this );

    layout->addWidget( bottom_label );

    QPalette palette = this->palette();
    palette.setBrush( bottom_label->backgroundRole(), QBrush(blimage) );
    bottom_label->setPalette( palette );
    bottom_label->setLayout( new QHBoxLayout( bottom_label ) );
    bottom_label->layout()->setMargin( 0 );
    bottom_label->layout()->setSpacing( 0 );
    bottom_label->setAttribute(Qt::WA_NoSystemBackground, true);

    mLabel = new QLabel( bottom_label );
    bottom_label->layout()->addWidget( mLabel );
    mLabel->setAutoFillBackground ( true );
    //mLabel->setBackgroundOrigin( QWidget::ParentOrigin );
    //mLabel->setPaletteForegroundColor( mLabelForeground );
    mLabel->setPalette( palette );
    mLabel->setText( "Lubos too" );
    QFont f(mLabel->font());
    f.setBold(true);
    mLabel->setFont(f);

    mProgressBar = new QProgressBar( mLabel );
    int h, s, v;
    mLabelForeground.getHsv( &h, &s, &v );
    mProgressBar->setPalette( QPalette( v > 128 ? Qt::black : Qt::white ));
    //mProgressBar->setBackgroundOrigin( QWidget::ParentOrigin );
    mProgressBar->setPalette( palette );

    bottom_label->setFixedWidth( qMax(blimage.width(),tlimage.width()) );

    if (QApplication::isRightToLeft()){
        mProgressBar->move( 2, 0 );
//	    mLabel->move( mProgressBar->width() + 4, 0);
    }
    else{
        mProgressBar->move( bottom_label->width() - mProgressBar->width() - 4, 0);
        mLabel->move( 2, 0 );
    }

    mProgressBar->hide();

    setFixedWidth( mInactivePixmap->width() );

    const QRect rect = qApp->desktop()->screenGeometry( mTheme->xineramaScreen() );
    // KGlobalSettings::splashScreenDesktopGeometry(); cannot be used here.
    // kDebug() << "ThemeDefault::_initUi" << rect << endl;

    move( rect.x() + (rect.width() - size().width())/2,
          rect.y() + (rect.height() - size().height())/2 );
}

// Attempt to find overrides elsewhere?
void ThemeDefault::_readSettings()
{
  if( !mTheme )
    return;

  KConfig *cfg = mTheme->themeConfig();
  if( !cfg )
    return;

  cfg->setGroup( QString("KSplash Theme: %1").arg(mTheme->theme()) );

  mIconsFlashing = cfg->readEntry( "Icons Flashing", true );
  QColor df(Qt::white);
  mLabelForeground = cfg->readEntry( "Label Foreground", df );
}

/*
 * ThemeDefault::slotUpdateState(): IF in Default mode, THEN adjust the bar
 * pixmap label. Whee, phun!
 *
 * A similar method exists in the old KSplash.
 */
void ThemeDefault::slotUpdateState()
{
  if( mState > 8 )
    mState = 8;

  if( mIconsFlashing )
  {

    *mFlashPixmap1 = updateBarPixmap( mState );
    *mFlashPixmap2 = updateBarPixmap( mState+1 );
    mBarLabel->setPixmap(*mFlashPixmap2);
    mFlashTimer->stop();

    if( mState < 8 )
      mFlashTimer->start(400);
  }
  else
    mBarLabel->setPixmap( updateBarPixmap( mState ) );

  mState++;
}

/*
 * ThemeDefault::updateBarPixmap(): IF in Default mode, THEN adjust the
 * bar pixmap to reflect the current state. WARNING! KSplash Default
 * does NOT support our "Restoring Session..." state. We will need
 * to reflect that somehow.
 */
QPixmap ThemeDefault::updateBarPixmap( int state )
{
  int offs;

  QPixmap x;
  if( !mActivePixmap ) return( x );
#if BIDI


  if( QApplication::isRightToLeft() )
    {
      if ( state > 7 )
	return (  x );
    }
#endif

  offs = state * 58;
  if (state == 3)
    offs += 8;
  else if (state == 6)
    offs -= 8;

  QPixmap tmp(*mActivePixmap);
  QPainter p(&tmp);
#if BIDI
  if ( QApplication::isRightToLeft() )
    p.drawPixmap(0, 0, *mInactivePixmap, 0, 0, tmp.width()-offs );
  else
#endif
    p.drawPixmap(offs, 0, *mInactivePixmap, offs, 0, -1, -1);
  return tmp ;
}

void ThemeDefault::flash()
{
  if( !mIconsFlashing )
    return;
  QPixmap *swap = mFlashPixmap1;
  mFlashPixmap1 = mFlashPixmap2;
  mFlashPixmap2 = swap;
  mBarLabel->setPixmap(*mFlashPixmap2);
}

QString ThemeDefault::_findPicture( const QString &pic )
{
  // Don't use ObjKsTheme::locateThemeData here for compatibility reasons.
  QString f = pic;
  if (mTheme->loColor())
    f = QString("locolor/")+f;
  //kDebug() << "Searching for " << f << endl;
  //kDebug() << "Theme directory: " << mTheme->themeDir() << endl;
  //kDebug() << "Theme name:      " << mTheme->theme() << endl;
  QString p = QString();
  if ((p = KStandardDirs::locate("appdata",mTheme->themeDir()+f)).isEmpty())
    if ((p = KStandardDirs::locate("appdata",mTheme->themeDir()+"pics/"+f)).isEmpty())
      if ((p = KStandardDirs::locate("appdata", QString("pics/")+mTheme->theme()+'/'+f)).isEmpty())
        if ((p = KStandardDirs::locate("appdata",f)).isEmpty())
          if ((p = KStandardDirs::locate("appdata",QString("pics/")+f)).isEmpty())
            p = KStandardDirs::locate("data",QString("pics/")+f);
  return p;
}
