//-----------------------------------------------------------------------------
//
// kpolygon - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#include <stdlib.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <kapp.h>
#include <time.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include "polygon.h"

#include "polygon.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"


#undef Below

#define MAXLENGTH	65
#define MAXVERTICES	19

template class QList<Polygon>;
template class QArray<XPoint>;

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
extern KLocale *glocale;

static kPolygonSaver *saver = NULL;

//-----------------------------------------------------------------------------
// standard screen saver interface functions
//
void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kPolygonSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kPolygonSetup dlg;

	return dlg.exec();
}

//-----------------------------------------------------------------------------
// dialog to setup screen saver parameters
//
kPolygonSetup::kPolygonSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	saver = NULL;
	length = 10;
	vertices = 3;
	speed = 50;

	readSettings();

	QString str;
	QLabel *label;
	QPushButton *button;
	QSlider *sb;

	setCaption( glocale->translate("Setup kpolygon") );

	QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);
	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);	

	label = new QLabel( glocale->translate("Length:"), this );
	min_size(label);
	tl11->addWidget(label);

	sb = new QSlider(1, MAXLENGTH, 10, length, QSlider::Horizontal, this );
	sb->setMinimumSize( 90, 20 );
    sb->setTickmarks(QSlider::Below);
    sb->setTickInterval(10);
	connect( sb, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotLength( int ) ) );
	tl11->addWidget(sb);
	tl11->addSpacing(5);

	label = new QLabel( glocale->translate("Vertices:"), this );
	min_size(label);
	tl11->addWidget(label);


	sb = new QSlider(3, MAXVERTICES, 2, vertices, QSlider::Horizontal, this);
	sb->setMinimumSize( 90, 20 );
    sb->setTickmarks(QSlider::Below);
    sb->setTickInterval(2);
	connect( sb, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotVertices( int ) ) );
	tl11->addWidget(sb);
	tl11->addSpacing(5);

	label = new QLabel( glocale->translate("Speed:"), this );
	min_size(label);
	tl11->addWidget(label);

	sb = new QSlider(0, 100, 10, speed, QSlider::Horizontal, this);
	sb->setMinimumSize( 90, 20 );
    sb->setTickmarks(QSlider::Below);
    sb->setTickInterval(10);
	connect( sb, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotSpeed( int ) ) );
	tl11->addWidget(sb);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new kPolygonSaver( preview->winId() );
	tl1->addWidget(preview);

	KButtonBox *bbox = new KButtonBox(this);	
	button = bbox->addButton( glocale->translate("About"));
	connect( button, SIGNAL( clicked() ), SLOT(slotAbout() ) );
	bbox->addStretch(1);

	button = bbox->addButton( glocale->translate("OK"));	
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton(glocale->translate("Cancel"));
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
	bbox->layout();
	tl->addWidget(bbox);

	tl->freeze();
}

kPolygonSetup::~kPolygonSetup()
{
    delete saver;
}

// read settings from config file
void kPolygonSetup::readSettings()
{
    KConfig *config = KApplication::kApplication()->config();
    config->setGroup( "Settings" );
    
    QString str;
    
    str = config->readEntry( "Length" );
    if ( !str.isNull() )
        length = atoi( str );
    if ( length > MAXLENGTH )
        length = MAXLENGTH;
    else if ( length < 1 )
        length = 1;
    
    str = config->readEntry( "Vertices" );
    if ( !str.isNull() )
        vertices = atoi( str );
    if ( vertices > MAXVERTICES )
        vertices = MAXVERTICES;
    else if ( vertices < 3 )
        vertices = 3;
    
    str = config->readEntry( "Speed" );
    if ( !str.isNull() )
        speed = atoi( str );
    if ( speed > 100 )
        speed = 100;
    else if ( speed < 50 )
        speed = 50;
}

void kPolygonSetup::slotLength( int len )
{
	length = len;
	if ( saver )
		saver->setPolygon( length, vertices );
}

void kPolygonSetup::slotVertices( int num )
{
	vertices = num;
	if ( saver )
		saver->setPolygon( length, vertices );
}

void kPolygonSetup::slotSpeed( int num )
{
	speed = num;
	if ( saver )
		saver->setSpeed( speed );
}

// Ok pressed - save settings and exit
void kPolygonSetup::slotOkPressed()
{
    KConfig *config = KApplication::kApplication()->config();
    config->setGroup( "Settings" );
    
    QString slength;
    slength.setNum( length );
    config->writeEntry( "Length", slength );
    
    QString svertices;
    svertices.setNum( vertices );
    config->writeEntry( "Vertices", svertices );
    
    QString sspeed;
    sspeed.setNum( speed );
    config->writeEntry( "Speed", sspeed );
    
    config->sync();
    
    accept();
}

void kPolygonSetup::slotAbout()
{
	KMessageBox::information(this,
			     glocale->translate("Polygon Version 0.1\n\n"\
					       "written by Martin R. Jones 1996\n"\
					       "mjones@kde.org"));
}

