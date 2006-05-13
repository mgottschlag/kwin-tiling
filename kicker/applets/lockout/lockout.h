#ifndef LOCKOUT_H
#define LOCKOUT_H

#include <QEvent>
#include <QString>
//Added by qt3to4:
#include <QMouseEvent>
#include <QBoxLayout>
#include <kpanelapplet.h>

#include "utils.h"

class QBoxLayout;
class QToolButton;

class Lockout : public KPanelApplet
{
    Q_OBJECT

public:
    Lockout( const QString& configFile,
	     QWidget *parent = 0, const char *name = 0 );
    ~Lockout();

    int widthForHeight(int height) const;
    int heightForWidth(int width) const;

protected:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseDoubleClickEvent( QMouseEvent * );

    virtual bool eventFilter( QObject *, QEvent * );

private Q_SLOTS:
    void lock();
    void logout();

    void slotLockPrefs();
    void slotLogoutPrefs();
    void slotTransparent();
    void slotIconChanged();

private:
    void propagateMouseEvent( QMouseEvent * );
    void checkLayout( int height ) const;

    QToolButton *lockButton, *logoutButton;
    QBoxLayout *layout;

    bool bTransparent;
};

#endif // LOCKOUT_H
