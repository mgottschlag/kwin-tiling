/***************************************************************************
 *   Copyright 2007 by Marco Martin <notmart@gmail.com>                    *
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

#include "trash.h"

//QT
#include <QGraphicsSceneDragDropEvent>
#include <QDesktopWidget>
#include <QApplication>

//KDE
#include <KGlobalSettings>
#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KMimeType>
#include <KRun>
#include <KSharedConfig>
#include <KMessageBox>
#include <KUrl>
#include <KProcess>
#include <KStandardDirs>

#include <kfileplacesmodel.h>
#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>

//Plasma
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

//Solid
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/predicate.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>
#include <solid/opticaldisc.h>


Trash::Trash(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_icon(0),
      m_trashUrl(KUrl("trash:/")),
      m_count(0)
{
    setBackgroundHints(NoBackground);
}

Trash::~Trash()
{
}

void Trash::init()
{
    m_icon = new Plasma::Icon(KIcon("user-trash"),QString(),this);
    m_icon->setNumDisplayLines(2);

    m_places = new  KFilePlacesModel(this);

    createMenu();

    connect(m_icon, SIGNAL(activated()), this, SLOT(slotOpen()));

    connect(&m_menu, SIGNAL(aboutToHide()),
            m_icon, SLOT(setUnpressed()));

    setAcceptDrops(true);

    m_dirLister = new KDirLister();
    connect( m_dirLister, SIGNAL( clear() ),
             this, SLOT( slotClear() ) );
    connect( m_dirLister, SIGNAL( completed() ),
             this, SLOT( slotCompleted() ) );
    connect( m_dirLister, SIGNAL( deleteItem( const KFileItem & ) ),
             this, SLOT( slotDeleteItem( const KFileItem & ) ) );

    m_dirLister->openUrl(m_trashUrl);
    m_icon->setDrawBackground(true);
    registerAsDragHandle(m_icon);
    //setMinimumSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
    resize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
    //FIXME PORT TO TOOLTIP MANAGER
    //m_data.mainText = i18n("Trash");
}

void Trash::createMenu()
{
    QAction* open = new QAction(SmallIcon("document-open"),i18n("&Open"), this);
    actions.append(open);
    connect(open, SIGNAL(triggered(bool)), this , SLOT(slotOpen()));

    emptyTrash = new QAction(SmallIcon("trash-empty"),i18n("&Empty Trashcan"), this);
    actions.append(emptyTrash);
    connect(emptyTrash, SIGNAL(triggered(bool)), this , SLOT(slotEmpty()));

    m_menu.addTitle(i18n("Trash"));
    m_menu.addAction(open);
    m_menu.addAction(emptyTrash);

    //add the menu as an action icon
    QAction* menu = new QAction(SmallIcon("arrow-up-double"),i18n("&Menu"), this);
    connect(menu, SIGNAL(triggered(bool)), this , SLOT(popup()));
    m_icon->addAction(menu);
}

void Trash::popup()
{
    if (m_menu.isVisible()) {
        m_menu.hide();
        return;
    }
    m_menu.popup(popupPosition(m_menu.sizeHint()));
    m_icon->setPressed();
}

void Trash::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);

    if (constraints & Plasma::FormFactorConstraint) {
        disconnect(m_icon, SIGNAL(activated()), this, SLOT(slotOpen()));
        disconnect(m_icon, SIGNAL(clicked()), this, SLOT(slotOpen()));
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
           
            //in a panel the icon always behaves like a button
            connect(m_icon, SIGNAL(clicked()), this, SLOT(slotOpen()));

            m_icon->setText(0);
            m_icon->setInfoText(0);
            m_showText = false;
            m_icon->setDrawBackground(false);
            //Adding an arbitrary width to make room for a larger count of items
            setMinimumSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop))+=QSizeF(20,0));
        } else {
            m_icon->setText(0);
            m_icon->setInfoText(0);
            m_showText = false;
            m_icon->setDrawBackground(false);

            setMinimumSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
	}
        setIcon();
    }
    if (constraints & Plasma::SizeConstraint && m_icon) {
        resize(size());
        m_icon->resize(size());
    }

    updateGeometry();
}

void Trash::slotOpen()
{
    emit releaseVisualFocus();
    KRun::runUrl(m_trashUrl, "inode/directory", 0);
}

void Trash::slotEmpty()
{
    emit releaseVisualFocus();
    const QString text(i18nc("@info", "Do you really want to empty the trash? All items will be deleted."));
    const bool del = KMessageBox::warningContinueCancel(&m_menu,
                                                        text,
                                                        QString(),
                                                        KGuiItem(i18nc("@action:button", "Empty Trash"),
                                                                  KIcon("user-trash"))
                                                        ) == KMessageBox::Continue;
    if (del) {
         // We can't use KonqOperations here. To avoid duplicating its code (small, though),
        // we can simply call ktrash.
        //KonqOperations::emptyTrash(&m_menu);
        KProcess process;
        process << KStandardDirs::findExe("ktrash") << "--empty";
        process.execute();

    }
}

void Trash::setIcon()
{
    if (m_count > 0) {
        m_icon->setIcon(KIcon("user-trash-full"));
	//FIXME PORT TO TOOLTIP MANAGER
        //m_data.subText = i18np("One item", "%1 items", m_count);
        if (m_showText) {
            m_icon->setInfoText(i18np("One item", "%1 items", m_count));
        }
    } else {
        m_icon->setIcon(KIcon("user-trash"));
        //FIXME PORT TO TOOLTIP MANAGER
	//m_data.subText = i18nc("The trash is empty. This is not an action, but a state", "Empty");
        if (m_showText){
            m_icon->setInfoText(i18nc("The trash is empty. This is not an action, but a state", "Empty"));
        }
    }

    m_icon->update();
    
    //FIXME TOOLTIP MANAGER
    /*m_data.image = m_icon->icon().pixmap(IconSize(KIconLoader::Desktop));

    if (!m_showText) {
        m_icon->setToolTip(m_data);
    } else {
        m_icon->setToolTip(Plasma::ToolTipData());
    }*/

    emptyTrash->setEnabled(m_count>0);
}

