/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004 by Oswald Buddenhagen <ossi@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kdmthemer.h"
#include "kdmitem.h"
#include "kdmpixmap.h"
#include "kdmrect.h"
#include "kdmlist.h"
#include "kdmlabel.h"
#include "kdmbutton.h"

#include <kdm_greet.h> // debug stuff
#include <kfdialog.h> // kfmsgbox

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QFile>
#include <QFileInfo>
#include <QPainter>

#include <unistd.h>

/*
 * KdmThemer. The main theming interface
 */
KdmThemer::KdmThemer(const QString &_filename,
                     const QMap<QString, bool> &showTypes, QWidget *w)
    : QObject()
    , m_showTypes(showTypes)
    , rootItem(0)
    , m_geometryOutdated(true)
    , m_geometryInvalid(true)
    , m_widget(0)
{
    // read the XML file and create DOM tree
    QString filename = _filename;
    if (!::access(QFile::encodeName(filename + "/KdmGreeterTheme.desktop"), R_OK)) {
        KConfig _cfg(filename + "/KdmGreeterTheme.desktop", KConfig::SimpleConfig);
        KConfigGroup cfg(&_cfg, "KdmGreeterTheme");
        filename += '/' + cfg.readEntry("Greeter");
    }
    QFile opmlFile(filename);
    if (!opmlFile.open(QIODevice::ReadOnly)) {
        KFMsgBox::box(w, errorbox, i18n("Cannot open theme file %1" , filename));
        return;
    }
    QDomDocument domTree;
    if (!domTree.setContent(&opmlFile)) {
        KFMsgBox::box(w, errorbox, i18n("Cannot parse theme file %1" , filename));
        return;
    }
    // generate all the items defined in the theme
    const QDomElement &theme = domTree.documentElement();
    // Get its tag, and check it's correct ("greeter")
    if (theme.tagName() != "greeter") {
        KFMsgBox::box(w, errorbox, i18n("%1 does not seem to be a correct theme file" , filename));
        return;
    }

    // Set the root (screen) item
    rootItem = new KdmRect(this, theme);

    basedir = QFileInfo(filename).absolutePath();

    generateItems(rootItem, theme);
}

KdmThemer::~KdmThemer()
{
}

void
KdmThemer::setWidget(QWidget *w)
{
    if ((m_widget = w)) {
        rootItem->updateVisible();
        m_widget->setStyle(rootItem->style.guistyle);
        m_widget->setPalette(rootItem->style.palette);
        m_widget->setFont(rootItem->style.font.font);
        rootItem->plugActions();
    }
}

KdmItem *
KdmThemer::findNode(const QString &item) const
{
    return rootItem->findChild<KdmItem *>(item);
}

void
KdmThemer::slotNeedPlacement()
{
    m_geometryOutdated = m_geometryInvalid = true;
    if (widget())
        widget()->update();
}

void
KdmThemer::slotNeedPlugging()
{
    if (widget())
        rootItem->plugActions();
}

void
KdmThemer::update(int x, int y, int w, int h)
{
    if (widget())
        widget()->update(x, y, w, h);
}

// BEGIN other functions

void
KdmThemer::widgetEvent(QEvent *e)
{
    if (!rootItem)
        return;
    switch (e->type()) {
    case QEvent::MouseMove: {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        rootItem->mouseEvent(me->x(), me->y());
        break; }
    case QEvent::MouseButtonPress: {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        rootItem->mouseEvent(me->x(), me->y(), true);
        break; }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        rootItem->mouseEvent(me->x(), me->y(), false, true);
        break; }
    case QEvent::Resize:
        m_geometryOutdated = true;
        widget()->update();
        break;
    case QEvent::Paint:
        if (m_geometryOutdated) {
            debug() << "==== updating geometry ====";
            QStack<QSize> ps;
            QRect rect(QPoint(0, 0), widget()->size());
            rootItem->setGeometry(ps, rect, m_geometryInvalid);
            if (debugLevel & DEBUG_THEMING)
                showStructure();
            m_geometryOutdated = m_geometryInvalid = false;
        }
        {
            QRect paintRect = static_cast<QPaintEvent *>(e)->rect();
            //kDebug() << "paint on: " << paintRect;

            QPainter p(widget());
            rootItem->paint(&p, paintRect, false, true);
            rootItem->showWidget();
        }
        break;
    default:
        break;
    }
}

void
KdmThemer::paintBackground(QPainter *p, const QRect &rect, bool primaryScreen)
{
    debug() << "==== setting background geometry ====";
    QStack<QSize> ps;
    rootItem->setGeometry(ps, rect, true);
    rootItem->paint(p, rect, true, primaryScreen);
}

void
KdmThemer::generateItems(KdmItem *parent, const QDomNode &node)
{
    /*
     * Go through each of the child nodes
     */
    const QDomNodeList &subnodeList = node.childNodes();
    for (int nod = 0; nod < subnodeList.count(); nod++) {
        QDomNode subnode = subnodeList.item(nod);
        QDomElement el = subnode.toElement();
        QString tagName = el.tagName();

        if (tagName == "item") {
            QString type = el.attribute("type");
            KdmItem *newItem;
            if (type == "label") {
                newItem = new KdmLabel(parent, subnode);
            } else if (type == "button") {
                newItem = new KdmButton(parent, subnode);
            } else if (type == "pixmap") {
                newItem = new KdmPixmap(parent, subnode);
            } else if (type == "rect") {
                newItem = new KdmRect(parent, subnode);
            } else if (type == "entry") {
                //newItem = new KdmEntry(parent, subnode);
                newItem = new KdmRect(parent, subnode);
                newItem->setType(type);
            } else if (type == "list") {
                newItem = new KdmList(parent, subnode);
            } else if (type == "svg") {
                newItem = new KdmPixmap(parent, subnode);
            } else {
                continue;
            }
            if (!newItem->isVisible()) {
                delete newItem;
                continue;
            }
            connect(newItem, SIGNAL(needUpdate(int,int,int,int)),
                    SLOT(update(int,int,int,int)));
            connect(newItem, SIGNAL(needPlacement()),
                    SLOT(slotNeedPlacement()));
            connect(newItem, SIGNAL(needPlugging()),
                    SLOT(slotNeedPlugging()));
            connect(newItem, SIGNAL(activated(QString)),
                    SIGNAL(activated(QString)));
            generateLayouts(newItem, subnode);
        }
    }
}

void
KdmThemer::generateLayouts(KdmItem *parent, const QDomNode &node)
{
    const QDomNodeList &subnodeList = node.childNodes();
    for (int nod = 0; nod < subnodeList.count(); nod++) {
        QDomNode subnode = subnodeList.item(nod);
        QString tagName = subnode.toElement().tagName();

        if (tagName == "box") {
            parent->setBoxLayout(subnode);
            generateItems(parent, subnode);
        } else if (tagName == "fixed") {
            parent->setFixedLayout(subnode);
            generateItems(parent, subnode);
        }
    }
}

void
KdmThemer::showStructure()
{
    QDebug(QtDebugMsg) << "======= item tree =======";
    rootItem->showStructure(QString());
}

void
KdmThemer::setTypeVisible(const QString &t, bool show)
{
    m_showTypes[t] = show;
    rootItem->updateVisible();
}

#include "kdmthemer.moc"
