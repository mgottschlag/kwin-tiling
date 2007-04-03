#ifndef __VIEWER_H__
#define __VIEWER_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
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

#include <kapplication.h>
#include <kparts/part.h>
#include <kparts/mainwindow.h>

class QAction;

namespace KFI
{

class CViewer : public KParts::MainWindow
{
    Q_OBJECT

    public:

    CViewer(const KUrl &url);
    virtual ~CViewer();

    public Q_SLOTS:

    void fileOpen();
    void enableAction(const char *name, bool enable);

    private:

    KParts::ReadOnlyPart *itsPreview;
    QAction              *itsPrintAct;
};

}

#endif
