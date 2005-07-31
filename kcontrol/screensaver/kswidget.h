#ifndef KSWIDGET_H
#define KSWIDGET_H

#include <QX11EmbedWidget>
#include <qwidget.h>

class KSWidget : public QX11EmbedWidget
{
    Q_OBJECT
public:
    KSWidget( QWidget *parent = NULL );
};

#endif
