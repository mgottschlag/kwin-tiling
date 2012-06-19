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

#include <QFile>
#include <QFileInfo>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QGraphicsLinearLayout>

#include <KDebug>
#include <KDesktopFile>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KLocale>
#include <KMenu>
#include <KPropertiesDialog>
#include <KRun>
#include <KSharedConfig>
#include <KShell>
#include <KSycoca>
#include <KUrl>
#include <KWindowSystem>
#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/NetAccess>
#include <kfileitem.h>

#include <Plasma/Theme>
#include <Plasma/IconWidget>
#include <Plasma/Containment>
#include <Plasma/ToolTipManager>

IconApplet::IconApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_icon(0),
      m_watcher(0),
      m_hasDesktopFile(false)
{
    setAcceptDrops(true);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(true);
    m_icon = new Plasma::IconWidget(this);

    if (!args.isEmpty()) {
        setUrl(args.value(0).toString());
    }

    resize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
    //kDebug() << "sized to:" << geometry();
}

void IconApplet::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addItem(m_icon);

    KConfigGroup cg = config();

    if (m_url.isValid()) {
        // we got this in via the ctor, e.g. as a result of a drop
        cg.writeEntry("Url", m_url);
        emit configNeedsSaving();
    } else {
        configChanged();
    }

    setDisplayLines(2);
    registerAsDragHandle(m_icon);
    setAspectRatioMode(Plasma::ConstrainedSquare);

    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
        this, SLOT(iconSizeChanged(int)));
}

IconApplet::~IconApplet()
{
    delete m_dialog.data();
    delete m_watcher;
}

void IconApplet::configChanged()
{
    KConfigGroup cg = config();
    setUrl(cg.readEntry("Url", m_url));
}

void IconApplet::saveState(KConfigGroup &cg) const
{
    cg.writeEntry("Url", m_url);

    Plasma::FormFactor f = formFactor();
    if (f == Plasma::Vertical || f == Plasma::Horizontal) {
        cg.readEntry("LastFreeSize", m_lastFreeSize);
    } else {
        cg.readEntry("LastFreeSize", size());
    }
}

void IconApplet::checkService(const QStringList &changedResources)
{
    if (changedResources.contains("apps")) {
        setUrl(m_url);
    }
}

void IconApplet::iconSizeChanged(int group)
{
    if (group == KIconLoader::Desktop || group == KIconLoader::Panel) {
        updateGeometry();
    }
}

