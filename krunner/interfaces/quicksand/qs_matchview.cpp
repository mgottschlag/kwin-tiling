/*
 *   Copyright (C) 2007-2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#include <cmath>

#include <QBoxLayout>
#include <QFocusEvent>
#include <QGraphicsItemAnimation>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QTimeLine>
#include <QTimer>
#include <QToolButton>

#include <KDebug>
#include <KIcon>
#include <KLineEdit>
#include <KLocale>

#include <Plasma/Theme>

#include "qs_completionbox.h"
#include "qs_statusbar.h"
#include "qs_matchitem.h"
#include "qs_matchview.h"

//Widget dimensions
const int WIDTH = 390;
const int HEIGHT = 80; //10px overlap with text
const int ICON_AREA_HEIGHT = 70; //3 px margins
const int LARGE_ICON_PADDING = 3;
const int SMALL_ICON_PADDING = 19; //(70 - ITEM_SIZE)/2
//FIXME: Magic numbers galore...

namespace QuickSand{

class QsMatchView::Private
{
    public:
        QLabel *m_titleLabel;
        QLabel *m_itemCountLabel;
        QToolButton *m_arrowButton;
        QStackedWidget *m_stack;
        QGraphicsScene *m_scene;
        QGraphicsView *m_view;
        KLineEdit *m_lineEdit;
        QsCompletionBox *m_compBox;
        QList<MatchItem*> m_items;
        QString m_searchTerm;
        QString m_itemCountSuffix;
        QGraphicsRectItem *m_descRect;
        QGraphicsTextItem *m_descText;
        int m_currentItem;
        bool m_hasFocus;
        bool m_itemsRemoved;
        bool m_listVisible;
        bool m_selectionMade;
};

QsMatchView::QsMatchView(QWidget *parent)
    : QWidget(parent),
      d(new Private())
{
    setFocusPolicy(Qt::StrongFocus);
    //Track focus because focus changes between internal widgets trigger focus events
    d->m_hasFocus = false;
    d->m_itemsRemoved = false;
    d->m_listVisible = true;
    d->m_selectionMade = false; //Prevent completion box from popping up once a user chooses a match
    //FIXME: don't hardcode black
    setStyleSheet("QListWidget {color: black} QLineEdit {color: black}");

    d->m_descRect = 0;
    d->m_descText = 0;

    d->m_view = new QGraphicsView(this);
    d->m_view->setRenderHint(QPainter::Antialiasing);
    d->m_view->viewport()->setAutoFillBackground(false);
    d->m_view->setInteractive(true);
    d->m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_view->setOptimizationFlag(QGraphicsView::DontSavePainterState);
    d->m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->m_view->setFocusPolicy(Qt::NoFocus);

    d->m_scene = new QGraphicsScene(-WIDTH/2, 0, WIDTH, HEIGHT, this);
    d->m_view->setScene(d->m_scene);

    d->m_currentItem = 0;

    d->m_lineEdit = new KLineEdit(this);
    d->m_compBox = new QuickSand::QsCompletionBox(this);
    d->m_compBox->setTabHandling(false);

    d->m_stack = new QStackedWidget(this);
    d->m_stack->addWidget(d->m_view);
    d->m_stack->addWidget(d->m_lineEdit);
    d->m_stack->setCurrentIndex(0);

    d->m_titleLabel = new QLabel(this);
    d->m_itemCountLabel = new QLabel(this);
    d->m_itemCountSuffix = i18n("items");

    d->m_arrowButton = new QToolButton(this);
    d->m_arrowButton->setFocusPolicy(Qt::NoFocus);
    d->m_arrowButton->setArrowType(Qt::RightArrow);
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QString buttonStyleSheet = QString("QToolButton { border-radius: 4px; border: 0px; background-color: transparent }");
    buttonStyleSheet += QString("QToolButton:hover { border: 1px solid %1; }")
                            .arg(theme->color(Plasma::Theme::HighlightColor).name());
    d->m_arrowButton->setStyleSheet(buttonStyleSheet);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(d->m_titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(d->m_itemCountLabel);
    topLayout->addWidget(d->m_arrowButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(topLayout);
    layout->addWidget(d->m_stack);

    connect(d->m_compBox, SIGNAL(currentRowChanged(int)), this, SLOT(scrollToItem(int)));
    connect(d->m_compBox, SIGNAL(activated(const QString&)), this, SLOT(showSelected()));
    connect(d->m_lineEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(textChanged(const QString&)));
    connect(d->m_arrowButton, SIGNAL(pressed()), this, SLOT(toggleView()));

    reset();
}

QsMatchView::~QsMatchView()
{
    qDeleteAll(d->m_items);
    d->m_items.clear();
    delete d;
}

void QsMatchView::reset()
{
    clear(true);

    d->m_stack->setCurrentIndex(0);
    d->m_arrowButton->hide();
    d->m_listVisible = true;
    d->m_selectionMade = false;
    d->m_hasFocus = false;
    d->m_searchTerm = QString();
    d->m_compBox->clear();
    d->m_compBox->hide();
    d->m_itemCountLabel->setText(QString());

    QGraphicsPixmapItem *p = new QGraphicsPixmapItem(KIcon("edit-find").pixmap(MatchItem::ITEM_SIZE));
    p->setPos(-MatchItem::ITEM_SIZE/2, LARGE_ICON_PADDING);
    d->m_scene->addItem(p);
    //Replace with a suitable message
    setDescriptionText(i18n("Type to search."));
}

void QsMatchView::setItems(const QList<MatchItem*> &items, bool popup, bool append)
{
    int spacing = MatchItem::ITEM_SIZE/2;

    int pos = spacing;

    if (!append) {
        clear(true);
        d->m_compBox->clear();

        d->m_currentItem = -1;
        d->m_items = items;
    } else {
        // FIXME: This completely disregards item ranking
        // Maybe should we just sort then scroll to previously selected item
        if (!d->m_items.isEmpty()) {
            pos += d->m_items.last()->pos().x();
        }
        d->m_items << items;
    }

    foreach(MatchItem *item, items) {
        if (item) {
            item->setPos(pos, SMALL_ICON_PADDING);
            item->scale(0.5, 0.5);
            pos += spacing;
            if (d->m_listVisible) {
                d->m_scene->addItem(item);
            }
            QString description;
            if (item->description().isEmpty()) {
                description = item->name();
            } else {
                description = QString("%1 (%2)").arg(item->name()).arg(item->description());
            }
            QListWidgetItem *wi = new QListWidgetItem(item->icon(), description, d->m_compBox);
            d->m_compBox->addItem(wi);
        }
    }
    d->m_itemsRemoved = false;
    setItemCount(d->m_items.size());

    if (d->m_selectionMade) {
        //kDebug() << "A user selection was already made" << endl;
        return;
    }

    scrollToItem(0);

    //Ensure popup is shown if desired
    if (popup) {
        if (items.size()) {
            d->m_compBox->popup();
            d->m_compBox->setCurrentRow(0);
        } else {
            d->m_compBox->hide();
        }
        d->m_arrowButton->setArrowType(Qt::DownArrow);
    } else {
        d->m_currentItem = 0;
        showSelected();
    }
}

void QsMatchView::setTitle(const QString &title)
{
    d->m_titleLabel->setText(title);
}

void QsMatchView::setItemCount(int count)
{
    //TODO: place a context to aid translation
    d->m_itemCountLabel->setText(i18n("%1 %2", count, d->m_itemCountSuffix));
    if (count) {
        d->m_arrowButton->show();
    }
}

void QsMatchView::setItemCountSuffix(const QString &suffix)
{
    d->m_itemCountSuffix = suffix;
}

void QsMatchView::setDescriptionText(const QString &text)
{
    QColor color(Qt::white);
    setDescriptionText(text, color);
}

void QsMatchView::setDescriptionText(const QString &text, const QColor &color)
{
    if (d->m_descRect) {
        d->m_scene->removeItem(d->m_descRect);
        delete d->m_descRect;
        d->m_descRect = 0;
    }

    QColor bg(color);
    bg.setAlphaF(0.6);
    QBrush b(bg);

    QPen p(QColor(0, 0, 0, 0));
    d->m_descRect = new QGraphicsRectItem(-WIDTH/2, 60, WIDTH, 20);
    d->m_descRect->setBrush(b);
    d->m_descRect->setPen(p);

    QFontMetrics fm(font());

    // Show ellipsis in the middle to distinguish between strings with identical
    // beginnings e.g. paths
    d->m_descText = new QGraphicsTextItem(fm.elidedText(text, Qt::ElideMiddle, WIDTH), d->m_descRect);
    //Center text
    d->m_descText->setPos(-(d->m_descText->boundingRect().width()/2), 60);

    d->m_scene->addItem(d->m_descRect);
}

void QsMatchView::clearItems()
{
    if (!d->m_itemsRemoved) {
         foreach (MatchItem *item, d->m_items) {
            d->m_scene->removeItem(item);
         }
        d->m_itemsRemoved = true;
    }
}

void QsMatchView::clear(bool deleteItems)
{
    if (!deleteItems) {
        clearItems();
    } else {
        d->m_items.clear();
        d->m_itemsRemoved = false;
    }
    d->m_scene->clear();
    d->m_descRect = 0;
}

void QsMatchView::toggleView()
{
    //It might be better not to rely on m_arrowButton...
    //should make things more readable
    if (d->m_arrowButton->arrowType() == Qt::RightArrow) {
        showList();
    } else {
        showSelected();
    }
}

//TODO: Fix animation
void QsMatchView::showLoading()
{
    clear(true);

    d->m_descText = new QGraphicsTextItem(i18n("Loading..."), d->m_descRect);
    d->m_descText->setDefaultTextColor(QColor(Qt::white));
    QFontMetrics fm(d->m_descText->font());

    //Center text
    d->m_descText->setPos(-(d->m_descText->boundingRect().width()/2), (HEIGHT - fm.height())/2);
    d->m_scene->addItem(d->m_descText);
}

void QsMatchView::showList()
{
    if (d->m_items.size()) {
        clear();

        foreach (MatchItem *item, d->m_items) {
            d->m_scene->addItem(item);
        }

        d->m_itemsRemoved = false;
        d->m_arrowButton->setArrowType(Qt::DownArrow);

        //Restore highlighted icon
        focusItem(d->m_currentItem);
        //Popup the completion box - should make this configurable
        showPopup();
    }
    d->m_listVisible = true;
}

void QsMatchView::showSelected()
{
    if (!d->m_items.size()) {
        if (d->m_searchTerm.isEmpty()) {
            reset();
        }
        return;
    }

    MatchItem *it = d->m_items[d->m_currentItem];
    if (!it) {
        return;
    }

    d->m_listVisible = false;
    d->m_arrowButton->setArrowType(Qt::RightArrow);

    clear();

    d->m_stack->setCurrentIndex(0);

    QGraphicsPixmapItem *pixmap = new QGraphicsPixmapItem(it->icon().pixmap(64));
    pixmap->setPos(-WIDTH/2 + 5, LARGE_ICON_PADDING);

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QColor c = theme->color(Plasma::Theme::TextColor);

    QGraphicsTextItem *name = new QGraphicsTextItem();
    //TODO: Modify QFont instead of using setHtml?
    name->setHtml(QString("<b>%1</b>").arg(it->name()));
    name->setDefaultTextColor(c);
    QFontMetrics fm(name->font());

    int tm = ICON_AREA_HEIGHT/2 - fm.height();
    name->setPos(-115, tm);

    QGraphicsTextItem *desc = new QGraphicsTextItem(it->description());
    desc->setDefaultTextColor(c);
    desc->setPos(-115, ICON_AREA_HEIGHT/2);

    d->m_scene->addItem(name);
    d->m_scene->addItem(desc);
    d->m_scene->addItem(pixmap);

    emit selectionChanged(it);

    d->m_compBox->hide();
}

void QsMatchView::focusItem(int index)
{
    if (!d->m_items.size()) {
        if (d->m_searchTerm.isEmpty()) {
            reset();
        } else {
            setDescriptionText(i18n("No results found."));
        }
        emit selectionChanged(0);
        return;
    }
    if (index > -1 && index < d->m_items.size()) {
        MatchItem *it = d->m_items[index];
        d->m_scene->setFocusItem(it);
        QString description;
        if (it->description().isEmpty()) {
            description = it->name();
        } else {
            description = QString("%1 (%2)").arg(it->name()).arg(it->description());
        }
        setDescriptionText(description, it->backgroundColor());
        emit selectionChanged(it);
    }
}

void QsMatchView::selectItem(int index)
{
    Q_UNUSED(index)
    showSelected();
}

void QsMatchView::scrollLeft()
{
    if (d->m_currentItem > 0){
        --d->m_currentItem;
    } else {
        d->m_currentItem = d->m_items.size() - 1;
    }

    QTimeLine *t = new QTimeLine(150);
    foreach (MatchItem *item, d->m_items) {
        QGraphicsItemAnimation *anim = item->anim(true);
        int spacing = MatchItem::ITEM_SIZE/2;
        int y = SMALL_ICON_PADDING;
        int x = -spacing;
        int index = d->m_items.indexOf(item);
        if (index == d->m_currentItem) {
            anim->setScaleAt(1, 1, 1);
            y = LARGE_ICON_PADDING;
        } else {
            if ((!index && d->m_currentItem == d->m_items.size() - 1)
                || index == d->m_currentItem + 1) {
                x = item->pos().x() + spacing*2;
            } else {
                x = item->pos().x() + spacing;
            }
            anim->setScaleAt(0, 0.5, 0.5);
            anim->setScaleAt(1, 0.5, 0.5);
        }
        anim->setPosAt(1.0, QPointF(x, y));
        anim->setTimeLine(t);
    }
    t->start();
    focusItem(d->m_currentItem);
}

void QsMatchView::scrollRight()
{
    if (d->m_currentItem < d->m_items.size() - 1) {
        ++d->m_currentItem;
    } else {
        d->m_currentItem = 0;
    }

    QTimeLine *t = new QTimeLine(150);
    foreach (MatchItem *item, d->m_items) {
        QGraphicsItemAnimation *anim = item->anim(true);
        int spacing = MatchItem::ITEM_SIZE/2;
        int y = SMALL_ICON_PADDING;
        int x = -spacing;
        if (d->m_items.indexOf(item) == d->m_currentItem) {
            anim->setScaleAt(1, 1, 1);
            y = LARGE_ICON_PADDING;
        } else {
            anim->setScaleAt(0, 0.5, 0.5);
            anim->setScaleAt(1, 0.5, 0.5);
            x = item->pos().x() - spacing;
        }
        anim->setPosAt(1.0, QPointF(x, y));
        anim->setTimeLine(t);
    }
    t->start();
    focusItem(d->m_currentItem);
}

void QsMatchView::scrollToItem(int index)
{
    if (index < 0 || d->m_items.size() == 0) {
        return;
    }

    qreal shift = d->m_items[index]->pos().x();

    QTimeLine *t = new QTimeLine(150);
    foreach (MatchItem *item, d->m_items) {
        QGraphicsItemAnimation *anim = item->anim(true);
        qreal y = SMALL_ICON_PADDING;
        qreal x = -MatchItem::ITEM_SIZE/2;
        int ix = d->m_items.indexOf(item);
        if (ix == index) {
            anim->setScaleAt(1, 1, 1);
            y = LARGE_ICON_PADDING;
        } else {
            x = item->pos().x() - shift;
            if ((shift > 0 && ix < index && ix > d->m_currentItem)
                || (shift < 0 && !(ix <= d->m_currentItem && ix > index))) {
                x -= MatchItem::ITEM_SIZE/2;
            }
            anim->setScaleAt(0, 0.5, 0.5);
            anim->setScaleAt(1, 0.5, 0.5);
        }
        anim->setPosAt(1.0, QPointF(x, y));
        anim->setTimeLine(t);
    }
    t->start();
    d->m_currentItem = index;
    focusItem(index);
}

void QsMatchView::showPopup()
{
    if (d->m_hasFocus && d->m_items.size()) {
        //Prevent triggering of scroll to item
        disconnect(d->m_compBox, SIGNAL(currentRowChanged(int)), this, SLOT(scrollToItem(int)));
        d->m_compBox->popup();
        QListWidgetItem *item = d->m_compBox->item(d->m_currentItem);
        if (item) {
            d->m_compBox->scrollToItem(item, QAbstractItemView::PositionAtTop);
            d->m_compBox->setCurrentItem(item, QItemSelectionModel::SelectCurrent);
        }
        connect(d->m_compBox, SIGNAL(currentRowChanged(int)), this, SLOT(scrollToItem(int)));
    }
}

void QsMatchView::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    QTimer::singleShot(150, this, SLOT(showPopup()));
}

void QsMatchView::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    if (!d->m_hasFocus) {
        d->m_hasFocus = true;
        showList();
    }
}

void QsMatchView::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    if (hasFocus()) {
        return;
    }
    d->m_hasFocus = false;
    showSelected();
}

//TODO: Make it possible to disable text mode
void QsMatchView::keyPressEvent(QKeyEvent *e)
{
    //Do not handle non-alphanumeric events
    if (e->modifiers() & ~Qt::ShiftModifier) {
        QWidget::keyPressEvent(e);
        return;
    }

    switch (e->key()) {
        case Qt::Key_Period:
            //Switch to line edit
            d->m_stack->setCurrentIndex(1);
            d->m_lineEdit->setFocus();
            break;
        case Qt::Key_Backspace:
            //d->m_stack->setCurrentIndex(0);
            d->m_searchTerm.chop(1);
            setTitle(d->m_searchTerm);
            d->m_lineEdit->setText(d->m_searchTerm);
            return;
        case Qt::Key_Left:
            if (!d->m_listVisible) {
                showList();
            }
            scrollLeft();
            return;
        case Qt::Key_Right:
            if (!d->m_listVisible) {
                showList();
            }
            scrollRight();
            return;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            //Do not activate item if popup is open
            if (d->m_compBox->isVisible()) {
                d->m_compBox->hide();
            } else if (d->m_items.size() && d->m_currentItem > -1
                       &&  d->m_currentItem < d->m_items.size()) {
                    emit itemActivated(d->m_items[d->m_currentItem]);
            }
            d->m_selectionMade = true;
            showSelected();
            return;
        default:
            break;
    }

    //Don't add control characters to the search term
    foreach (QChar c, e->text()) {
        if (c.isPrint()) {
            if (d->m_stack->currentIndex() == 1) {
                d->m_searchTerm = d->m_lineEdit->text() + c;
            } else {
                d->m_searchTerm += c;
            }
            d->m_selectionMade = false;
        }
    }
    // If line edit has focus, not all keypresses make it to
    // the parent widget, hence the internal search term is
    // out of date
    if (!d->m_lineEdit->hasFocus()) {
        d->m_lineEdit->setText(d->m_searchTerm);
    }
    QWidget::keyPressEvent(e);
}

} // namespace QuickSand

#include "qs_matchview.moc"
