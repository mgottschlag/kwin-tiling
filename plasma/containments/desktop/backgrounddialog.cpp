/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#define USE_BACKGROUND_PACKAGES

#include "backgrounddialog.h"
#include <memory>
#include <QAbstractItemView>
#include <QAbstractListModel>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QPainter>
#include <QStackedWidget>
#include <QTimeEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <KColorButton>
#include <KDebug>
#include <KDirSelectDialog>
#include <KFileDialog>
#include <KGlobalSettings>
#include <KLocalizedString>
#include <KPushButton>
#include <KSeparator>
#include <KStandardDirs>
#include <KSvgRenderer>
#include <knewstuff2/engine.h>
#include <ThreadWeaver/Weaver>

#ifdef USE_BACKGROUND_PACKAGES

#include <plasma/packagemetadata.h>
#include <plasma/package.h>

#endif

class BackgroundContainer
{
public:
    virtual ~BackgroundContainer();
    
    virtual bool contains(const QString &path) const = 0;
};

QList<Background *> 
findAllBackgrounds(const BackgroundContainer *container, 
                   const QString &path, 
                   float ratio)
{
    QList<Background *> res;

#ifdef USE_BACKGROUND_PACKAGES

    // get all packages in this directory
    QStringList packages = Plasma::Package::knownPackages(path);
    foreach (QString packagePath, packages)
    {
        kDebug() << packagePath;
        std::auto_ptr<Background> pkg(
            new BackgroundPackage(path+packagePath, ratio));
//             kDebug() << "Package is valid?" << pkg->isValid();
//             kDebug() << "Path passed to the constructor" << path+packagePath;
        if (pkg->isValid() && 
            (!container || !container->contains(pkg->path()))) {
            res.append(pkg.release());
        }
    }
//     kDebug() << packages << res;

#endif

    // search normal wallpapers
    QDir dir(path);
    QStringList filters;
    filters << "*.png" << "*.jpeg" << "*.jpg" << "*.svg" << "*.svgz";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden);
    QFileInfoList files = dir.entryInfoList();
    foreach (QFileInfo wp, files)
    {
        if (!container || !container->contains(wp.filePath())) {
            res.append(new BackgroundFile(wp.filePath(), ratio));
        }
    }

    return res;
}

BackgroundContainer::~BackgroundContainer()
{
}

class BackgroundListModel : public QAbstractListModel
                          , public BackgroundContainer
{
public:
    BackgroundListModel(float ratio, QObject *listener);
    virtual ~BackgroundListModel();
    
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Background* package(int index) const;
    
    void reload();
    void reload(const QStringList &selected);
    void addBackground(const QString &path);
    int indexOf(const QString &path) const;
    virtual bool contains(const QString &bg) const;
private:
    QObject *m_listener;
    QList<Background*> m_packages;
    float m_ratio;
};

class BackgroundDelegate : public QAbstractItemDelegate
{
public:
    enum {
        AuthorRole = Qt::UserRole,
        ScreenshotRole
    };

    BackgroundDelegate(QObject *listener,
                       float ratio, QObject *parent = 0);
    
    virtual void paint(QPainter *painter, 
                       const QStyleOptionViewItem &option, 
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, 
                           const QModelIndex &index) const;
private:
    static const int MARGIN = 5;
    QObject *m_listener;
    float m_ratio;
};

BackgroundListModel::BackgroundListModel(float ratio, QObject *listener)
: m_listener(listener)
, m_ratio(ratio)
{
}

void BackgroundListModel::reload() 
{
    reload(QStringList());
}

void BackgroundListModel::reload(const QStringList& selected)
{
    QStringList dirs = KGlobal::dirs()->findDirs("wallpaper", "");
    QList<Background *> tmp;
    foreach (QString file, selected) {
        if (!contains(file)) {
            tmp << new BackgroundFile(file, m_ratio);
        }
    }
    foreach (QString dir, dirs) {
        tmp += findAllBackgrounds(this, dir, m_ratio);
    }
    
    if (!tmp.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, tmp.size() - 1);
        m_packages = tmp + m_packages;
        endInsertRows();
    }
}

void BackgroundListModel::addBackground(const QString& path) {
    if (!contains(path)) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_packages.prepend(new BackgroundFile(path, m_ratio));
        endInsertRows();
    }
}

