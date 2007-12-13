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

#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

IconApplet::IconApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0)
{
    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    //new Plasma::HBoxLayout(this);
    m_icon = new Plasma::Icon(this);
    setMinimumSize(QSize(48,78));
    if (args.count() > 2) {
        setUrl(args.at(2).toString());
    }
}

void IconApplet::init()
{
    KConfigGroup cg = config();

    connect(m_icon, SIGNAL(clicked()), this, SLOT(openUrl()));
    setUrl(cg.readEntry("Url", m_url));
    setDrawStandardBackground(false);
    kDebug() << "size hint is" << sizeHint() << size();
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
        } else {
            m_icon->setText(0);
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        m_icon->resize(size());
    }
}

void IconApplet::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KPropertiesDialog(m_url);
        connect(m_dialog, SIGNAL(applied()), this, SLOT(acceptedPropertiesDialog()));
    }

    m_dialog->show();
}

void IconApplet::acceptedPropertiesDialog()
{
    KConfigGroup cg = config();
    m_url = m_dialog->kurl();
    cg.writeEntry("Url", m_url);
    setUrl(m_url);
    update();
    m_dialog->deleteLater();
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


#include "icon.moc"

