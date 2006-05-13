/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

******************************************************************/

#include <QPixmap>
#include <QMenu>

#include <kapplication.h>
#include <kdebug.h>
#include <dcopclient.h>

#include "client_mnu.h"
#include "client_mnu.moc"

KickerClientMenu::KickerClientMenu( QWidget * parent )
    : QMenu( parent ), DCOPObject()
{
}

KickerClientMenu::~KickerClientMenu()
{
}

void KickerClientMenu::clear()
{
    QMenu::clear();
}

void KickerClientMenu::insertItem( QPixmap icon, QString text, int id )
{
    int globalid = QMenu::insertItem( icon, text, this, SLOT( slotActivated(int) ) );
    setItemParameter( globalid, id );
}

void KickerClientMenu::insertItem( QString text, int id )
{
    int globalid = QMenu::insertItem( text, this, SLOT( slotActivated(int) ) );
    setItemParameter( globalid, id );
}

DCOPCString KickerClientMenu::insertMenu( QPixmap icon, QString text, int id )
{
    QString subname("%1-submenu%2");
    QString subid = subname.arg(QString::fromLocal8Bit(objId())).arg(id);
    KickerClientMenu *sub = new KickerClientMenu(this);
    sub->setObjectName(subid);
    int globalid = QMenu::insertItem( icon, text, sub, id);
    setItemParameter( globalid, id );

    return subid.toLocal8Bit();
}

void KickerClientMenu::connectDCOPSignal( DCOPCString signal, DCOPCString appId, DCOPCString objId )
{
    // very primitive right now
    if ( signal == "activated(int)" ) {
	app = appId;
	obj = objId;
    } else {
	kWarning() << "DCOP: no such signal " << className() << "::" << signal.data() << endl;
    }
}

bool KickerClientMenu::process(const DCOPCString &fun, const QByteArray &data,
			       DCOPCString &replyType, QByteArray &replyData)
{
    if ( fun == "clear()" ) {
	clear();
	replyType = "void";
	return true;
    }
    else if ( fun == "insertItem(QPixmap,QString,int)" ) {
	QDataStream dataStream( data );
	dataStream.setVersion( QDataStream::Qt_3_1 );
	QPixmap icon;
	QString text;
	int id;
	dataStream >> icon >> text >> id;
	insertItem( icon, text, id );
	replyType = "void";
	return true;
    }
    else if ( fun == "insertMenu(QPixmap,QString,int)" ) {
	QDataStream dataStream( data );
	dataStream.setVersion( QDataStream::Qt_3_1 );
	QPixmap icon;
	QString text;
	int id;
	dataStream >> icon >> text >> id;
	DCOPCString ref = insertMenu( icon, text, id );
	replyType = "QCString";
	QDataStream replyStream( &replyData, QIODevice::WriteOnly );

	replyStream.setVersion(QDataStream::Qt_3_1);
	replyStream << ref;
	return true;
    }
    else if ( fun == "insertItem(QString,int)" ) {
	QDataStream dataStream( data );
	dataStream.setVersion( QDataStream::Qt_3_1 );
	QString text;
	int id;
	dataStream >> text >> id;
	insertItem( text, id );
	replyType = "void";
	return true;
    }
    else if ( fun == "connectDCOPSignal(QCString,QCString,QCString)" ) {
	QDataStream dataStream( data );
	dataStream.setVersion( QDataStream::Qt_3_1 );
	DCOPCString signal, appId, objId;
	dataStream >> signal >> appId >> objId;
	connectDCOPSignal( signal, appId, objId );
	replyType = "void";
	return true;
    }
    return false;
}

void KickerClientMenu::slotActivated(int id)
{
    if ( !app.isEmpty()  ) {
	DCOPRef(app, obj).send("activated", id);
    }
}
