////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CMetaDialog
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 19/06/2002
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
// (C) Craig Drummond, 2002
////////////////////////////////////////////////////////////////////////////////

#include "MetaDialog.h"
#include <qlistview.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <kurl.h>
#include <klocale.h>
#include <qlayout.h>
#include "Misc.h"

CMetaDialog::CMetaDialog(QWidget *parent)
           : KDialog(parent, NULL, true)
{
    QGridLayout *layout=new QGridLayout(this, 2, 2, 11, 6);
    QPushButton *btn=new QPushButton(i18n("Close"), this);

    itsList=new QListView(this);
    itsList->addColumn(i18n("File"));
    connect(btn, SIGNAL(clicked()), this, SLOT(close()));
    setCaption(i18n("Meta Data"));
    layout->addMultiCellWidget(itsList, 0, 0, 0, 1);
    layout->addItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 0);
    layout->addWidget(btn, 1, 1);
    resize(600, 300);
}

void CMetaDialog::showFiles(const QStringList &files)
{
    itsList->clear();

    KURL::List urls;
    int        c;

    for(c=0; c<itsList->columns(); ++c)
        itsList->setColumnWidth(c, 10);

    QStringList::ConstIterator it;

    for(it=files.begin(); it!=files.end(); ++it)
    {
        KURL    url;
        QString fName(CMisc::getFile(*it));

        url.setPath(CMisc::getDir(*it));
        url.setFileName(fName);
        urls.append(url);
    }

    itsJob=KIO::fileMetaInfo(urls);
    connect(itsJob, SIGNAL(gotMetaInfo(const KFileItem *)), this, SLOT(gotMetaInfo(const KFileItem *)));
    show();
}

void CMetaDialog::gotMetaInfo(const KFileItem *item)
{
    KFileMetaInfo         meta=item->metaInfo();
    QStringList           keys=meta.preferredKeys();
    QStringList::Iterator it;
    QListViewItem         *li=new QListViewItem(itsList, CMisc::getFile(item->text()));

    for(it=keys.begin(); it!=keys.end(); ++it)
    {
        int               i;
        KFileMetaInfoItem mi=meta.item(*it);
        QString           tk=mi.translatedKey();

        for(i=0; i<itsList->columns(); ++i)
            if(itsList->columnText(i)==tk)
                break;

        if(i>=itsList->columns())
            itsList->addColumn(tk);

        li->setText(i, mi.value().toString());
    }
}

#include "MetaDialog.moc"
