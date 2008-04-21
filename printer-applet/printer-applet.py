#!/usr/bin/env python

#############################################################################
##
## Copyright 2007-2008 Canonical Ltd
## Author: Jonathan Riddell <jriddell@ubuntu.com>
##
## Includes code from System Config Printer
## Copyright 2007 Tim Waugh <twaugh@redhat.com>
## Copyright 2007 Red Hat, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2 of 
## the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##
#############################################################################

"""
A systray applet to show that documents are being printed, show printer warnings and errors and a GUI for hal-cups-utils automatic setup for new printers.

It is a Qt port of the applet from Red Hat's System Config Printer
http://cyberelk.net/tim/software/system-config-printer/
svn co http://svn.fedorahosted.org/svn/system-config-printer/trunk
"""

import os
import subprocess
import sys

SYSTEM_CONFIG_PRINTER_DIR = "/usr/share/system-config-printer"

MIN_REFRESH_INTERVAL = 1 # seconds
CONNECTING_TIMEOUT = 60 # seconds

import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import uic
from PyKDE4.kdecore import i18n, ki18n, KAboutData, KCmdLineArgs, KCmdLineOptions, KStandardDirs
from PyKDE4.kdeui import KApplication, KXmlGuiWindow, KStandardAction, KIcon, KToggleAction

if QFile.exists(SYSTEM_CONFIG_PRINTER_DIR + "/ppds.py"):
    AUTOCONFIGURE = True
else:
    AUTOCONFIGURE = False

def translate(self, prop):
    """reimplement method from uic to change it to use gettext"""
    if prop.get("notr", None) == "true":
        return self._cstring(prop)
    else:
        if prop.text is None:
            return ""
        text = prop.text.encode("UTF-8")
        return i18n(text)

uic.properties.Properties._string = translate

import cups

import dbus
import dbus.mainloop.qt
import dbus.service

class MainWindow(KXmlGuiWindow):
    """Our main GUI dialogue, overridden so that closing it doesn't quit the app"""

    def closeEvent(self, event):
        event.ignore()
        self.hide()

class PrintersWindow(QWidget):
    """The printer status dialogue, overridden so that closing it doesn't quit the app and to untick the show menu entry"""

    def __init__(self, applet):
        QWidget.__init__(self)
        self.applet = applet

    def closeEvent(self, event):
        event.ignore()
        self.applet.on_printer_status_delete_event()
        self.hide()

class StateReason:
    REPORT=1
    WARNING=2
    ERROR=3

    LEVEL_ICON={
        REPORT: "dialog-info",
        WARNING: "dialog-warning",
        ERROR: "dialog-error"
        }

    def __init__(self, printer, reason):
        self.printer = printer
        self.reason = reason
        self.level = None
        self.canonical_reason = None

    def get_printer (self):
        return self.printer

    def get_level (self):
        if self.level != None:
            return self.level

        if (self.reason.endswith ("-report") or
            self.reason == "connecting-to-device"):
            self.level = self.REPORT
        elif self.reason.endswith ("-warning"):
            self.level = self.WARNING
        else:
            self.level = self.ERROR
        return self.level

    def get_reason (self):
        if self.canonical_reason:
            return self.canonical_reason

        level = self.get_level ()
        reason = self.reason
        if level == self.WARNING and reason.endswith ("-warning"):
            reason = reason[:-8]
        elif level == self.ERROR and reason.endswith ("-error"):
            reason = reason[:-6]
        self.canonical_reason = reason
        return self.canonical_reason

    def get_description (self):
        messages = {
            'toner-low': (i18n("Toner low"),
                          i18n("Printer '%s' is low on toner.")),
            'toner-empty': (i18n("Toner empty"),
                            i18n("Printer '%s' has no toner left.")),
            'cover-open': (i18n("Cover open"),
                           i18n("The cover is open on printer '%s'.")),
            'door-open': (i18n("Door open"),
                          i18n("The door is open on printer '%s'.")),
            'media-low': (i18n("Paper low"),
                          i18n("Printer '%s' is low on paper.")),
            'media-empty': (i18n("Out of paper"),
                            i18n("Printer '%s' is out of paper.")),
            'marker-supply-low': (i18n("Ink low"),
                                  i18n("Printer '%s' is low on ink.")),
            'marker-supply-empty': (i18n("Ink empty"),
                                    i18n("Printer '%s' has no ink left.")),
            'connecting-to-device': (i18n("Not connected?"),
                                     i18n("Printer '%s' may not be connected.")),
            }
        try:
            (title, text) = messages[self.get_reason ()]
            text = text % self.get_printer ()
        except KeyError:
            if self.get_level () == self.REPORT:
                title = i18n("Printer report")
            elif self.get_level () == self.WARNING:
                title = i18n("Printer warning")
            elif self.get_level () == self.ERROR:
                title = i18n("Printer error")
            text = ki18n("Printer '%1': '%2'.").subs(self.get_printer()).subs(self.get_reason ()).toString()
        return (title, text)

    def get_tuple (self):
        return (self.get_level (), self.get_printer (), self.get_reason ())

    def __cmp__(self, other):
        if other == None:
            return 1
        if other.get_level () != self.get_level ():
            return self.get_level () < other.get_level ()
        if other.get_printer () != self.get_printer ():
            return other.get_printer () < self.get_printer ()
        return other.get_reason () < self.get_reason ()

