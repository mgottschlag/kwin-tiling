/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QSvgRenderer>
#include <QVBoxLayout>
#include <QShortcut>

#include <kdebug.h>
#include <KDialog>
#include <KLineEdit>
#include <KWin>

#include "../plasma/lib/theme.h"

#include "shellrunner.h"
#include "apprunner.h"
#include "searchrunner.h"
#include "interface.h"
#include "interfaceadaptor.h"
#include <QApplication>

Interface::Interface(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_haveCompositionManager(false),
      m_bgRenderer(0),
      m_renderDirty(true),
      m_currentRunner(0)
{
    // Make newInstance activate the window
    qApp->setActiveWindow( this );

    m_theme = new Plasma::Theme(this);
    themeChanged();
    connect(m_theme, SIGNAL(changed()), this, SLOT(themeChanged()));

    loadRunners();

    QVBoxLayout* layout = new QVBoxLayout(this);

    m_searchTerm = new KLineEdit(this);
    layout->addWidget(m_searchTerm);
    connect(m_searchTerm, SIGNAL(textChanged(QString)),
            this, SLOT(runText(QString)));
    connect(m_searchTerm, SIGNAL(returnPressed()),
            this, SLOT(exec()));

    m_matches = new QWidget(this);
    layout->addWidget(m_matches);

    m_optionsLabel = new QLabel(this);
    m_optionsLabel->setText("Options");
    m_optionsLabel->setEnabled(false);
    layout->addWidget(m_optionsLabel);

    m_compositeWatcher = new KSelectionWatcher("_NET_WM_CM_S0");
    m_haveCompositionManager = m_compositeWatcher->owner() != None;
    kDebug() << "checkForCompositionManager " << m_compositeWatcher->owner() << " != " << None << endl;
    kDebug() << "m_haveCompositionManager: " << m_haveCompositionManager << endl;
    Display *dpy = XOpenDisplay(0); // open default display
    m_haveCompositionManager = !XGetSelectionOwner(dpy,
                                               XInternAtom(dpy,
                                                           "_NET_WM_CM_S0",
                                                           false)
                                               );
    connect(m_compositeWatcher, SIGNAL(newOwner(Window)),
            this, SLOT(checkForCompositionManager(Window)));

    new InterfaceAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/Interface", this);

    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(hide()));

    resize(400, 250); //FIXME
}

Interface::~Interface()
{
}

void Interface::display()
{
    kDebug() << "display() called" << endl;
    show();
    raise();
    KWin::setOnDesktop(winId(), KWin::currentDesktop());
    KDialog::centerOnScreen(this);
}

void Interface::runText(const QString& term)
{
    kDebug() << "looking for a runner for: " << term << endl;
    if (m_currentRunner)
    {
        if (m_currentRunner->accepts(term))
        {
            kDebug() << "\tgoing with the same runner: " << m_currentRunner->name() << endl;
            return;
        }

        m_currentRunner->disconnect(this);
        m_currentRunner = 0;
    }

    foreach (Runner* runner, m_runners)
    {
        if (runner->accepts(term))
        {
            m_currentRunner = runner;
            m_optionsLabel->setEnabled(runner->hasOptions());
            connect(runner, SIGNAL(matches()), this, SLOT(updateMatches()));
            kDebug() << "\tswitching runners: " << m_currentRunner->name() << endl;
            return;
        }

        kDebug() << "\trunner passed: " << runner->name() << endl;
    }

    m_optionsLabel->setEnabled(false);
}

void Interface::checkForCompositionManager(Window owner)
{
kDebug() << "checkForCompositionManager " << owner << " " << None << endl;
    m_haveCompositionManager = (owner != None);
}

void Interface::themeChanged()
{
    delete m_bgRenderer;
    kDebug() << "themeChanged() to " << m_theme->themeName()
             << "and we have " << m_theme->imagePath("/background/dialog") << endl;
    m_bgRenderer = new QSvgRenderer(m_theme->imagePath("/background/dialog"), this);
}

void Interface::updateMatches()
{
    //TODO: implement
}

void Interface::exec()
{
    if (!m_currentRunner)
    {
        //TODO: give them some feedback
        return;
    }

    if (m_currentRunner->exec(m_searchTerm->text()))
    {
        hide();
    }
}

void Interface::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());

    if (m_haveCompositionManager)
    {
        //kDebug() << "gots us a compmgr!" << m_haveCompositionManager << endl;
        p.save();
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
        p.restore();
    }

    if (m_renderDirty)
    {
        m_renderedSvg.fill(Qt::transparent);
        QPainter p(&m_renderedSvg);
        p.setRenderHint(QPainter::Antialiasing);
        m_bgRenderer->render(&p);
        p.end();
        m_renderDirty = false;
    }

    p.drawPixmap(0, 0, m_renderedSvg);
}

void Interface::resizeEvent(QResizeEvent *e)
{
    if (e->size() != m_renderedSvg.size())
    {
        m_renderedSvg = QPixmap(e->size());
        m_renderDirty = true;
        int w = e->size().width();
        int h = e->size().height();
    }
}

void Interface::loadRunners()
{
    // ha! ha! get it? _load_ _runner_?! oh, i kill me.
    // but seriously, that game was the shiznit back in the day

    foreach (Runner* runner, m_runners)
    {
        delete runner;
    }
    m_runners.clear();
    m_currentRunner = 0;

    m_runners.append(new ShellRunner(this));
    m_runners.append(new AppRunner(this));
    m_runners.append(new SearchRunner(this));
}

#include "interface.moc"
