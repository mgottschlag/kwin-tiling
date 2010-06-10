/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans <l.appelhans@gmx.de>            *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "quicklaunchicon.h"

// Qt
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsSceneDragDropEvent>

// KDE
#include <KDesktopFile>
#include <KIcon>
#include <KMimeType>
#include <KRun>
#include <KUrl>

// Plasma
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

namespace Quicklaunch {

QuicklaunchIcon::QuicklaunchIcon(const KUrl &url, QGraphicsItem *parent)
  : Plasma::IconWidget(parent),
    m_appUrl(),
    m_appName(),
    m_appGenericName()
{
    if (!url.isEmpty()) {
        setUrl(url);
    } else {
        setIcon(KIcon("unknown"));
    }

    Plasma::ToolTipManager::self()->registerWidget(this);
    connect(this, SIGNAL(clicked()), SLOT(execute()));
    setOwnedByLayout(true);
}

QuicklaunchIcon::~QuicklaunchIcon()
{
    Plasma::ToolTipManager::self()->unregisterWidget(this);
}

void QuicklaunchIcon::clear()
{
    m_appUrl.clear();
    m_appName.clear();
    m_appGenericName.clear();
    setIcon(QIcon());
    setText(QString());
}

void QuicklaunchIcon::setUrl(const KUrl &url)
{
    // Takes care of improperly escaped characters and resolves paths
    // into file:/// URLs
    KUrl newUrl(url.url());

    if (newUrl == m_appUrl) {
        return;
    }

    m_appUrl = newUrl;

    KIcon icon;

    if (m_appUrl.isLocalFile() &&
        KDesktopFile::isDesktopFile(m_appUrl.toLocalFile())) {
        KDesktopFile f(m_appUrl.toLocalFile());

        icon = KIcon(f.readIcon());
        m_appName = f.readName();
        m_appGenericName = f.readGenericName();
    } else {
        icon = KIcon(KMimeType::iconNameForUrl(m_appUrl));
    }

    if (m_appName.isNull()) {
        m_appName = m_appUrl.fileName();
    }

    if (icon.isNull()) {
        icon = KIcon("unknown");
    }

    setIcon(icon);
}

KUrl QuicklaunchIcon::url() const
{
    return m_appUrl;
}

QString QuicklaunchIcon::appName() const
{
    return m_appName;
}

void QuicklaunchIcon::execute()
{
    new KRun(m_appUrl, 0);
}

void QuicklaunchIcon::toolTipAboutToShow()
{
  Plasma::ToolTipContent toolTipContent;
  toolTipContent.setMainText(m_appName);
  toolTipContent.setSubText(m_appGenericName);
  toolTipContent.setImage(icon());

  Plasma::ToolTipManager::self()->setContent(this, toolTipContent);
}

void QuicklaunchIcon::toolTipHidden()
{
    Plasma::ToolTipManager::self()->clearContent(this);
}
}
