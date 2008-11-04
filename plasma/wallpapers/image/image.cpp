/*
  Copyright (c) 2007 by Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2007 by Aaron Seigo <aseigo@kde.org>
  Copyright (c) 2008 by Alexis Ménard <darktears31@gmail.com>
  Copyright (c) 2008 by Petri Damsten <damu@iki.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "image.h"

#include <QPainter>
#include <QFile>

#include <KDirSelectDialog>
#include <KDirWatch>
#include <KFileDialog>
#include <KGlobalSettings>
#include <KImageFilePreview>
#include <KNS/Engine>
#include <KRandom>
#include <KStandardDirs>

#include <Plasma/Theme>
#include "backgroundlistmodel.h"
#include "backgrounddelegate.h"
#include "ksmserver_interface.h"


Image::Image(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args),
      m_currentSlide(-1),
      m_dialog(0),
      m_rendererToken(-1),
      m_randomize(true)
{
    qRegisterMetaType<QImage>("QImage");
    connect(&m_renderer, SIGNAL(done(int, QImage)), this, SLOT(updateBackground(int, QImage)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(nextSlide()));
}

Image::~Image()
{
    qDeleteAll(m_slideshowBackgrounds);
    delete m_dialog;
}

void Image::init(const KConfigGroup &config)
{
    m_timer.stop();
    m_mode = renderingMode().name();
    calculateGeometry();

    m_delay = config.readEntry("slideTimer", 600);
    m_resizeMethod = (Background::ResizeMethod)config.readEntry("wallpaperposition",
                                                                (int)Background::Scale);
    m_wallpaper = config.readEntry("wallpaper", QString());
    if (m_wallpaper.isEmpty()) {
        m_wallpaper = Plasma::Theme::defaultTheme()->wallpaperPath();
        int index = m_wallpaper.indexOf("/contents/images/");
        if (index > -1) { // We have file from package -> get path to package
            m_wallpaper = m_wallpaper.left(index);
        }
    }
    m_color = config.readEntry("wallpapercolor", QColor(56, 111, 150));
    m_usersWallpapers = config.readEntry("userswallpapers", QStringList());
    m_dirs = config.readEntry("slidepaths", QStringList());
    if (m_dirs.isEmpty()) {
        m_dirs << KStandardDirs::installPath("wallpaper");
    }

    if (m_mode == "SingleImage") {
        setSingleImage();
    } else {
        startSlideshow();
    }
}

void Image::save(KConfigGroup &config)
{
    config.writeEntry("slideTimer", m_delay);
    config.writeEntry("wallpaperposition", (int)m_resizeMethod);
    config.writeEntry("slidepaths", m_dirs);
    config.writeEntry("wallpaper", m_wallpaper);
    config.writeEntry("wallpapercolor", m_color);
    config.writeEntry("userswallpapers", m_usersWallpapers);
}

QWidget* Image::createConfigurationInterface(QWidget* parent)
{
    m_widget = new QWidget(parent);

    if (m_mode == "SingleImage") {
        m_uiImage.setupUi(m_widget);

        m_model = new BackgroundListModel(m_ratio, this);
        m_uiImage.m_view->setModel(m_model);
        m_uiImage.m_view->setItemDelegate(new BackgroundDelegate(m_uiImage.m_view->view(),
                                                                 m_ratio, this));
        m_uiImage.m_view->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_model->reload(m_usersWallpapers);
        int index = m_model->indexOf(m_wallpaper);
        if (index != -1) {
            m_uiImage.m_view->setCurrentIndex(index);
            Background *b = m_model->package(index);
            if (b) {
                fillMetaInfo(b);
            }
        }
        connect(m_uiImage.m_view, SIGNAL(currentIndexChanged(int)), this, SLOT(pictureChanged(int)));

        m_uiImage.m_pictureUrlButton->setIcon(KIcon("document-open"));
        connect(m_uiImage.m_pictureUrlButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));

        m_uiImage.m_emailLine->setTextInteractionFlags(Qt::TextSelectableByMouse);

        m_uiImage.m_resizeMethod->addItem(i18n("Scaled & Cropped"), Background::ScaleCrop);
        m_uiImage.m_resizeMethod->addItem(i18n("Scaled"), Background::Scale);
        m_uiImage.m_resizeMethod->addItem(i18n("Scaled, keep proportions"), Background::Maxpect);
        m_uiImage.m_resizeMethod->addItem(i18n("Centered"), Background::Center);
        m_uiImage.m_resizeMethod->addItem(i18n("Tiled"), Background::Tiled);
        m_uiImage.m_resizeMethod->addItem(i18n("Center Tiled"), Background::CenterTiled);
        for (int i = 0; i < m_uiImage.m_resizeMethod->count(); ++i) {
            if (m_resizeMethod == m_uiImage.m_resizeMethod->itemData(i).value<int>()) {
                m_uiImage.m_resizeMethod->setCurrentIndex(i);
                break;
            }
        }
        connect(m_uiImage.m_resizeMethod, SIGNAL(currentIndexChanged(int)),
                this, SLOT(positioningChanged(int)));

        m_uiImage.m_color->setColor(m_color);
        connect(m_uiImage.m_color, SIGNAL(changed(const QColor&)), this, SLOT(colorChanged(const QColor&)));

        connect(m_uiImage.m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));
    } else {
        m_uiSlideshow.setupUi(m_widget);

        m_uiSlideshow.m_dirlist->clear();
        foreach (const QString &dir, m_dirs) {
            m_uiSlideshow.m_dirlist->addItem(dir);
        }
        m_uiSlideshow.m_dirlist->setCurrentRow(0);
        updateDirs();
        m_uiSlideshow.m_addDir->setIcon(KIcon("list-add"));
        connect(m_uiSlideshow.m_addDir, SIGNAL(clicked()), this, SLOT(slotAddDir()));
        m_uiSlideshow.m_removeDir->setIcon(KIcon("list-remove"));
        connect(m_uiSlideshow.m_removeDir, SIGNAL(clicked()), this, SLOT(slotRemoveDir()));

        QTime time(0, 0, 0);
        time = time.addSecs(m_delay);
        m_uiSlideshow.m_slideshowDelay->setTime(time);
        m_uiSlideshow.m_slideshowDelay->setMinimumTime(QTime(0, 0, 30));
        connect(m_uiSlideshow.m_slideshowDelay, SIGNAL(timeChanged(const QTime&)),
                this, SLOT(timeChanged(const QTime&)));

        m_uiSlideshow.m_resizeMethod->addItem(i18n("Scaled & Cropped"), Background::ScaleCrop);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Scaled"), Background::Scale);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Maxpect"), Background::Maxpect);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Centered"), Background::Center);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Tiled"), Background::Tiled);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Center Tiled"), Background::CenterTiled);
        for (int i = 0; i < m_uiSlideshow.m_resizeMethod->count(); ++i) {
            if (m_resizeMethod == m_uiSlideshow.m_resizeMethod->itemData(i).value<int>()) {
                m_uiSlideshow.m_resizeMethod->setCurrentIndex(i);
                break;
            }
        }
        connect(m_uiSlideshow.m_resizeMethod, SIGNAL(currentIndexChanged(int)),
                this, SLOT(positioningChanged(int)));

        m_uiSlideshow.m_color->setColor(m_color);
        connect(m_uiSlideshow.m_color, SIGNAL(changed(const QColor&)), this, SLOT(colorChanged(const QColor&)));
    }

    return m_widget;
}

void Image::calculateGeometry()
{
    m_size = boundingRect().size().toSize();
    m_renderer.setSize(m_size);
    m_ratio = boundingRect().width() / boundingRect().height();
    m_renderer.setRatio(m_ratio);
}

void Image::paint(QPainter *painter, const QRectF& exposedRect)
{
    // Check if geometry changed
    //kDebug() << m_size << boundingRect().size().toSize();
    if (m_size != boundingRect().size().toSize()) {
        calculateGeometry();
        if (!m_img.isEmpty()) { // We have previous image
            render();
            //kDebug() << "re-rendering";
            return;
        }
    }
    if (m_pixmap.isNull()) {
        painter->fillRect(exposedRect, QBrush(m_color));
        //kDebug() << "pixmap null";
        return;
    }
    painter->save();

    if (painter->worldMatrix() == QMatrix()) {
        // draw the background untransformed when possible;(saves lots of per-pixel-math)
        painter->resetTransform();
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    // for pixmaps we draw only the exposed part (untransformed since the
    // bitmapBackground already has the size of the viewport)
    painter->drawPixmap(exposedRect, m_pixmap, exposedRect);

    // restore transformation and composition mode
    painter->restore();
}

void Image::timeChanged(const QTime& time)
{
    m_delay = QTime(0, 0, 0).secsTo(time);
    if (!m_slideshowBackgrounds.isEmpty()) {
        m_timer.start(m_delay * 1000);
    }
}

void Image::slotAddDir()
{
    KUrl empty;
    KDirSelectDialog dialog(empty, true, m_widget);
    if (dialog.exec()) {
        m_uiSlideshow.m_dirlist->addItem(dialog.url().path());
        updateDirs();
        startSlideshow();
    }
}

void Image::slotRemoveDir()
{
    int row = m_uiSlideshow.m_dirlist->currentRow();
    if (row != -1) {
        m_uiSlideshow.m_dirlist->takeItem(row);
        updateDirs();
        startSlideshow();
    }
}

void Image::updateDirs()
{
    m_dirs.clear();
    for (int i = 0; i < m_uiSlideshow.m_dirlist->count(); i++) {
        m_dirs.append(m_uiSlideshow.m_dirlist->item(i)->text());
    }

    if (m_uiSlideshow.m_dirlist->count() == 0) {
        m_uiSlideshow.m_dirlist->hide();
    } else {
        const int itemHeight = m_uiSlideshow.m_dirlist->visualItemRect(m_uiSlideshow.m_dirlist->item(0)).height();
        const int vMargin = m_uiSlideshow.m_dirlist->height() - m_uiSlideshow.m_dirlist->viewport()->height();

        if (m_uiSlideshow.m_dirlist->count() <= 6) {
            m_uiSlideshow.m_dirlist->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            m_uiSlideshow.m_dirlist->setFixedHeight(itemHeight * m_uiSlideshow.m_dirlist->count() + vMargin);
        } else {
            m_uiSlideshow.m_dirlist->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }

        if (!m_uiSlideshow.m_dirlist->isVisible()) {
            m_uiSlideshow.m_dirlist->setCurrentRow(0);
        }

        m_uiSlideshow.m_dirlist->show();
        m_uiSlideshow.gridLayout->invalidate();
    }

    m_uiSlideshow.m_removeDir->setEnabled(m_uiSlideshow.m_dirlist->currentRow() != -1);
}

void Image::setSingleImage()
{
    QString img;
    BackgroundPackage b(m_wallpaper, m_ratio);

    img = b.findBackground(m_size, m_resizeMethod); // isValid() returns true for jpg?
    kDebug() << img << m_wallpaper;
    if (img.isEmpty()) {
        img = m_wallpaper;
    }
    render(img);
}

void Image::startSlideshow()
{
    // populate background list
    m_timer.stop();
    qDeleteAll(m_slideshowBackgrounds);
    m_slideshowBackgrounds.clear();
    foreach (const QString& dir, m_dirs) {
        m_slideshowBackgrounds += BackgroundListModel::findAllBackgrounds(0, dir, m_ratio);
    }

    // start slideshow
    if (m_slideshowBackgrounds.isEmpty()) {
        m_pixmap = QPixmap();
        emit update(boundingRect());
    } else {
        m_currentSlide = -1;
        nextSlide();
        m_timer.start(m_delay * 1000);
    }
}

void Image::getNewWallpaper()
{
    KNS::Engine engine(m_widget);
    if (engine.init("wallpaper.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(m_widget);

        if (entries.size() > 0) {
            m_model->reload();
        }
    }
}

void Image::colorChanged(const QColor& color)
{
    m_color = color;
    setSingleImage();
}

void Image::pictureChanged(int index)
{
    if (index == -1) {
        return;
    }
    Background *b = m_model->package(index);
    if (!b) {
        return;
    }
    fillMetaInfo(b);
    m_wallpaper = b->path();
    setSingleImage();
}

void Image::positioningChanged(int index)
{
    if (m_mode == "SingleImage") {
        m_resizeMethod =
                (Background::ResizeMethod)m_uiImage.m_resizeMethod->itemData(index).value<int>();
        setSingleImage();
    } else {
        m_resizeMethod =
                (Background::ResizeMethod)m_uiSlideshow.m_resizeMethod->itemData(index).value<int>();
        startSlideshow();
    }
}

void Image::fillMetaInfo(Background *b)
{
    // Prepare more user-friendly forms of some pieces of data.
    // - license by config is more a of a key value,
    //   try to get the proper name if one of known licenses.

    // not needed for now...
    //QString license = b->license();
    /*
    KAboutLicense knownLicense = KAboutLicense::byKeyword(license);
    if (knownLicense.key() != KAboutData::License_Custom) {
        license = knownLicense.name(KAboutData::ShortName);
    }
    */
    // - last ditch attempt to localize author's name, if not such by config
    //   (translators can "hook" names from outside if resolute enough).
    if (!b->author().isEmpty()) {
        QString author = i18nc("Wallpaper info, author name", "%1", b->author());
        m_uiImage.m_authorLabel->setAlignment(Qt::AlignRight);
        setMetadata(m_uiImage.m_authorLine, author);
    } else {
        setMetadata(m_uiImage.m_authorLine, QString());
        m_uiImage.m_authorLabel->setAlignment(Qt::AlignLeft);
    }
    setMetadata(m_uiImage.m_licenseLine, QString());
    setMetadata(m_uiImage.m_emailLine, QString());
    m_uiImage.m_emailLabel->hide();
    m_uiImage.m_licenseLabel->hide();
}

