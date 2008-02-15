/*
 *   Copyright (C) 2007 Thomas Georgiou <TAGeorgiou@gmail.com> and Jeff Cooper <weirdsox11@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DICTENGINE_H
#define DICTENGINE_H
#include <plasma/dataengine.h>
#include <QMap>
#include <QString>
#include <QList>
class QTcpSocket;

/**
 * This class evaluates the basic expressions given in the interface.
 */


class DictEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        DictEngine( QObject* parent, const QVariantList& args );
        ~DictEngine();

    protected:
        bool sourceRequested(const QString &word);

    protected slots:
	void getDefinition();
        void socketClosed();
        void getDicts();

    private:
        void setDict(const QString &dict);
        void setServer(const QString &server);

        QHash<QString, QString> *dictHash;
	QString parseToHtml(QByteArray &text);
	QTcpSocket *tcpSocket;
	QString currentWord;
	QString dictName;
        QString serverName;

};

K_EXPORT_PLASMA_DATAENGINE(dict, DictEngine)

#endif
