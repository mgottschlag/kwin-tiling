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
#include <KRandom>
#include <KStandardDirs>
#include <KIO/Job>
#include <KNS/Engine>

#include <Plasma/Theme>
#include <Plasma/Animator>
#include "backgroundlistmodel.h"
#include "backgrounddelegate.h"
#include "ksmserver_interface.h"

K_EXPORT_PLASMA_WALLPAPER(image, Image)

Image::Image(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args),
      m_configWidget(0),
      m_wallpaperPackage(0),
      m_currentSlide(-1),
      m_model(0),
      m_dialog(0),
      m_randomize(true),
      m_startupResumed(false)
{
    connect(this, SIGNAL(renderCompleted(QImage)), this, SLOT(updateBackground(QImage)));
    connect(this, SIGNAL(urlDropped(KUrl)), this, SLOT(setWallpaper(KUrl)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(nextSlide()));
}

Image::~Image()
{
}

void Image::init(const KConfigGroup &config)
{
    suspendStartup(true); // during KDE startup, make ksmserver until the wallpaper is ready
  
    m_timer.stop();
    m_mode = renderingMode().name();
    calculateGeometry();

    m_delay = config.readEntry("slideTimer", 10);
    m_resizeMethod = (ResizeMethod)config.readEntry("wallpaperposition", (int)ScaledResize);
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

    setUsingRenderingCache(m_mode == "SingleImage");

    if (m_mode == "SingleImage") {
        setSingleImage();
    } else {
        QTimer::singleShot(0, this, SLOT(startSlideshow()));
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

void Image::configWidgetDestroyed()
{
    m_configWidget = 0;
    m_model = 0;
}

QWidget* Image::createConfigurationInterface(QWidget* parent)
{
    m_configWidget = new QWidget(parent);
    connect(m_configWidget, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));

    if (m_mode == "SingleImage") {
        m_uiImage.setupUi(m_configWidget);

        qreal ratio = m_size.isEmpty() ? 1.0 : m_size.width() / qreal(m_size.height());
        m_model = new BackgroundListModel(ratio, this, m_configWidget);
        m_model->setResizeMethod(m_resizeMethod);
        m_model->setWallpaperSize(m_size);
        m_model->reload(m_usersWallpapers);
        m_uiImage.m_view->setModel(m_model);
        m_uiImage.m_view->setItemDelegate(new BackgroundDelegate(m_uiImage.m_view,
                                                                 ratio, m_configWidget));
        m_uiImage.m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        QModelIndex index = m_model->indexOf(m_wallpaper);
        kDebug() << m_wallpaper << index;
        if (index.isValid()) {
            m_uiImage.m_view->setCurrentIndex(index);
            Plasma::Package *b = m_model->package(index.row());
            if (b) {
                fillMetaInfo(b);
            }
        }
        connect(m_uiImage.m_view, SIGNAL(activated(const QModelIndex &)), this, SLOT(pictureChanged(const QModelIndex &)));

        m_uiImage.m_pictureUrlButton->setIcon(KIcon("document-open"));
        connect(m_uiImage.m_pictureUrlButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));

        m_uiImage.m_emailLine->setTextInteractionFlags(Qt::TextSelectableByMouse);

        m_uiImage.m_resizeMethod->addItem(i18n("Scaled & Cropped"), ScaledAndCroppedResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Scaled"), ScaledResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Scaled, keep proportions"), MaxpectResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Centered"), CenteredResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Tiled"), TiledResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Center Tiled"), CenterTiledResize);
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

        m_uiImage.m_newStuff->setIcon(KIcon("get-hot-new-stuff"));
        connect(m_uiImage.m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));

        connect(m_uiImage.m_color, SIGNAL(changed(const QColor&)), this, SLOT(modified()));
        connect(m_uiImage.m_resizeMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(modified()));
        connect(m_uiImage.m_view, SIGNAL(activated(const QModelIndex &)), this, SLOT(modified()));

    } else {
        m_uiSlideshow.setupUi(m_configWidget);
        m_uiSlideshow.m_newStuff->setIcon(KIcon("get-hot-new-stuff"));
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
        m_uiSlideshow.m_slideshowDelay->setMinimumTime(QTime(0, 0, 10));
        connect(m_uiSlideshow.m_slideshowDelay, SIGNAL(timeChanged(const QTime&)),
                this, SLOT(timeChanged(const QTime&)));

        m_uiSlideshow.m_resizeMethod->addItem(i18n("Scaled & Cropped"), ScaledAndCroppedResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Scaled"), ScaledResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Scaled, keep proportions"), MaxpectResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Centered"), CenteredResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Tiled"), TiledResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Center Tiled"), CenterTiledResize);
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
        connect(m_uiSlideshow.m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));

        connect(m_uiSlideshow.m_color, SIGNAL(changed(const QColor&)), this, SLOT(modified()));
        connect(m_uiSlideshow.m_resizeMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(modified()));
        connect(m_uiSlideshow.m_addDir, SIGNAL(clicked()), this, SLOT(modified()));
        connect(m_uiSlideshow.m_removeDir, SIGNAL(clicked()), this, SLOT(modified()));
        connect(m_uiSlideshow.m_slideshowDelay, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(modified()));
    }

    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    return m_configWidget;
}