bool Image::setMetadata(QLabel *label, const QString &text)
{
    if (text.isEmpty()) {
        label->hide();
        return false;
    }
    else {
        label->show();
        label->setText(text);
        return true;
    }
}

void Image::showFileDialog()
{
    if (!m_dialog) {
        m_dialog = new KFileDialog(KUrl(), "*.png *.jpeg *.jpg *.svg *.svgz", m_widget);
        m_dialog->setOperationMode(KFileDialog::Opening);
        m_dialog->setInlinePreviewShown(true);
        m_dialog->setCaption(i18n("Select Wallpaper Image File"));
        m_dialog->setModal(false);
    }
    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();

    connect(m_dialog, SIGNAL(okClicked()), this, SLOT(browse()));
}

void Image::browse()
{
    QString wallpaper = m_dialog->selectedFile();
    disconnect(m_dialog, SIGNAL(okClicked()), this, SLOT(browse()));

    if (wallpaper.isEmpty()) {
        return;
    }

    // add background to the model
    m_model->addBackground(wallpaper);

    // select it
    int index = m_model->indexOf(wallpaper);
    if (index != -1) {
        m_uiImage.m_view->setCurrentIndex(index);
    }
    // save it
    m_usersWallpapers << wallpaper;
}

void Image::nextSlide()
{
    if (m_slideshowBackgrounds.size() < 1) {
        return;
    }

    QString previous;
    if (m_currentSlide >= 0 && m_currentSlide < m_slideshowBackgrounds.size()) {
        previous = m_slideshowBackgrounds[m_currentSlide]->findBackground(m_size, m_resizeMethod);
    }

    if (m_randomize) {
        m_currentSlide = KRandom::random() % m_slideshowBackgrounds.size();
    } else if (++m_currentSlide >= m_slideshowBackgrounds.size()) {
        m_currentSlide = 0;
    }

    QString current = m_slideshowBackgrounds[m_currentSlide]->findBackground(m_size, m_resizeMethod);
    if (current == previous) {
        QFileInfo info(previous);
        if (m_previousModified == info.lastModified()) {
            // it hasn't changed since we last loaded it, so try the next one instead
            if (m_slideshowBackgrounds.count() == 1) {
                // only one slide, same image, continue on
                return;
            }

            if (++m_currentSlide >= m_slideshowBackgrounds.size()) {
                m_currentSlide = 0;
            }

            current = m_slideshowBackgrounds[m_currentSlide]->findBackground(m_size, m_resizeMethod);
        }
    }

    QFileInfo info(current);
    m_previousModified = info.lastModified();

    render(current);
}

void Image::render(const QString& image)
{
    if (!image.isEmpty()) {
        m_img = image;
    }
    // else re-render previous image
    m_rendererToken = m_renderer.render(m_img, m_color, m_resizeMethod,
                                        Qt::SmoothTransformation);
    suspendStartup(true); // during KDE startup, make ksmserver until the wallpaper is ready
}

void Image::updateBackground(int token, const QImage &img)
{
    if (m_rendererToken == token) {
        m_pixmap = QPixmap::fromImage(img);
        emit update(boundingRect());
        suspendStartup(false);
    }
}

void Image::suspendStartup(bool suspend)
{
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());
    const QString startupID("desktop wallaper");
    if (suspend) {
        ksmserver.suspendStartup(startupID);
    } else {
        ksmserver.resumeStartup(startupID);
    }
}

void Image::updateScreenshot(QPersistentModelIndex index)
{
    m_uiImage.m_view->view()->update(index);
}

void Image::removeBackground(const QString &path)
{
    m_model->removeBackground(path);
}

#include "image.moc"