int BackgroundListModel::indexOf(const QString &path) const
{
    for (int i = 0; i < m_packages.size(); i++) {
        if (m_packages[i]->path() == path) {
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
    default:
        return QVariant();
    }
}

Background* BackgroundListModel::package(int index) const
{
    return m_packages.at(index);
}

BackgroundDelegate::BackgroundDelegate(QObject *listener,
                                       float ratio, QObject *parent)
: QAbstractItemDelegate(parent)
, m_listener(listener)
, m_ratio(ratio)
{
}

void BackgroundDelegate::paint(QPainter *painter, 
                               const QStyleOptionViewItem &option, 
                               const QModelIndex &index) const
{
    QString title = index.model()->data(index, Qt::DisplayRole).toString();
    QString author = index.model()->data(index, AuthorRole).toString();
    QPixmap pix = index.model()->data(index, ScreenshotRole).value<QPixmap>();
        
    // draw selection outline
    if (option.state & QStyle::State_Selected) {
        QPen oldPen = painter->pen();
        painter->setPen(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect.adjusted(2, 2, -2, -2));
        painter->setPen(oldPen);
    }
    
    // draw pixmap
    int maxheight = Background::SCREENSHOT_HEIGHT;
    int maxwidth = int(maxheight * m_ratio);
    if (!pix.isNull()) {
        QSize sz = pix.size();
        int x = MARGIN + (maxwidth - pix.width()) / 2;
        int y = MARGIN + (maxheight - pix.height()) / 2;
        QRect imgRect = QRect(option.rect.topLeft(), pix.size()).translated(x, y);
        painter->drawPixmap(imgRect, pix);
    }
    
    // draw text
    painter->save();
    QFont font = painter->font();
    font.setWeight(QFont::Bold);
    painter->setFont(font);
    int x = option.rect.left() + MARGIN * 5 + maxwidth;
    
    QRect textRect(x,
                   option.rect.top() + MARGIN,
                   option.rect.width() - x - MARGIN * 2,
                   maxheight);
    QString text = title.replace("_", " ");
    if (!author.isEmpty()) {
        text += "\n" + author;
    }
    QRect boundingRect = painter->boundingRect(
        textRect, Qt::AlignVCenter | Qt::TextWordWrap, text);
    painter->drawText(boundingRect, Qt::TextWordWrap, title);
    if (!author.isEmpty()) {
        QRect titleRect = painter->boundingRect(boundingRect, Qt::TextWordWrap, title);
        QRect authorRect(titleRect.bottomLeft(), textRect.size());
        painter->setFont(KGlobalSettings::smallestReadableFont());
        painter->drawText(authorRect, Qt::TextWordWrap, "by " + author);
    }

    painter->restore();
}

QSize BackgroundDelegate::sizeHint(const QStyleOptionViewItem &, 
                                   const QModelIndex &) const
{
    return QSize(100, Background::SCREENSHOT_HEIGHT + MARGIN * 2);
}


BackgroundDialog::BackgroundDialog(const QSize &res, 
                                   const KConfigGroup &config, 
                                   QWidget *parent)
: KDialog(parent)
, m_res(res)
, m_ratio((float) res.width() / res.height())
, m_currentSlide(-1)
, m_preview_renderer(QSize(128, 101), (float) 128 / res.width())
{
    setWindowIcon(KIcon("preferences-desktop-wallpaper"));
    setCaption(i18n("Configure Desktop"));
    setButtons(Ok | Cancel | Apply);

    QWidget *main = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *leftLayout = new QVBoxLayout;
    QVBoxLayout *rightLayout = new QVBoxLayout;
    
    // static or slideshow?
    m_mode = new QComboBox(main);
    m_mode->addItem(i18n("Wallpaper Image"));
    m_mode->addItem(i18n("Slideshow"));
    leftLayout->addWidget(m_mode);
    connect(m_mode, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeBackgroundMode(int)));
    
    // stacked widget
    QStackedWidget *stack = new QStackedWidget(main);
    leftLayout->addWidget(stack);
    connect(m_mode, SIGNAL(currentIndexChanged(int)),
            stack, SLOT(setCurrentIndex(int)));
    
    // static picture
    QWidget *staticPictureWidget = new QWidget(stack);
    stack->addWidget(staticPictureWidget);
    QGridLayout *staticPictureLayout = new QGridLayout;
    //staticPictureLayout->setColumnStretch(2, 10);
    staticPictureWidget->setLayout(staticPictureLayout);
    
    // combo with backgrounds
    QLabel *pictureLabel = new QLabel("&Picture:", staticPictureWidget);
    QHBoxLayout *pictureLayout = new QHBoxLayout;    
    m_view = new QComboBox(main);
    m_model = new BackgroundListModel(m_ratio, this);
    m_view->setModel(m_model);
    m_view->setItemDelegate(new BackgroundDelegate(m_view->view(), m_ratio, this));
    connect(m_view, SIGNAL(currentIndexChanged(int)),
            this, SLOT(update()));
    pictureLabel->setBuddy(m_view);
    QToolButton *pictureUrlButton = new QToolButton(this);
    pictureUrlButton->setIcon(KIcon("document-open"));
    pictureUrlButton->setToolTip(i18n("Browse"));
    connect(pictureUrlButton, SIGNAL(clicked()), this, SLOT(browse()));
    
    pictureLayout->addWidget(m_view);
    pictureLayout->addWidget(pictureUrlButton);
    
    staticPictureLayout->addWidget(pictureLabel, 0, 0, Qt::AlignRight);
    staticPictureLayout->addLayout(pictureLayout, 0, 1, 1, 2);

    // resize method
    QLabel *resizeMethodLabel = new QLabel(i18n("P&ositioning:"), staticPictureWidget);
    m_resizeMethod = new QComboBox(main);
    m_resizeMethod->addItem(i18n("Scale and crop"),
                            Background::ScaleCrop);
    m_resizeMethod->addItem(i18n("Scaled"), 
                            Background::Scale);
    m_resizeMethod->addItem(i18n("Centered"), 
                            Background::Center);
    m_resizeMethod->addItem(i18n("Tiled"),
                            Background::Tiled);
    m_resizeMethod->addItem(i18n("Center tiled"),
                            Background::CenterTiled);
    connect(m_resizeMethod, SIGNAL(currentIndexChanged(int)),
            this, SLOT(update()));
    resizeMethodLabel->setBuddy(m_resizeMethod);
    staticPictureLayout->addWidget(resizeMethodLabel, 1, 0, Qt::AlignRight);
    staticPictureLayout->addWidget(m_resizeMethod, 1, 1);

    // color
    QLabel *colorLabel = new QLabel(i18n("&Color:"), staticPictureWidget);
    m_color = new KColorButton(this);
    m_color->setColor(palette().color(QPalette::Window));
    connect(m_color, SIGNAL(changed(QColor)), main, SLOT(update()));
    colorLabel->setBuddy(m_color);
    staticPictureLayout->addWidget(colorLabel, 2, 0, Qt::AlignRight);
    staticPictureLayout->addWidget(m_color, 2, 1);

    m_metadataSeparator = new KSeparator(Qt::Horizontal, staticPictureWidget);
    m_authorLine = new QLabel(staticPictureWidget);
    m_emailLine = new QLabel(staticPictureWidget);
    m_licenseLine = new QLabel(staticPictureWidget);
    
    staticPictureLayout->addWidget(m_metadataSeparator, 3, 0, 1, 2);
    staticPictureLayout->addWidget(m_authorLine, 4, 0, 1, 2);
    staticPictureLayout->addWidget(m_emailLine, 5, 0, 1, 2);
    staticPictureLayout->addWidget(m_licenseLine, 6, 0, 1, 2);
    //staticPictureLayout->setRowStretch(7, 10);

    // slideshow
    QWidget *slideshowWidget = new QWidget(stack);
    stack->addWidget(slideshowWidget);
    QVBoxLayout *slideshowLayout = new QVBoxLayout;
    slideshowWidget->setLayout(slideshowLayout);
        
    QHBoxLayout *dirlistLayout = new QHBoxLayout;
    m_dirlist = new QListWidget(slideshowWidget);
    connect(m_dirlist, SIGNAL(currentRowChanged(int)), this, SLOT(updateSlideshow()));
    QVBoxLayout *dirlistButtons = new QVBoxLayout;
    m_addDir = new QPushButton(
        KIcon("list-add"),
        i18n("&Add directory..."),
        slideshowWidget);
    connect(m_addDir, SIGNAL(clicked()), this, SLOT(slotAddDir()));
    m_removeDir = new QPushButton(
        KIcon("list-remove"),
        i18n("&Remove directory"),
        slideshowWidget);
    connect(m_removeDir, SIGNAL(clicked()), this, SLOT(slotRemoveDir()));
    
    QHBoxLayout *slideshowDelayLayout = new QHBoxLayout;
    QLabel *slideshowDelayLabel = new QLabel(
        i18n("&Change images every:"), 
        slideshowWidget);
    m_slideshowDelay = new QTimeEdit(slideshowWidget);
    m_slideshowDelay->setDisplayFormat("hh:mm:ss");
    m_slideshowDelay->setMinimumTime(QTime(0, 0, 30));
    slideshowDelayLabel->setBuddy(m_slideshowDelay);
    slideshowDelayLayout->addWidget(slideshowDelayLabel);
    slideshowDelayLayout->addWidget(m_slideshowDelay);
    slideshowDelayLayout->addStretch();
    
    dirlistLayout->addWidget(m_dirlist);
    dirlistLayout->addLayout(dirlistButtons);
    dirlistButtons->addWidget(m_addDir);
    dirlistButtons->addWidget(m_removeDir);
    dirlistButtons->addStretch();
    slideshowLayout->addLayout(dirlistLayout);
    slideshowLayout->addLayout(slideshowDelayLayout);
    
    // preview
    QLabel *monitor = new QLabel(main);
    QString monitorPath = KStandardDirs::locate("data",  "kcontrol/pics/monitor.png");
    
    // Size of monitor image: 200x186
    // Geometry of "display" part of monitor image: (23,14)-[151x115]
    qreal previewRatio = 128.0 / (101.0 * m_ratio);
    QSize monitorSize(200, int(186 * previewRatio));
    QRect previewRect(23, int(14 * previewRatio), 151, int(115 * previewRatio));
    m_preview_renderer.setSize(previewRect.size());
    
    monitor->setPixmap(QPixmap(monitorPath).scaled(monitorSize));
    monitor->setWhatsThis(i18n(
        "This picture of a monitor contains a preview of "
        "what the current settings will look like on your desktop."));
    m_preview = new QLabel(monitor);
    m_preview->setScaledContents(true);
    m_preview->setGeometry(previewRect);
    
    // get new stuff
    QHBoxLayout *newStuffLayout = new QHBoxLayout;
    m_newStuff = new QPushButton(i18n("&New wallpaper..."), main);
    connect(m_newStuff, SIGNAL(clicked()), this, SLOT(getNewStuff()));
    newStuffLayout->addStretch();
    newStuffLayout->addWidget(m_newStuff);
    newStuffLayout->addStretch();
    
    rightLayout->addWidget(monitor);
    rightLayout->addStretch();
    rightLayout->addLayout(newStuffLayout);
    rightLayout->addStretch();
    
    layout->addLayout(leftLayout, 1);
    layout->addLayout(rightLayout);
    main->setLayout(layout);
    
    qRegisterMetaType<QImage>("QImage");
    connect(&m_preview_timer, SIGNAL(timeout()), this, SLOT(updateSlideshowPreview()));
    connect(&m_preview_renderer, SIGNAL(done(int, const QImage &)), 
            this, SLOT(previewRenderingDone(int, const QImage &)));
    connect(this, SIGNAL(finished(int)), this, SLOT(cleanup()));
    
    setMainWidget(main);
    
    reloadConfig(config);
}

