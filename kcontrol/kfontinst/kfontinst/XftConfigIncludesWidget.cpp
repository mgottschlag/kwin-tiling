////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfigIncludesWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 04/07/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "XftConfigIncludesWidget.h"
#include <qpushbutton.h>

#ifdef HAVE_XFT
#include "Misc.h"
#include "KfiGlobal.h"
#include "Config.h"
#include <kurlrequesterdlg.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qstring.h>

void CXftConfigIncludesWidget::setList(const QStringList &files)
{
    itsWritable=CMisc::fWritable(CKfiGlobal::cfg().getXftConfigFile());
    itsList->clear();
    itsList->insertStringList(files);
    itsEditButton->setEnabled(false);
    itsRemoveButton->setEnabled(false);
    itsAddButton->setEnabled(itsWritable);
}

void CXftConfigIncludesWidget::addPressed()
{
    QString f=getFile(QString::null, true);

    if(QString::null!=f)
    {
        itsList->insertItem(f);
        emit changed();
    }
}

void CXftConfigIncludesWidget::editPressed()
{
    if(-1!=itsList->currentItem())
    {
        QString f=getFile(itsList->currentText());

        if(QString::null!=f && f!=itsList->currentText())
        {
            itsList->changeItem(f, itsList->currentItem());
            emit changed();
        }
    }
}

void CXftConfigIncludesWidget::removePressed()
{
    if(-1!=itsList->currentItem() && KMessageBox::questionYesNo(this, i18n("Remove selected include"), i18n("Remove?"))==KMessageBox::Yes)
    {
        itsList->removeItem(itsList->currentItem());
        itsRemoveButton->setEnabled(false);
        itsEditButton->setEnabled(false);
        emit changed();
    }
}

void CXftConfigIncludesWidget::itemSelected(QListBoxItem *item)
{
    if(item && itsWritable)
    {
        itsRemoveButton->setEnabled(true);
        itsEditButton->setEnabled(true);
    }
}

QString CXftConfigIncludesWidget::getFile(const QString &current, bool checkDuplicates)
{
    KURL url=KURLRequesterDlg::getURL(current, this, i18n("Xft Configuration File"));
 
    if("file"==url.protocol())
        if(QString::null!=url.path())
        {
            QString file=url.path();
 
            if(file.contains('\"'))
                KMessageBox::error(this, i18n("Entry cannot contain quotes!"), i18n("Error"));
            else if(CMisc::dExists(file))
                KMessageBox::error(this, i18n("Entry is a directory!"), i18n("Error"));
            else if(checkDuplicates && NULL!=itsList->findItem(file))
                KMessageBox::error(this, i18n("Entry already exists!"), i18n("Error"));
            else
                return file;
        }

    return QString::null;
}

QStringList CXftConfigIncludesWidget::getList()
{
    QStringList  list;
    unsigned int item;

    for(item=0; item<itsList->count(); ++item)
        list.append(itsList->text(item));

    return list;
}
#include "XftConfigIncludesWidget.moc"

#endif
