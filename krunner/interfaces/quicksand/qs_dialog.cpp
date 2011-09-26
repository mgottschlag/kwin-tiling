/*
 *   Copyright (C) 2006 by Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007-2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright (C) 2008 by Davide Bettio <davide.bettio@kdemail.net>
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

#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QTimer>
#include <QShortcut>
#include <KAction>
#include <KActionCollection>
#include <KStandardGuiItem>
#include <KWindowSystem>

//#include <Plasma/QueryAction>
#include <Plasma/QueryMatch>
#include <Plasma/RunnerManager>
#include <Plasma/Svg>
#include <Plasma/Theme>

#include "toolbutton.h"

#include "krunnerapp.h"
#include "qs_dialog.h"
#include "qs_matchview.h"
#include "qs_querymatchitem.h"
#include "qs_queryactionitem.h"

QsDialog::QsDialog(Plasma::RunnerManager *runnerManager, QWidget *parent)
    : KRunnerDialog(runnerManager, parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(0);

    m_configButton = new ToolButton(this);
    m_configButton->setText(i18n("Settings"));
    m_configButton->setToolTip(i18n("Settings"));
    m_configButton->setIcon(m_iconSvg->pixmap(QLatin1String( "configure" )));
    connect(m_configButton, SIGNAL(clicked()), SLOT(toggleConfigDialog()));

    m_activityButton = new ToolButton(this);
    KRunnerApp *krunnerApp = KRunnerApp::self();
    QAction *showSystemActivityAction = krunnerApp->actionCollection()->action(QLatin1String( "Show System Activity" ));
    m_activityButton->setDefaultAction(showSystemActivityAction);
    m_activityButton->setIcon(m_iconSvg->pixmap(QLatin1String( "status" )));

    updateSystemActivityToolTip();
    connect(showSystemActivityAction, SIGNAL(globalShortcutChanged(QKeySequence)), this, SLOT(updateSystemActivityToolTip()));
    connect(showSystemActivityAction, SIGNAL(triggered(bool)), this, SLOT(close()));

    m_singleRunnerIcon = new QLabel(this);

    QLabel *label = new QLabel(this);
    label->setText(QLatin1String( "<b>QuickSand</b>" ));

    const QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette titlePallete =  label->palette();
    titlePallete.setColor(QPalette::WindowText, textColor);
    label->setPalette(titlePallete);

    QToolButton *m_closeButton = new ToolButton(this);
    KGuiItem guiItem = KStandardGuiItem::close();
    m_closeButton->setText(guiItem.text());
    m_closeButton->setToolTip(guiItem.text().remove(QLatin1Char( '&' )));
    m_closeButton->setIcon(m_iconSvg->pixmap(QLatin1String( "close" )));
    connect(m_closeButton, SIGNAL(clicked(bool)), this, SLOT(close()));

    hLayout->addWidget(m_configButton);
    hLayout->addWidget(m_activityButton);
    hLayout->addWidget(m_singleRunnerIcon);
    hLayout->addStretch();
    hLayout->addWidget(label);
    hLayout->addStretch();
    hLayout->addWidget(m_closeButton);
    layout->addLayout(hLayout);

    m_matchView = new QuickSand::QsMatchView(this);
    layout->addWidget(m_matchView);
    connect(m_matchView, SIGNAL(textChanged(QString)), m_matchView, SLOT(setTitle(QString)));

    m_currentMatch = 0;

    m_actionView = new QuickSand::QsMatchView(this);
    layout->addWidget(m_actionView);
    m_actionView->setTitle(i18n("Actions"));
    m_actionView->setCountingActions(true);
    m_actionView->hide();

    connect(m_runnerManager, SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)),
             this, SLOT(setMatches(QList<Plasma::QueryMatch>)));
    connect(m_matchView, SIGNAL(textChanged(QString)), this, SLOT(launchQuery(QString)));
    connect(m_matchView, SIGNAL(selectionChanged(MatchItem*)), this, SLOT(loadActions(MatchItem*)));
    connect(m_matchView, SIGNAL(itemActivated(MatchItem*)), this, SLOT(run(MatchItem*)));
    connect(m_actionView, SIGNAL(selectionChanged(MatchItem*)), this, SLOT(setAction(MatchItem*)));
    connect(m_actionView, SIGNAL(itemActivated(MatchItem*)), this, SLOT(run(MatchItem*)));

    m_matchView->setFocus();

    m_newQuery = true;

    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
}

QsDialog::~QsDialog()
{}

// FIXME: We still have no notion of history... Actually adaptive search should partly take care of this
void QsDialog::clearHistory()
{}

void QsDialog::setConfigWidget(QWidget *w)
{
    //Yet another code copy pasted from default interface, should we move this to a common place?
    const int screenId = qApp->desktop()->screenNumber(this); //Kephal::ScreenUtils::screenId(geometry().center());
    const int maxHeight = qApp->desktop()->availableGeometry(screenId).height(); //Kephal::Screens::self()->screen(screenId)->geometry().height();

    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    const int padding = top + bottom + m_activityButton->height();
    resize(width(), qMin(maxHeight, qMax(w->sizeHint().height() + padding, size().height())));

    QVBoxLayout *layout = static_cast<QVBoxLayout*>(this->layout());
    layout->addWidget(w);
    m_matchView->hide();

    connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));
}

void QsDialog::configWidgetDestroyed()
{
    QTimer::singleShot(0, this, SLOT(cleanupAfterConfigWidget()));
}

void QsDialog::cleanupAfterConfigWidget()
{
    m_matchView->show();
    m_matchView->setFocus();
    adjustSize();
}

void QsDialog::adjustInterface()
{
    if (m_runnerManager->singleModeRunner()) {
        m_singleRunnerIcon->setPixmap(m_runnerManager->singleModeRunner()->icon().pixmap( QSize( 22, 22 )) );
        m_singleRunnerIcon->show();
        m_configButton->hide();
    } else {
        m_singleRunnerIcon->hide();
        m_configButton->show();
    }
}

void QsDialog::display(const QString &term)
{
    KWindowSystem::setOnDesktop(winId(), KWindowSystem::currentDesktop());

    adjustInterface();
    m_matchView->reset();
    m_actionView->reset();
    m_actionView->hide();
    adjustSize();
    m_matchView->setFocus();

    int screen = 0;
    if (QApplication::desktop()->numScreens() > 1) {
        screen = QApplication::desktop()->screenNumber(QCursor::pos());
    }

    //positionOnScreen will call QWidget::show anyways so we don't need to call it here
    positionOnScreen();
    KWindowSystem::forceActiveWindow(winId());
    if (term.isEmpty() && !m_runnerManager->singleMode()) {
        m_matchView->setTitle(QString());
    } else {
        m_matchView->setTitle(term);
        launchQuery(term);
    }
}

void QsDialog::launchQuery(const QString &query)
{
    if (query.isEmpty() && !m_runnerManager->singleMode()) {
        m_matchView->reset();
    } else {
        m_matchView->showLoading();
    }

    m_runnerManager->launchQuery(query);
    m_newQuery = true;
}

void QsDialog::run(MatchItem *item)
{
    if (QuickSand::QueryMatchItem *match = qobject_cast<QuickSand::QueryMatchItem*>(item)) {
        m_runnerManager->run(match->match());
        close();
    } else if (qobject_cast<QuickSand::QueryActionItem*>(item)) {
        m_runnerManager->run(m_currentMatch->match());
        close();
    }
}

void QsDialog::loadActions(MatchItem *item)
{
    if (item == m_currentMatch) {
        return;
    }
    m_currentMatch = qobject_cast<QuickSand::QueryMatchItem*>(item);
    QList<MatchItem*> actions;
    if (m_currentMatch) {
        QList<QAction*> queryActions = m_runnerManager->actionsForMatch(m_currentMatch->match());
        foreach(QAction *action, queryActions) {
            MatchItem *m = new QuickSand::QueryActionItem(action);
            actions.append(m);
        }
    }
    if (actions.size()) {
        m_actionView->show();
    } else if (m_actionView->isVisible()) {
        m_actionView->hide();
    }
    adjustSize();
    m_actionView->setItems(actions, false);
}

void QsDialog::setMatches(const QList<Plasma::QueryMatch> &matches)
{
    QList<MatchItem*> items;
    QMultiMap<QString, Plasma::QueryMatch> temp;
    QMultiMap<QString, Plasma::QueryMatch>::iterator end = m_matches.end();
    foreach (const Plasma::QueryMatch &match, matches) {
        temp.insert(match.id(), match);
        // Do not create new MatchItems for existing matches when the query hasn't changed
        if (!m_newQuery && m_matches.find(match.id()) != end) {
            // kDebug() << "A match with id " << match.id() << " already exists." << endl;
            QList<Plasma::QueryMatch> duplicates = m_matches.values(match.id());
            bool exists = false;
            foreach (const Plasma::QueryMatch &m, duplicates) {
                // FIXME: Matching the displayed text isn't always reliable
                // maybe adding an operator== to QueryMatch would help
                if (m.text() == match.text()) {
                    exists = true;
                    break;
                }
            }

            if (exists) {
                continue;
            }
        }
        MatchItem *m = new QuickSand::QueryMatchItem(match);

//TODO: I'm leaving the switch because this feature has sence, but we have to find another way to implement it
/*        switch(match.type())
        {
        case Plasma::QueryMatch::ExactMatch:
            m->setBackgroundColor(QColor(Qt::yellow));
            break;
        case Plasma::QueryMatch::InformationalMatch:
        case Plasma::QueryMatch::HelperMatch:
            m->setBackgroundColor(QColor(Qt::blue));
            break;
        default:
            m->setBackgroundColor(QColor(Qt::green));
            break;
        }*/

        items.append(m);
    }
    // kDebug() << "Add " << items.size() << " matches. Append?" << !m_newQuery << endl;
    m_matchView->setItems(items, true, !m_newQuery);
    m_matches = temp;
    // If new matches are obtained for the same query, append them to the list
    m_newQuery = false;
}

void QsDialog::setAction(MatchItem *item)
{
    if (QuickSand::QueryActionItem *action = qobject_cast<QuickSand::QueryActionItem*>(item)) {
        m_currentMatch->match().setSelectedAction(action->action());
    }
}

void QsDialog::updateSystemActivityToolTip()
{
    /* Set the tooltip for the Show System Activity button to include the global shortcut */
    KRunnerApp *krunnerApp = KRunnerApp::self();
    KAction *showSystemActivityAction = dynamic_cast<KAction *>(krunnerApp->actionCollection()->action(QLatin1String( "Show System Activity" )));
    if (showSystemActivityAction) {
        QString shortcut = showSystemActivityAction->globalShortcut().toString();
        if (shortcut.isEmpty()) {
            m_activityButton->setToolTip( showSystemActivityAction->toolTip() );
        } else {
            m_activityButton->setToolTip( i18nc("tooltip, shortcut", "%1 (%2)", showSystemActivityAction->toolTip(), shortcut));
        }
    }
}
#include "qs_dialog.moc"
