#ifndef STDCLIENT_H
#define STDCLIENT_H
#include "client.h"
class QToolButton;
class QLabel;
class QSpacerItem;
class QColorGroup;

class StdClient : public Client
{
    Q_OBJECT
public:
    StdClient( Workspace *ws, WId w, QWidget *parent=0, const char *name=0 );
    ~StdClient();
protected:
    void resizeEvent( QResizeEvent* );
    void paintEvent( QPaintEvent* );

    void mouseDoubleClickEvent( QMouseEvent * );
    void init();
    QColorGroup makeColorGroup(const QColor &bg, const QColor &fg=Qt::white);
    void captionChange( const QString& name );
    void iconChange();
    void maximizeChange( bool );
    void stickyChange( bool );

private:
    QToolButton* button[6];
    QSpacerItem* titlebar;
    QColorGroup aFrameGrp, iFrameGrp, aTitleGrp, iTitleGrp;
};


#endif
