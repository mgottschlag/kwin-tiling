/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include <KDE/KLocale>
#include <KDE/KIconLoader>
#include <KDE/KToggleAction>
#include <KDE/KSelectAction>
#include <KDE/KIcon>
#include <QtGui/QLabel>
#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QStyleOption>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtGui/QMenu>
#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>
#include <QtGui/QActionGroup>

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
    itsPixmaps[CRIT_FOUNDRY]=SmallIcon("user");
    itsPixmaps[CRIT_FONTCONFIG]=SmallIcon("edit-find");
    itsPixmaps[CRIT_FILENAME]=SmallIcon("font-type1");
    itsPixmaps[CRIT_LOCATION]=SmallIcon("folder");
    itsPixmaps[CRIT_WS]=SmallIcon("pencil");

    itsActionGroup=new QActionGroup(this);
    addAction(CRIT_FAMILY, i18n("Family"), true, true);
    addAction(CRIT_STYLE, i18n("Style"), false, true);

    KSelectAction *foundryMenu=new KSelectAction(KIcon(itsPixmaps[CRIT_FOUNDRY]), i18n("Foundry"), this);
    itsActions[CRIT_FOUNDRY]=foundryMenu;
    itsMenu->addAction(itsActions[CRIT_FOUNDRY]);
    foundryMenu->setData((int)CRIT_FOUNDRY);
    foundryMenu->setVisible(false);
    connect(foundryMenu, SIGNAL(triggered(const QString &)), SLOT(foundryChanged(const QString &)));

    addAction(CRIT_FONTCONFIG, i18n("FontConfig Match"), false, true);
    addAction(CRIT_FILENAME, i18n("File"), false, false);
    addAction(CRIT_LOCATION, i18n("File Location"), false, false);

    KSelectAction *wsMenu=new KSelectAction(KIcon(itsPixmaps[CRIT_WS]), i18n("Writing System"), this);
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
        wsAct->setChecked(false);
        wsAct->setData(i);
    }
    connect(wsMenu, SIGNAL(triggered(const QString &)), SLOT(wsChanged()));

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
    itsActions[CRIT_FOUNDRY]->setVisible(m);
    itsActions[CRIT_FONTCONFIG]->setVisible(m);
    itsActions[CRIT_FILENAME]->setVisible(m);
    itsActions[CRIT_LOCATION]->setVisible(m);
    itsActions[CRIT_WS]->setVisible(m);
}

void CFontFilter::setFoundries(const QSet<QString> &foundries)
{
    QAction     *act(((KSelectAction *)itsActions[CRIT_FOUNDRY])->currentAction());
    QString     prev(act && act->isChecked() ? act->text() : QString());
    QStringList list(foundries.toList());

    list.sort();

    // Add foundries to menu - replacing '&' with '&&', as '&' is taken to be
    // a shortcut!
    QStringList::ConstIterator it(list.begin()),
                               end(list.end());

    for(; it!=end; ++it)
    {
        QString foundry(*it);

        foundry.replace("&", "&&");
        ((KSelectAction *)itsActions[CRIT_FOUNDRY])->addAction(foundry);
    }

    if(!prev.isEmpty())
    {
        act=((KSelectAction *)itsActions[CRIT_FOUNDRY])->action(prev);
        if(act)
            ((KSelectAction *)itsActions[CRIT_FOUNDRY])->setCurrentAction(act);
        else
            ((KSelectAction *)itsActions[CRIT_FOUNDRY])->setCurrentItem(0);
    }
}

void CFontFilter::filterChanged()
{
    QAction *act(itsActionGroup->checkedAction());

    if(act)
    {
        ECriteria crit((ECriteria)act->data().toInt());

        if(itsCurrentCriteria!=crit)
        {
            QAction *prev(((KSelectAction *)itsActions[CRIT_FOUNDRY])->currentAction());
            if(prev)
                prev->setChecked(false);

            setText(QString());
            if(itsCurrentWs!=QFontDatabase::Any)
            {
                prev=((KSelectAction *)itsActions[CRIT_WS])->currentAction();
                if(prev)
                    prev->setChecked(false);

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
    QAction *act(((KSelectAction *)itsActions[CRIT_WS])->currentAction());

    if(act)
    {
        QFontDatabase::WritingSystem ws((QFontDatabase::WritingSystem)act->data().toInt());

        if(CRIT_FOUNDRY==itsCurrentCriteria)
        {
            QAction *prev(((KSelectAction *)itsActions[CRIT_FOUNDRY])->currentAction());
            if(prev)
                prev->setChecked(false);
        }

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
            setText(act->text());
            setClickMessage(text());
        }
    }
}

void CFontFilter::foundryChanged(const QString &foundry)
{
    QAction *prev(itsActionGroup->checkedAction());
    if(prev)
        prev->setChecked(false);

    if(CRIT_WS==itsCurrentCriteria)
    {
        prev=((KSelectAction *)itsActions[CRIT_WS])->currentAction();
        if(prev)
            prev->setChecked(false);
    }

    itsCurrentCriteria=CRIT_FOUNDRY;
    setReadOnly(true);
    modifyPadding();
    setText(foundry);
    setClickMessage(text());
    setCriteria(itsCurrentCriteria);
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

    QColor bgnd(palette().color(QPalette::Active, QPalette::Base));
    bgnd.setAlphaF(0.0);
    arrowmap.fill(bgnd);

    QPainter p(&arrowmap);

    p.drawPixmap(0, 0, itsPixmaps[crit]);
    QStyleOption opt;
    opt.state = QStyle::State_Enabled;
    opt.rect = QRect(arrowmap.width()-(constArrowPad+1), arrowmap.height()-(constArrowPad+1), constArrowPad, constArrowPad);
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
