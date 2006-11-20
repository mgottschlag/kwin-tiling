// TODO insert license here
#ifndef XINERAMAWIDGET_H
#define XINERAMAWIDGET_H

#include "ui_xineramawidget.h"


class XineramaWidget : public QWidget, public Ui_XineramaWidget
{
    Q_OBJECT

public:
    XineramaWidget( QWidget* parent = 0 );
    
signals:
    void configChanged();

public slots:
   virtual void emitConfigChanged();
private:

};

#endif
