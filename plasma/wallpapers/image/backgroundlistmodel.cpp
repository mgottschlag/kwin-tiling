/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgroundlistmodel.h"

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QProgressBar>

#include <KGlobal>
#include <KProgressDialog>
#include <KStandardDirs>

#include "backgroundpackage.h"
#include "backgrounddelegate.h"

BackgroundListModel::BackgroundListModel(float ratio, QObject *listener)
    : m_listener(listener),
      m_ratio(ratio),
      m_size(0,0),
      m_resizeMethod(Plasma::Wallpaper::ScaledResize)
{
    connect(&m_dirwatch, SIGNAL(deleted(QString)), listener, SLOT(removeBackground(QString)));
}

void BackgroundListModel::removeBackground(const QString &path)
{
    int index;
    while ((index = indexOf(path)) != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_packages.removeAt(index);
        endRemoveRows();
    }
}

void BackgroundListModel::reload()
{
    reload(QStringList());
}

void BackgroundListModel::reload(const QStringList& selected)
{
    QStringList dirs = KGlobal::dirs()->findDirs("wallpaper", "");
    QList<Background *> tmp;

    foreach (const QString &file, selected) {
        if (!contains(file) && QFile::exists(file)) {
            tmp << new BackgroundFile(file, m_ratio);
        }
    }

    {
        KProgressDialog progressDialog;
        initProgressDialog(&progressDialog);

        foreach (const QString &dir, dirs) {
            tmp += findAllBackgrounds(this, dir, m_ratio, &progressDialog);
        }
    }

    // add new files to dirwatch
    foreach (Background *b, tmp) {
        //TODO: packages need to be added to the dir watch as well
        if (!m_dirwatch.contains(b->path())) {
            m_dirwatch.addFile(b->path());
        }
    }

    if (!tmp.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, tmp.size() - 1);
        m_packages = tmp + m_packages;
        endInsertRows();
    }
}

void BackgroundListModel::addBackground(const QString& path) {
    if (!contains(path)) {
        if (!m_dirwatch.contains(path)) {
            m_dirwatch.addFile(path);
        }
        beginInsertRows(QModelIndex(), 0, 0);
        m_packages.prepend(new BackgroundFile(path, m_ratio));
        endInsertRows();
    }
}

int BackgroundListModel::indexOf(const QString &path) const
{
    for (int i = 0; i < m_packages.size(); i++) {
        if (path.startsWith(m_packages[i]->path())) {
            return i;
        }
    }
    return -1;
}

bool BackgroundListModel::contains(const QString &path) const
{
    return indexOf(path) != -1;
}

BackgroundListModel::~BackgroundListModel()
{
    foreach (Background* pkg, m_packages) {
        delete pkg;
    }
}

int BackgroundListModel::rowCount(const QModelIndex &) const
{
    return m_packages.size();
}

QVariant BackgroundListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_packages.size()) {
        return QVariant();
    }

    Background *b = package(index.row());
    if (!b) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return b->title();
    case BackgroundDelegate::ScreenshotRole: {
        QPixmap pix = b->screenshot();
        if (pix.isNull() && !b->screenshotGenerationStarted()) {
            connect(b, SIGNAL(screenshotDone(QPersistentModelIndex)),
                    m_listener, SLOT(updateScreenshot(QPersistentModelIndex)),
                    Qt::QueuedConnection);
            b->generateScreenshot(index);
        }
        return pix;
    }
    case BackgroundDelegate::AuthorRole:
        return b->author();
    case BackgroundDelegate::ResolutionRole:{
        QSize size = b->bestSize(m_size, m_resizeMethod);
        if (size.isValid()) {
            return QString("%1x%2").arg(size.width()).arg(size.height());
        }
        return QString();
    }
    default:
        return QVariant();
    }
}

Background* BackgroundListModel::package(int index) const
{
    return m_packages.at(index);
}

void BackgroundListModel::initProgressDialog(KProgressDialog *progress)
{
    progress->setAllowCancel(false);
    progress->setModal(true);
    progress->setLabelText(i18n("Finding images for the wallpaper slideshow."));
    progress->progressBar()->setRange(0, 0);
}

QList<Background *> BackgroundListModel::findAllBackgrounds(const BackgroundContainer *container,
                                                            const QString &path, float ratio,
                                                            KProgressDialog *progress)
{
    KProgressDialog *myProgress = 0;
    if (!progress) {
        myProgress = progress = new KProgressDialog;
        initProgressDialog(myProgress);
    }

    //kDebug() << "looking for" << path;
    QList<Background *> res;

    // get all packages in this directory
    //kDebug() << "getting packages";
    QStringList packages = Plasma::Package::listInstalledPaths(path);
    QSet<QString> validPackages;
    foreach (const QString &packagePath, packages) {
        QCoreApplication::processEvents();
        progress->setLabelText(i18n("Finding images for the wallpaper slideshow.") + "\n\n" +
                               i18n("Testing %1 for a Wallpaper package", packagePath));
        std::auto_ptr<Background> pkg(new BackgroundPackage(path + packagePath, ratio));
        if (pkg->isValid() && (!container || !container->contains(pkg->path()))) {
            progress->setLabelText(i18n("Finding images for the wallpaper slideshow.") + "\n\n" +
                                   i18n("Adding wallpaper package in %1", packagePath));
            res.append(pkg.release());
            //kDebug() << "    adding valid package:" << packagePath;
            validPackages << packagePath;
        }
    }

    // search normal wallpapers
    //kDebug() << "listing normal files";
    QDir dir(path);
    QStringList filters;
    filters << "*.png" << "*.jpeg" << "*.jpg" << "*.svg" << "*.svgz";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::Readable);
    QFileInfoList files = dir.entryInfoList();
    foreach (const QFileInfo &wp, files) {
        QCoreApplication::processEvents();
        if (!container || !container->contains(wp.filePath())) {
            //kDebug() << "     adding image file" << wp.filePath();
            progress->setLabelText(i18n("Finding images for the wallpaper slideshow.") + "\n\n" +
                                   i18n("Adding image %1", wp.filePath()));
            res.append(new BackgroundFile(wp.filePath(), ratio));
        }
    }

    // now recurse the dirs, skipping ones we found packages in
    //kDebug() << "recursing dirs";
    dir.setFilter(QDir::AllDirs | QDir::Readable);
    files = dir.entryInfoList();

    foreach (const QFileInfo &wp, files) {
        QCoreApplication::processEvents();
        QString name = wp.fileName();
        if (name != "." && name != ".." && !validPackages.contains(wp.fileName())) {
            //kDebug() << "    " << name << wp.filePath();
            res += findAllBackgrounds(container, wp.filePath(), ratio, progress);
        }
    }

    //kDebug() << "completed.";
    delete myProgress;
    return res;
}

void BackgroundListModel::setWallpaperSize(QSize size)
{
    m_size = size;
}

void BackgroundListModel::setResizeMethod(Plasma::Wallpaper::ResizeMethod resizeMethod)
{
    m_resizeMethod = resizeMethod;
}