void IconApplet::setUrl(const KUrl& url, bool fromConfigDialog)
{
    if (!fromConfigDialog) {
        delete m_dialog.data();
    }

    m_url = url;
    if (!m_url.protocol().isEmpty()) {
        m_url = KIO::NetAccess::mostLocalUrl(url, 0);
    }

    m_service = 0;
    disconnect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)),
               this, SLOT(checkService(QStringList)));

    m_hasDesktopFile = false;
    delete m_watcher;
    m_watcher = 0;

    // if local
    //   if not a directory and executable
    //     make desktop file
    //    desktop file
    if (m_url.isLocalFile()) {
        m_watcher = new KDirWatch;
        m_watcher->addFile(m_url.toLocalFile());
        connect(m_watcher, SIGNAL(deleted(QString)), this, SLOT(delayedDestroy()));

        QFileInfo fi(m_url.toLocalFile());
        KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, m_url.toLocalFile());

        if (fileItem.isDesktopFile()) {
            m_hasDesktopFile = true;
        } else if (!fi.isDir() && fi.isExecutable()) {
            const QString suggestedName = fi.baseName();
            const QString file = KService::newServicePath(false, suggestedName);
            KDesktopFile df(file);
            KConfigGroup desktopGroup = df.desktopGroup();
            desktopGroup.writeEntry("Name", suggestedName);
            QString entryType;
            desktopGroup.writeEntry("Exec", m_url.toLocalFile());
            desktopGroup.writeEntry("Icon", KMimeType::iconNameForUrl(url));
            desktopGroup.writeEntry("Type", "Application");
            df.sync();
            m_url.setPath(file);
            m_hasDesktopFile = true;
        }
    }

    if (m_hasDesktopFile) {
        KDesktopFile f(m_url.toLocalFile());
        m_text = f.readName();
        //corrupted desktop file?
        if (m_text.isNull()) {
            m_text = m_url.fileName();
        }
        m_icon->setIcon(f.readIcon());

        m_genericName = f.readGenericName();

        connect(m_watcher, SIGNAL(dirty(QString)), this, SLOT(updateDesktopFile()));
    } else {
        m_text = m_url.fileName();
        m_service = KService::serviceByStorageId(m_text);
        connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)),
                this, SLOT(checkService(QStringList)));

        if (m_service) {
            m_text = m_service->name();
            m_icon->setIcon(m_service->icon());
        } else {
            if (m_text.isEmpty() && m_url.isLocalFile()) {
                //handle special case like the / folder
                m_text = m_url.directory();
            } else if (m_url.protocol().contains("http")) {
                m_text = m_url.prettyUrl();
                m_text.remove(QRegExp("http://(www.)*"));
            } else if (m_text.isEmpty()) {
                m_text = m_url.prettyUrl();

                if (m_text.endsWith(QLatin1String(":/"))) {
                    m_text = m_url.protocol();
                }
            }

            m_icon->setIcon(KMimeType::iconNameForUrl(url));
        }
    }

    if (m_icon->icon().isNull()) {
        m_icon->setIcon("unknown");
    }

    //Update the icon text (if the icon is not on a panel)
    if (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
        m_icon->setText(m_text);
    } else {
        //Update the tooltip (if the icon is on a panel)
        Plasma::ToolTipContent data(m_text, m_genericName, m_icon->icon());
        Plasma::ToolTipManager::self()->setContent(m_icon, data);
    }

    //kDebug() << "url was" << url << "and is" << m_url;
}

void IconApplet::delayedDestroy()
{
    QTimer::singleShot(5000, this, SLOT(checkExistenceOfUrl()));
}

void IconApplet::checkExistenceOfUrl()
{
    if (m_url.isLocalFile() && !QFile::exists(m_url.toLocalFile())) {
        destroy();
    }
}

void IconApplet::updateDesktopFile()
{
    setUrl(m_url, true);
}

void IconApplet::openUrl()
{
    if (m_service) {
        emit releaseVisualFocus();
        KUrl::List urls;
        KRun::run(*m_service, urls, 0);
    } else if (m_url.isValid()) {
        emit releaseVisualFocus();
        new KRun(m_url, 0);
    }
}

void IconApplet::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);

    if (constraints & Plasma::FormFactorConstraint) {
        disconnect(m_icon, SIGNAL(activated()), this, SLOT(openUrl()));
        disconnect(m_icon, SIGNAL(clicked()), this, SLOT(openUrl()));

        if (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
            connect(m_icon, SIGNAL(activated()), this, SLOT(openUrl()));

            if (!m_lastFreeSize.isEmpty()) {
                resize(m_lastFreeSize);
            }

            m_icon->setText(m_text);
            Plasma::ToolTipManager::self()->unregisterWidget(m_icon);
            m_icon->setDrawBackground(true);
        } else {
            //in the panel the icon behaves like a button
            connect(m_icon, SIGNAL(clicked()), this, SLOT(openUrl()));
            m_icon->setText(QString());
            Plasma::ToolTipContent data(m_text, m_genericName, m_icon->icon());
            Plasma::ToolTipManager::self()->setContent(m_icon, data);
            m_icon->setDrawBackground(false);

            if (!m_lastFreeSize.isEmpty()) {
                config().writeEntry("LastFreeSize", size().toSize());
                emit configNeedsSaving();
            }
        }
    }

    if (constraints & Plasma::SizeConstraint && !m_lastFreeSize.isEmpty() &&
        (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter)) {
        m_lastFreeSize = size().toSize();
    }

    if (constraints & Plasma::StartupCompletedConstraint) {
        m_lastFreeSize = config().readEntry("LastFreeSize", size().toSize());
    }
}

