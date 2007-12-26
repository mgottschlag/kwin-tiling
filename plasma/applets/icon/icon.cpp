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

#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KMimeType>
#include <KPropertiesDialog>
#include <KRun>
#include <KSharedConfig>
#include <KUrl>
#include <KDesktopFile>

#include <plasma/theme.h>
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

IconApplet::IconApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0)
{
    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    m_icon = new Plasma::Icon(this);

    if (args.count() > 2) {
        setUrl(args.at(2).toString());
    }
}

void IconApplet::init()
{
    KConfigGroup cg = config();
    //setMinimumSize(QSize(48,68));
    
    connect(m_icon, SIGNAL(clicked()), this, SLOT(openUrl()));
    setUrl(cg.readEntry("Url", m_url));
    setDrawStandardBackground(false);
    setDisplayLines(2);
}

IconApplet::~IconApplet()
{
}

void IconApplet::saveState(KConfigGroup *cg) const
{
    cg->writeEntry("Url", m_url);
}

void IconApplet::setUrl(const KUrl& url)
{
    m_url = url;

    KMimeType::Ptr mime = KMimeType::findByUrl(url);
    m_mimetype = mime->name();

    if (m_url.isLocalFile() && KDesktopFile::isDesktopFile(m_url.toLocalFile())) {
        KDesktopFile *f= new KDesktopFile(m_url.toLocalFile());
        m_text = f->readName();
        m_icon->setIcon(f->readIcon());
    }else{
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
        KRun::runUrl(m_url, m_mimetype, 0);
    }
}

void IconApplet::constraintsUpdated(Plasma::Constraints constraints)
{
    setDrawStandardBackground(false);

    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            m_icon->setText(m_text);
            setMinimumContentSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
        } else {
            m_icon->setText(0);
            setMinimumContentSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Panel)));
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        setContentSize(size());
        m_icon->resize(size());
    }
}

void IconApplet::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KPropertiesDialog(m_url);
        connect(m_dialog, SIGNAL(applied()), this, SLOT(acceptedPropertiesDialog()));
        connect(m_dialog, SIGNAL(propertiesClosed()), this, SLOT(propertiesDialogClosed()));
    }

    m_dialog->show();
}

QSizeF IconApplet::sizeHint() const
{
    QSizeF iconSize = m_icon->sizeHint();
    int width = qBound(minimumSize().width(), iconSize.width(), maximumSize().width());
    int height = qBound(minimumSize().height(), iconSize.height(), maximumSize().height());
    return QSizeF(width,height);
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
    if (KUrl::List::canDecode(event->mimeData())) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());

        if (urls.count() > 0) {
            event->accept();
            setUrl(urls.first());
        }
    }
}

Qt::Orientations IconApplet::expandingDirections() const
{
    return 0;
}

#include "icon.moc"

