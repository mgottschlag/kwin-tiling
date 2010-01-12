/*
 *   Copyright (C) 2007-2010 John Tapsell <johnflux@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KSYSTEMACTIVITYDIALOG__H
#define KSYSTEMACTIVITYDIALOG__H

#ifndef Q_WS_WIN

#include "processui/ksysguardprocesslist.h"
#include "krunnersettings.h"

#include <QAbstractScrollArea>
#include <QCloseEvent>
#include <QLayout>
#include <QString>
#include <QTreeView>
#include <KConfigGroup>
#include <KDialog>
#include <KGlobal>
#include <KWindowSystem>

/** This creates a simple dialog box with a KSysguardProcessList
 *
 *  It remembers the size and position of the dialog, and sets
 *  the dialog to always be over the other windows
 */
class KSystemActivityDialog : public KDialog
{
    public:
        KSystemActivityDialog(QWidget *parent = NULL) : KDialog(parent), processList(0) {
            setWindowTitle(i18n("System Activity"));
            setWindowIcon(KIcon("utilities-system-monitor"));
            setButtons(0);
            setMainWidget(&processList);
            processList.setScriptingEnabled(true);
            setSizeGripEnabled(true);
            (void)minimumSizeHint(); //Force the dialog to be laid out now
            layout()->setContentsMargins(0,0,0,0);
            processList.treeView()->setCornerWidget(new QWidget);

            setInitialSize(QSize(650, 420));
            KConfigGroup cg = KGlobal::config()->group("TaskDialog");
            restoreDialogSize(cg);

            processList.loadSettings(cg);
            // Since we default to forcing the window to be KeepAbove, if the user turns this off, remember this
            const bool keepAbove = KRunnerSettings::keepTaskDialogAbove();
            if (keepAbove) {
                KWindowSystem::setState(winId(), NET::KeepAbove );
            }
        }

        /** Show the dialog and set the focus
         *
         *  This can be called even when the dialog is already showing to bring it
         *  to the front again and move it to the current desktop etc.
         */
        void run() {
            show();
            raise();
            KWindowSystem::setOnDesktop(winId(), KWindowSystem::currentDesktop());
            KWindowSystem::forceActiveWindow(winId());
        }
        /** Set the text in the filter line in the process list widget */
        void setFilterText(const QString &filterText) {
            processList.filterLineEdit()->setText(filterText);
            processList.filterLineEdit()->setFocus();
        }
        /** Save the settings if the user clicks (x) button on the window */
        void closeEvent(QCloseEvent *event) {
            saveDialogSettings();
            if(event)
                event->accept();
        }

        /** Save the settings if the user presses the ESC key */
        virtual void reject () {
            saveDialogSettings();
            QDialog::reject();
        }

    private:
        void saveDialogSettings() {
            //When the user closes the dialog, save the position and the KeepAbove state
            KConfigGroup cg = KGlobal::config()->group("TaskDialog");
            saveDialogSize(cg);
            processList.saveSettings(cg);

            // Since we default to forcing the window to be KeepAbove, if the user turns this off, remember this
            bool keepAbove = KWindowSystem::windowInfo(winId(), NET::WMState).hasState(NET::KeepAbove);
            KRunnerSettings::setKeepTaskDialogAbove(keepAbove);
            KGlobal::config()->sync();
        }
        KSysGuardProcessList processList;
};
#endif // not Q_WS_WIN

#endif // KSYSTEMACTIVITYDIALOG__H
