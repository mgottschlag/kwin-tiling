//-----------------------------------------------------------------------------
//
// kfountain - Partical Fountain Screen Saver for KDE 2
//
// Copyright (c)  Ian Reinhart Geiser 2001
//
/////
//NOTE:
// The base particle engine did not come from me, it was designed by Jeff Molofee <nehe@connect.ab.ca>
// I did some extensive modifications to make it work with QT's OpenGL but the base principal is about
// the same.
////

#ifndef __FOUNTAIN_H__
#define __FOUNTAIN_H__

#include <qdialog.h>
#include <qgl.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <kscreensaver.h>
#include <qtimer.h>
#include <qimage.h>
#include "fountaincfg.h"

#define	MAX_PARTICLES	1000


class Fountain : public QGLWidget
{
Q_OBJECT
	class particles						// Create A Structure For Particle
	{
	public:
		bool	active;					// Active (Yes/No)
		float	life;					// Particle Life
		float	fade;					// Fade Speed
		float	r;					// Red Value
		float	g;					// Green Value
		float	b;					// Blue Value
		float	x;					// X Position
		float	y;					// Y Position
		float	z;					// Z Position
		float	xi;					// X Direction
		float	yi;					// Y Direction
		float	zi;					// Z Direction
		float	xg;					// X Gravity
		float	yg;					// Y Gravity
		float	zg;					// Z Gravity
		float	size;					// Particle Size
	};

public:
	Fountain( QWidget * parent=0, const char * name=0 );
	~Fountain();
	void setSize( float newSize );
	void setStars( bool doStars );
protected:
	/** paint the GL view */
	void paintGL ();
	/** resize the gl view */
	void resizeGL ( int w, int h );
	/** setup the GL enviroment */
	void initializeGL ();


private:
	/** load the partical file */
	bool loadParticle();

	particles particle[MAX_PARTICLES];


	bool	rainbow;					// Rainbow Mode?
	bool	sp;						// Spacebar Pressed?
	bool	rp;						// Enter Key Pressed?

	float	slowdown;					// Slow Down Particles
	float	xspeed;						// Base X Speed (To Allow Keyboard Direction Of Tail)
	float	yspeed;						// Base Y Speed (To Allow Keyboard Direction Of Tail)
	float	zoom;					// Used To Zoom Out
	float	size;
	float	stars;
	GLuint	loop;						// Misc Loop Variable
	GLuint	col;						// Current Color Selection
	GLuint	delay;						// Rainbow Effect Delay
	GLuint	texture[1];
	QImage  tex;
	float	index;
	float	transIndex;
	GLfloat scale;
	GLUquadricObj *obj;
};

class KFountainSaver : public KScreenSaver
{
Q_OBJECT
public:
	KFountainSaver( WId drawable );
	virtual ~KFountainSaver();
	void readSettings();
public slots:
	void blank();
	void updateSize(int newSize);
	void doStars(bool starState);
//	void loadTextures(bool textures);
private:
	Fountain *fountain;
	QTimer  *timer;
};

class KFountainSetup : public SetupUi
{
	Q_OBJECT
public:
	KFountainSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotOkPressed();
	void aboutPressed();
private:
	KFountainSaver *saver;
	float	size;
	float	stars;
};

#endif


