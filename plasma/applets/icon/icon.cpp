/***************************************************************************
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "icon.h"

#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QEvent>
#include <QMimeData>

#include <KGlobalSettings>
#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KPropertiesDialog>
#include <KRun>
#include <KSharedConfig>
#include <KUrl>
#include <KDesktopFile>
#include <KShell>
#include <KMenu>
#include <kio/copyjob.h>

#include <plasma/theme.h>
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

IconApplet::IconApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0),
      m_mimetype(0)
{
    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    setDrawStandardBackground(false);
    m_icon = new Plasma::Icon(this);
    //setMinimumSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
    resize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
    kDebug() << "sized to:" << geometry();

    if (args.count() > 2) {
        setUrl(args.at(2).toString());
    }
}

void IconApplet::init()
{
    KConfigGroup cg = config();

    if (!m_url.isValid()) {
        setUrl(cg.readEntry("Url", m_url));
    }
    setDisplayLines(2);

    watchForMouseMove(m_icon, true);

    // we do this right away since we may have our config
    // read shortly by the containment. usually applets don't need
    // this, but desktop icons requires some hacks.
    //
    // in particular, if we were created with a url passed into via
    // the args parameter in the ctor, then there won't be an entry
    // in our config, and desktop icons support banks on the fact
    // that there will be
    cg.writeEntry("Url", m_url);
}

IconApplet::~IconApplet()
{
    delete m_dialog;
}

void IconApplet::saveState(KConfigGroup *cg) const
{
    cg->writeEntry("Url", m_url);
}

void IconApplet::setUrl(const KUrl& url)
{
    m_url = url;

    m_mimetype = KMimeType::findByUrl(url);

    if (m_url.isLocalFile() && KDesktopFile::isDesktopFile(m_url.toLocalFile())) {
        KDesktopFile f(m_url.toLocalFile());
        m_text = f.readName();
        //corrupted desktop file?
        if (m_text.isNull()) {
            m_text = m_url.fileName();
        }
        m_icon->setIcon(f.readIcon());

        m_genericName = f.readGenericName();
    } else {
        m_text = m_url.fileName();
        m_icon->setIcon(KMimeType::iconNameForUrl(url));
    }

    if (m_icon->icon().isNull()) {
        m_icon->setIcon("unknown");
    }
}

void IconApplet::openUrl()
{
    if (m_url.isValid()) {
        if (containment()) {
            containment()->emitLaunchActivated();
        }
        KRun::runUrl(m_url, m_mimetype->name(), 0);
    }
}

void IconApplet::constraintsUpdated(Plasma::Constraints constraints)
{
    setDrawStandardBackground(false);

    if (constraints & Plasma::FormFactorConstraint) {
        disconnect(m_icon, SIGNAL(activated()), this, SLOT(openUrl()));
        disconnect(m_icon, SIGNAL(clicked()), this, SLOT(openUrl()));

        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            connect(m_icon, SIGNAL(activated()), this, SLOT(openUrl()));
            m_icon->setText(m_text);
            //FIXME TOOL TIP MANAGER
	    //m_icon->setToolTip(Plasma::ToolTipData());
            m_icon->setDrawBackground(true);
        } else {
            //in the panel the icon behaves like a button
            connect(m_icon, SIGNAL(clicked()), this, SLOT(openUrl()));
            m_icon->setText(QString());
            Plasma::ToolTipData data;
            data.mainText = m_text;
            data.subText = m_genericName;
            data.image = m_icon->icon().pixmap(IconSize(KIconLoader::Desktop));
            //FIXME TOOL TIP MANAGER
	    //m_icon->setToolTip(data);
            m_icon->setDrawBackground(false);
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        m_icon->resize(geometry().size());
    }
}

void IconApplet::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KPropertiesDialog(m_url, 0 /*no parent widget*/);
        connect(m_dialog, SIGNAL(applied()), this, SLOT(acceptedPropertiesDialog()));
        connect(m_dialog, SIGNAL(propertiesClosed()), this, SLOT(propertiesDialogClosed()));
    }

    m_dialog->show();
}

void IconApplet::setDisplayLines(int displayLines)
{
    if (m_icon) {
        if (m_icon->numDisplayLines() == displayLines) {
            return;
        }
        m_icon->setNumDisplayLines(displayLines);
        update();
    }
}
int IconApplet::displayLines()
{
    if (m_icon) {
        return m_icon->numDisplayLines();
    }
    return 0;
}

void IconApplet::acceptedPropertiesDialog()
{
    KConfigGroup cg = config();
    m_url = m_dialog->kurl();
    cg.writeEntry("Url", m_url);
    setUrl(m_url);
    update();
}

void IconApplet::propertiesDialogClosed()
{
    m_dialog = 0;
}

