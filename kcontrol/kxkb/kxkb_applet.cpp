#include <QWidget>
#include <QString>

#include <kglobal.h>
#include <klocale.h>

#include "kxkbwidget.h"
#include "kxkbcore.h"
#include "kxkb_applet.h"

#include "kxkb_applet.moc"


extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalog("kxkb");
        int actions = Plasma::Preferences | Plasma::About | Plasma::Help;
        return new KxkbApplet(configFile, Plasma::Normal, actions, parent);
    }
}


KxkbApplet::KxkbApplet(const QString& configFile, Plasma::Type type,
                 int actions, QWidget *parent, Qt::WFlags f)
    : KPanelApplet(configFile, type, actions, parent, f)
{
    move( 0, 0 );
    kxkbWidget = new KxkbLabel( this );
	KxkbCore* kxkbCore = new KxkbCore(kxkbWidget);
	kxkbCore->newInstance();
    //setCustomMenu(widget->history()->popup());
    //centerWidget();
    //kxkbWidget->show();
}

KxkbApplet::~KxkbApplet()
{
}

int KxkbApplet::widthForHeight(int height) const
{
	return 32;//kxkbWidget->width();
}

int KxkbApplet::heightForWidth(int width) const
{
	return 32;//kxkbWidget->height();
}
