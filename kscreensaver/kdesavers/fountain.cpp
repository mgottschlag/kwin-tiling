//-----------------------------------------------------------------------------
//
// kblankscrn - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//
// 1998/04/19 Layout management added by Mario Weilguni <mweilguni@kde.org>
// 2001/03/04 Converted to use libkscreensaver by Martin R. Jones

#include <stdlib.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <kcolordlg.h>
#include <kbuttonbox.h>
#include <kcolorbutton.h>
#include <kglobal.h>
#include "fountain.h"
#include "fountain.moc"
#include <GL/glu.h>
#include <GL/gl.h>
#include <qimage.h>
#include <kdebug.h>
#include <qpainter.h>
// libkscreensaver interface
extern "C"
{
	const char *kss_applicationName = "kfountain.kss";
	const char *kss_description = I18N_NOOP( "Partical Fountain Screen Saver" );
	const char *kss_version = "2.2.0";

	KScreenSaver *kss_create( WId id )
	{
		return new KFountainSaver( id );
	}

	QDialog *kss_setup()
	{
		return new KFountainSetup();
	}
}

//-----------------------------------------------------------------------------
// dialog to setup screen saver parameters
//
KFountainSetup::KFountainSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
	readSettings();

	QLabel *label;
	QPushButton *button;

	setCaption( i18n("Setup Blank Screen Saver") );

	QVBoxLayout *tl = new QVBoxLayout(this, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);

	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);

	label = new QLabel( i18n("Color:"), this );
	tl11->addWidget(label);

//	colorPush = new KColorButton( color, this );
//	colorPush->setMinimumWidth(80);
//	connect( colorPush, SIGNAL( changed(const QColor &) ),
//		SLOT( slotColor(const QColor &) ) );
//	tl11->addWidget(colorPush);
//	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new KFountainSaver( preview->winId() );
	tl1->addWidget(preview);

	KButtonBox *bbox = new KButtonBox(this);
	bbox->addStretch(1);

	button = bbox->addButton( i18n("OK"));
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton(i18n("Cancel"));
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
	bbox->layout();
	tl->addWidget(bbox);

	tl->freeze();
}

// read settings from config file
void KFountainSetup::readSettings()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "Settings" );

//	color = config->readColorEntry( "Color", &black );
}

// Ok pressed - save settings and exit
void KFountainSetup::slotOkPressed()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "Settings" );

//	config->writeEntry( "Color", color );

	config->sync();

	accept();
}

//-----------------------------------------------------------------------------


KFountainSaver::KFountainSaver( WId id ) : KScreenSaver( id )
{
	kdDebug() << "Blank" << endl;
	readSettings();

	timer = new QTimer( this );
    	timer->start( 50, TRUE );
	setBackgroundColor( black );
        erase();
	fountain = new Fountain();
	embed(fountain);
	fountain->show();
	connect( timer, SIGNAL(timeout()), this, SLOT(blank()) );;
}

KFountainSaver::~KFountainSaver()
{

}

// read configuration settings from config file
void KFountainSaver::readSettings()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "Settings" );

//	color = config->readColorEntry( "Color", &black );
}

void KFountainSaver::blank()
{
	// Play fountain

	fountain->updateGL();
	timer->start( 30, TRUE );

}
Fountain::Fountain( QWidget * parent, const char * name) : QGLWidget (parent,name)
{
	rainbow=true;
	slowdown=2.0f;
	zoom=-40.0f;
}

Fountain::~Fountain()
{

}

