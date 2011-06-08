/***************************************************************************
 *   Copyright (C) 2005-2006 by Stephen Leaf <smileaf@smileaf.org>         *
 *   Copyright (C) 2006 by Oswald Buddenhagen <ossi@kde.org>               *
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

#include "kdm-theme.h"

#include "helper.h"

#include <KDialog>
#include <KLocale>
#include <KMessageBox>
#include <KProgressDialog>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KTar>
#include <KUrlRequester>
#include <KUrlRequesterDialog>
#include <KTempDir>
#include <knewstuff3/downloaddialog.h>
#include <KDebug>
#include <KIO/Job>
#include <KIO/DeleteJob>
#include <KIO/NetAccess>

#include <QDir>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QPixmap>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWhatsThis>
#include <QWidget>

#include <unistd.h>

extern KConfig *config;

class ThemeData : public QTreeWidgetItem {
  public:
    ThemeData(QTreeWidget *parent = 0) : QTreeWidgetItem(parent) {}

    QString path;
    QString screenShot;
    QString copyright;
    QString description;
};

extern int handleActionReply(QWidget *parent, const KAuth::ActionReply &reply);

static int executeThemeAction(QWidget *parent,
        const QVariantMap &helperargs, QVariantMap *returnedData = 0)
{
    parent->setEnabled(false);

    KAuth::Action action("org.kde.kcontrol.kcmkdm.managethemes");
    action.setHelperID("org.kde.kcontrol.kcmkdm");
    action.setArguments(helperargs);

    KAuth::ActionReply reply = action.execute();

    parent->setEnabled(true);

    if (returnedData)
        *returnedData = reply.data();

    return handleActionReply(parent, reply);
}

KDMThemeWidget::KDMThemeWidget(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *ml = new QGridLayout(this);
    ml->setSizeConstraint(QLayout::SetMinAndMaxSize);
    ml->setSpacing(KDialog::spacingHint());
    ml->setMargin(KDialog::marginHint());

    themeWidget = new QTreeWidget(this);
    themeWidget->setHeaderLabels(QStringList()
        << i18nc("@title:column", "Theme")
        << i18nc("@title:column", "Author"));
    themeWidget->setSortingEnabled(true);
    themeWidget->sortItems(0, Qt::AscendingOrder);
    themeWidget->setRootIsDecorated(false);
    themeWidget->setWhatsThis(i18n("This is a list of installed themes.\n"
                                   "Click the one to be used."));

    ml->addWidget(themeWidget, 0, 0, 2, 4);

    preview = new QLabel(this);
    preview->setFixedSize(QSize(200, 150));
    preview->setScaledContents(true);
    preview->setWhatsThis(i18n("This is a screen shot of what KDM will look like."));

    ml->addWidget(preview, 0, 4);

    info = new QLabel(this);
    info->setMaximumWidth(200);
    info->setAlignment(Qt::AlignTop);
    info->setWordWrap(true);
    info->setWhatsThis(i18n("This contains information about the selected theme."));

    ml->addWidget(info, 1, 4);

    bInstallTheme = new QPushButton(i18nc("@action:button", "Install &new theme"), this);
    bInstallTheme->setWhatsThis(i18n("This will install a theme into the theme directory."));

    ml->addWidget(bInstallTheme, 2, 0);

    bRemoveTheme = new QPushButton(i18nc("@action:button", "&Remove theme"), this);
    bRemoveTheme->setWhatsThis(i18n("This will remove the selected theme."));

    ml->addWidget(bRemoveTheme, 2, 1);

    bGetNewThemes = new QPushButton(i18nc("@action:button", "&Get New Themes"), this);

    ml->addWidget(bGetNewThemes, 2, 2);

    connect(themeWidget, SIGNAL(itemSelectionChanged()), SLOT(themeSelected()));
    connect(bInstallTheme, SIGNAL(clicked()), SLOT(installNewTheme()));
    connect(bRemoveTheme, SIGNAL(clicked()), SLOT(removeSelectedThemes()));
    connect(bGetNewThemes, SIGNAL(clicked()), SLOT(getNewStuff()));

    themeDir = KStandardDirs::installPath("data") + "kdm/themes/";
    defaultTheme = 0;

    foreach (const QString &ent,
             QDir(themeDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Unsorted))
        insertTheme(themeDir + ent);
}

void KDMThemeWidget::selectTheme(const QString &path)
{
    for (int i = 0; i < themeWidget->topLevelItemCount(); i++) {
        ThemeData *td = (ThemeData *)themeWidget->topLevelItem(i);
        if (td->path == path) {
            themeWidget->clearSelection();
            td->setSelected(true);
            updateInfoView(td);
        }
    }
}

void KDMThemeWidget::load()
{
    selectTheme(config->group("X-*-Greeter").readEntry("Theme", QString(themeDir + "oxygen")));
}

void KDMThemeWidget::save()
{
    config->group("X-*-Greeter").writeEntry("Theme", defaultTheme ? defaultTheme->path : "");
}

void KDMThemeWidget::defaults()
{
    selectTheme(themeDir + "oxygen");

    emit changed();
}

void KDMThemeWidget::insertTheme(const QString &_theme)
{
    KConfig themeConfig(_theme + "/KdmGreeterTheme.desktop", KConfig::SimpleConfig);
    KConfigGroup themeGroup = themeConfig.group("KdmGreeterTheme");

    QString name = themeGroup.readEntry("Name");
    if (name.isEmpty())
        return;

    ThemeData *child = new ThemeData(themeWidget);
    child->setText(0, name);
    child->setText(1, themeGroup.readEntry("Author"));
    child->path = _theme;
    child->screenShot = themeGroup.readEntry("Screenshot");
    child->copyright = themeGroup.readEntry("Copyright");
    child->description = themeGroup.readEntry("Description");
}

void KDMThemeWidget::removeTheme(const QString &name)
{
    if (name.isEmpty())
        return;

    QList<QTreeWidgetItem *> ls = themeWidget->findItems(name, Qt::MatchExactly);
    if (!ls.isEmpty())
        delete ls.first();
}

void KDMThemeWidget::updateInfoView(ThemeData *theme)
{
    if (!(defaultTheme = theme)) {
        info->setText(QString());
        preview->setPixmap(QPixmap());
        preview->setText(QString());
    } else {
        info->setText(
            ((theme->copyright.length() > 0) ?
                i18n("<qt><strong>Copyright:</strong> %1<br/></qt>",
                     theme->copyright) : "") +
            ((theme->description.length() > 0) ?
                i18n("<qt><strong>Description:</strong> %1</qt>",
                     theme->description) : ""));
        preview->setPixmap(QString(theme->path + QLatin1Char('/') + theme->screenShot));
        preview->setText(theme->screenShot.isEmpty() ?
            "Screenshot not available" : QString());
    }
}

void KDMThemeWidget::checkThemesDir()
{
    QDir testDir(themeDir);
    if (!testDir.exists()) {
        QVariantMap helperargs;
        helperargs["subaction"] = Helper::CreateThemesDir;

        if (executeThemeAction(parentWidget(), helperargs))
            KMessageBox::sorry(this,
                    i18n("Unable to create folder %1", testDir.absolutePath()));
    }
}

// Theme installation code inspired by kcm_icon
void KDMThemeWidget::installNewTheme()
{
    QString url;
    KUrlRequesterDialog fileRequester(url, i18n("Drag or Type Theme URL"), this);
    fileRequester.urlRequester()->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly);
    KUrl themeURL = fileRequester.getUrl();
    if (themeURL.isEmpty())
        return;

#if 0
    if (themeURL.isLocalFile() && QDir(themeURL.toLocalFile()).exists()) {
        insertTheme(themeURL.toLocalFile());
        emit changed();
        return;
    }
#endif

    QString themeTmpFile;

    if (!KIO::NetAccess::download(themeURL, themeTmpFile, this)) {
        QString sorryText;
        if (themeURL.isLocalFile())
            sorryText = i18n("Unable to find the KDM theme archive %1.", themeURL.prettyUrl());
        else
            sorryText = i18n("Unable to download the KDM theme archive;\n"
                             "please check that address %1 is correct.", themeURL.prettyUrl());
        KMessageBox::sorry(this, sorryText);
        return;
    }

    QList<const KArchiveDirectory *> foundThemes;

    KTar archive(themeTmpFile);
    archive.open(QIODevice::ReadOnly);

    const KArchiveDirectory *archDir = archive.directory();
    foreach (const QString &ent, archDir->entries()) {
        const KArchiveEntry *possibleDir = archDir->entry(ent);
        if (possibleDir->isDirectory()) {
            const KArchiveDirectory *subDir =
                static_cast<const KArchiveDirectory *>(possibleDir);
            if (subDir->entry("KdmGreeterTheme.desktop"))
                foundThemes.append(subDir);
        }
    }

    if (foundThemes.isEmpty())
        KMessageBox::error(this, i18n("The file is not a valid KDM theme archive."));
    else {
        KProgressDialog progressDiag(this,
            i18nc("@title:window", "Installing KDM themes"), QString());
        progressDiag.setModal(true);
        progressDiag.setAutoClose(true);
        progressDiag.progressBar()->setMaximum(foundThemes.size());
        progressDiag.show();

        KTempDir themesTempDir(KStandardDirs::locateLocal("tmp", "kdmthemes"));

        QStringList themesTempDirs, themesDirs;
        foreach (const KArchiveDirectory *ard, foundThemes) {
            progressDiag.setLabelText(
                i18nc("@info:progress",
                      "<qt>Unpacking <strong>%1</strong> theme</qt>", ard->name()));

            themesTempDirs.append(themesTempDir.name() + ard->name());
            themesDirs.append(themeDir + ard->name());

            ard->copyTo(themesTempDirs.last(), true);

            progressDiag.progressBar()->setValue(progressDiag.progressBar()->value() + 1);
            if (progressDiag.wasCancelled())
                break;
        }

        progressDiag.setLabelText(i18nc("@info:progress", "<qt>Installing the themes</qt>"));

        QVariantMap helperargs;
        QVariantMap returnedData;
        helperargs["subaction"] = Helper::InstallThemes;
        helperargs["themes"] = themesTempDirs;

        if (executeThemeAction(parentWidget(), helperargs, &returnedData)) {
            QString errorMessage =
                i18n("There were errors while installing the following themes:\n");
            QStringList failedThemes = returnedData["failedthemes"].toStringList();
            foreach (const QString &path, failedThemes)
                errorMessage += path + '\n';
            KMessageBox::error(this, errorMessage);
        }

        foreach (const QString &dir, themesDirs)
            if (QDir(dir).exists())
                insertTheme(dir);
    }

    archive.close();

    KIO::NetAccess::removeTempFile(themeTmpFile);
    emit changed();
}

void KDMThemeWidget::themeSelected()
{
    if (themeWidget->selectedItems().size() > 0)
        updateInfoView((ThemeData *)(themeWidget->selectedItems().first()));
    else
        updateInfoView(0);
    bRemoveTheme->setEnabled(!themeWidget->selectedItems().isEmpty());
    emit changed();
}

void KDMThemeWidget::removeSelectedThemes()
{
    QStringList delList, nameList;
    QList<QTreeWidgetItem *> themes = themeWidget->selectedItems();
    if (themes.isEmpty())
        return;
    foreach (QTreeWidgetItem *itm, themes) {
        nameList.append(itm->text(0));
        delList.append(((ThemeData *)itm)->path);
    }
    if (KMessageBox::questionYesNoList(this,
            i18n("Are you sure you want to remove the following themes?"),
            nameList, i18nc("@title:window", "Remove themes?")) != KMessageBox::Yes)
        return;

    QVariantMap helperargs;
    QVariantMap returnedData;
    helperargs["subaction"] = Helper::RemoveThemes;
    helperargs["themes"] = delList;

    int code = executeThemeAction(parentWidget(), helperargs, &returnedData);
    delList = returnedData["themes"].toStringList();

    if (code) {
        QString errorMessage =
            i18n("There were errors while deleting the following themes:\n");
        foreach (const QString &path, delList)
            if (!path.isNull())
                errorMessage += path + '\n';
        KMessageBox::error(this, errorMessage);
    }

    for (int i = 0; i < delList.size(); ++i)
        if (delList.at(i).isEmpty())
            themeWidget->takeTopLevelItem(themeWidget->indexOfTopLevelItem(themes.at(i)));
}

void KDMThemeWidget::getNewStuff()
{
    KNS3::DownloadDialog dialog("kdm.knsrc", this);
    dialog.exec();
    KNS3::Entry::List entries = dialog.changedEntries();
    for (int i = 0; i < entries.size(); i ++) {
        if (entries.at(i).status() == KNS3::Entry::Installed) {
            if (!entries.at(i).installedFiles().isEmpty()) {
                QString name = entries.at(i).installedFiles().at(0).section('/', -2, -2);
                insertTheme(themeDir + name);
            }
        } else if (entries.at(i).status() == KNS3::Entry::Deleted) {
            if (!entries.at(i).uninstalledFiles().isEmpty()) {
                QString name = entries.at(i).uninstalledFiles().at(0).section('/', -2, -2);
                removeTheme(themeDir + name);
            }
        }
    }
}

#include "kdm-theme.moc"
