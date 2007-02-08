/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
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
           : KLineEdit(parent),
             itsClickInMenuButton(false)
{
    //setClickMessage(i18n("Filter"));
    setClearButtonShown(true);
    setTrapReturnKey(true);

    itsMenuButton = new QLabel(this);
    itsMenuButton->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    itsMenuButton->setCursor(Qt::ArrowCursor);
    itsMenuButton->setToolTip(i18n("Set Criteria"));

    itsMenu=new QMenu(this);
    itsPixmaps[CRIT_FAMILY]=SmallIcon("text");
    itsPixmaps[CRIT_STYLE]=SmallIcon("text_bold");
    itsPixmaps[CRIT_FILENAME]=SmallIcon("font_type1");
    itsPixmaps[CRIT_LOCATION]=SmallIcon("folder");

    itsActionGroup=new QActionGroup(this);
    addAction(CRIT_FAMILY, i18n("Filter On Font Family"), true);
    addAction(CRIT_STYLE, i18n("Filter On Font Style"), false);
    addAction(CRIT_FILENAME, i18n("Filter On Font File"), false);
    addAction(CRIT_LOCATION, i18n("Filter On Font File Location"), false);

    setCriteria(CRIT_FAMILY);
}

void CFontFilter::filterChanged()
{
    QAction *act(itsActionGroup->checkedAction());

    if(act)
    {
        ECriteria crit((ECriteria)act->data().toInt());

        if(itsCurrentCriteria!=crit)
        {
            setCriteria(crit);
            setClickMessage(act->text());
        }
    }
}

void CFontFilter::addAction(ECriteria crit, const QString &text, bool on)
{
    KToggleAction *action=new KToggleAction(KIcon(itsPixmaps[crit]),
                                            text, this);
    itsMenu->addAction(action);
    itsActionGroup->addAction(action);
    action->setData((int)crit);
    action->setChecked(on);
    if(on)
        setClickMessage(text);
    connect(action, SIGNAL(toggled(bool)), SLOT(filterChanged()));
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

        cr.addCoords(itsMenuButton->width()+4, 0, -(itsMenuButton->width()+4), 0);
        p.drawText(cr, Qt::AlignLeft|Qt::AlignVCenter, clickMessage());
        p.setPen(oldPen);
    }
}

void CFontFilter::resizeEvent(QResizeEvent *ev)
{
    KLineEdit::resizeEvent(ev);
    setStyleSheet(QString("QLineEdit { padding-left: %1; padding-right : %2; }")
                  .arg(itsMenuButton->width())
                  .arg(itsMenuButton->width()-constArrowPad));

    int frameWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    if (qApp->isLeftToRight())
        itsMenuButton->move(frameWidth + 1, frameWidth + 1);
    else
        itsMenuButton->move(size().width() - frameWidth - itsMenuButton->width() - 1, frameWidth + 1);
}

void CFontFilter::mousePressEvent(QMouseEvent *ev)
{
    if(Qt::LeftButton==ev->button() && itsMenuButton->underMouse())
        itsClickInMenuButton=true;
    else
        KLineEdit::mousePressEvent(ev);
}

void CFontFilter::mouseReleaseEvent(QMouseEvent *ev)
{
    if (itsClickInMenuButton)
    {
        itsClickInMenuButton=false;
        if (itsMenuButton->underMouse())
            itsMenu->popup(mapToGlobal(QPoint(0, height())), 0);
        return;
    }

    KLineEdit::mouseReleaseEvent(ev);
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
    itsCurrentCriteria=crit;

    emit criteriaChanged(crit);
}

}

#include "FontFilter.moc"