void Trash::slotClear()
{
    m_count = 0;
    setIcon();
}

void Trash::slotCompleted()
{
    m_count = m_dirLister->items(KDirLister::AllItems).count();
    setIcon();
}

void Trash::slotDeleteItem(const KFileItem &)
{
    m_count--;
    setIcon();
}

QList<QAction*> Trash::contextualActions()
{
    return actions;
}

void Trash::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (KUrl::List::canDecode(event->mimeData())) {
        const KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
        if (urls.count() > 0) {
            event->accept();

            //some special operation was done instead of simply deleting a file
            bool specialOperation = false;

             foreach (const KUrl& url, urls) {
                const Solid::Predicate predicate(Solid::DeviceInterface::StorageAccess, "filePath", url.path());

                //query for mounted devices
                const QList<Solid::Device> devList = Solid::Device::listFromQuery(predicate, QString());


                //seek for an item in the places (e.g. Dolphin sidebar)
                const QModelIndex index = m_places->closestItem(url);

                if (devList.count() > 0) {
                    //Assuming a mountpoint has a single device
                    Solid::Device device = devList.first();

                    if (device.is<Solid::OpticalDisc>()) {
                        device.parent().as<Solid::OpticalDrive>()->eject();
                    } else {
                        device.as<Solid::StorageAccess>()->teardown();
                    }

                    specialOperation = true;
                //hide if there is exactly that item in the places model
                } else if (m_places->bookmarkForIndex(index).url() == url) {
                    m_places->removePlace(index);
                    specialOperation = true;
                }
            }

            //finally, try to trash a file
             if (!specialOperation) {
                KIO::Job* job = KIO::trash(urls);
                job->ui()->setWindow(0);
                job->ui()->setAutoErrorHandlingEnabled(true);
            }

        }
    }

}

#include "trash.moc"

