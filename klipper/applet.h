/* -------------------------------------------------------------

   toplevel.h (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

   Licensed under the Artistic License

 ------------------------------------------------------------- */


#ifndef _APPLET_H_
#define _APPLET_H_

#include <kpanelapplet.h>

#include "toplevel.h"

class KlipperAppletWidget;

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
    void preferences();
    void help();
    void about();

private:
    void centerWidget();
    KlipperAppletWidget* widget;
};

class KlipperAppletWidget : public KlipperWidget
{
    Q_OBJECT
    K_DCOP
k_dcop:
    int newInstance();
public:
    KlipperAppletWidget( QWidget* parent = NULL );
    virtual ~KlipperAppletWidget();
private:
    DCOPClient* m_dcop;

};

#endif
