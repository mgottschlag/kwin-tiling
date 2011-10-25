/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2008 by Petri Damsten <damu@iki.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <QTimer>
#include <QPixmap>
#include <QStringList>

#include <Plasma/Wallpaper>
#include <Plasma/Package>

#include "ui_imageconfig.h"
#include "ui_slideshowconfig.h"

class QPropertyAnimation;

class KDirWatch;
class KFileDialog;
class KJob;

namespace KNS3 {
    class DownloadDialog;
}

class BackgroundListModel;

class Image : public Plasma::Wallpaper
{
    Q_OBJECT
    Q_PROPERTY(qreal fadeValue READ fadeValue WRITE setFadeValue)

    public:
        Image(QObject* parent, const QVariantList& args);
        ~Image();

        virtual void save(KConfigGroup &config);
        virtual void paint(QPainter* painter, const QRectF& exposedRect);
        virtual QWidget* createConfigurationInterface(QWidget* parent);
        void updateScreenshot(QPersistentModelIndex index);
        qreal fadeValue() const;

    signals:
        void settingsChanged(bool);

    protected slots:
        void removeWallpaper(QString name);
        void timeChanged(const QTime& time);
        void positioningChanged(int index);
        void addDir();
        void removeDir();
        void getNewWallpaper();
        void colorChanged(const QColor& color);
        void pictureChanged(const QModelIndex &);
        void wallpaperBrowseCompleted();
        void nextSlide();
        /**
         * Open the current slide in the default image application
         */
        void openSlide();
        void updateBackground(const QImage &img);
        void showFileDialog();
        void setFadeValue(qreal value);
        void configWidgetDestroyed();
        void startSlideshow();
        void modified();
        void fileDialogFinished();
        void addUrl(const KUrl &url, bool setAsCurrent);
        void addUrls(const KUrl::List &urls);
        void setWallpaper(const QString &path);
        void setWallpaperRetrieved(KJob *job);
        void addWallpaperRetrieved(KJob *job);
        void newStuffFinished();
        void setConfigurationInterfaceModel();
        void updateDirs();
        void addDirFromSelectionDialog();
        void systemCheckBoxToggled(bool);
        void downloadedCheckBoxToggled(bool);
        void imageFileAltered(const QString &path);
        void backgroundsFound(const QStringList &paths, const QString &token);

    protected:
        void init(const KConfigGroup &config);
        void renderWallpaper(const QString& image = QString());
        void suspendStartup(bool suspend); // for ksmserver
        void calculateGeometry();
        void setSingleImage();
        void updateWallpaperActions();
        void useSingleImageDefaults();

    private:
        static bool s_startupResumed;
        static bool s_startupSuspended;

        int m_delay;
        QStringList m_dirs;
        QString m_wallpaper;
        QColor m_color;
        QStringList m_usersWallpapers;
        KDirWatch *m_fileWatch;

        QWidget* m_configWidget;
        Ui::ImageConfig m_uiImage;
        Ui::SlideshowConfig m_uiSlideshow;
        QString m_mode;
        Plasma::Package *m_wallpaperPackage;
        QStringList m_slideshowBackgrounds;
        QTimer m_timer;
        QPixmap m_pixmap;
        QPixmap m_oldPixmap;
        QPixmap m_oldFadedPixmap;
        int m_currentSlide;
        qreal m_fadeValue;
        QPropertyAnimation *m_animation;
        BackgroundListModel *m_model;
        KFileDialog *m_dialog;
        QSize m_size;
        QString m_img;
        QDateTime m_previousModified;
        bool m_randomize;
        QWeakPointer<KNS3::DownloadDialog> m_newStuffDialog;
        QString m_findToken;

        QAction* m_nextWallpaperAction;
        QAction* m_openImageAction;
};

#endif
