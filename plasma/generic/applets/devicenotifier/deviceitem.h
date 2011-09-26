/***************************************************************************
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

#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include <QtGui/QGraphicsWidget>
#include <QtGui/QIcon>
#include <QtCore/QTimer>

class QGraphicsProxyWidget;
class QGraphicsLinearLayout;

namespace Plasma
{
    class Animation;
    class IconWidget;
    class BusyWidget;
    class Label;
    class Meter;
}

namespace Notifier
{

/**
* @short The item representing the devices
*
*/
class DeviceItem : public QGraphicsWidget
{
    Q_OBJECT

    public:
        enum LeftActions {
            Nothing = 0,
            Mount = 1,
            Umount = 2,
            Lock = 3,
            Unlock = 4
        };

        enum State {
            Idle = 0,
            Mounting = 1,
            Umounting =2
        };

        /**
        * Constructor of the item
        * @param udi the udi of the device
	* @param unpluggable indicates if the device can be unplugged or not
        * @param parent the parent of the device
        */
        explicit DeviceItem(const QString &udi, bool unpluggable, QGraphicsWidget *parent = 0);

        /**
        * Default destructor
        **/
        ~DeviceItem();

        /**
        * Adds an action inside this item
        * @param action the predicate file of the action
        **/
        void addAction(const QString &action);

        QStringList actions() const;

        void removeAction(const QString &action);

        /**
        * Hides the actions
        **/
        void collapse();

        /**
        * Shows the actions
        **/
        void expand();

        /**
        * The udi of the device
        * @return the udi
        **/
        QString udi() const;

        /**
        * The name of the device
        * @return the name
        **/
        QString name() const;

        /**
        * The icon of the device
        * @return the icon
        **/
        QIcon icon() const;

        /**
        * The name of the icon of the device
        * @return the name
        **/
        QString iconName() const;

        /**
        * Used to know if the actions are visible
        * @return true if the actions are hidden
        **/
        bool isCollapsed() const;

        /**
        * The subtitle of the device
        * @return the subtitle
        **/
        QString description() const;

        /**
        * Used to know if the device is set to be hidden or visible
        * @return true if it is visible
        **/
        bool visibility() const;

        /**
        * Used to know if the device is mounted
        * @return true if it is mounted
        **/
        bool isMounted() const;

        /**
        * Used to know the action the icon will do if pressed
        * @return the action
        **/
        LeftActions leftAction();

        /**
        * Used to know if this item is set hovered
        * @return true if it is hovered
        **/
        bool hovered() const;

	/**
	 * Used to know if this item can be safely removed
	 * @return true if it can be safely removed
	 **/
	bool safelyRemovable() const;

	/**
	 * Indicates if this item can be safely removed
	 * @param safe true if it can be safely removed
	 **/
	void setSafelyRemovable(const bool safe = true);

	/**
	 * Used to know if this item can be unplugged.
	 *
	 */
	bool unpluggable();

        /**
        * Indicates if the device is mounted
        * @param mounted true if it is mounted
        **/
        void setMounted(const bool mounted = true);

        /**
        * Indicates the action executed by the left icon
        * @param action the action
        **/
        void setLeftAction(LeftActions action);

        /**
        * Indicates if the device is hovered
        * @param mounted true if it is hovered
        **/
        void setHovered(const bool hovered = true);

        /**
        * Indicates how much free space there is in the device
        * @param freeSpace the free space
        * @param size the total size of the device
        **/
        void setFreeSpace(qulonglong freeSpace, qulonglong size);

        /**
        * Reimplemented from QGraphicsItem
        **/
        void setData(int key, const QVariant & value);

        /**
        * Sets the state of the device
        **/
        void setState(State state);

        /**
        * Gets the state of the device
        **/
        State state() const;

        /**
        * Update colors on a theme change
        **/
        void updateColors();

        void clicked();
        void actionClicked(Plasma::IconWidget*);

        bool selectNextAction(Plasma::IconWidget*);
        bool selectPreviousAction(Plasma::IconWidget*, bool forceLast = false);

        static const int MARGIN = 3;
        static const int TEXT_MARGIN = 3;
        static const int LEFTACTION_SIZE = 22;

    public slots:
        void setHoverDisplayOpacity(qreal opacity);

    signals:
        /**
        * Emitted when the left action has been clicked
        **/
        void leftActionActivated(DeviceItem *item);

        /**
        * Emitted when an action has been clicked
        **/
        void actionActivated(DeviceItem *item, const QString &udi, const QString &action);

        /**
        * Emitted when the device has been clicked and expanded
        **/
        void activated(DeviceItem *item);

        /**
        * Emitted when the device has been clicked and collapsed
        **/
        void collapsed(DeviceItem *item);

        /**
        * Emitted when an item in the device item should be highlighted
        */
        void highlightActionItem(QGraphicsItem *item);

    protected:
        /**
        * Reimplemented from QGraphicsItem
        **/
        void mousePressEvent(QGraphicsSceneMouseEvent *e);

        /**
        * Reimplemented from QGraphicsItem
        **/
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);

        /**
        * Reimplemented from QObject
        **/
        bool eventFilter(QObject *obj, QEvent *event);

    private slots:
        /**
        * @internal slot called when the left action is clicked
        **/
        void leftActionClicked();
        /**
        * @internal shows the busy widget
        **/
        void triggerBusyWidget();

    private:

        bool allowsCapacityBar() const;

	void updateTooltip();

        ///The icon of the device
        QIcon m_icon;

        ///The udi of the device
        QString m_udi;

        ///The user-friendly name of the device
        QString m_name;

        ///The name of the icon
        QString m_iconName;

        ///True if the device is visible
        bool m_visibility;

        ///True if the device is hovered
        bool m_hovered;

        ///True if the device is mounted
        bool m_mounted;

	///True if the device can be safely removed
	bool m_safelyRemovable;

	///True if the device is unpluggable
	bool m_unpluggable;

        ///The action the left icon will do if activated
        LeftActions m_leftAction;

        ///The layout arranging the items showing information about the device
        QGraphicsLinearLayout *m_mainLayout;

        ///The layout arranging the actions
        QGraphicsLinearLayout *m_actionsLayout;

        ///The main layout arranging the device and the action
        QGraphicsLinearLayout *m_treeLayout;

        ///The widget hosting the actions
        QGraphicsWidget *m_actionsWidget;

        ///The left action
        Plasma::IconWidget *m_leftActionIcon;

        ///The icon of the device
        Plasma::IconWidget *m_deviceIcon;

        ///The label that draws the name of the device
        Plasma::Label *m_nameLabel;

        ///The label that draws the description of the device
        Plasma::Label *m_descriptionLabel;

        ///The meter that draws free space for the device
        Plasma::Meter *m_freeSpaceBar;

        ///The busy widget
        Plasma::BusyWidget *m_busyWidget;

        ///The timer that makes the busy widget show up
        QTimer m_busyWidgetTimer;

        State m_state;

        Plasma::Animation *m_labelFade;
        Plasma::Animation *m_barFade;
};

}
#endif
