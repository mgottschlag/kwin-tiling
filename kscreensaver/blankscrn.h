//-----------------------------------------------------------------------------
//
// kblankscrn - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __BLANKSCRN_H__
#define __BLANKSCRN_H__

#include <qcolor.h>
#include <kdialogbase.h>
#include <kscreensaver.h>

class KColorButton;


class KBlankSaver : public KScreenSaver
{
	Q_OBJECT
public:
	KBlankSaver( WId drawable );
	virtual ~KBlankSaver();

	void setColor( const QColor &col );

private:
	void readSettings();
	void blank();

private:
	QColor color;
};

class KBlankSetup : public KDialogBase
{
	Q_OBJECT
public:
	KBlankSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotColor( const QColor & );
	void slotOk();

private:
	QWidget *preview;
	KBlankSaver *saver;

	QColor color;
};

#endif