void Image::modified()
{
    emit settingsChanged(true);
}

void Image::calculateGeometry()
{
    m_size = boundingRect().size().toSize();

    if (m_model) {
        m_model->setWallpaperSize(m_size);
    }
}

void Image::paint(QPainter *painter, const QRectF& exposedRect)
{
    // Check if geometry changed
    //kDebug() << m_size << boundingRect().size().toSize();
    if (m_size != boundingRect().size().toSize()) {
        calculateGeometry();
        if (!m_size.isEmpty() && !m_img.isEmpty()) { // We have previous image
            renderWallpaper();
            //kDebug() << "re-rendering";
            return;
        }
    }

    if (m_pixmap.isNull()) {
        painter->fillRect(exposedRect, QBrush(m_color));
        //kDebug() << "pixmap null";
        return;
    }

    if (painter->worldMatrix() == QMatrix()) {
        // draw the background untransformed when possible;(saves lots of per-pixel-math)
        painter->resetTransform();
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    // for pixmaps we draw only the exposed part (untransformed since the
    // bitmapBackground already has the size of the viewport)
    painter->drawPixmap(exposedRect, m_pixmap, exposedRect.translated(-boundingRect().topLeft()));

    if (!m_oldFadedPixmap.isNull()) {
        // Put old faded image on top.
        painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter->drawPixmap(exposedRect, m_oldFadedPixmap,
                            exposedRect.translated(-boundingRect().topLeft()));
    }
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
    KDirSelectDialog dialog(empty, true, m_configWidget);
    if (dialog.exec()) {
        QString urlDir = dialog.url().path();
        if (!urlDir.isEmpty() && m_uiSlideshow.m_dirlist->findItems(urlDir, Qt::MatchExactly).isEmpty()) {
            m_uiSlideshow.m_dirlist->addItem(dialog.url().path());
            updateDirs();
            startSlideshow();
        }
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
    const int dirCount = m_uiSlideshow.m_dirlist->count();
    for (int i = 0; i < dirCount; ++i) {
        m_dirs.append(m_uiSlideshow.m_dirlist->item(i)->text());
    }

    m_uiSlideshow.m_removeDir->setEnabled(m_uiSlideshow.m_dirlist->currentRow() != -1);
}

void Image::setSingleImage()
{
    if (m_wallpaper.isEmpty()) {
        return;
    }

    QString img;
    Plasma::Package b(m_wallpaper, packageStructure(this));
    img = b.filePath("preferred");
    kDebug() << img << m_wallpaper;

    if (img.isEmpty()) {
        img = m_wallpaper;
    }

    if (!m_size.isEmpty()) {
        renderWallpaper(img);
    }
}

void Image::setWallpaper(const KUrl &url)
{
    ///kDebug() << "droppage!" << url << url.isLocalFile();
    if (url.isLocalFile()) {
        setWallpaper(url.toLocalFile());
    } else {
        QString wallpaperPath = KGlobal::dirs()->locateLocal("wallpaper", url.fileName());

        if (!wallpaperPath.isEmpty()) {
            KIO::FileCopyJob *job = KIO::file_copy(url, KUrl(wallpaperPath));
            connect(job, SIGNAL(result(KJob*)), this, SLOT(wallpaperRetrieved(KJob*)));
        }
    }
}

void Image::wallpaperRetrieved(KJob *job)
{
    KIO::FileCopyJob *copyJob = qobject_cast<KIO::FileCopyJob *>(job);
    if (copyJob) {
        setWallpaper(copyJob->destUrl().toLocalFile());
    }
}

void Image::setWallpaper(const QString &path)
{
    if (m_wallpaper.isEmpty()) {
        m_slideshowBackgrounds.append(path);
        m_currentSlide = m_slideshowBackgrounds.size() - 2;
        bool random = m_randomize;
        nextSlide();
        m_randomize = random;
    } else {
        m_wallpaper = path;
        setSingleImage();
    }

    if (!m_usersWallpapers.contains(path)) {
        m_usersWallpapers.append(path);
    }
}

void Image::startSlideshow()
{
    // populate background list
    m_timer.stop();
    m_slideshowBackgrounds.clear();
    m_slideshowBackgrounds = BackgroundListModel::findAllBackgrounds(this, 0, m_dirs);

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
    KNS::Engine engine(m_configWidget);
    if (engine.init("wallpaper.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(m_configWidget);

        if (entries.size() > 0 && m_model) {
            m_model->reload();
        }
    }
}

void Image::colorChanged(const QColor& color)
{
    m_color = color;
    setSingleImage();
}

void Image::pictureChanged(const QModelIndex &index)
{
    if (index.row() == -1 || !m_model) {
        return;
    }

    Plasma::Package *b = m_model->package(index.row());
    if (!b) {
        return;
    }

    fillMetaInfo(b);
    if (b->structure()->contentsPrefix().isEmpty()) {
        // it's not a full package, but a single paper
        m_wallpaper = b->filePath("preferred");
    } else {
        m_wallpaper = b->path();
    }

    setSingleImage();
}

void Image::positioningChanged(int index)
{
    if (m_mode == "SingleImage") {
        m_resizeMethod = (ResizeMethod)m_uiImage.m_resizeMethod->itemData(index).value<int>();
        setSingleImage();
    } else {
        m_resizeMethod = (ResizeMethod)m_uiSlideshow.m_resizeMethod->itemData(index).value<int>();
        startSlideshow();
    }

    setResizeMethodHint(m_resizeMethod);

    if (m_model) {
        m_model->setResizeMethod(m_resizeMethod);
    }
}

void Image::fillMetaInfo(Plasma::Package *b)
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
    QString author = b->metadata().author();
    #if 0
    if (author.isEmpty()) {
        setMetadata(m_uiImage.m_authorLine, QString());
        m_uiImage.m_authorLabel->setAlignment(Qt::AlignLeft);
    } else {
        QString authorIntl = i18nc("Wallpaper info, author name", "%1", author);
        m_uiImage.m_authorLabel->setAlignment(Qt::AlignRight);
        setMetadata(m_uiImage.m_authorLine, authorIntl);
    }
    #endif
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
        m_dialog = new KFileDialog(KUrl(), "*.png *.jpeg *.jpg *.xcf *.svg *.svgz", m_configWidget);
        m_dialog->setOperationMode(KFileDialog::Opening);
        m_dialog->setInlinePreviewShown(true);
        m_dialog->setCaption(i18n("Select Wallpaper Image File"));
        m_dialog->setModal(false);

        connect(m_dialog, SIGNAL(okClicked()), this, SLOT(wallpaperBrowseCompleted()));
        connect(m_dialog, SIGNAL(destroyed(QObject*)), this, SLOT(fileDialogFinished()));
    }

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void Image::fileDialogFinished()
{
    m_dialog = 0;
}

void Image::wallpaperBrowseCompleted()
{
    Q_ASSERT(m_model);

    const QString wallpaper = m_dialog->selectedFile();

    if (wallpaper.isEmpty()) {
        return;
    }

    if (m_model->contains(wallpaper)) {
        m_uiImage.m_view->setCurrentIndex(m_model->indexOf(wallpaper));
        return;
    }

    // add background to the model
    m_model->addBackground(wallpaper);

    // select it
    QModelIndex index = m_model->indexOf(wallpaper);
    if (index.isValid()) {
        m_uiImage.m_view->setCurrentIndex(index);
        pictureChanged(index);
        modified();
    }

    // save it
    m_usersWallpapers << wallpaper;
}

void Image::nextSlide()
{
    if (m_slideshowBackgrounds.isEmpty()) {
        return;
    }

    QString previous;
    if (m_currentSlide >= 0 && m_currentSlide < m_slideshowBackgrounds.size()) {
        m_wallpaperPackage->setPath(m_slideshowBackgrounds.at(m_currentSlide));
        previous = m_wallpaperPackage->filePath("preferred");
    }

    if (m_randomize) {
        m_currentSlide = KRandom::random() % m_slideshowBackgrounds.size();
    } else if (++m_currentSlide >= m_slideshowBackgrounds.size()) {
        m_currentSlide = 0;
    }

    if (!m_wallpaperPackage) {
        m_wallpaperPackage = new Plasma::Package(m_slideshowBackgrounds.at(m_currentSlide),
                                                 packageStructure(this));
    } else {
        m_wallpaperPackage->setPath(m_slideshowBackgrounds.at(m_currentSlide));
    }

    QString current = m_wallpaperPackage->filePath("preferred");
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

            m_wallpaperPackage->setPath(m_slideshowBackgrounds.at(m_currentSlide));
            current = m_wallpaperPackage->filePath("preferred");
        }
    }

    QFileInfo info(current);
    m_previousModified = info.lastModified();

    renderWallpaper(current);
}

