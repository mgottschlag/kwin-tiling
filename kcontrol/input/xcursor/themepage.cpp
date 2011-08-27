/*
 * Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config-X11.h>

#include <KConfig>
#include <KLocale>
#include <KStandardDirs>

#include <KGlobalSettings>
#include <KToolInvocation>
#include <KMessageBox>
#include <KUrlRequesterDialog>
#include <KIO/Job>
#include <KIO/DeleteJob>
#include <KIO/NetAccess>
#include <knewstuff3/downloaddialog.h>


#include <KTar>

#include <klauncher_iface.h>
#include "../../krdb/krdb.h"

#include <QWidget>
#include <QPushButton>
#include <QDir>
#include <QX11Info>

#include "themepage.h"
#include "themepage.moc"

#include "thememodel.h"
#include "itemdelegate.h"
#include "sortproxymodel.h"
#include "cursortheme.h"

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#ifdef HAVE_XFIXES
#  include <X11/extensions/Xfixes.h>
#endif


ThemePage::ThemePage(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    installKnsButton->setIcon(KIcon("get-hot-new-stuff"));
    installButton->setIcon(KIcon("document-import"));
    removeButton->setIcon(KIcon("edit-delete"));

    model = new CursorThemeModel(this);

    proxy = new SortProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseSensitive);
    proxy->sort(NameColumn, Qt::AscendingOrder);

    int size = style()->pixelMetric(QStyle::PM_LargeIconSize);

    view->setModel(proxy);
    view->setItemDelegate(new ItemDelegate(this));
    view->setIconSize(QSize(size, size));
    view->setSelectionMode(QAbstractItemView::SingleSelection);

    // Make sure we find out about selection changes
    connect(view->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectionChanged()));

    // Disable the install button if we can't install new themes to ~/.icons,
    // or Xcursor isn't set up to look for cursor themes there.
    if (!model->searchPaths().contains(QDir::homePath() + "/.icons") || !iconsIsWritable()) {
            installButton->setEnabled(false);
            installKnsButton->setEnabled(false);
    }

    connect(installKnsButton, SIGNAL(clicked()), SLOT(getNewClicked()));
    connect(installButton, SIGNAL(clicked()), SLOT(installClicked()));
    connect(removeButton,  SIGNAL(clicked()), SLOT(removeClicked()));
}


ThemePage::~ThemePage()
{
}


bool ThemePage::iconsIsWritable() const
{
    const QFileInfo icons = QFileInfo(QDir::homePath() + "/.icons");
    const QFileInfo home  = QFileInfo(QDir::homePath());

    return ((icons.exists() && icons.isDir() && icons.isWritable()) ||
            (!icons.exists() && home.isWritable()));
}


bool ThemePage::haveXfixes()
{
    bool result = false;

#ifdef HAVE_XFIXES
    int event_base, error_base;
    if (XFixesQueryExtension(QX11Info::display(), &event_base, &error_base))
    {
        int major, minor;
        XFixesQueryVersion(QX11Info::display(), &major, &minor);
        result = (major >= 2);
    }
#endif

    return result;
}


bool ThemePage::applyTheme(const CursorTheme *theme)
{
    // Require the Xcursor version that shipped with X11R6.9 or greater, since
    // in previous versions the Xfixes code wasn't enabled due to a bug in the
    // build system (freedesktop bug #975).
#if HAVE_XFIXES && XFIXES_MAJOR >= 2 && XCURSOR_LIB_VERSION >= 10105
    if (!haveXfixes())
        return false;

    QByteArray themeName = QFile::encodeName(theme->name());

    // Set up the proper launch environment for newly started apps
    KToolInvocation::klauncher()->setLaunchEnv("XCURSOR_THEME", themeName);

    // Update the Xcursor X resources
    runRdb(0);

    // Notify all applications that the cursor theme has changed
    KGlobalSettings::self()->emitChange(KGlobalSettings::CursorChanged);

    // Reload the standard cursors
    QStringList names;

    // Qt cursors
    names << "left_ptr"       << "up_arrow"      << "cross"      << "wait"
          << "left_ptr_watch" << "ibeam"         << "size_ver"   << "size_hor"
          << "size_bdiag"     << "size_fdiag"    << "size_all"   << "split_v"
          << "split_h"        << "pointing_hand" << "openhand"
          << "closedhand"     << "forbidden"     << "whats_this";

    // X core cursors
    names << "X_cursor"            << "right_ptr"           << "hand1"
          << "hand2"               << "watch"               << "xterm"
          << "crosshair"           << "left_ptr_watch"      << "center_ptr"
          << "sb_h_double_arrow"   << "sb_v_double_arrow"   << "fleur"
          << "top_left_corner"     << "top_side"            << "top_right_corner"
          << "right_side"          << "bottom_right_corner" << "bottom_side"
          << "bottom_left_corner"  << "left_side"           << "question_arrow"
          << "pirate";

    foreach (const QString &name, names)
    {
        QCursor cursor = theme->loadCursor(name);
        XFixesChangeCursorByName(x11Info().display(), cursor.handle(), QFile::encodeName(name));
    }

    return true;
#else
    Q_UNUSED(theme)
    return false;
#endif
}


void ThemePage::save()
{
    if (appliedIndex == selectedIndex() || !selectedIndex().isValid())
        return;

    const CursorTheme *theme = proxy->theme(selectedIndex());

    KConfig config("kcminputrc");
    KConfigGroup c(&config, "Mouse");
    c.writeEntry("cursorTheme", theme->name());
    c.sync();

    if (!applyTheme(theme))
    {
        KMessageBox::information(this,
                                 i18n("You have to restart KDE for these changes to take effect."),
                                 i18n("Cursor Settings Changed"), "CursorSettingsChanged");
    }

    appliedIndex = selectedIndex();
}


void ThemePage::load()
{
    view->selectionModel()->clear();
    // Get the name of the theme libXcursor currently uses
    QString currentTheme = XcursorGetTheme(x11Info().display());

    // Get the name of the theme KDE is configured to use
    KConfig c("kcminputrc");
    KConfigGroup cg(&c, "Mouse");
    currentTheme = cg.readEntry("cursorTheme", currentTheme);

    // Find the theme in the listview
    if (!currentTheme.isEmpty())
        appliedIndex = proxy->findIndex(currentTheme);
    else
        appliedIndex = proxy->defaultIndex();

    // Disable the listview and the buttons if we're in kiosk mode
    if (cg.isEntryImmutable("cursorTheme"))
    {
        view->setEnabled(false);
        installButton->setEnabled(false);
        removeButton->setEnabled(false);
    }

    const CursorTheme *theme = proxy->theme(appliedIndex);

    if (appliedIndex.isValid())
    {
        // Select the current theme
        selectRow(appliedIndex);
        view->scrollTo(appliedIndex, QListView::PositionAtCenter);

        // Update the preview widget as well
        preview->setTheme(theme);
    }

    if (!theme || !theme->isWritable())
        removeButton->setEnabled(false);
}


void ThemePage::defaults()
{
    view->selectionModel()->clear();
    QModelIndex defaultIndex = proxy->findIndex("Oxygen_Black");
    view->setCurrentIndex(defaultIndex);
}


void ThemePage::selectRow(int row) const
{
    // Create a selection that stretches across all columns
    QModelIndex from = proxy->index(row, 0);
    QModelIndex to   = proxy->index(row, model->columnCount() - 1);
    QItemSelection selection(from, to);

    view->selectionModel()->select(selection, QItemSelectionModel::Select);
}


void ThemePage::selectionChanged()
{
    QModelIndex selected = selectedIndex();

    if (selected.isValid())
    {
        const CursorTheme *theme = proxy->theme(selected);
        preview->setTheme(theme);
        removeButton->setEnabled(theme->isWritable());
    } else
        preview->setTheme(NULL);

    emit changed(appliedIndex != selected);
}

QModelIndex ThemePage::selectedIndex() const
{
    QModelIndexList selection = view->selectionModel()->selectedIndexes();
    if (!selection.isEmpty()) {
        return (selection.at(0));
    }
    return QModelIndex();
}

void ThemePage::getNewClicked()
{
    KNS3::DownloadDialog dialog("xcursor.knsrc", this);
    if (dialog.exec()) {
        KNS3::Entry::List list = dialog.changedEntries();
        if (list.count() > 0)
            model->refreshList();
    }
}

void ThemePage::installClicked()
{
    // Get the URL for the theme we're going to install
    KUrl url = KUrlRequesterDialog::getUrl(QString(), this, i18n("Drag or Type Theme URL"));

    if (url.isEmpty())
        return;

    QString tempFile;
    if (!KIO::NetAccess::download(url, tempFile, this))
    {
        QString text;

        if (url.isLocalFile())
            text = i18n("Unable to find the cursor theme archive %1.",
                        url.prettyUrl());
        else
            text = i18n("Unable to download the cursor theme archive; "
                        "please check that the address %1 is correct.",
                        url.prettyUrl());

        KMessageBox::sorry(this, text);
        return;
    }

    if (!installThemes(tempFile))
        KMessageBox::error(this, i18n("The file %1 does not appear to be a valid "
                                      "cursor theme archive.", url.fileName()));

    KIO::NetAccess::removeTempFile(tempFile);
}


void ThemePage::removeClicked()
{
    // We don't have to check if the current index is valid, since
    // the remove button will be disabled when there's no selection.
    const CursorTheme *theme = proxy->theme(selectedIndex());

    // Don't let the user delete the currently configured theme
    if (selectedIndex() == appliedIndex) {
        KMessageBox::sorry(this, i18n("<qt>You cannot delete the theme you are currently "
                "using.<br />You have to switch to another theme first.</qt>"));
        return;
    }

    // Get confirmation from the user
    QString question = i18n("<qt>Are you sure you want to remove the "
            "<i>%1</i> cursor theme?<br />"
            "This will delete all the files installed by this theme.</qt>",
            theme->title());

    int answer = KMessageBox::warningContinueCancel(this, question,
            i18n("Confirmation"), KStandardGuiItem::del());

    if (answer != KMessageBox::Continue)
        return;

    // Delete the theme from the harddrive
    KIO::del(KUrl(theme->path())); // async

    // Remove the theme from the model
    proxy->removeTheme(selectedIndex());

    // TODO:
    //  Since it's possible to substitute cursors in a system theme by adding a local
    //  theme with the same name, we shouldn't remove the theme from the list if it's
    //  still available elsewhere. We could add a
    //  bool CursorThemeModel::tryAddTheme(const QString &name), and call that, but
    //  since KIO::del() is an asynchronos operation, the theme we're deleting will be
    //  readded to the list again before KIO has removed it.
}


bool ThemePage::installThemes(const QString &file)
{
    KTar archive(file);

    if (!archive.open(QIODevice::ReadOnly))
        return false;

    const KArchiveDirectory *archiveDir = archive.directory();
    QStringList themeDirs;

    // Extract the dir names of the cursor themes in the archive, and
    // append them to themeDirs
    foreach(const QString &name, archiveDir->entries())
    {
        const KArchiveEntry *entry = archiveDir->entry(name);
        if (entry->isDirectory() && entry->name().toLower() != "default")
        {
            const KArchiveDirectory *dir = static_cast<const KArchiveDirectory *>(entry);
            if (dir->entry("index.theme") && dir->entry("cursors"))
                themeDirs << dir->name();
        }
    }

    if (themeDirs.isEmpty())
        return false;

    // The directory we'll install the themes to
    QString destDir = QDir::homePath() + "/.icons/";
    KStandardDirs::makeDir(destDir); // Make sure the directory exists

    // Process each cursor theme in the archive
    foreach (const QString &dirName, themeDirs)
    {
        QDir dest(destDir + dirName);
        if (dest.exists())
        {
            QString question = i18n("A theme named %1 already exists in your icon "
                    "theme folder. Do you want replace it with this one?", dirName);

            int answer = KMessageBox::warningContinueCancel(this, question,
                                i18n("Overwrite Theme?"),
                                KStandardGuiItem::overwrite());

            if (answer != KMessageBox::Continue)
                continue;

            // ### If the theme that's being replaced is the current theme, it
            //     will cause cursor inconsistencies in newly started apps.
        }

        // ### Should we check if a theme with the same name exists in a global theme dir?
        //     If that's the case it will effectively replace it, even though the global theme
        //     won't be deleted. Checking for this situation is easy, since the global theme
        //     will be in the listview. Maybe this should never be allowed since it might
        //     result in strange side effects (from the average users point of view). OTOH
        //     a user might want to do this 'upgrade' a global theme.

        const KArchiveDirectory *dir = static_cast<const KArchiveDirectory*>
                        (archiveDir->entry(dirName));
        dir->copyTo(dest.path());
        model->addTheme(dest);
    }

    archive.close();
    return true;
}

