#ifndef __INSTALLER_H__
#define __INSTALLER_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
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

#include <QObject>
#include <kio/job.h>

class QStringList;
class QWidget;
class KTempDir;
class KJob;

namespace KFI
{
class CUpdateDialog;

class CInstaller : public QObject
{
    Q_OBJECT

    public:

    enum EReturn
    {
        ROOT_INSTALL,
        USER_CANCELLED,
        NO_FONTS,
        INSTALLING
    };

    CInstaller(const char *a, int xid, QWidget *p)
         : itsXid(xid), itsParent(p), itsTempDir(NULL), itsUpdateDialog(NULL), itsApp(a) { }
    ~CInstaller();

    EReturn install(const QStringList &fonts);
    void    error();

    public Q_SLOTS:

    void    fontsInstalled(KJob *job);
    void    systemConfigured(KJob *job);

    private:

    int           itsXid;
    QWidget       *itsParent;
    KTempDir      *itsTempDir;
    CUpdateDialog *itsUpdateDialog;
    const char    *itsApp;
};

}

#endif
