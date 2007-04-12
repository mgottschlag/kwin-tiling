#include <math.h>
#include <stdlib.h>

#include <config.h>
#ifdef HAVE_SYS_LOADAVG_H
#include <sys/loadavg.h> // e.g. Solaris
#endif

#include <QPainter>
#include <QBitmap>
#include <QDateTime>
#include <QtGui/qdrawutil.h>
#include <QTimer>
//Added by qt3to4:
#include <QMouseEvent>

#include <kdebug.h>
#include <kglobal.h>
#include <kwm.h>
#include <kiconloader.h>
#include <QPixmap>
#include <kpixmapeffect.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmenu.h>

#include <taskmanager.h>

#include "kaspopup.h"
#include "kastasker.h"

#include "kasloaditem.h"
#include "kasloaditem.moc"

KasLoadItem::KasLoadItem( KasBar *parent )
    : KasItem( parent )
{
    QTimer *t = new QTimer( this );
    connect( t, SIGNAL( timeout() ), SLOT( updateDisplay() ) );
    t->start( 1000 );
    updateDisplay();

    connect( this, SIGNAL(rightButtonClicked(QMouseEvent *)), SLOT(showMenuAt(QMouseEvent *) ) );
}

KasLoadItem::~KasLoadItem()
{
}

void KasLoadItem::updateDisplay()
{
  double load[3];
  
  int ret = getloadavg( load, 3 );
  if ( ret == -1 )
    return;

  valuesOne.append( load[0] );
  valuesFive.append( load[1] );
  valuesFifteen.append( load[2] );

  if ( valuesOne.count() > 2/*(extent()-2)*/ ) {
    valuesOne.pop_front();
    valuesFive.pop_front();
    valuesFifteen.pop_front();
  }

  setText( QString("%1").arg( valuesOne.last(), 3, 'f', 2 ) );
}

void KasLoadItem::paint( QPainter *p )
{
    double val = valuesOne.last();
    double maxValue = 1.0;
    double scaleVal = qMax( val, valuesFive.last() );

    if ( scaleVal >= maxValue )
	maxValue = 2.0;
    if ( scaleVal >= maxValue )
	maxValue = 5.0;
    if ( scaleVal >= maxValue )
	maxValue = 10.0;
    if ( scaleVal >= maxValue )
	maxValue = 20.0;
    if ( scaleVal >= maxValue )
	maxValue = 50.0;
    if ( scaleVal >= maxValue )
	maxValue = 100.0;

    double dh = extent()-16;
    dh = dh / maxValue;

    int h = (int) floor( dh * val );
    int w = extent()-4;
    h = (h > 0) ? h : 1;
    w = (w > 0) ? w : 1;

    KasItem::paint( p );

    QColor light = kasbar()->colorGroup().highlight();
    QColor dark = light.dark();

    QPixmap pix;
    pix.resize( w, h );
    KPixmapEffect::gradient( pix, light, dark, KPixmapEffect::DiagonalGradient );
    p->drawPixmap( 2, extent()-2-h, pix );

    p->setPen( kasbar()->colorGroup().mid() );
    for ( double pos = 0.2 ; pos < 1.0 ; pos += 0.2 ) {
	int ypos = (int) floor((extent()-2) - (dh*maxValue*pos));
	p->drawLine( 2, ypos, extent()-3, ypos );
    }
}

void KasLoadItem::showMenuAt( QMouseEvent *ev )
{
    hidePopup();
    showMenuAt( ev->globalPos() );
}

void KasLoadItem::showMenuAt( QPoint p )
{
    mouseLeave();
    kasbar()->updateMouseOver();

    KasTasker *bar = dynamic_cast<KasTasker *> (KasItem::kasbar());
    if ( !bar )
	return;

    KMenu *menu = bar->contextMenu();
    menu->exec( p );
}
