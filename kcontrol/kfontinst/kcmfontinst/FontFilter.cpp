/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FontFilter.h"
#include <klocale.h>
#include <kiconloader.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <kicon.h>
#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QStyleOption>
#include <QMenu>
#include <QMouseEvent>
#include <QApplication>
#include <QActionGroup>

namespace KFI
{

static const int constArrowPad(5);

CFontFilter::CFontFilter(QWidget *parent)
           : KLineEdit(parent)
{
    setClearButtonShown(true);
    setTrapReturnKey(true);

    itsMenuButton = new QLabel(this);
    itsMenuButton->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    itsMenuButton->setCursor(Qt::ArrowCursor);
    itsMenuButton->setToolTip(i18n("Set Criteria"));

    itsMenu=new QMenu(this);
    itsPixmaps[CRIT_FAMILY]=SmallIcon("text");
    itsPixmaps[CRIT_STYLE]=SmallIcon("format-text-bold");
    itsPixmaps[CRIT_FONTCONFIG]=SmallIcon("file-find");
    itsPixmaps[CRIT_FILENAME]=SmallIcon("font-type1");
    itsPixmaps[CRIT_LOCATION]=SmallIcon("folder");
    itsPixmaps[CRIT_WS]=SmallIcon("pencil");

    itsActionGroup=new QActionGroup(this);
    itsWsGroup=new QActionGroup(this);
    addAction(CRIT_FAMILY, i18n("Family"), true, true);
    addAction(CRIT_STYLE, i18n("Style"), false, true);
    addAction(CRIT_FONTCONFIG, i18n("FontConfig Match"), false, true);
    addAction(CRIT_FILENAME, i18n("File"), false, false);
    addAction(CRIT_LOCATION, i18n("File Location"), false, false);

    KActionMenu *wsMenu=new KActionMenu(KIcon(itsPixmaps[CRIT_WS]), i18n("Writing System"), this);
    itsActions[CRIT_WS]=wsMenu;
    itsMenu->addAction(itsActions[CRIT_WS]);
    wsMenu->setData((int)CRIT_WS);
    wsMenu->setVisible(false);

    itsCurrentWs=QFontDatabase::Any;
    for(int i=QFontDatabase::Latin; i<QFontDatabase::WritingSystemsCount; ++i)
    {
        KToggleAction *wsAct=new KToggleAction(QFontDatabase::Other==i
                                                ? i18n("Symbol/Other")
                                                : QFontDatabase::writingSystemName((QFontDatabase::WritingSystem)i), this);

        wsMenu->addAction(wsAct);
        itsWsGroup->addAction(wsAct);
        wsAct->setChecked(false);
        wsAct->setData(i);
        connect(wsAct, SIGNAL(toggled(bool)), SLOT(wsChanged()));
    }

    setCriteria(CRIT_FAMILY);
}

void CFontFilter::setMgtMode(bool m)
{
    if(!m && (itsActions[CRIT_FILENAME]->isChecked() ||
              itsActions[CRIT_LOCATION]->isChecked()))
    {
        setCriteria(CRIT_FAMILY);
        itsActions[CRIT_FAMILY]->setChecked(true);
        setText(QString());
    }
    itsActions[CRIT_FONTCONFIG]->setVisible(m);
    itsActions[CRIT_FILENAME]->setVisible(m);
    itsActions[CRIT_LOCATION]->setVisible(m);
    itsActions[CRIT_WS]->setVisible(m);
}

void CFontFilter::filterChanged()
{
    QAction *act(itsActionGroup->checkedAction());

    if(act)
    {
        ECriteria crit((ECriteria)act->data().toInt());

        if(itsCurrentCriteria!=crit)
        {
            if(itsCurrentWs!=QFontDatabase::Any)
            {
                QAction *prev(itsWsGroup->checkedAction());
                if(prev)
                    prev->setChecked(false);

                setText(QString());
                itsCurrentWs=QFontDatabase::Any;
            }
            setCriteria(crit);
            setClickMessage(i18n("Type here to filter on %1", act->text()));
            setReadOnly(false);
            modifyPadding();
        }
    }
}

void CFontFilter::wsChanged()
{
    QAction *act(itsWsGroup->checkedAction());

    if(act)
    {
        QFontDatabase::WritingSystem ws((QFontDatabase::WritingSystem)act->data().toInt());

        if(itsCurrentWs!=ws)
        {
            QAction *prev(itsActionGroup->checkedAction());
            if(prev)
                prev->setChecked(false);
            itsCurrentWs=ws;
            itsCurrentCriteria=CRIT_WS;
            setReadOnly(true);
            modifyPadding();
            setCriteria(itsCurrentCriteria);
            setText(i18n("%1 (Writing System)", act->text()));
            setClickMessage(text());
        }
    }
}

void CFontFilter::addAction(ECriteria crit, const QString &text, bool on, bool visible)
{
    itsActions[crit]=new KToggleAction(KIcon(itsPixmaps[crit]),
                                       text, this);
    itsMenu->addAction(itsActions[crit]);
    itsActionGroup->addAction(itsActions[crit]);
    itsActions[crit]->setData((int)crit);
    itsActions[crit]->setChecked(on);
    itsActions[crit]->setVisible(visible);
    if(on)
        setClickMessage(i18n("Type here to filter on %1", text));
    connect(itsActions[crit], SIGNAL(toggled(bool)), SLOT(filterChanged()));
}

void CFontFilter::paintEvent(QPaintEvent *ev)
{
    QLineEdit::paintEvent(ev);

    if (!hasFocus() && text().isEmpty())
    {
        QPainter p(this);
        QPen     oldPen(p.pen());

        p.setPen(palette().color(QPalette::Disabled, QPalette::Text));

        //FIXME: this rect is not where the text actually starts
        // qlineedit uses an internal qstyleoption set to figure this out
        // and then adds a hardcoded 2 pixel interior to that.
        // probably requires fixes to Qt itself to do this cleanly
        QRect cr(contentsRect());

        cr.adjust(itsMenuButton->width()+4, 0, -(itsMenuButton->width()+4), 0);
        p.drawText(cr, Qt::AlignLeft|Qt::AlignVCenter, clickMessage());
        p.setPen(oldPen);
    }
}

void CFontFilter::resizeEvent(QResizeEvent *ev)
{
    KLineEdit::resizeEvent(ev);
    modifyPadding();

    int frameWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    if (qApp->isLeftToRight())
        itsMenuButton->move(frameWidth + 1, frameWidth + 1);
    else
        itsMenuButton->move(size().width() - frameWidth - itsMenuButton->width() - 1, frameWidth + 1);
}

void CFontFilter::mousePressEvent(QMouseEvent *ev)
{
    if(Qt::LeftButton==ev->button() && itsMenuButton->underMouse())
        itsMenu->popup(mapToGlobal(QPoint(0, height())), 0);
    else
        KLineEdit::mousePressEvent(ev);
}

void CFontFilter::setCriteria(ECriteria crit)
{
    QPixmap arrowmap(itsPixmaps[crit].width()+constArrowPad, itsPixmaps[crit].height());

    arrowmap.fill(palette().color(QPalette::Active, QPalette::Base));

    QPainter p(&arrowmap);

    p.drawPixmap(0, 0, itsPixmaps[crit]);
    QStyleOption opt;
    opt.state = QStyle::State_None;
    opt.rect = QRect(arrowmap.width()-(constArrowPad+1), arrowmap.height()-constArrowPad, constArrowPad+1, constArrowPad);
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, &p, itsMenuButton);
    p.end();

    itsMenuButton->setPixmap(arrowmap);
    itsMenuButton->resize(arrowmap.width(), arrowmap.height());
    itsCurrentCriteria=crit;

    emit criteriaChanged(crit, ((qulonglong)1) << (int)itsCurrentWs);
}

void CFontFilter::modifyPadding()
{
    setStyleSheet(QString("QLineEdit { padding-left: %1; padding-right : %2; }")
                  .arg(itsMenuButton->width())
                  .arg(itsMenuButton->width()-constArrowPad));
}

}

#include "FontFilter.moc"