def collect_printer_state_reasons (connection):
    result = []
    printers = connection.getPrinters ()
    for name, printer in printers.iteritems ():
        reasons = printer["printer-state-reasons"]
        if type (reasons) == str:
            # Work around a bug that was fixed in pycups-1.9.20.
            reasons = [reasons]
        for reason in reasons:
            if reason == "none":
                break
            if (reason.startswith ("moving-to-paused") or
                reason.startswith ("paused") or
                reason.startswith ("shutdown") or
                reason.startswith ("stopping") or
                reason.startswith ("stopped-partly")):
                continue
            result.append (StateReason (name, reason))
    return result

def worst_printer_state_reason (connection, printer_reasons=None):
    """Fetches the printer list and checks printer-state-reason for
    each printer, returning a StateReason for the most severe
    printer-state-reason, or None."""
    worst_reason = None
    if printer_reasons == None:
        printer_reasons = collect_printer_state_reasons (connection)
    for reason in printer_reasons:
        if worst_reason == None:
            worst_reason = reason
            continue
        if reason > worst_reason:
            worst_reason = reason

    return worst_reason


class JobManager(QObject):
    """our main class creates the systray icon and the dialogues and refreshes the dialogues for new information"""
    def __init__(self, parent = None):
        QObject.__init__(self)

        self.will_refresh = False # whether timeout is set
        self.last_refreshed = 0
        self.trayicon = True
        self.suppress_icon_hide = False
        self.which_jobs = "not-completed"
        self.hidden = False
        self.jobs = {}
        self.jobiters = {}
        self.will_update_job_creation_times = False # whether timeout is set
        self.statusbar_set = False
        self.reasons_seen = {}
        self.connecting_to_device = {} # dict of printer->time first seen
        self.still_connecting = set()
        self.special_status_icon = False

        #Use local files if in current directory
        if os.path.exists("printer-applet.ui"):
            APPDIR = QDir.currentPath()
        else:
            file =  KStandardDirs.locate("appdata", "printer-applet.ui")
            APPDIR = file.left(file.lastIndexOf("/"))

        self.mainWindow = MainWindow()
        uic.loadUi(APPDIR + "/" + "printer-applet.ui", self.mainWindow)

        self.printersWindow = PrintersWindow(self)
        uic.loadUi(APPDIR + "/" + "printer-applet-printers.ui", self.printersWindow)

        self.sysTray = QSystemTrayIcon(KIcon("printer"), self.mainWindow)
        #self.sysTray.show()
        self.connect(self.sysTray, SIGNAL("activated( QSystemTrayIcon::ActivationReason )"), self.showMainWindow)

        self.menu = QMenu()
        self.menu.addAction(i18n("_Hide").replace("_", ""), self.on_icon_hide_activate)
        self.menu.addAction(KIcon("application-exit"), i18n("Quit"), self.on_icon_quit_activate)
        self.sysTray.setContextMenu(self.menu)

        self.mainWindow.treeWidget.setContextMenuPolicy(Qt.CustomContextMenu)
        self.connect(self.mainWindow.treeWidget, SIGNAL("customContextMenuRequested(const QPoint&)"), self.on_treeview_button_press_event)
        #self.connect(self.mainWindow.treeWidget, SIGNAL("itemClicked(QTreeWidgetItem*, int)"), self.printItemClicked)
        self.rightClickMenu = QMenu(self.mainWindow.treeWidget)
        self.cancel = self.rightClickMenu.addAction(i18n("Cancel"), self.on_job_cancel_activate)
        self.hold = self.rightClickMenu.addAction(i18n("_Hold").replace("_",""), self.on_job_hold_activate)
        self.release = self.rightClickMenu.addAction(i18n("_Release").replace("_",""), self.on_job_release_activate)
        self.reprint = self.rightClickMenu.addAction(i18n("Re_print").replace("_",""), self.on_job_reprint_activate)

        closeAction = KStandardAction.close(self.mainWindow, SLOT("hideMainWindow()"), self.mainWindow.actionCollection());
        self.connect(closeAction, SIGNAL("triggered(bool)"), self.hideMainWindow)
        #FIXME PyKDE bug? KStandardAction.close(self.hideMainWindow, self.mainWindow.actionCollection())

        refreshAction = self.mainWindow.actionCollection().addAction("refresh")
        refreshAction.setIcon( KIcon("view-refresh") )
        refreshAction.setText( i18n( "&Refresh" ) )
        refreshAction.setShortcut(QKeySequence(Qt.Key_F5))
        self.connect(refreshAction, SIGNAL("triggered(bool)"), self.hideMainWindow);

        showCompletedJobsAction = KToggleAction("Show Completed Jobs", self.mainWindow)
        self.mainWindow.actionCollection().addAction("show_completed_jobs", showCompletedJobsAction)
        self.connect(showCompletedJobsAction, SIGNAL("triggered(bool)"), self.on_show_completed_jobs_activate);

        showPrinterStatusAction = KToggleAction("Show Printer Status", self.mainWindow)
        self.mainWindow.actionCollection().addAction("show_printer_status", showPrinterStatusAction)
        self.connect(showPrinterStatusAction, SIGNAL("triggered(bool)"), self.on_show_printer_status_activate);

        self.mainWindow.createGUI(APPDIR + "/printer-appletui.rc")

        cups.setPasswordCB(self.cupsPasswdCallback)

        dbus.mainloop.qt.DBusQtMainLoop(set_as_default=True)

        try:
            bus = dbus.SystemBus()
        except:
            print >> sys.stderr, "%s: failed to connect to system D-Bus" % PROGRAM_NAME
            sys.exit (1)

        if AUTOCONFIGURE:
            notification = NewPrinterNotification(bus, self)

        # D-Bus
        bus.add_signal_receiver (self.handle_dbus_signal,
                                 path="/com/redhat/PrinterSpooler",
                                 dbus_interface="com.redhat.PrinterSpooler")
        self.refresh()

    """Used in gtk frontend to set magnifing glass icon when configuring printer, I don't have a suitable icon so using bubbles instead
    # Handle "special" status icon
    def set_special_statusicon (self, iconname):
        self.special_status_icon = True
        self.statusicon.set_from_icon_name (iconname)
        self.set_statusicon_visibility ()

    def unset_special_statusicon (self):
        self.special_status_icon = False
        self.statusicon.set_from_pixbuf (self.saved_statusicon_pixbuf)
    """

    def notify_new_printer (self, printer, title, text):
        self.hidden = False
        self.showMessage(title, text)

    def showMessage(self, title, message):
        """show a message, delayed slightly to ensure the systray is visible else it appears in the wrong place
        Gtk uses libnotify, for Qt we just show the message directly"""
        self.sysTray.show()
        self.sysTrayTitle = title
        self.sysTrayMessage = message
        QTimer.singleShot(1000, self.showSysTrayMessage)

    def showSysTrayMessage(self):
        self.sysTray.showMessage(self.sysTrayTitle, self.sysTrayMessage)

    """unused, see set_special_statusicon
    def set_statusicon_from_pixbuf (self, pb):
        self.saved_statusicon_pixbuf = pb
        if not self.special_status_icon:
            self.statusicon.set_from_pixbuf (pb)
    """

    """unused, see MainWindow and PrintersWindow
    def on_delete_event(self, *args):
        if self.trayicon:
            self.MainWindow.hide ()
            if self.show_printer_status.get_active ():
                self.PrintersWindow.hide ()
        else:
            self.loop.quit ()
        return True
    """

    def on_printer_status_delete_event(self):
        self.mainWindow.actionShow_Printer_Status.setChecked(False)

    def cupsPasswdCallback(self, querystring):
        (text, ok) = QInputDialog.getText(self.mainWindow, i18n("Password required"), querystring, QLineEdit.Password)
        if ok:
            print "ok"
            return text
        return ''

    def show_IPP_Error(self, exception, message):
        if exception == cups.IPP_NOT_AUTHORIZED:
            error_text = ('<span weight="bold" size="larger">' +
                          i18n('Not authorized') + '</span>\n\n' +
                          i18n('The password may be incorrect.'))
        else:
            error_text = ('<span weight="bold" size="larger">' +
                          i18n('CUPS server error') + '</span>\n\n' +
                          i18n("There was an error during the CUPS "\
                            "operation: '%s'.")) % message
        #fix Gtk's non-HTML for Qt
        error_text = error_text.replace("\n", "<br />")
        error_text = error_text.replace("span", "strong")
        QMessageBox.critical(self.mainWindow, i18n("Error"), error_text)

    """
    def toggle_window_display(self, icon):
    """
    #FIXME, hide printer status window?
    def hideMainWindow(self):
        self.mainWindow.hide()

    def showMainWindow(self, activationReason):
        if activationReason == QSystemTrayIcon.Trigger:
            if self.mainWindow.isVisible():
                self.mainWindow.hide()
            else:
                self.mainWindow.show()

    def on_show_completed_jobs_activate(self, activated):
        if activated:
            self.which_jobs = "all"
        else:
            self.which_jobs = "not-completed"
        self.refresh()

    def on_show_printer_status_activate(self, activated):
        if activated:
            self.printersWindow.show()
        else:
            self.printersWindow.hide()

    def check_still_connecting(self):
        """Timer callback to check on connecting-to-device reasons."""
        c = cups.Connection ()
        printer_reasons = collect_printer_state_reasons (c)
        del c

        if self.update_connecting_devices (printer_reasons):
            self.refresh ()

        # Don't run this callback again.
        return False

    def update_connecting_devices(self, printer_reasons=[]):
        """Updates connecting_to_device dict and still_connecting set.
        Returns True if a device has been connecting too long."""
        time_now = time.time ()
        connecting_to_device = {}
        trouble = False
        for reason in printer_reasons:
            if reason.get_reason () == "connecting-to-device":
                # Build a new connecting_to_device dict.  If our existing
                # dict already has an entry for this printer, use that.
                printer = reason.get_printer ()
                t = self.connecting_to_device.get (printer, time_now)
                connecting_to_device[printer] = t
                if time_now - t >= CONNECTING_TIMEOUT:
                    trouble = True

        # Clear any previously-notified errors that are now fine.
        remove = set()
        for printer in self.still_connecting:
            if not self.connecting_to_device.has_key (printer):
                remove.add (printer)

        self.still_connecting = self.still_connecting.difference (remove)

        self.connecting_to_device = connecting_to_device
        return trouble

    def check_state_reasons(self, connection, my_printers=set()):
        printer_reasons = collect_printer_state_reasons (connection)

        # Look for any new reasons since we last checked.
        old_reasons_seen_keys = self.reasons_seen.keys ()
        reasons_now = set()
        need_recheck = False
        for reason in printer_reasons:
            tuple = reason.get_tuple ()
            printer = reason.get_printer ()
            reasons_now.add (tuple)
            if not self.reasons_seen.has_key (tuple):
                # New reason.
                iter = QTreeWidgetItem(self.printersWindow.treeWidget)
                #iter.setText(0, reason.get_level ())
                iter.setText(0, reason.get_printer ())
                title, text = reason.get_description ()
                iter.setText(1, text)
                self.printersWindow.treeWidget.addTopLevelItem(iter)

                self.reasons_seen[tuple] = iter
                if (reason.get_reason () == "connecting-to-device" and
                    not self.connecting_to_device.has_key (printer)):
                    # First time we've seen this.
                    need_recheck = True

        if need_recheck:
            # Check on them again in a minute's time.
            QTimer.singleShot(CONNECTING_TIMEOUT * 1000, self.check_still_connecting)

        self.update_connecting_devices (printer_reasons)
        items = self.reasons_seen.keys ()
        for tuple in items:
            if not tuple in reasons_now:
                # Reason no longer present.
                iter = self.reasons_seen[tuple]
                index = self.mainWindow.treeWidget.indexOfTopLevelItem(iter)
                self.mainWindow.treeWidget.takeTopLevelItem(index)
                del self.reasons_seen[tuple]
        # Update statusbar and icon with most severe printer reason
        # across all printers.
        self.icon_has_emblem = False
        reason = worst_printer_state_reason (connection, printer_reasons)
        if reason != None and reason.get_level () >= StateReason.WARNING:
            title, text = reason.get_description ()
            #if self.statusbar_set:
            #    self.statusbar.pop (0)
            self.mainWindow.statusBar().showMessage(text)
            #self.statusbar.push (0, text)
            self.worst_reason_text = text
            self.statusbar_set = True

            if self.trayicon:
                icon = StateReason.LEVEL_ICON[reason.get_level ()]
                emblem = QPixmap(KIcon(icon).pixmap(16, 16))
                pixbuf = QPixmap(KIcon("printer").pixmap(22, 22))
                painter = QPainter(pixbuf)
                painter.drawPixmap(pixbuf.width()-emblem.width(),pixbuf.height()-emblem.height(),emblem)
                painter.end()
                self.sysTray.setIcon(QIcon(pixbuf))
                self.icon_has_emblem = True
        else:
            # No errors
            if self.statusbar_set:
                #self.statusbar.pop (0)
                self.mainWindow.statusBar().clearMessage()
                self.statusbar_set = False

    """not using notifications in qt frontend
    def on_notification_closed(self, notify):
    """

    def update_job_creation_times(self):
        now = time.time ()
        need_update = False
        for job, data in self.jobs.iteritems():
            if self.jobs.has_key (job):
                iter = self.jobiters[job]

            t = "Unknown"
            if data.has_key ('time-at-creation'):
                created = data['time-at-creation']
                ago = now - created
                if ago > 86400:
                    t = time.ctime (created)
                elif ago > 3600:
                    need_update = True
                    hours = int (ago / 3600)
                    mins = int ((ago % 3600) / 60)
                    if hours == 1:
                        if mins == 0:
                            t = i18n("1 hour ago")
                        elif mins == 1:
                            t = i18n("1 hour and 1 minute ago")
                        else:
                            t = i18n("1 hour and %d minutes ago") % mins
                    else:
                        if mins == 0:
                            t = i18n("%d hours ago")
                        elif mins == 1:
                            t = i18n("%d hours and 1 minute ago") % hours
                        else:
                            t = i18n("%d hours and %d minutes ago") % \
                                (hours, mins)
                else:
                    need_update = True
                    mins = ago / 60
                    if mins < 2:
                        t = i18n("a minute ago")
                    else:
                        t = i18n("%d minutes ago") % mins

            #self.store.set_value (iter, 4, t)
            iter.setText(4, t)

        if need_update and not self.will_update_job_creation_times:
            #gobject.timeout_add (60 * 1000,
            #                     self.update_job_creation_times)
            QTimer.singleShot(60 * 1000, self.update_job_creation_times)
            self.will_update_job_creation_times = True

        if not need_update:
            self.will_update_job_creation_times = False

        # Return code controls whether the timeout will recur.
        return self.will_update_job_creation_times

    def refresh(self):
        """updates the print dialogue"""
        now = time.time ()
        if (now - self.last_refreshed) < MIN_REFRESH_INTERVAL:
            if self.will_refresh:
                return

            #gobject.timeout_add (MIN_REFRESH_INTERVAL * 1000,
            #                     self.refresh)
            QTimer.singleShot(MIN_REFRESH_INTERVAL * 1000, self.update_job_creation_times)
            self.will_refresh = True
            return

        self.will_refresh = False
        self.last_refreshed = now

        try:
            c = cups.Connection ()
            jobs = c.getJobs (which_jobs=self.which_jobs, my_jobs=True)
        except cups.IPPError, (e, m):
            self.show_IPP_Error (e, m)
            return
        except RuntimeError:
            return

        if self.which_jobs == "not-completed":
            num_jobs = len (jobs)
        else:
            try:
                num_jobs = len (c.getJobs (my_jobs=True))
            except cups.IPPError, (e, m):
                self.show_IPP_Error (e, m)
                return
            except RuntimeError:
                return

        if self.trayicon:
            self.num_jobs = num_jobs
            if self.hidden and self.num_jobs != self.num_jobs_when_hidden:
                self.hidden = False
            if num_jobs == 0:
                tooltip = i18n("No documents queued")
                #FIXMEself.set_statusicon_from_pixbuf (self.icon_no_jobs)
            elif num_jobs == 1:
                tooltip = i18n("1 document queued")
                #self.set_statusicon_from_pixbuf (self.icon_jobs)
            else:
                tooltip = i18n("%d documents queued") % num_jobs
                #self.set_statusicon_from_pixbuf (self.icon_jobs)

        my_printers = set()
        for job, data in jobs.iteritems ():
            state = data.get ('job-state', cups.IPP_JOB_CANCELED)
            if state >= cups.IPP_JOB_CANCELED:
                continue
            uri = data.get ('job-printer-uri', '/')
            i = uri.rfind ('/')
            my_printers.add (uri[i + 1:])

        self.check_state_reasons (c, my_printers)
        del c

        if self.trayicon:
            # If there are no jobs but there is a printer
            # warning/error indicated by the icon, set the icon
            # tooltip to the reason description.
            if self.num_jobs == 0 and self.icon_has_emblem:
                tooltip = self.worst_reason_text

            self.sysTray.setToolTip (tooltip)
            self.set_statusicon_visibility ()

        for job in self.jobs:
            if not jobs.has_key (job):
                #self.store.remove (self.jobiters[job])
                index = self.mainWindow.treeWidget.indexOfTopLevelItem(self.jobiters[job])
                self.mainWindow.treeWidget.takeTopLevelItem(index)
                del self.jobiters[job]

        for job, data in jobs.iteritems():
            if self.jobs.has_key (job):
                iter = self.jobiters[job]
            else:
                iter = QTreeWidgetItem(self.mainWindow.treeWidget)
                iter.setText(0, str(job))
                iter.setText(1, data.get('job-name', 'Unknown'))
                self.mainWindow.treeWidget.addTopLevelItem(iter)
                self.jobiters[job] = iter

            uri = data.get('job-printer-uri', '')
            i = uri.rfind ('/')
            if i != -1:
                printer = uri[i + 1:]
            iter.setText(2, printer)

            if data.has_key ('job-k-octets'):
                size = str (data['job-k-octets']) + 'k'
            else:
                size = 'Unknown'
            iter.setText(3, size)
            #self.store.set_value (iter, 3, size)

            state = None
            if data.has_key ('job-state'):
                try:
                    jstate = data['job-state']
                    s = int (jstate)
                    state = { cups.IPP_JOB_PENDING:i18n("Pending"),
                              cups.IPP_JOB_HELD:i18n("Held"),
                              cups.IPP_JOB_PROCESSING: i18n("Processing"),
                              cups.IPP_JOB_STOPPED: i18n("Stopped"),
                              cups.IPP_JOB_CANCELED: i18n("Canceled"),
                              cups.IPP_JOB_ABORTED: i18n("Aborted"),
                              cups.IPP_JOB_COMPLETED: i18n("Completed") }[s]
                except ValueError:
                    pass
                except IndexError:
                    pass    
            if state == None:
                state = i18n("Unknown")
            iter.setText(5, state)
            columns = self.mainWindow.treeWidget.columnCount()
            for i in range(columns):
                self.mainWindow.treeWidget.resizeColumnToContents(i)

        self.jobs = jobs
        self.update_job_creation_times ()

    def set_statusicon_visibility (self):
        if self.trayicon:
            if self.suppress_icon_hide:
                # Avoid hiding the icon if we've been woken up to notify
                # about a new printer.
                self.suppress_icon_hide = False
                return

            if (not self.hidden) and (self.num_jobs > 0 or self.icon_has_emblem) or self.special_status_icon:
                self.sysTray.show()
            else:
                self.sysTray.hide()

    def on_treeview_button_press_event(self, postition):
        # Right-clicked.
        items = self.mainWindow.treeWidget.selectedItems ()
        print "items" + str(items)
        print len(items)
        if len(items) != 1:
            return
        print "selected: " + str(items)
        iter = items[0]
        if iter == None:
            return

        self.jobid = int(iter.text(0))
        job = self.jobs[self.jobid]
        self.cancel.setEnabled (True)
        self.hold.setEnabled (True)
        self.release.setEnabled (True)
        self.reprint.setEnabled (True)
        if job.has_key ('job-state'):
            s = job['job-state']
            print s, "jobstate"
            if s >= cups.IPP_JOB_CANCELED:
                self.cancel.setEnabled (False)
            if s != cups.IPP_JOB_PENDING and s != cups.IPP_JOB_PROCESSING:
                self.hold.setEnabled (False)
            if s != cups.IPP_JOB_HELD:
                self.release.setEnabled (False)
            if (s != cups.IPP_JOB_CANCELED or
                not job.get('job-preserved', False)):
                self.reprint.setEnabled (False)
        self.rightClickMenu.popup(QCursor.pos())

    def on_icon_popupmenu(self, icon, button, time):
        self.icon_popupmenu.popup (None, None, None, button, time)

    def on_icon_hide_activate(self):
        self.num_jobs_when_hidden = self.num_jobs
        self.hidden = True
        self.set_statusicon_visibility ()

    def on_icon_quit_activate(self):
        app.quit()

    def on_job_cancel_activate(self):
        try:
            c = cups.Connection ()
            c.cancelJob (self.jobid)
            del c
        except cups.IPPError, (e, m):
            self.show_IPP_Error (e, m)
            return
        except RuntimeError:
            return

    def on_job_hold_activate(self):
        try:
            c = cups.Connection ()
            c.setJobHoldUntil (self.jobid, "indefinite")
            del c
        except cups.IPPError, (e, m):
            self.show_IPP_Error (e, m)
            return
        except RuntimeError:
            return

    def on_job_release_activate(self):
        try:
            c = cups.Connection ()
            c.setJobHoldUntil (self.jobid, "no-hold")
            del c
        except cups.IPPError, (e, m):
            self.show_IPP_Error (e, m)
            return
        except RuntimeError:
            return

    def on_job_reprint_activate(self):
        try:
            c = cups.Connection ()
            c.restartJob (self.jobid)
            del c
        except cups.IPPError, (e, m):
            self.show_IPP_Error (e, m)
            return
        except RuntimeError:
            return

        self.refresh ()

    def on_refresh_activate(self, menuitem):
        self.refresh ()

    def handle_dbus_signal(self, *args):
        self.refresh ()

    ## Printer status window
    """FIXME
    def set_printer_status_icon (self, column, cell, model, iter, *user_data):
        level = model.get_value (iter, 0)
        icon = StateReason.LEVEL_ICON[level]
        theme = gtk.icon_theme_get_default ()
        try:
            pixbuf = theme.load_icon (icon, 22, 0)
            cell.set_property("pixbuf", pixbuf)
        except gobject.GError, exc:
            pass # Couldn't load icon
    """
    """FIXME
    def set_printer_status_name (self, column, cell, model, iter, *user_data):
        cell.set_property("text", model.get_value (iter, 1))
    """