/** load the partical file */
bool Fountain::loadPartical()
{
    /* Status indicator */
    bool Status = FALSE;
	QImage buf;

    /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    //if ( ( TextureImage[0] = SDL_LoadBMP( "data/particle.bmp" ) ) )
    if (buf.load( "particle.bmp" ) )
        {

            /* Set the status to true */
            Status = TRUE;
	    tex = QGLWidget::convertToGLFormat( buf );  // flipped 32bit RGBA
            /* Create The Texture */
           glGenTextures( 1, &texture[0] );

            /* Typical Texture Generation Using Data From The Bitmap */
            glBindTexture( GL_TEXTURE_2D, texture[0] );

            /* Generate The Texture */
            glTexImage2D( GL_TEXTURE_2D, 0, 3, tex.width(),
                          tex.height(), 0, GL_BGR,GL_UNSIGNED_BYTE, tex.bits() );

            /* Linear Filtering */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	    //glEnable( GL_TEXTURE_2D );
        }


    return Status;
}
/** setup the GL enviroment */
void Fountain::initializeGL ()
{

	kdDebug() << "InitGL" << endl;
	GLfloat colors[12][3]=
	{{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}};

	if (loadPartical())						// Jump To Texture Loading Routine
	{
		kdDebug() << "InitGL" << endl;
		glShadeModel(GL_SMOOTH);					// Enable Smooth Shading
		qglClearColor(black);				// Black Background
		glClearDepth(1.0f);						// Depth Buffer Setup
		glDisable(GL_DEPTH_TEST);					// Disable Depth Testing
		glEnable(GL_BLEND);						// Enable Blending
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);				// Type Of Blending To Perform
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);		// Really Nice Perspective Calculations
		glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);				// Really Nice Point Smoothing
		glEnable(GL_TEXTURE_2D);					// Enable Texture Mapping
		glBindTexture(GL_TEXTURE_2D,texture[0]);			// Select Our Texture

		for (loop=0;loop<MAX_PARTICLES;loop++)				// Initials All The Textures
		{
			particle[loop].active=true;				// Make All The Particles Active
			particle[loop].life=1.0f;				// Give All The Particles Full Life
			particle[loop].fade=float(rand()%100)/1000.0f+0.003f;	// Random Fade Speed
			particle[loop].r=colors[(loop+1)/(MAX_PARTICLES/12)][0];	// Select Red Rainbow Color
			particle[loop].g=colors[(loop+1)/(MAX_PARTICLES/12)][1];	// Select Green Rainbow Color
			particle[loop].b=colors[(loop+1)/(MAX_PARTICLES/12)][2];	// Select Blue Rainbow Color
			particle[loop].xi=float((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
			particle[loop].yi=float((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
			particle[loop].zi=float((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis
			particle[loop].xg=0.0f;					// Set Horizontal Pull To Zero
			particle[loop].yg=-0.8f;				// Set Vertical Pull Downward
			particle[loop].zg=0.0f;					// Set Pull On Z Axis To Zero
		}
	}
	else
		exit(0);
}
/** resize the gl view */
void Fountain::resizeGL ( int width, int height )
{
	kdDebug() << "ResizeGL " << width << "," <<height<< endl;
	if (height==0)							// Prevent A Divide By Zero By
	{
		height=1;						// Making Height Equal One
	}

	glViewport(0,0,width,height);					// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);					// Select The Projection Matrix
	glLoadIdentity();						// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,200.0f);

	glMatrixMode(GL_MODELVIEW);					// Select The Modelview Matrix
	glLoadIdentity();
}
/** paint the GL view */
void Fountain::paintGL ()
{
	//kdDebug() << "PaintGL" << endl;

	GLfloat colors[12][3]=
	{{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer

	glLoadIdentity();						// Reset The ModelView Matrix

	for (loop=0;loop<MAX_PARTICLES;loop++)				// Loop Through All The Particles
	{
		if (particle[loop].active)				// If The Particle Is Active
		{
			float x=particle[loop].x;			// Grab Our Particle X Position
			float y=particle[loop].y;			// Grab Our Particle Y Position
			float z=particle[loop].z+zoom;			// Particle Z Pos + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(particle[loop].r,particle[loop].g,particle[loop].b,particle[loop].life);
			glBegin(GL_TRIANGLE_STRIP);				// Build Quad From A Triangle Strip

				glTexCoord2d(1,1);
				glVertex3f(x+0.5f,y+0.5f,z);	// Top Right
				glTexCoord2d(0,1);
				glVertex3f(x-0.5f,y+0.5f,z); // Top Left
				glTexCoord2d(1,0);
				glVertex3f(x+0.5f,y-0.5f,z); // Bottom Right
				glTexCoord2d(0,0);
				glVertex3f(x-0.5f,y-0.5f,z); // Bottom Left
			glEnd();						// Done Building Triangle Strip

			particle[loop].x+=particle[loop].xi/(slowdown*1000);// Move On The X Axis By X Speed
			particle[loop].y+=particle[loop].yi/(slowdown*1000);// Move On The Y Axis By Y Speed
			particle[loop].z+=particle[loop].zi/(slowdown*1000);// Move On The Z Axis By Z Speed

			particle[loop].xi+=particle[loop].xg;			// Take Pull On X Axis Into Account
			particle[loop].yi+=particle[loop].yg;			// Take Pull On Y Axis Into Account
			particle[loop].zi+=particle[loop].zg;			// Take Pull On Z Axis Into Account
			particle[loop].life-=particle[loop].fade;		// Reduce Particles Life By 'Fade'

			if (particle[loop].life<0.0f)					// If Particle Is Burned Out
			{
				particle[loop].life=1.0f;				// Give It New Life
				particle[loop].fade=float(rand()%100)/1000.0f+0.003f;	// Random Fade Value
				particle[loop].x=0.0f;					// Center On X Axis
				particle[loop].y=0.0f;					// Center On Y Axis
				particle[loop].z=0.0f;					// Center On Z Axis
				particle[loop].xi=xspeed+float((rand()%60)-32.0f);	// X Axis Speed And Direction
				particle[loop].yi=yspeed+float((rand()%60)-30.0f);	// Y Axis Speed And Direction
				particle[loop].zi=float((rand()%60)-30.0f);		// Z Axis Speed And Direction
				particle[loop].r=colors[col][0];			// Select Red From Color Table
				particle[loop].g=colors[col][1];			// Select Green From Color Table
				particle[loop].b=colors[col][2];			// Select Blue From Color Table
			}
		}
	}

	swapBuffers ();
	glFlush();
}