void BackgroundDialog::reloadConfig(const KConfigGroup &config)
{
    // initialize
    int mode = config.readEntry("backgroundmode", int(kStaticBackground));
    m_mode->setCurrentIndex(mode);
    int delay = config.readEntry("slideTimer", 60);
    QTime time(0, 0, 0);
    time = time.addSecs(delay);
    m_slideshowDelay->setTime(time);

    m_dirlist->clear();
    QStringList dirs = config.readEntry("slidepaths", QStringList());
    if (dirs.isEmpty()) {
        dirs << KStandardDirs::installPath("wallpaper");
    }
    foreach (QString dir, dirs) {
        m_dirlist->addItem(dir);
    }
    m_selected = config.readEntry("selected", QStringList());
    m_model->reload(m_selected);
    QString currentPath = config.readEntry("wallpaper",
        KStandardDirs::locate("wallpaper", "plasma-default.png"));
    int index = m_model->indexOf(currentPath);
    if (index != -1) {
        m_view->setCurrentIndex(index);
    }
    
    if (mode == kSlideshowBackground) {
        updateSlideshow();
    }
    else {
        update();
    }
}

void BackgroundDialog::saveConfig(KConfigGroup config)
{
    int mode = m_mode->currentIndex();
    config.writeEntry("backgroundmode", mode);
    if (mode == kStaticBackground) {
        config.writeEntry("wallpaper", m_img);
        config.writeEntry("wallpapercolor", m_color->color());
        config.writeEntry("wallpaperposition", 
            m_resizeMethod->itemData(m_resizeMethod->currentIndex()).toInt());
        config.writeEntry("selected", m_selected);
    }
    else {
        QStringList dirs;
        for (int i = 0; i < m_dirlist->count(); i++) {
            dirs << m_dirlist->item(i)->text();
        }
        config.writeEntry("slidepaths", dirs);
        int seconds = QTime(0, 0, 0).secsTo(m_slideshowDelay->time());
        config.writeEntry("slideTimer", seconds);
    }
}