void Image::renderWallpaper(const QString& image)
{
    if (!image.isEmpty()) {
        m_img = image;
    }

    if (m_img.isEmpty()) {
        return;
    }

    render(m_img, m_size, m_resizeMethod, m_color);
}

void Image::updateBackground(const QImage &img)
{
    m_oldPixmap = m_pixmap;
    m_oldFadedPixmap = m_oldPixmap;
    m_pixmap = QPixmap::fromImage(img);
    
    if(!img.isNull()){
        suspendStartup(false);
    }
    
    if (!m_oldPixmap.isNull()) {
        Plasma::Animator::self()->customAnimation(254, 1000, Plasma::Animator::EaseInCurve, this, "updateFadedImage");
    } else {        
        emit update(boundingRect());
    }
}

void Image::suspendStartup(bool suspend)
{
    if (m_startupResumed) {
        return;
    }

    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());
    const QString startupID("desktop wallaper");
    if (suspend) {
        ksmserver.suspendStartup(startupID);
    } else {
        m_startupResumed = true;
        ksmserver.resumeStartup(startupID);
    }
}

void Image::updateScreenshot(QPersistentModelIndex index)
{
    m_uiImage.m_view->update(index);
}

void Image::updateFadedImage(qreal frame)
{
    //If we are done, delete the pixmaps and don't draw.
    if (qFuzzyCompare(frame, qreal(1.0))) {
        m_oldFadedPixmap = QPixmap();
        m_oldPixmap = QPixmap();
        emit update(boundingRect());
        return;
    }

    //Create the faded image.
    m_oldFadedPixmap.fill(Qt::transparent);

    QPainter p;
    p.begin(&m_oldFadedPixmap);
    p.drawPixmap(0, 0, m_oldPixmap);

    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);  
    p.fillRect(m_oldFadedPixmap.rect(), QColor(0, 0, 0, 254 * (1-frame)));//255*((150 - frame)/150)));

    p.end();

    emit update(boundingRect());
}

#include "image.moc"
