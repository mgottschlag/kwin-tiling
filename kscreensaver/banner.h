//-----------------------------------------------------------------------------
//
// kbanner - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef __BANNER_H__
#define __BANNER_H__

#include <qtimer.h>
#include <qlist.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include "saver.h"


#define SATURATION 100
#define VALUE 255

class KBannerSaver : public kScreenSaver
{
	Q_OBJECT
public:
	KBannerSaver( Drawable drawable );
	virtual ~KBannerSaver();

	void setSpeed( int spd );
	void setFont( const QString &family, int size, const QColor &color,
			bool b, bool i );
	void setMessage( const QString &msg );
	void setTimeDisplay();
   void setCyclingColor(bool on);
   void setColor( QColor &color);
   
private:
	void readSettings();
	void initialize();
	void blank();

protected slots:
	void slotTimeout();

protected:
	XFontStruct *fontInfo;
	QTimer		timer;
	QString		fontFamily;
	int			fontSize;
	bool		bold;
	bool		italic;
	QColor		fontColor;
   bool cyclingColor;
   int currentHue;
   QString		message;
   bool showTime;
	int			xpos, ypos, step;
	int			fwidth;
	int			speed;
	int			colorContext;
};


class KBannerSetup : public QDialog
{
	Q_OBJECT
public:
	KBannerSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private slots:
	void slotFamily( const QString & );
	void slotSize( int );
	void slotColor();
   void slotCyclingColor(bool on);
	void slotBold( bool );
	void slotItalic( bool );
	void slotSpeed( int );
	void slotMessage( const QString & );
	void slotOkPressed();
	void slotAbout();
   void slotTimeToggled(bool on);

private:
	QWidget *preview;
	QPushButton *colorPush;
	KBannerSaver *saver;
   QLineEdit *ed;

	QString	message;
   bool showTime;
   QString fontFamily;
	int		fontSize;
	QColor	fontColor;
   bool cyclingColor;
   bool	bold;
	bool	italic;
	int		speed;
};

#endif