void IconApplet::showConfigurationInterface()
{
    KPropertiesDialog *dialog = m_dialog.data();
    m_configTarget = m_url;
    if (m_hasDesktopFile) {
        const QFileInfo fi(m_url.toLocalFile());
        if (!fi.isWritable()) {
            const QString suggestedName = fi.baseName();
            m_configTarget = KService::newServicePath(false, suggestedName);
            KIO::Job *job = KIO::file_copy(m_url, m_configTarget);
            job->exec();
        }
    }

    if (dialog) {
        KWindowSystem::setOnDesktop(dialog->winId(), KWindowSystem::currentDesktop());
        dialog->show();
        KWindowSystem::activateWindow(dialog->winId());
    } else {
        dialog = new KPropertiesDialog(m_configTarget, 0 /*no parent widget*/);
        m_dialog = dialog;
        connect(dialog, SIGNAL(applied()), this, SLOT(acceptedPropertiesDialog()));
        connect(dialog, SIGNAL(canceled()), this, SLOT(cancelledPropertiesDialog()));
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->setWindowTitle(i18n("%1 Icon Settings", m_configTarget.fileName()));
        dialog->show();
    }
}

void IconApplet::acceptedPropertiesDialog()
{
    if (!m_dialog) {
        return;
    }

    m_url = m_dialog.data()->kurl();

    KConfigGroup cg = config();
    cg.writeEntry("Url", m_url);

    setUrl(m_url, true);
    update();
}

void IconApplet::cancelledPropertiesDialog()
{
    if (m_hasDesktopFile && m_configTarget != m_url) {
        // clean up after ourselves if we created a temporary file
        QFile::remove(m_configTarget.toLocalFile());
    }
}

QSizeF IconApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        int iconSize;

        switch (formFactor()) {
            case Plasma::Planar:
            case Plasma::MediaCenter:
                iconSize = IconSize(KIconLoader::Desktop);
                break;

            case Plasma::Horizontal:
            case Plasma::Vertical:
                iconSize = IconSize(KIconLoader::Panel);
                break;
        }

        return QSizeF(iconSize, iconSize);
    }

    return Plasma::Applet::sizeHint(which, constraint);
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

void IconApplet::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!KUrl::List::canDecode(event->mimeData())) {
        return;
    }

    KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
    if (urls.isEmpty()) {
        return;
    }

    event->accept();

    if (m_url.isEmpty()) {
        setUrl(urls.first());
        //TODO: why we don't call updateConstraints()?
        constraintsEvent(Plasma::FormFactorConstraint);
        return;
    } else if (m_service) {
        KRun::run(*m_service, urls, 0);
        return;
    }

    KMimeType::Ptr mimetype = KMimeType::findByUrl(m_url);

    if (m_url.isLocalFile() &&
        ((mimetype && (mimetype->is("application/x-executable") ||
                       mimetype->is("application/x-shellscript"))) ||
          KDesktopFile::isDesktopFile(m_url.toLocalFile()))) {

        if (KDesktopFile::isDesktopFile(m_url.toLocalFile())) {
            //Extract the command from the Desktop file
            KService service(m_url.toLocalFile());
            KRun::run(service, urls, 0);
            return;
        }

        // Just exec the local executable
        QString params;
        foreach (const KUrl &url, urls) {
            if (url.isLocalFile()) {
                params += ' ' + KShell::quoteArg(url.toLocalFile());
            } else {
                params += ' ' + KShell::quoteArg(url.prettyUrl());
            }
        }

        QString commandStr = KShell::quoteArg(m_url.path());
        KRun::runCommand(commandStr + ' ' + params, 0);
    } else if (mimetype && mimetype->is("inode/directory")) {
        dropUrls(urls, m_url, event->modifiers());
    }
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

