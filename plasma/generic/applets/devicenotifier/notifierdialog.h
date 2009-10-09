/***************************************************************************
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>            *
 *   Copyright 2009 by Jacopo De Simoi <wilderkde@gmail.com>               *
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

#ifndef NOTIFIERDIALOG_H
#define NOTIFIERDIALOG_H

//Qt
#include <QObject>
#include <QTimer>

//Plasma
#include <Plasma/Label>
#include <Plasma/IconWidget>

//solid
#include <solid/solidnamespace.h>

#include "deviceitem.h"

class QGraphicsLinearLayout;
class DeviceNotifier;

//desktop view
namespace Plasma
{
    class ItemBackground;
    class ScrollWidget;
}

namespace Solid
{
    class Device;
}

namespace Notifier
{
  /**
  * @short The panel used to display devices in a popup
  *
  */
  class NotifierDialog : public QObject
  {
  Q_OBJECT

      public:
          ///Specific role for the data-model
          enum SpecificRoles {
              SolidUdiRole = Qt::UserRole + 1,
              ActionRole = Qt::UserRole + 2,
              IconNameRole = Qt::UserRole + 3,
              DescriptionRole = Qt::UserRole + 4,
              VisibilityRole = Qt::UserRole + 5,
              IsBusy = Qt::UserRole + 6
          };

          /**
          * Constructor of the dialog
          * @param notifier the notifier attached to this dialog
          * @param parent the parent of this object
          **/
          explicit NotifierDialog(DeviceNotifier *notifier, QObject *parent = 0);

          /**
          * Default destructor
          **/
          virtual ~NotifierDialog();

          /**
          * Returns the related QGraphicsWidget.
          **/
          QGraphicsWidget *dialog();

          /**
          * Collapse all the devices
          **/
          void collapseDevices();

          /**
          * insert an action child of a device
          * @param udi the udi of the device
          * @param action the predicate file name of the action
          **/
          void insertAction(const QString &udi, const QString &action);

          /**
          * insert a device in the layout of the dialog
          * @param udi the udi of the device
          **/
          void insertDevice(const QString &udi);

          /**
          * Get the DeviceItem with a specified udi
          * @param udi the udi of the device
          * @return a pointer to the DeviceItem
          **/
          DeviceItem *itemForUdi(const QString &udi) const;

          /**
          * Allow to get a data displayed by the view
          * @param udi the udi of the device
          * @param role the role where is the data
          * @return the data
          **/
          QVariant getDeviceData(const QString &udi, int role);

          /**
          * Allow to set data which will be displayed by the view
          * @param udi the udi of the device
          * @param data the data
          * @param role the role in the data-model
          **/
          void setDeviceData(const QString &udi, QVariant data, int role);

          /**
          * set if a device is mounted
          * @param mounted true if it is mounted
          * @param udi the udi of the device
          **/
          void setMounted(bool mounted, const QString &udi);

          /**
          * Set the left action for a device
          * @param udi the udi of the device
          * @param action the action
          **/
          void setDeviceLeftAction(const QString &udi, DeviceItem::LeftActions action);

          /**
          * Remove a device in the dialog
          * @param udi the udi of the device
          **/
          void removeDevice(const QString &udi);

          /**
          * Gets a list of the action to be shown in the context menu
          * @return the list
          **/
          QList<QAction *> contextualActions();

          /**
          * Sets the visibility for the context menu actions
          * @param scenePos the position of the moude pointer in scene coordinates
          **/
          void setMenuActionsAt(QPointF scenePos);

      signals:
          /**
          * Emitted when a devices has been selected
          **/
          void deviceSelected();

          /**
          * Emitted when an action has been selected
          **/
          void actionSelected();

          /**
          * Emitted when changes the global visibility
          * @param visibility the global visibility
          **/
          void globalVisibilityChanged(bool visibility);

      protected:
          /**
          * Reimplemented from QObject
          **/
          bool eventFilter(QObject *obj, QEvent *event);

      private slots:
          /**
          * @internal slot used to reset the hover item background with a small delay,
          * allowing for the mouse to move between items smoothly
          */
          void clearItemBackgroundTarget();

          /**
          * @internal slot called when the selected item background has reached its target
          */
          void selectedItemAnimationComplete(QGraphicsItem *);

          /**
          * @internal slot called when user has clicked on the left button on a device
          * @param item the DeviceItem which has the button clicked
          **/
          void leftActionActivated(DeviceItem *item);

          /**
          * @internal slot called when user has clicked on a device in the dialog
          * @param item the DeviceItem which is clicked
          **/
          void deviceActivated(DeviceItem *item);

          /**
          * @internal slot called when the user highlights an action in a device; the
          * item background is then moved to it
          * @param the QGraphicsItem representing the action
          */
          void highlightDeviceAction(QGraphicsItem* item);

          /**
          * @internal slot called when user hased click on an action of a device in the dialog
          * @param udi the udi of the device
          * @param action the predicate file of the action
          **/
          void actionActivated(DeviceItem *item, const QString &udi, const QString &action);

          /**
          * @internal slot called when an eject is finished
          * @param errorData the error if problem
          * @param error type of error given by solid
          * @param udi device identifier given by solid
          **/
          void storageEjectDone(Solid::ErrorType error, QVariant errorData, const QString & udi);

          /**
          * @internal slot called when a storage tear is finished
          * @param errorData the error if problem
          * @param error type of error given by solid
          * @param udi device identifier given by solid
          **/
          void storageTeardownDone(Solid::ErrorType error, QVariant errorData, const QString & udi);

          /**
          * @internal slot called when a setup is finished
          * @param errorData the error if problem
          * @param error type of error given by solid
          * @param udi device identifier given by solid
          **/
          void storageSetupDone(Solid::ErrorType error, QVariant errorData, const QString & udi);

          /**
          * @internal slot called to restore to the notifier his icon
          **/
          void resetNotifierIcon();

          /**
          * @internal slot called when the ItemBackground has reached a device item
          * @param item a pointer to the item with the ItemBackground
          **/
          void itemHovered(QGraphicsItem *item);

          /**
          * @internal slot called when the user changes the visibility of a device
          **/
          void setItemVisibility();

          /**
          * @internal update the color of the label to follow plasma theme
          **/

          void updateColorsLater();

          /**
          * @internal update the color of the label to follow plasma theme
          **/

          void updateColors();

    private :
          /**
          * @internal build the dialog depending where it is
          **/
          void buildDialog();

          /**
          * @internal hides the ItemBackground and resets its target
          **/
          void resetSelection();

          /**
          * @internal reloads the free space of a device
          * @param item a pointer to the item
          **/
          void updateFreeSpace(DeviceItem *item);

          /**
          * @internal Search a category with same name. If not find, create a new category in top of treeview
          * @param categoryName the name of the category for device
          * @return the index of the category in the layout
          **/
          int searchOrCreateDeviceCategory(const QString &categoryName);

          /**
          * @internal get the category name of a device plugged
          * @param device the solid device plugged in hardware
          * @return the name of the category
          **/
          QString getCategoryNameOfDevice(const Solid::Device& device);

          /**
          * @internal updates the color for the category label
          **/
          void updateCategoryColors(Plasma::Label *);


          void updateMainLabelText();

          /// The graphics widget which displays the panel
          QGraphicsWidget *m_widget;

          ///The layout handling the devices inside the scroll widget
          QGraphicsLinearLayout *m_deviceLayout;

          ///Plasma::ItemBackground used to mark the currently highlighted item
          Plasma::ItemBackground *m_itemBackground;

          ///Plasma::ItemBackground used to mark the currently expanded device
          ///e.g. when there are multiple actions
          Plasma::ItemBackground *m_selectedItemBackground;

          ///The applet attached to this item
          DeviceNotifier *m_notifier;

          ///The ScrollWidget managing the view
          Plasma::ScrollWidget *m_devicesScrollWidget;

          ///The context menu action that allows to show all the devices
          QAction *m_showAll;

          ///the context menu action that allows to hide a device
          QAction *m_hideItem;

          ///the separator for the context menu
          QAction *m_separator;

          QTimer m_clearItemBackgroundTargetTimer;

          Plasma::Label *m_mainLabel;
  };

}

#endif

