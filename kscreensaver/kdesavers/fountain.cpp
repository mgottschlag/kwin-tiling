//-----------------------------------------------------------------------------
//
// kfountain - Partical Fountain Screen Saver for KDE 2
//
// Copyright (c)  Ian Reinhart Geiser 2001
//
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
#include <kglobal.h>
#include <kstddirs.h>
#include <math.h>
// libkscreensaver interface
extern "C"
{
	const char *kss_applicationName = "kfountain.kss";
	const char *kss_description = I18N_NOOP( "Particle Fountain Screen Saver" );
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
	: SetupUi( parent, name, TRUE )
{
	readSettings();

	//QLabel *label;
	//QPushButton *button;

	//setCaption( i18n("Setup The Particle Fountain") );

	//QVBoxLayout *tl = new QVBoxLayout(this, 10);
	//QHBoxLayout *tl1 = new QHBoxLayout;
	//tl->addLayout(tl1);

	//QVBoxLayout *tl11 = new QVBoxLayout(5);
	//tl1->addLayout(tl11);

	//label = new QLabel( i18n("No options here yet...:"), this );
	///tl11->addWidget(label);;

	//preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new KFountainSaver( preview->winId() );
	//tl1->addWidget(preview);

	//KButtonBox *bbox = new KButtonBox(this);
	//bbox->addStretch(1);

	//button = bbox->addButton( i18n("OK"));
	connect( PushButton1, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	//button = bbox->addButton(i18n("Cancel"));
	connect( PushButton2, SIGNAL( clicked() ), SLOT( reject() ) );
	//bbox->layout();
	//tl->addWidget(bbox);

	//tl->freeze();
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
    	timer->start( 25, TRUE );
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
	timer->start( 25, TRUE );

}
Fountain::Fountain( QWidget * parent, const char * name) : QGLWidget (parent,name)
{
	rainbow=true;
	slowdown=2.0f;
	zoom=-40.0f;
	index=0;
	obj = gluNewQuadric();

}

Fountain::~Fountain()
{
	glDeleteTextures( 1, &texture[0] );
	gluDeleteQuadric(obj);
}

/** load the particle file */
bool Fountain::loadParticle()
{
    /* Status indicator */
    bool Status = TRUE;
	QImage buf;

    kdDebug() << "Loading: " << locate("data", "kscreensaver/particle.png") << endl;
 if (buf.load( locate("data", "kscreensaver/particle.png") ) )

        {
		tex = convertToGLFormat(buf);  // flipped 32bit RGBA
		kdDebug() << "Texture loaded: " << tex.numBytes () << endl;
	}
	else
	{
		QImage dummy( 32, 32, 32 );
  		dummy.fill( Qt::white.rgb() );
        	buf = dummy;
		tex = convertToGLFormat( buf );
	}

            /* Set the status to true */
            //Status = TRUE;
	glGenTextures(1, &texture[0]);   /* create three textures */
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	/* use linear filtering */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/* actually generate the texture */
	glTexImage2D(GL_TEXTURE_2D, 0, 4, tex.width(), tex.height(), 0,
	GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());



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

	if (loadParticle())						// Jump To Texture Loading Routine
	{
    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* Enables Depth Testing */
    glDisable( GL_DEPTH_TEST );

    /* Enable Blending */
    glEnable( GL_BLEND );
    /* Type Of Blending To Perform */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );


    /* Really Nice Perspective Calculations */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    /* Really Nice Point Smoothing */
    glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );

    /* Enable Texture Mapping */
    glEnable( GL_TEXTURE_2D );
    /* Select Our Texture */
    glBindTexture( GL_TEXTURE_2D, texture[0] );

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
			particle[loop].size=1.0f;				// Set particle size.
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
	col = ( ++col ) % 12;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer

	glLoadIdentity();
						// Reset The ModelView Matrix
	transIndex++;
	glTranslatef( GLfloat(5.0*sin(4*3.14*transIndex/360)), GLfloat(4.0*cos(2*3.14*transIndex/360)), 0.0 );
	xspeed = GLfloat(100.0*cos(3*3.14*transIndex/360)+100);
	yspeed = GLfloat(100.0*sin(3*3.14*transIndex/360)+100);
	//slowdown = GLfloat(4.0*sin(2*3.14*transIndex/360)+4.01);

	for (loop=0;loop<MAX_PARTICLES;loop++)				// Loop Through All The Particles
	{
		if (particle[loop].active)				// If The Particle Is Active
		{
			float x=particle[loop].x;			// Grab Our Particle X Position
			float y=particle[loop].y;			// Grab Our Particle Y Position
			float z=particle[loop].z+zoom;			// Particle Z Pos + Zoom
    /* Select Our Texture */

                    /* Draw The Particle Using Our RGB Values,
                     * Fade The Particle Based On It's Life
                     */

                    glColor4f( particle[loop].r,
                               particle[loop].g,
                               particle[loop].b,
                               particle[loop].life );

                    /* Build Quad From A Triangle Strip */
		  if( !stars )
		    glBegin( GL_TRIANGLE_STRIP );
		  else
		    glBegin( GL_TRIANGLE_FAN );
                      /* Top Right */
                      glTexCoord2d( 1, 1 );
                      glVertex3f( x + particle[loop].size, y + particle[loop].size, z );
                      /* Top Left */
                      glTexCoord2d( 0, 1 );
                      glVertex3f( x - particle[loop].size, y + particle[loop].size, z );
                      /* Bottom Right */
                      glTexCoord2d( 1, 0 );
                      glVertex3f( x + particle[loop].size, y - particle[loop].size, z );
                      /* Bottom Left */
                      glTexCoord2d( 0, 0 );
                      glVertex3f( x - particle[loop].size, y - particle[loop].size, z );
                    glEnd( );

			particle[loop].x+=particle[loop].xi/(slowdown*1000);// Move On The X Axis By X Speed
			particle[loop].y+=particle[loop].yi/(slowdown*1000);// Move On The Y Axis By Y Speed
			particle[loop].z+=particle[loop].zi/(slowdown*1000);// Move On The Z Axis By Z Speed

			particle[loop].xi+=particle[loop].xg;			// Take Pull On X Axis Into Account
			particle[loop].yi+=particle[loop].yg;			// Take Pull On Y Axis Into Account
			particle[loop].zi+=particle[loop].zg;			// Take Pull On Z Axis Into Account
			particle[loop].life-=particle[loop].fade;		// Reduce Particles Life By 'Fade'

			if (particle[loop].life<0.0f)					// If Particle Is Burned Out
			{
				particle[loop].life=2.0f;				// Give It New Life
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
				particle[loop].size=0.75f;
				if ((1+(random()%20)) == 10)
				{
				// Explode
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
					particle[loop].size=1.0f;				// Set particle size.
				}
			}
			// Lets stir some things up
			index += 0.001;
			particle[loop].yg =2.0*sin(2*3.14*transIndex/360);
			particle[loop].xg =2.0*cos(2*3.14*transIndex/360);
			particle[loop].zg =4.0+(4.0*cos(2*3.14*transIndex/360));

		}
	}

	swapBuffers ();
	glFlush();
}
void Fountain::setSize( float newSize )
{
	size = newSize;
}
void Fountain::setStars( bool doStars )
{
	stars = doStars;
}
