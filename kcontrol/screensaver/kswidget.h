#ifndef KSWIDGET_H
#define KSWIDGET_H

#include <qx11embed_x11.h>
#include <qwidget.h>

class KSWidget : public QX11EmbedWidget
{
    Q_OBJECT
public:
    KSWidget( QWidget *parent = NULL, const char* name = NULL, Qt::WFlags flags = 0 );
};

#endif
