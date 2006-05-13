/*

(C) Daniel M. Duley <mosfet@kde.org>
(C) Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "kpanelappmenu.h"
#include <QStringList>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

static int panelmenu_get_seq_id()
{
    static int panelmenu_seq_no = -2;
    return panelmenu_seq_no--;
}

class KPanelAppMenu::Private
{
public:
    QByteArray realObjId;
};

KPanelAppMenu::KPanelAppMenu(const QString &title, QObject *parent,
                       const char *name)
    : QObject(parent), DCOPObject(), d(new Private())
{
    setObjectName( name );
    init(QPixmap(), title);
}

KPanelAppMenu::KPanelAppMenu(const QPixmap &icon, const QString &title,
                       QObject *parent, const char *name)
: QObject(parent), DCOPObject(), d(new Private())
{

    setObjectName( name );
    init(icon, title);
}


KPanelAppMenu::KPanelAppMenu(QObject *parent, const char *name)
  : QObject(parent), DCOPObject(name), d(new Private())
{
    setObjectName( name );
    d->realObjId = name;
}


void KPanelAppMenu::init(const QPixmap &icon, const QString &title)
{
    DCOPClient *client = kapp->dcopClient();
    if(!client->isAttached())
	client->attach();
    QByteArray sendData, replyData;
    DCOPCString replyType;
    {
	QDataStream stream(&sendData, QIODevice::WriteOnly);

	stream.setVersion(QDataStream::Qt_3_1);
	stream << icon << title;
	if ( client->call("kicker", "kickerMenuManager", "createMenu(QPixmap,QString)", sendData, replyType, replyData ) ) {
	  if (replyType != "QCString")
	    kDebug() << "error! replyType for createMenu should be QCstring in KPanelAppMenu::init" << endl;
	  else {
	    QDataStream reply( replyData );
	    reply >> d->realObjId;
	  }
	}
    }
    {
	QDataStream stream(&sendData, QIODevice::WriteOnly);

	stream.setVersion(QDataStream::Qt_3_1);
	stream << QByteArray("activated(int)") << client->appId() << objId();
	client->send("kicker", d->realObjId, "connectDCOPSignal(QCString,QCString,QCString)", sendData);
    }
}

KPanelAppMenu::~KPanelAppMenu()
{
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData;
    QDataStream stream(&sendData, QIODevice::WriteOnly);

    stream.setVersion(QDataStream::Qt_3_1);
    stream << d->realObjId;
    client->send("kicker", "kickerMenuManager", "removeMenu", sendData );

    delete d;
}

int KPanelAppMenu::insertItem(const QPixmap &icon, const QString &text, int id )
{
    if ( id < 0 )
	id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData;
    QDataStream stream(&sendData, QIODevice::WriteOnly);

    stream.setVersion(QDataStream::Qt_3_1);
    stream << icon << text << id;
    client->send("kicker", d->realObjId, "insertItem(QPixmap,QString,int)", sendData );
    return id;
}


KPanelAppMenu *KPanelAppMenu::insertMenu(const QPixmap &icon, const QString &text, int id )
{
    if ( id < 0 )
        id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData, replyData;
    DCOPCString replyType;
    QDataStream stream(&sendData, QIODevice::WriteOnly);

    stream.setVersion(QDataStream::Qt_3_1);
    stream << icon << text << id;
    client->call("kicker", d->realObjId, "insertMenu(QPixmap,QString,int)", sendData, replyType, replyData );
    if ( replyType != "QCString")
      return 0;
    QDataStream ret(replyData);
    QByteArray subid;
    ret >> subid;

    QByteArray sendData2;
    QDataStream stream2(&sendData2, QIODevice::WriteOnly);

    stream2.setVersion(QDataStream::Qt_3_1);
    stream2 << QByteArray("activated(int)") << client->appId() << subid;
    client->send("kicker", subid, "connectDCOPSignal(QCString,QCString,QCString)", sendData2);

    return new KPanelAppMenu(this, subid);
}


int KPanelAppMenu::insertItem(const QString &text, int id )
{
    if ( id < 0 )
	id = panelmenu_get_seq_id();
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData;
    QDataStream stream(&sendData, QIODevice::WriteOnly);

    stream.setVersion(QDataStream::Qt_3_1);
    stream << text << id;
    client->send("kicker", d->realObjId, "insertItem(QString,int)", sendData );
    return id;
}


void KPanelAppMenu::clear()
{
    DCOPClient *client = kapp->dcopClient();
    QByteArray sendData;
    client->send("kicker", d->realObjId, "clear()", sendData);
}


bool KPanelAppMenu::process(const DCOPCString &fun, const QByteArray &data,
			 DCOPCString &replyType, QByteArray &)
{
    if ( fun == "activated(int)" ) {
	QDataStream dataStream( data );
	int id;
	dataStream >> id;
	emit activated( id );
	replyType = "void";
	return true;
    }
    return false;
}


#include "kpanelappmenu.moc"