####
#### NewPrinterNotification DBus server (the 'new' way).  Note: this interface
#### is not final yet.
####
PDS_PATH="/com/redhat/NewPrinterNotification"
PDS_IFACE="com.redhat.NewPrinterNotification"
PDS_OBJ="com.redhat.NewPrinterNotification"
class NewPrinterNotification(dbus.service.Object):
    """listen for dbus signals"""
    STATUS_SUCCESS = 0
    STATUS_MODEL_MISMATCH = 1
    STATUS_GENERIC_DRIVER = 2
    STATUS_NO_DRIVER = 3

    def __init__ (self, bus, jobmanager):
        self.bus = bus
        self.getting_ready = 0
        self.jobmanager = jobmanager
        bus_name = dbus.service.BusName (PDS_OBJ, bus=bus)
        dbus.service.Object.__init__ (self, bus_name, PDS_PATH)
        #self.jobmanager.notify_new_printer ("", i18n("New Printer"), i18n("Configuring New Printer"))

    """
    def wake_up (self):
        global waitloop, runloop, jobmanager
        do_imports ()
        if jobmanager == None:
            waitloop.quit ()
            runloop = gobject.MainLoop ()
            jobmanager = JobManager(bus, runloop,
                                    service_running=service_running,
                                    trayicon=trayicon, suppress_icon_hide=True)
    """
    
    @dbus.service.method(PDS_IFACE, in_signature='', out_signature='')
    def GetReady (self):
        """hal-cups-utils is settings up a new printer"""
        self.jobmanager.notify_new_printer ("", i18n("New Printer"), i18n("Configuring New Printer"))
    """
        self.wake_up ()
        if self.getting_ready == 0:
            jobmanager.set_special_statusicon (SEARCHING_ICON)

        self.getting_ready += 1
        gobject.timeout_add (60 * 1000, self.timeout_ready)

    def timeout_ready (self):
        global jobmanager
        if self.getting_ready > 0:
            self.getting_ready -= 1
        if self.getting_ready == 0:
            jobmanager.unset_special_statusicon ()

        return False
    """

    # When I plug in my printer HAL calls this with these args:
    #status: 0
    #name: PSC_1400_series
    #mfg: HP
    #mdl: PSC 1400 series
    #des:
    #cmd: LDL,MLC,PML,DYN
    @dbus.service.method(PDS_IFACE, in_signature='isssss', out_signature='')
    def NewPrinter (self, status, name, mfg, mdl, des, cmd):
        """hal-cups-utils has set up a new printer"""
        """
        print "status: " + str(status)
        print "name: " + name
        print "mfg: " + mfg
        print "mdl: " + mdl
        print "des: " + des
        print "cmd: " + cmd
        """

        c = cups.Connection ()
        try:
            printer = c.getPrinters ()[name]
        except KeyError:
            return
        del c

        sys.path.append (SYSTEM_CONFIG_PRINTER_DIR)
        from ppds import ppdMakeModelSplit
        (make, model) = ppdMakeModelSplit (printer['printer-make-and-model'])
        driver = make + " " + model
        if status < self.STATUS_GENERIC_DRIVER:
            title = i18n("Printer added")
        else:
            title = i18n("Missing printer driver")

        if status == self.STATUS_SUCCESS:
            text = i18n("'%s' is ready for printing.") % name
        else: # Model mismatch
            text = ki18n("'%1' has been added, using the '%2' driver.").subs(name).subs(driver).toString()

        self.jobmanager.notify_new_printer (name, title, text)


