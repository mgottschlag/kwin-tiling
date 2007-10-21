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

#ifndef SYSTEMMODEL_H
#define SYSTEMMODEL_H

// Qt
#include <QStandardItemModel>

namespace Kickoff
{

/** 
 * Model which provides a tree of items for important system setup tools (eg. System Settings) , 
 * folders (eg. the user's home folder and the local network) and fixed and removable storage.
 */
class SystemModel : public QStandardItemModel
{
Q_OBJECT

public:
    /** Constructs a new SystemModel with the specified parent. */
    SystemModel(QObject *parent = 0);
    virtual ~SystemModel();

private Q_SLOTS:
    void deviceRemoved(const QString& udi);
    void deviceAdded(const QString& udi);
    void freeSpaceInfoAvailable(const QString& mountPoint,quint64 kbSize,quint64 kbUsed,quint64 kbAvailable);
private:
    class Private;
    Private * const d;
};

}

#endif // SYSTEMMODEL_H

