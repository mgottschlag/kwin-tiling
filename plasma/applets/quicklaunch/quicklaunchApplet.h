/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans                                 *
 *   l.appelhans@gmx.de                                                    *
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
#ifndef QUICKLAUNCHAPPLET_H
#define QUICKLAUNCHAPPLET_H

#include <plasma/applet.h>
#include <plasma/widgets/iconwidget.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsLayoutItem>
#include <QList>
#include <KIcon>
#include <KDialog>

#include "ui_quicklaunchConfig.h"
#include "ui_quicklaunchAdd.h"

#include "quicklaunchIcon.h"
#include "QuicklaunchLayout.h"

namespace Plasma
{
    class Dialog;
}

class QuicklaunchApplet : public Plasma::Applet
{
    Q_OBJECT
    public:
        QuicklaunchApplet(QObject *parent, const QVariantList &args);
        ~QuicklaunchApplet();

        /**
         * Returns hints about the geometry of the figure
         * @return Hints about proportionality of the applet
         */
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

        /**
         * Returns info about if we want to expand or not
         * For now, we never need to expand
         */
        Qt::Orientations expandingDirections() const { return 0; }

        /**
         * List of actions to add in context menu
         * @return List of QAction pointers
         */
        virtual QList<QAction*> contextActions(QuicklaunchIcon *icon = 0);

    public slots:
        void createConfigurationInterface(KConfigDialog *parent);
        /**
         * Slot for showing the Add Icon interface
         */
        void showAddInterface();

    protected:
        /**
         * Overloaded method to save the state on exit
         */
        void saveState(KConfigGroup &config) const;

        virtual bool eventFilter(QObject * object, QEvent * event);

        /**
         * Overloaded drag enter event listener
         * Check if the dragged item is valid
         */
        void dragEnterEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * Overloaded drag move event listener
         * Listens for drag moves
         */
        void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * Overloaded drop event listener
         * Determines if a dropped item is valid Url, and if so
         * add the icon to the applet
         */
        void dropEvent(QGraphicsSceneDragDropEvent *event);

        /**
         * Called when something in the geometry has changed
         */
        void constraintsEvent(Plasma::Constraints constraints);

    protected slots:
	/**
	 * Called when the user has clicked OK in the Add Icon interface
	 */
	void addAccepted();

	/**
	 * Called when the user has clicked OK or apply in the Config interface
	 */
	void configAccepted();

    private slots:
        void refactorUi();
        void showDialog();
        void removeCurrentIcon();

    private:
        void init();

        /**
         * Read a Url, and insert at the given position
         * @param index The position to insert the icon into
         * @param desktopFile The Url to read
         */
        void addProgram(int index, const QString &desktopFile);

        /**
         * Read all Urls from a list, and insert into icon list
         * @param desktopFiles List with Urls
         */
        void loadPrograms(const QStringList &desktopFiles);

        /**
         * Removes all items from a BoxLayout
         * @param layout Layout to clear
         */
        void clearLayout(QGraphicsLayout *layout);

        /**
         * Saves icons into plasma applet config file
         */
        void saveConfig() {}

        bool dropHandler(const int pos, const QMimeData *mimedata);

        QGraphicsLinearLayout *m_layout;
        QuicklaunchLayout *m_innerLayout;
        QList<QuicklaunchIcon*> m_icons;
        Plasma::IconWidget *m_arrow;
        int m_visibleIcons;
        int m_rowCount;
        int m_dialogRowCount;
        Plasma::Dialog *m_dialog;
        QGraphicsWidget * m_dialogWidget;
        QuicklaunchLayout *m_dialogLayout;
        KDialog *m_addDialog;
        Ui::quicklaunchConfig uiConfig;
        Ui::quicklaunchAdd addUi;
        QuicklaunchIcon *m_rightClickedIcon;
        QPointF m_mousePressPos;

        QAction* m_addAction;
        QAction* m_removeAction;
};

#endif
