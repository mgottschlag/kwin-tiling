/* -------------------------------------------------------------

   toplevel.h (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */


#ifndef _APPLET_H_
#define _APPLET_H_

#include <kpanelapplet.h>

class Klipper;

class KlipperApplet : public KPanelApplet
{
  Q_OBJECT
public:
	KlipperApplet(const QString& configFile, Type t = Normal, int actions = 0,
	            QWidget *parent = 0, const char *name = 0);
	~KlipperApplet();

	int widthForHeight(int h) const;
	int heightForWidth(int w) const;
protected:
        void resizeEvent( QResizeEvent* );
private:
        void centerWidget();
        Klipper* toplevel;
};

#endif
