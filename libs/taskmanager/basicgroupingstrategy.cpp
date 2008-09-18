/*****************************************************************

Copyright 2008 Christian Mollekopf <robertknight@gmail.com>

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

#include "basicgroupingstrategy.h"

#include <QColor>
#include <QString>
#include <QIcon>
#include <QSet>
#include <QList>
#include <QDebug>

#include <KIcon>

namespace TaskManager
{


BasicGroupingStrategy& BasicGroupingStrategy::instance(){
    static BasicGroupingStrategy singleton; //static so only first time created
    /*if (singleton == 0) {
        singleton = new BasicGroupingStrategy();
        //singleton->setParent(qApp); FIXME must be implemented, we could also use a static version like in Taskmanager http://geeklab.wikidot.com/cpp-singleton-pattern
    }*/
    return singleton;
}


BasicGroupingStrategy::BasicGroupingStrategy()
{
    qDebug();
}

QList<AbstractGroupingStrategy::GroupSuggestion> BasicGroupingStrategy::suggestGroups(const ItemList &items)
{
    qDebug();
    QList <AbstractGroupingStrategy::GroupSuggestion> list;

    QString name;
    QColor color;
    QIcon icon;
    ItemList set;

    name = suggestGroupNames(items).first();
    color = suggestGroupColors(items).first();
    icon = suggestGroupIcons(items).first();
    set = items;
   // AbstractGroupingStrategy::GroupSuggestion(name, color, icon, set);
    //list.append(new AbstractGroupingStrategy::GroupSuggestion(name, color, icon, set)); //FIXME

    return list;
}

    
QList<QString> BasicGroupingStrategy::suggestGroupNames(const ItemList &items)
{
    qDebug();
    QList<QString> list;
    QString name;
    
    for (int i = 1;  ; i++) {
        name = "Group "+QString::number(i);
        if (!m_usedNames.contains(name)) {
            break;;
        }
    }
    list.append(name);
    return list;
}


QList<QColor> BasicGroupingStrategy::suggestGroupColors(const ItemList &items)
{
    qDebug();
    QList<QColor> list;
    QColor color;
    for (int i = 1;i <= 255; i++) {
        color = QColor(qrand()%255,qrand()%255,qrand()%255,125); //random color
        if (!m_usedColors.contains(color)) {
            break;
        }
    }
    list.append(color);

    return list;
}

QList<QIcon> BasicGroupingStrategy::suggestGroupIcons(const ItemList &items)
{
    qDebug();
    QList<QIcon> list;

   /* QPixmap pixmap = KIconLoader::global()->loadIcon( "xorg",KIconLoader::Panel );
    QIcon icon = QIcon(pixmap);*/

    list.append(KIcon("xorg"));

    return list;
}

void BasicGroupingStrategy::groupAccepted(const AbstractGroupingStrategy::GroupSuggestion &suggestion)
{
    nameAccepted(suggestion.name());
    colorAccepted(suggestion.color());
}

void BasicGroupingStrategy::nameAccepted(const QString &name)
{
    m_usedNames.append(name);
}

void BasicGroupingStrategy::colorAccepted(const QColor &color)
{
    m_usedColors.append(color);
}

void BasicGroupingStrategy::groupRemoved(const AbstractGroupingStrategy::GroupSuggestion &suggestion)
{
    nameRemoved(suggestion.name());
    colorRemoved(suggestion.color());
}

void BasicGroupingStrategy::nameRemoved(const QString &name)
{
    m_usedNames.removeAll(name);
}

void BasicGroupingStrategy::colorRemoved(const QColor &color)
{
    m_usedColors.removeAll(color);
}

}

#include "basicgroupingstrategy.moc"

