/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "ui/searchbar.h"

// Qt
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>

// KDE
#include <KIcon>
#include <KLineEdit>
#include <KLocalizedString>

using namespace Kickoff;

class SearchBar::Private
{
public:
    Private() : editWidget(0),timer(0) {}

    KLineEdit *editWidget;
    QTimer *timer;
};

SearchBar::SearchBar(QWidget *parent)
 : QWidget(parent)
 , d(new Private)
{
    // timer for buffered updates
    d->timer = new QTimer(this);
    d->timer->setInterval(300);
    d->timer->setSingleShot(true);
    connect(d->timer,SIGNAL(timeout()),this,SLOT(updateTimerExpired()));
    connect(this,SIGNAL(startUpdateTimer()),d->timer,SLOT(start()));

    // setup UI
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(3);

    QLabel *searchLabel = new QLabel(i18n("Search:"),this);

    d->editWidget = new KLineEdit(this);
    d->editWidget->installEventFilter(this);
    d->editWidget->setClearButtonShown(true);
    connect(d->editWidget,SIGNAL(textChanged(QString)),this,SIGNAL(startUpdateTimer()));

    QLabel *searchIcon = new QLabel(this);
    searchIcon->setPixmap(KIcon("file-find").pixmap(32,32));

    layout->addWidget(searchLabel);
    layout->addWidget(d->editWidget);
    layout->addWidget(searchIcon);
    setLayout(layout);

    setFocusProxy(d->editWidget);
}
void SearchBar::updateTimerExpired()
{
    emit queryChanged(d->editWidget->text());
}
SearchBar::~SearchBar()
{
    delete d;
}
bool SearchBar::eventFilter(QObject *watched,QEvent *event)
{
    // left and right arrow key presses in the search edit which will
    // have no effect (because the cursor is already at the start or 
    // end of the text) are propagated up to the parent widget
    //
    // this allows views in the Launcher to use left and right arrows for
    // navigation whilst the search bar still has the focus
    if (watched == d->editWidget && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        if (keyEvent->key() == Qt::Key_Left && d->editWidget->cursorPosition()==0) {
            QCoreApplication::sendEvent(this,event);
            return true;

        } else if (keyEvent->key() == Qt::Key_Right &&
                   d->editWidget->cursorPosition() == d->editWidget->text().length()) {
            QCoreApplication::sendEvent(this,event);
            return true;
        }
    }
    return false; 
}

#include "searchbar.moc"