void BackgroundDialog::getNewStuff()
{
    
    KNS::Engine engine(0);
    if (engine.init("wallpaper.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(this);

        if (entries.size() > 0) {
            m_model->reload();
        }
    }
}

void BackgroundDialog::browse()
{
    QString wallpaper = KFileDialog::getOpenFileName(KUrl(), 
                                                     "*.png *.jpeg *.jpg *.svg *.svgz",
                                                     this, 
                                                     i18n("Select a wallpaper image file"));
    if (wallpaper.isEmpty()) {
        return;
    }
    
    // add background to the model
    m_model->addBackground(wallpaper);
    
    // select it
    int index = m_model->indexOf(wallpaper);
    if (index != -1) {
        m_view->setCurrentIndex(index);
    }
    
    // save it
    m_selected << wallpaper;
}

bool BackgroundDialog::setMetadata(QLabel *label, 
                                   const QString &caption, 
                                   const QString &text)
{
    if (text.isEmpty()) {
        label->hide();
        return false;
    }
    else {
        label->show();
        label->setText("<b>" + caption + ":</b> " + text);
        return true;
    }
}

void BackgroundDialog::update()
{
    int index = m_view->currentIndex();
    if (index == -1) {
        return;
    }
    Background *b = m_model->package(index);
    if (!b) {
        return;
    }
    bool someMetadata = setMetadata(m_authorLine, i18n("Author"), b->author());
    someMetadata = setMetadata(m_emailLine, i18n("E-mail"), b->email()) || someMetadata;
    m_metadataSeparator->setVisible(someMetadata);
    
    Background::ResizeMethod method = (Background::ResizeMethod)
        m_resizeMethod->itemData(m_resizeMethod->currentIndex()).value<int>();
    
    m_img = b->findBackground(m_res, method);
    setPreview(m_img, method);
}

void BackgroundDialog::setPreview(const QString& img, Background::ResizeMethod method)
{
    m_preview_token = m_preview_renderer.render(img, m_color->color(), method, Qt::FastTransformation);
}

void BackgroundDialog::slotAddDir()
{
    KDirSelectDialog dialog(KUrl(), true, this);
    if (dialog.exec()) {
        m_dirlist->addItem(dialog.url().path());
        updateSlideshow();
    }
}

void BackgroundDialog::slotRemoveDir()
{
    int row = m_dirlist->currentRow();
    if (row != -1) {
        m_dirlist->takeItem(row);
        updateSlideshow();
    }
}

void BackgroundDialog::updateSlideshow()
{
    int row = m_dirlist->currentRow();
    m_removeDir->setEnabled(row != -1);
    
    // populate background list
    m_slideshowBackgrounds.clear();
    for (int i = 0; i < m_dirlist->count(); i++) {
        QString dir = m_dirlist->item(i)->text();
        m_slideshowBackgrounds += findAllBackgrounds(0, dir, m_ratio);
    }
    
    // start preview
    if (m_slideshowBackgrounds.isEmpty()) {
        m_preview->setPixmap(QPixmap());
        m_preview_timer.stop();
    }
    else {
        m_currentSlide = -1;
        if (!m_preview_timer.isActive()) {
            m_preview_timer.start(3000);
        }
    }
}

void BackgroundDialog::updateSlideshowPreview()
{
    if (!m_slideshowBackgrounds.isEmpty()) {
        // increment current slide index
        m_currentSlide++;
        m_currentSlide = m_currentSlide % m_slideshowBackgrounds.count();

        Background *slide = m_slideshowBackgrounds[m_currentSlide];
        Q_ASSERT(slide);
        
        const Background::ResizeMethod method = Background::Scale;
        m_img = slide->findBackground(m_res, method);
        setPreview(m_img, method);
    }
    else {
        m_preview->setPixmap(QPixmap());
    }
}

void BackgroundDialog::changeBackgroundMode(int mode)
{
    switch (mode)
    {
    case kStaticBackground:
        m_preview_timer.stop();
        update();
        break;
    case kSlideshowBackground:
        updateSlideshow();
        break;
    }
}

bool BackgroundDialog::contains(const QString &path) const
{
    foreach (Background *bg, m_slideshowBackgrounds)
    {
        if (bg->path() == path) {
            return true;
        }
    }
    return false;
}

void BackgroundDialog::previewRenderingDone(int token, const QImage &image)
{
    // display preview only if it is the latest rendered file
    if (token == m_preview_token) {
        m_preview->setPixmap(QPixmap::fromImage(image));
    }
}

void BackgroundDialog::updateScreenshot(QPersistentModelIndex index)
{
    m_view->view()->update(index);
}

void BackgroundDialog::cleanup()
{
    m_preview_timer.stop();
}
