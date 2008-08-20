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
#include <plasma/wallpaper.h>
#include "backgroundpackage.h"
#include "renderthread.h"
#include "ui_imageconfig.h"
#include "ui_slideshowconfig.h"

class KFileDialog;
class BackgroundContainer;
class BackgroundListModel;

class Image : public Plasma::Wallpaper
{
    Q_OBJECT
    public:
        Image(QObject* parent, const QVariantList& args);
        ~Image();

        virtual void save(KConfigGroup config);
        virtual void paint(QPainter* painter, const QRectF& exposedRect);
        virtual QWidget* createConfigurationInterface(QWidget* parent);

    protected slots:
        void timeChanged(const QTime& time);
        void positioningChanged(int index);
        void slotAddDir();
        void slotRemoveDir();
        void getNewWallpaper();
        void colorChanged(QColor color);
        void pictureChanged(int index);
        void browse();
        void nextSlide();
        void updateBackground(int token, const QImage &img);
        void showFileDialog();
        void updateScreenshot(QPersistentModelIndex index);
        void removeBackground(const QString &path);

    protected:
        void init(const KConfigGroup &config);
        void updateDirs();
        void startSlideshow();
        void fillMetaInfo(Background* b);
        bool setMetadata(QLabel *label, const QString &text);
        void render(const QString& image = QString());
        void suspendStartup(bool suspend); // for ksmserver
        void calculateGeometry();
        void setSingleImage();

    private:
        int m_delay;
        Background::ResizeMethod m_resizeMethod;
        QStringList m_dirs;
        QString m_wallpaper;
        QColor m_color;
        QStringList m_usersWallpapers;

        QWidget* m_widget;
        Ui::ImageConfig m_uiImage;
        Ui::SlideshowConfig m_uiSlideshow;
        QString m_mode;
        QList<Background *> m_slideshowBackgrounds;
        QTimer m_timer;
        QPixmap m_pixmap;
        int m_currentSlide;
        qreal m_ratio;
        BackgroundListModel *m_model;
        KFileDialog *m_dialog;
        RenderThread m_renderer;
        int m_rendererToken;
        QSize m_size;
        QString m_img;
};

K_EXPORT_PLASMA_WALLPAPER(image, Image)

#endif
