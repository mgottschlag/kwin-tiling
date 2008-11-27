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

#include "dictengine.h"
#include <iostream>

#include <QtNetwork/QTcpSocket>
#include <KDebug>
#include <KLocale>

#include <Plasma/DataContainer>

DictEngine::DictEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
    , tcpSocket(0)
{
    Q_UNUSED(args)
    serverName="dict.org"; //In case we need to switch it later
    dictName="wn"; //Default, good dictionary
}

DictEngine::~DictEngine()
{
}

void DictEngine::setDict(const QString &dict)
{
    dictName = dict;
}

void DictEngine::setServer(const QString &server)
{
    serverName = server;
}

static QString wnToHtml(const QString &word, QByteArray &text)
{
    QList<QByteArray> splitText = text.split('\n');
    QString def;
    def += "<dl>\n";
    QRegExp linkRx("\\{(.*)\\}");
    linkRx.setMinimal(true);

    bool isFirst=true;
    while (!splitText.empty()) {
        //150 n definitions retrieved - definitions follow
        //151 word database name - text follows
        //250 ok (optional timing information here)
        //552 No match
        QString currentLine = splitText.takeFirst();
        if (currentLine.startsWith("151")) {
            isFirst = true;
            continue;
        }

        if (currentLine.startsWith('.')) {
            def += "</dd>";
            continue;
        }

        if (!(currentLine.startsWith("150") || currentLine.startsWith("151")
           || currentLine.startsWith("250") || currentLine.startsWith("552"))) {
            currentLine.replace(linkRx,"<a href=\"\\1\">\\1</a>");

            if (isFirst) {
                def += "<dt><b>" + currentLine + "</b></dt>\n<dd>";
                isFirst = false;
                continue;
            } else {
                if (currentLine.contains(QRegExp("([1-9]{1,2}:)"))) {
                    def += "\n<br>\n";
                }
                currentLine.replace(QRegExp("^([\\s\\S]*[1-9]{1,2}:)"), "<b>\\1</b>");
                def += currentLine;
                continue;
            }
        }

    }

    def += "</dl>";
    return def;
}

void DictEngine::getDefinition()
{
    tcpSocket->waitForReadyRead();
    tcpSocket->readAll();
    QByteArray ret;

    tcpSocket->write(QByteArray("DEFINE "));
    tcpSocket->write(dictName.toAscii());
    tcpSocket->write(QByteArray(" \""));
    tcpSocket->write(currentWord.toAscii());
    tcpSocket->write(QByteArray("\"\n"));
    tcpSocket->flush();

    while (!ret.contains("250") && !ret.contains("552")) {
        tcpSocket->waitForReadyRead();
        ret += tcpSocket->readAll();
    }

    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(socketClosed()));
    tcpSocket->disconnectFromHost();
    //       setData(currentWord, dictName, ret);
    //       qWarning()<<ret;
    setData(currentWord, "text", wnToHtml(currentWord,ret));
}




void DictEngine::getDicts()
{
    QMap<QString, QString> theHash;
    tcpSocket->waitForReadyRead();
    tcpSocket->readAll();
    QByteArray ret;

    tcpSocket->write(QByteArray("SHOW DB\n"));;
    tcpSocket->flush();

    tcpSocket->waitForReadyRead();
    while (!ret.contains("250")) {
        tcpSocket->waitForReadyRead();
        ret += tcpSocket->readAll();
    }

    QList<QByteArray> retLines = ret.split('\n');
    QString tmp1, tmp2;

    while (!retLines.empty()) {
        QString curr = QString(retLines.takeFirst());

        if (curr.startsWith("554")) {
            //TODO: What happens if no DB available?
            //TODO: Eventually there will be functionality to change the server...
            break;
        }

        if (!curr.startsWith('-')) {
            curr = curr.trimmed();
            tmp1 = curr.section(' ', 0, 1);
            tmp2 = curr.section(' ', 1);
  //          theHash.insert(tmp1, tmp2);
            //kDebug() << tmp1 + "  " + tmp2;
            setData("list-dictionaries", tmp1, tmp2);
        }
    }

    tcpSocket->disconnectFromHost();
//    setData("list-dictionaries", "dictionaries", QByteArray(theHash);
}



void DictEngine::socketClosed()
{
    tcpSocket->deleteLater();
    tcpSocket = 0;
}

bool DictEngine::sourceRequestEvent(const QString &word)
{
      if (tcpSocket && currentWord != word)
      {
          tcpSocket->abort(); //stop if lookup is in progress and new word is requested
          tcpSocket->deleteLater();
          tcpSocket = 0;
      }
      currentWord = word;
      if (currentWord.simplified().count() != 0)
      {
          setData(currentWord, dictName, QString());
          tcpSocket = new QTcpSocket(this);
          tcpSocket->abort();
          connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(socketClosed()));

          if (currentWord == "list-dictionaries") {
              connect(tcpSocket, SIGNAL(connected()), this, SLOT(getDicts()));
          } else {
              connect(tcpSocket, SIGNAL(connected()), this ,SLOT(getDefinition()));
          }
          tcpSocket->connectToHost(serverName, 2628);
      } else {
          setData(currentWord, dictName, QString());
      }
      return true;
}

#include "dictengine.moc"
