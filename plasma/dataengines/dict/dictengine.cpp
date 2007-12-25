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

#include <plasma/datacontainer.h>

DictEngine::DictEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent),
      dictHash(0),
      tcpSocket(0)
{
    Q_UNUSED(args)
    serverName="dict.org"; //In case we need to switch it later
    dictName="gcide"; //Default, good dictionary
}

DictEngine::~DictEngine()
{
}

void DictEngine::setDict(const QString &dict)
{
	dictName=dict;
}

void DictEngine::setServer(const QString &server)
{
    serverName=server;
}

void DictEngine::getDefinition()
{
      if(currentWord == QLatin1String("about"))
      {
          setData(currentWord, "gcide", "<!--PAGE START--><!--DEFINITION START--><dl><dt><b>Developers</b></dt><!--PAGE START--><dd>KDE4 Dictionary Applet for Plasma was written by <i>Thomas Georgiou</i> and <i>Jeff Cooper</i></dd></dl>");
            return;
      }

      tcpSocket->waitForReadyRead();
      tcpSocket->readAll();
      QByteArray ret;

      tcpSocket->write(QByteArray("DEFINE "));
      tcpSocket->write(dictName.toAscii());
      tcpSocket->write(QByteArray(" \""));
      tcpSocket->write(currentWord.toAscii());
      tcpSocket->write(QByteArray("\"\n"));
      tcpSocket->flush();

      while (!ret.contains("250") && !ret.contains("552"))
      {
        tcpSocket->waitForReadyRead();
        ret += tcpSocket->readAll();
      }

      connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(socketClosed()));
      tcpSocket->disconnectFromHost();
      setData(currentWord, dictName, parseToHtml(ret));
}


QString DictEngine::parseToHtml(QByteArray &text)
{
      QList<QByteArray> retLines = text.split('\n');
      QString def;
      if(currentWord == QLatin1String("plasma")) //EASTER EGG!
      {
          def += "<dl><!--PAGE START--><!--DEFINITION START--><dt><b>Plasma</b>  \\Plas\"ma\\, a.(for awesome)</dt><!--PAGE START--><dd>OOH! I know that one! Plasma is that awesome new desktop thing for KDE4! Oh wait, you want an actual definition? Here, No fun...</dd></dl><br />";
      }
      def += "<dl>\n";

      bool isFirst=true;
      QString wordRegex; //case insensitive regex of the word
      for(int i=0;i<currentWord.size();i++)
      {
          wordRegex += ('['+QString(currentWord[i].toUpper())+QString(currentWord[i].toLower())+']');
      }

      while (!retLines.empty()) //iterate through all the lines
      {
          QString currentLine = QString(retLines.takeFirst());
          if (currentLine.startsWith("552")) //if no match was found
          {
              def += "<dt>";
              def += i18n("<b>No match found for %1 in database %2.</b>", currentWord, dictName);
              def += "</dt>";
              break;
          }
          if (currentLine.startsWith("151")) //begin definition
          {
              isFirst = true;
              continue;
          }
          if (currentLine.startsWith('.')) //end definition
          {
              def += "</dd><!--PERIOD-->";
              continue;
          }
          if (!(currentLine.startsWith("150") || currentLine.startsWith("151")// if it is a definition line
             || currentLine.startsWith("250") || currentLine.startsWith("552")))
          {
              currentLine = currentLine.trimmed();
              if (currentLine.startsWith("1."))
                  def += "<br />";
              if (currentLine.contains(QRegExp("^([1-9]{1,2}\\.)")))
                  def += "<br />";
              currentLine.replace(QRegExp("\\{([A-Za-z ]+)\\}"), "<a href=\"\\1\" style=\"color: #0000FF\" >\\1</a>");
              currentLine.replace(QRegExp("^([1-9]{1,2}\\.)"), "<!--PAGE START--><b>\\1</b>");
              currentLine.replace(QRegExp("((^| |\\.)"+wordRegex+"( |\\.|(i?e)?s|$))"), "<b>\\1</b>"); //the i?e?s is for most plurals... i'll fix it soon
              currentLine.replace(QRegExp("(^| |\\.)(\\[[^]]+\\])( |\\.|$)"), "<i>\\2</i>");

              //currentLine.replace(currentWord, "<b>"+currentWord+"</b>", Qt::CaseInsensitive);

              if(isFirst)
              {
                  def += "<!--PAGE START--><!--DEFINITION START--><dt>" + currentLine.toAscii() + "</dt>\n<dd>\n";
                  isFirst = false;
                  continue;
              }

              if(currentLine == "." || currentLine.isEmpty())
                  def += "\n<br />\n";
              else
                  def += currentLine.toAscii() + '\n';
          }

      }
      def+="</dl>";
      return def;
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
    while (!ret.contains("250"))
      {
        tcpSocket->waitForReadyRead();
        ret += tcpSocket->readAll();
      }

    QList<QByteArray> retLines = ret.split('\n');

    QString tmp1, tmp2;


    while (!retLines.empty())
    {
        QString curr = QString(retLines.takeFirst());
        if (curr.startsWith("554"))
        {
            //TODO: What happens if no DB available?
            //TODO: Eventually there will be functionality to change the server...
            break;
        }
        if (!curr.startsWith('-'))
        {
            curr = curr.trimmed();
            tmp1=curr.section(' ',0,1);
            tmp2=curr.section(' ',1);
  //          theHash.insert(tmp1, tmp2);
            kDebug() << tmp1 + "  " + tmp2;
            setData("showDictionaries",tmp1,tmp2);
        }
    }
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(socketClosed()));
    tcpSocket->disconnectFromHost();
//    setData("showDictionaries", "dictionaries", QByteArray(theHash);
}



void DictEngine::socketClosed()
{
    tcpSocket->deleteLater();
    tcpSocket = 0;
}

bool DictEngine::sourceRequested(const QString &word)
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
          if (currentWord == "showDictionaries")
          {
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
