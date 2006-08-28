/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <QToolTip>
#include <QFile>

#include <kdesktopfile.h>
#include <kfileitem.h>
#include <konq_operations.h>
#include <kicontheme.h>
#include <kglobal.h>
#include <kiconeffect.h>
#include <kpropertiesdialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <ktoolinvocation.h>
#include <klocale.h>
#include <kworkspace.h>

#include <kdebug.h>

#include "utils.h"
#include "kicker.h"

#include "urlbutton.h"
#include "urlbutton.moc"

URLButton::URLButton( const QString& url, QWidget* parent )
  : PanelButton( parent )
  , fileItem( 0 )
  , pDlg( 0 )
{
    initialize( url );
}

URLButton::URLButton( const KConfigGroup& config, QWidget* parent )
  : PanelButton( parent )
  , fileItem( 0 )
  , pDlg( 0 )
{
    initialize( config.readPathEntry("URL") );
}

URLButton::~URLButton()
{
    delete fileItem;
}

void URLButton::initialize( const QString& _url )
{
    setObjectName("URLButton");
    KUrl url(_url);
    if (!url.isLocalFile() || !url.path().endsWith(".desktop"))
    {
       QString file = Plasma::newDesktopFile(url);
       KDesktopFile df(file);
       df.writeEntry("Encoding", "UTF-8");
       df.writeEntry("Type","Link");
       df.writeEntry("Name", url.prettyUrl());
       if (url.isLocalFile())
       {
          KFileItem item( KFileItem::Unknown, KFileItem::Unknown, url );
          df.writeEntry("Icon", item.iconName() );
       }
       else
       {
          df.writeEntry("Icon", KMimeType::favIconForUrl(url));
       }
       df.writeEntry("URL", url.url());
       url = KUrl();
       url.setPath(file);
    }
    fileItem = new KFileItem( KFileItem::Unknown, KFileItem::Unknown, url );
    setIcon( fileItem->iconName() );
    connect( this, SIGNAL(clicked()), SLOT(slotExec()) );
    setToolTip();

    if (url.isLocalFile())
    {
        backedByFile(url.path());
    }
}

void URLButton::saveConfig( KConfigGroup& config ) const
{
    config.writePathEntry("URL", fileItem->url().prettyUrl());
}

void URLButton::setToolTip()
{
    if (fileItem->isLocalFile()
        && KDesktopFile::isDesktopFile(fileItem->url().path()))
    {
        KDesktopFile df(fileItem->url().path());

        if (df.readComment().isEmpty())
        {
            QWidget::setToolTip(df.readName());
        }
        else
        {
            QWidget::setToolTip(df.readName() + " - " + df.readComment());
        }

        setTitle(df.readName());
    }
    else
    {
        QWidget::setToolTip(fileItem->url().prettyUrl());
        setTitle(fileItem->url().prettyUrl());
    }
}

void URLButton::dragEnterEvent(QDragEnterEvent *ev)
{
    if ((ev->source() != this) &&
        fileItem->acceptsDrops() &&
        KUrl::List::canDecode(ev->mimeData()))
    {
        ev->accept(rect());
    }
    else
    {
        ev->ignore(rect());
    }
    PanelButton::dragEnterEvent(ev);
}

void URLButton::dropEvent(QDropEvent *ev)
{
    KWorkSpace::propagateSessionManager();
    KUrl::List execList = KUrl::List::fromMimeData(ev->mimeData());
    if (!execList.isEmpty())
    {
        KUrl url( fileItem->url() );
        if (KDesktopFile::isDesktopFile(url.path()))
        {
            KToolInvocation::startServiceByDesktopPath(url.path(), execList.toStringList(),
                    0, 0, 0, "", true);
        }
        else // attempt to interpret path as directory
        {
            KonqOperations::doDrop( fileItem, url, ev, this );
        }
    }
    PanelButton::dropEvent(ev);
}

void URLButton::startDrag()
{
    emit dragme(KUrl::List(fileItem->url()), labelIcon());
}

void URLButton::slotExec()
{
    KWorkSpace::propagateSessionManager();
    fileItem->run();
}

void URLButton::updateURL()
{
    if (pDlg->kurl() != fileItem->url()) {
	fileItem->setUrl(pDlg->kurl());
	setIcon(fileItem->iconName());
	setToolTip();
	emit requestSave();
    } else {
	setIcon(fileItem->iconName());
	setToolTip();
    }

    pDlg = 0L;
}

void URLButton::properties()
{
    if ( (fileItem->isLocalFile() && !QFile::exists(fileItem->url().path()) )
         || !fileItem->url().isValid())
    {
        KMessageBox::error( 0L, i18n("The file %1 does not exist",
	                             fileItem->url().prettyUrl()) );
        return;
    }

    pDlg = new KPropertiesDialog(fileItem, 0L, 0L, false, false); // will delete itself
    pDlg->setFileNameReadOnly(true);
    connect(pDlg, SIGNAL(applied()), SLOT(updateURL()));
    pDlg->show();
}
