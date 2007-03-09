#ifndef KSWIDGET_H
#define KSWIDGET_H

#include <QX11EmbedWidget>
#include <QWidget>
#include <X11/X.h>

class KSWidget : public QX11EmbedWidget
{
    Q_OBJECT
public:
    KSWidget( QWidget *parent = NULL, Qt::WindowFlags flags = 0 );
    virtual ~KSWidget();
private:
    Colormap colormap;
};

#endif