void IconApplet::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!KUrl::List::canDecode(event->mimeData())) {
        return;
    }

    KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());

    if (urls.count() > 0) {
        event->accept();
    } else {
        return;
    }


    if (m_url.isEmpty()) {
        setUrl(urls.first());
        constraintsUpdated(Plasma::FormFactorConstraint);
    } else if (m_url.isLocalFile() &&
              (m_mimetype &&
               m_mimetype->is("application/x-executable") ||
               m_mimetype->is("application/x-shellscript") ||
                KDesktopFile::isDesktopFile(m_url.toLocalFile()))) {

        //Parameters
        QString params;
        foreach (KUrl url, urls) {
            if (url.isLocalFile()) {
                params += ' ' + KShell::quoteArg(url.path());
            } else {
                params += ' ' + KShell::quoteArg(url.prettyUrl());
            }
        }

        //Command
        QString commandStr;
        //Extract the command from the Desktop file
        if (KDesktopFile::isDesktopFile(m_url.toLocalFile())) {
            KDesktopFile f(m_url.toLocalFile());
            KConfigGroup config = f.desktopGroup();
            commandStr = config.readPathEntry( "Exec", QString() );

            if (commandStr.isEmpty()) {
                QString path = f.readUrl();
                if (path.isEmpty()) {
                    path = f.readPath();
                }

                if (path.isEmpty()) {
                    return;
                }

                KUrl dest(path);
                KMimeType::Ptr mime = KMimeType::findByUrl(dest);
                if (m_mimetype->is("inode/directory")) {
                    dropUrls(urls, dest, event->modifiers());
                }
            }
        } else {
            //Else just exec the local executable
            commandStr = KShell::quoteArg(m_url.path());
        }

        KRun::runCommand(commandStr + ' ' + params, 0);
    } else if (m_mimetype->is("inode/directory")) {
        dropUrls(urls, m_url, event->modifiers());
    }
}

Qt::Orientations IconApplet::expandingDirections() const
{
    return 0;
}

QPainterPath IconApplet::shape() const
{
    return m_icon->shape();
}

//dropUrls from DolphinDropController
void IconApplet::dropUrls(const KUrl::List& urls,
                          const KUrl& destination,
                          Qt::KeyboardModifiers modifier)
{
    kDebug() << "Source" << urls;
    kDebug() << "Destination:" << destination;

    Qt::DropAction action = Qt::CopyAction;

    const bool shiftPressed   = modifier & Qt::ShiftModifier;
    const bool controlPressed = modifier & Qt::ControlModifier;
    const bool altPressed = modifier & Qt::AltModifier;
    if (shiftPressed && controlPressed) {
        // shortcut for 'Link Here' is used
        action = Qt::LinkAction;
    } else if (shiftPressed) {
        // shortcut for 'Move Here' is used
        action = Qt::MoveAction;
    } else if (controlPressed) {
        // shortcut for 'Copy Here' is used
        action = Qt::CopyAction;
    } else if (altPressed) {
        // shortcut for 'Link Here' is used
        action = Qt::LinkAction;
    } else {
        // open a context menu which offers the following actions:
        // - Move Here
        // - Copy Here
        // - Link Here
        // - Cancel

        KMenu popup(0);

        QString seq = QKeySequence(Qt::ShiftModifier).toString();
        seq.chop(1); // chop superfluous '+'
        QAction* moveAction = popup.addAction(KIcon("go-jump"),
                                              i18nc("@action:inmenu",
                                                    "&Move Here\t<shortcut>%1</shortcut>", seq));

        seq = QKeySequence(Qt::ControlModifier).toString();
        seq.chop(1);
        QAction* copyAction = popup.addAction(KIcon("edit-copy"),
                                              i18nc("@action:inmenu",
                                                    "&Copy Here\t<shortcut>%1</shortcut>", seq));

        seq = QKeySequence(Qt::ControlModifier + Qt::ShiftModifier).toString();
        seq.chop(1);
        QAction* linkAction = popup.addAction(KIcon("insert-link"),
                                              i18nc("@action:inmenu",
                                                    "&Link Here\t<shortcut>%1</shortcut>", seq));

        popup.addSeparator();
        popup.addAction(KIcon("process-stop"), i18nc("@action:inmenu", "Cancel"));

        QAction* activatedAction = popup.exec(QCursor::pos());
        if (activatedAction == moveAction) {
            action = Qt::MoveAction;
        } else if (activatedAction == copyAction) {
            action = Qt::CopyAction;
        } else if (activatedAction == linkAction) {
            action = Qt::LinkAction;
        } else {
            return;
        }
    }

    switch (action) {
    case Qt::MoveAction:
        KIO::move(urls, destination);
        break;

    case Qt::CopyAction:
        KIO::copy(urls, destination);
        break;

    case Qt::LinkAction:
        KIO::link(urls, destination);
        break;

    default:
        break;
    }
}

#include "icon.moc"