//-----------------------------------------------------------------------------


kPolygonSaver::kPolygonSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	polygons.setAutoDelete( TRUE );

	readSettings();

	srandom((int)time((time_t *)0L));

	directions.resize( numVertices );
	colorContext = QColor::enterAllocContext();

	blank();

	initialiseColor();
	initialisePolygons();

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kPolygonSaver::~kPolygonSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

// set polygon properties
void kPolygonSaver::setPolygon( int len, int ver )
{
	timer.stop();
	numLines = len;
	numVertices = ver;

	directions.resize( numVertices );
	polygons.clear();
	initialisePolygons();
	blank();

	timer.start( speed );
}

// set the speed
void kPolygonSaver::setSpeed( int spd )
{
	timer.stop();
	speed = 100-spd;
	timer.start( speed );
}

// read configuration settings from config file
void kPolygonSaver::readSettings()
{
    QString str;
    
    KConfig *config = KApplication::kApplication()->config();
	config->setGroup( "Settings" );

	str = config->readEntry( "Length" );
	if ( !str.isNull() )
		numLines = atoi( str );
	else
		numLines = 10;

	str = config->readEntry( "Vertices" );
	if ( !str.isNull() )
		numVertices = atoi( str );
	else
		numVertices = 3;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = 100 - atoi( str );
	else
		speed = 50;

	if ( numLines > 50 )
		numLines = 50;
	else if ( numLines < 1 )
		numLines = 1;
	
	if ( numVertices > 20 )
		numVertices = 20;
	else if ( numVertices < 3 )
		numVertices = 3;
}

// draw next polygon and erase tail
void kPolygonSaver::slotTimeout()
{
	if ( polygons.count() > numLines )
	{
		XSetForeground( qt_xdisplay(), mGc, black.pixel() );
		XDrawLines( qt_xdisplay(), mDrawable, mGc, polygons.first()->vertices.data(),
			numVertices+1, CoordModeOrigin );
	}

	nextColor();

	XDrawLines( qt_xdisplay(), mDrawable, mGc, polygons.last()->vertices.data(),
		numVertices+1, CoordModeOrigin );

	if ( polygons.count() > numLines )
		polygons.removeFirst();

	polygons.append( new Polygon( *(polygons.last()) ) );
	moveVertices();
}

void kPolygonSaver::blank()
{
	XSetWindowBackground( qt_xdisplay(), mDrawable, black.pixel() );
	XClearWindow( qt_xdisplay(), mDrawable );
}

// initialise the polygon
void kPolygonSaver::initialisePolygons()
{
	int i;

	polygons.append( new Polygon( numVertices ) );

	Polygon *poly = polygons.last();

	for ( i = 0; i < numVertices; i++ )
	{
		poly->vertices[i].x = random() % mWidth;
		poly->vertices[i].y = random() % mHeight;
		directions[i].x = 16 - (random() % 8) * 4;
		if ( directions[i].x == 0 )
			directions[i].x = 1;
		directions[i].y = 16 - (random() % 8) * 4;
		if ( directions[i].y == 0 )
			directions[i].y = 1;
	}

	poly->vertices[i].x = poly->vertices[0].x;
	poly->vertices[i].y = poly->vertices[0].y;
}

// move polygon in current direction and change direction if a border is hit
void kPolygonSaver::moveVertices()
{
	int i;
	Polygon *poly = polygons.last();

	for ( i = 0; i < numVertices; i++ )
	{
		poly->vertices[i].x += directions[i].x;
		if ( poly->vertices[i].x >= (int)mWidth )
		{
			directions[i].x = -(random() % 4 + 1) * 4;
			poly->vertices[i].x = (int)mWidth;
		}
		else if ( poly->vertices[i].x < 0 )
		{
			directions[i].x = (random() % 4 + 1) * 4;
			poly->vertices[i].x = 0;
		}

		poly->vertices[i].y += directions[i].y;
		if ( poly->vertices[i].y >= (int)mHeight )
		{
			directions[i].y = -(random() % 4 + 1) * 4;
			poly->vertices[i].y = mHeight;
		}
		else if ( poly->vertices[i].y < 0 )
		{
			directions[i].y = (random() % 4 + 1) * 4;
			poly->vertices[i].y = 0;
		}
	}

	poly->vertices[i].x = poly->vertices[0].x;
	poly->vertices[i].y = poly->vertices[0].y;
}

// create a color table of 64 colors
void kPolygonSaver::initialiseColor()
{
	QColor color;

	for ( int i = 0; i < 64; i++ )
	{
		color.setHsv( i * 360 / 64, 255, 255 );
		colors[i] = color.alloc();
	}
}

// set foreground color to next in the table
void kPolygonSaver::nextColor()
{
	static int col = 0;

	XSetForeground( qt_xdisplay(), mGc, colors[col] );

	col++;

	if ( col > 63 )
		col = 0;
}

