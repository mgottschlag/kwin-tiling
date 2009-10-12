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

class KFileDialog;
class KJob;

class BackgroundListModel;

class Image : public Plasma::Wallpaper
{
    Q_OBJECT
    public:
        Image(QObject* parent, const QVariantList& args);
        ~Image();

        virtual void save(KConfigGroup &config);
        virtual void paint(QPainter* painter, const QRectF& exposedRect);
        virtual QWidget* createConfigurationInterface(QWidget* parent);
        void updateScreenshot(QPersistentModelIndex index);

    signals:
        void settingsChanged(bool);

    protected slots:
        void timeChanged(const QTime& time);
        void positioningChanged(int index);
        void slotAddDir();
        void slotRemoveDir();
        void getNewWallpaper();
        void colorChanged(const QColor& color);
        void pictureChanged(const QModelIndex &);
        void wallpaperBrowseCompleted();
        void nextSlide();
        void updateBackground(const QImage &img);
        void showFileDialog();
        void updateFadedImage(qreal frame);
        void configWidgetDestroyed();
        void startSlideshow();
        void modified();
        void fileDialogFinished();
        void setWallpaper(const KUrl &url);
        void setWallpaper(const QString &path);
        void wallpaperRetrieved(KJob *job);

    protected:
        void init(const KConfigGroup &config);
        void updateDirs();
        void fillMetaInfo(Plasma::Package *b);
        bool setMetadata(QLabel *label, const QString &text);
        void renderWallpaper(const QString& image = QString());
        void suspendStartup(bool suspend); // for ksmserver
        void calculateGeometry();
        void setSingleImage();

    private:
        int m_delay;
        Plasma::Wallpaper::ResizeMethod m_resizeMethod;
        QStringList m_dirs;
        QString m_wallpaper;
        QColor m_color;
        QStringList m_usersWallpapers;

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
        BackgroundListModel *m_model;
        KFileDialog *m_dialog;
        QSize m_size;
        QString m_img;
        QDateTime m_previousModified;
        bool m_randomize;
        bool m_startupResumed;

        QAction* nextWallpaperAction;
};

#endif