if __name__ == "__main__":
    """start the application.  TODO, gtk frontend does clever things here to not start the GUI until it has to"""
    appName     = "printer-applet"
    catalogue   = "printer-applet"
    programName = ki18n("Printer Applet")
    version     = "1.0"
    description = ki18n("Applet to view current print jobs and configure new printers")
    license     = KAboutData.License_GPL
    copyright   = ki18n("2007-2008 Canonical Ltd")
    text        = ki18n("none")
    homePage    = "https://launchpad.net/system-config-printer"
    bugEmail    = ""

    aboutData   = KAboutData (appName, catalogue, programName, version, description,
                                license, copyright, text, homePage, bugEmail)

    aboutData.addAuthor(ki18n("Jonathan Riddell"), ki18n("Author"))
    aboutData.addAuthor(ki18n("Tim Waugh/Red Hat"), ki18n("System Config Printer Author"))

    options = KCmdLineOptions()
    options.add("show", ki18n("Show even when nothing printing"))

    KCmdLineArgs.init(sys.argv, aboutData)
    KCmdLineArgs.addCmdLineOptions(options)

    app = KApplication()

    args = KCmdLineArgs.parsedArgs()

    app.setWindowIcon(KIcon("printer"))
    if app.isSessionRestored():
         sys.exit(1)
    applet = JobManager()
    if args.isSet("show"):
        applet.mainWindow.show()
        applet.sysTray.show()
    sys.exit(app.exec_())
