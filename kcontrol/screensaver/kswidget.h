#ifndef KSWIDGET_H
#define KSWIDGET_H

#include <qxembed.h>
#include <qwidget.h>

class KSWidget : public QXEmbed
{
    Q_OBJECT
public:
    KSWidget( QWidget *parent = NULL, const char* name = NULL, Qt::WFlags flags = 0 );
};

#endif
