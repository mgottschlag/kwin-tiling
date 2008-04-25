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
#include <QCheckBox>
#include <KColorButton>
#include <KDebug>
#include <KDirSelectDialog>
#include <KDirWatch>
#include <KFileDialog>
#include <KGlobalSettings>
#include <KImageFilePreview>
#include <KLocalizedString>
#include <KPushButton>
#include <KSeparator>
#include <KStandardDirs>
#include <KSvgRenderer>
#include <knewstuff2/engine.h>
#include <ThreadWeaver/Weaver>
#include <KColorScheme>

#ifdef USE_BACKGROUND_PACKAGES

#include <plasma/packagemetadata.h>
#include <plasma/panelsvg.h>
#include <plasma/package.h>
#include <plasma/theme.h>

#endif

class ThemeModel : public QAbstractListModel
{
public:
    ThemeModel(QObject *parent = 0);
    virtual ~ThemeModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int indexOf(const QString &path) const;
    void reload();
private:
    QStringList m_themes;
    QList<Plasma::PanelSvg *> m_svgs;
};

ThemeModel::ThemeModel( QObject *parent )
: QAbstractListModel( parent )
{
    reload();
}

ThemeModel::~ThemeModel()
{
}

void ThemeModel::reload()
{
    m_themes.clear();
    m_svgs.clear();

    // get all desktop themes
    KStandardDirs dirs;
    QStringList themes = dirs.findAllResources("data", "desktoptheme/*/metadata.desktop", KStandardDirs::NoDuplicates);

    foreach (const QString &theme, themes) {
        int themeSepIndex = theme.lastIndexOf("/", -1);
        QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf("/", -1);
        QString name = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);
        m_themes << name;

        Plasma::PanelSvg *svg = new Plasma::PanelSvg(this);
        svg->setImagePath(themeRoot + "/widgets/background.svg");
        svg->setEnabledBorders(Plasma::PanelSvg::AllBorders);
        m_svgs.append( svg );
    }
}

int ThemeModel::rowCount(const QModelIndex &) const
{
    return m_themes.size();
}

QVariant ThemeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_themes.size()) {
        return QVariant();
    }


    switch (role) {
    case Qt::DisplayRole:
        return m_themes[index.row()];
    case Qt::UserRole:
        return qVariantFromValue((void*)m_svgs[index.row()]);
    default:
        return QVariant();
    }
}

int ThemeModel::indexOf(const QString &name) const
{
    for (int i = 0; i < m_themes.size(); i++) {
        if (name == m_themes[i]) {
            return i;
        }
    }
    return -1;
}




class ThemeDelegate : public QAbstractItemDelegate
{
public:
    ThemeDelegate( QObject * parent = 0 );

    virtual void paint(QPainter *painter, 
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
private:
    static const int MARGIN = 5;
};

ThemeDelegate::ThemeDelegate( QObject * parent )
: QAbstractItemDelegate( parent )
{
    kDebug();
}

void ThemeDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QString title = index.model()->data(index, Qt::DisplayRole).toString();

    // highlight selected item
    painter->save();
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.color(QPalette::Highlight));
    } else {
        painter->setBrush(Qt::gray);
    }
    painter->drawRect(option.rect);
    painter->restore();

    // draw image
    Plasma::PanelSvg *svg = static_cast<Plasma::PanelSvg *>(index.model()->data(index, Qt::UserRole).value<void *>());
    svg->resizePanel(QSize(option.rect.width()-(2*MARGIN), 100-(2*MARGIN)));
    QRect imgRect = QRect(option.rect.topLeft(), QSize( option.rect.width()-(2*MARGIN), 100-(2*MARGIN) )).
        translated(MARGIN, MARGIN);
    svg->paintPanel( painter, imgRect, QPoint(option.rect.left() + MARGIN, option.rect.top() + MARGIN) );

    // draw text
    painter->save();
    QFont font = painter->font();
    font.setWeight(QFont::Bold);
    QString colorFile = KStandardDirs::locate("data", "desktoptheme/" + title + "/colors");
    if( !colorFile.isEmpty() ) {
        KSharedConfigPtr colors = KSharedConfig::openConfig(colorFile);
        KColorScheme colorScheme(QPalette::Active, KColorScheme::View, colors);
        painter->setPen( colorScheme.foreground().color() );
    }
    painter->setFont(font);
    painter->drawText(option.rect, Qt::AlignCenter | Qt::TextWordWrap, title);
    painter->restore();
}



QSize ThemeDelegate::sizeHint(const QStyleOptionViewItem &, 
                                   const QModelIndex &) const
{
    return QSize(200, 100);
}




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
    void removeBackground(const QString &path);
    virtual bool contains(const QString &bg) const;
private:
    QObject *m_listener;
    QList<Background*> m_packages;
    float m_ratio;
    KDirWatch m_dirwatch;
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
    foreach (QString file, selected) {
        if (!contains(file) && QFile::exists(file)) {
            tmp << new BackgroundFile(file, m_ratio);
        }
    }
    foreach (QString dir, dirs) {
        tmp += findAllBackgrounds(this, dir, m_ratio);
    }
    
    // add new files to dirwatch
    foreach (Background *b, tmp) {
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
    QString text = title;
    QString authorCaption;
    if (!author.isEmpty()) {
        authorCaption = i18nc("Caption to wallpaper preview, %1 author name",
                              "by %1", author);
        text += '\n' + authorCaption;
    }
    QRect boundingRect = painter->boundingRect(
        textRect, Qt::AlignVCenter | Qt::TextWordWrap, text);
    painter->drawText(boundingRect, Qt::TextWordWrap, title);
    if (!author.isEmpty()) {
        QRect titleRect = painter->boundingRect(boundingRect, Qt::TextWordWrap, title);
        QRect authorRect(titleRect.bottomLeft(), textRect.size());
        painter->setFont(KGlobalSettings::smallestReadableFont());
        painter->drawText(authorRect, Qt::TextWordWrap, authorCaption);
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
                                   const KConfigGroup &globalConfig,
                                   QWidget *parent)
: KDialog(parent)
, m_res(res)
, m_ratio((float) res.width() / res.height())
, m_currentSlide(-1)
, m_preview_renderer(QSize(128, 101), (float) 128 / res.width())
{
    setWindowIcon(KIcon("preferences-desktop-wallpaper"));
    setCaption(i18n("Desktop Settings"));
    setButtons(Ok | Cancel | Apply);

    QWidget * main = new QWidget(this);
    setupUi(main);

    // static, slideshow or none?
    connect(m_mode, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeBackgroundMode(int)));

    // static picture
    m_model = new BackgroundListModel(m_ratio, this);
    m_view->setModel(m_model);
    m_view->setItemDelegate(new BackgroundDelegate(m_view->view(), m_ratio, this));
    connect(m_view, SIGNAL(currentIndexChanged(int)),
            this, SLOT(update()));
    m_pictureUrlButton->setIcon(KIcon("document-open"));
    connect(m_pictureUrlButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));

    // resize method
    m_resizeMethod->addItem(i18n("Scale & Crop"),
                            Background::ScaleCrop);
    m_resizeMethod->addItem(i18n("Scaled"), 
                            Background::Scale);
    m_resizeMethod->addItem(i18n("Centered"), 
                            Background::Center);
    m_resizeMethod->addItem(i18n("Tiled"),
                            Background::Tiled);
    m_resizeMethod->addItem(i18n("Center Tiled"),
                            Background::CenterTiled);
    connect(m_resizeMethod, SIGNAL(currentIndexChanged(int)),
            this, SLOT(update()));

    // color
    m_color->setColor(palette().color(QPalette::Window));
    connect(m_color, SIGNAL(changed(QColor)), this, SLOT(update()));

    // slideshow
    m_addDir->setIcon(KIcon("list-add"));
    connect(m_addDir, SIGNAL(clicked()), this, SLOT(slotAddDir()));
    m_removeDir->setIcon(KIcon("list-remove"));
    connect(m_removeDir, SIGNAL(clicked()), this, SLOT(slotRemoveDir()));
    connect(m_dirlist, SIGNAL(currentRowChanged(int)), this, SLOT(updateSlideshow()));

    m_slideshowDelay->setMinimumTime(QTime(0, 0, 30));

    // preview
    QString monitorPath = KStandardDirs::locate("data",  "kcontrol/pics/monitor.png");
    
    // Size of monitor image: 200x186
    // Geometry of "display" part of monitor image: (23,14)-[151x115]
    qreal previewRatio = 128.0 / (101.0 * m_ratio);
    QSize monitorSize(200, int(186 * previewRatio));
    QRect previewRect(23, int(14 * previewRatio), 151, int(115 * previewRatio));
    m_preview_renderer.setSize(previewRect.size());
    
    m_monitor->setPixmap(QPixmap(monitorPath).scaled(monitorSize));
    m_monitor->setWhatsThis(i18n(
        "This picture of a monitor contains a preview of "
        "what the current settings will look like on your desktop."));
    m_preview = new QLabel(m_monitor);
    m_preview->setScaledContents(true);
    m_preview->setGeometry(previewRect);

    connect(m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));
    connect(m_newThemeButton, SIGNAL(clicked()), this, SLOT(getNewThemes()));

    qRegisterMetaType<QImage>("QImage");
    connect(&m_preview_timer, SIGNAL(timeout()), this, SLOT(updateSlideshowPreview()));
    connect(&m_preview_renderer, SIGNAL(done(int, const QImage &)), 
            this, SLOT(previewRenderingDone(int, const QImage &)));
    connect(this, SIGNAL(finished(int)), this, SLOT(cleanup()));

    m_themeModel = new ThemeModel(this);
    m_theme->setModel(m_themeModel);
    m_theme->setItemDelegate(new ThemeDelegate(m_theme->view()));

    setMainWidget(main);
    m_emailLine->setTextInteractionFlags(Qt::TextSelectableByMouse);

    reloadConfig(config, globalConfig);
    adjustSize();
}

void BackgroundDialog::reloadConfig(const KConfigGroup &config, const KConfigGroup &globalConfig)
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
            KStandardDirs::locate("wallpaper", "EOS/contents/images/1920x1200.jpg"));

    kDebug() << "Default would be" << KStandardDirs::locate("wallpaper", "EOS/contents/images/1920x1200.jpg");
    kDebug() << "but we're loading" << currentPath << "instead";

    int index = m_model->indexOf(currentPath);
    if (index != -1) {
        m_view->setCurrentIndex(index);
    }

    m_color->setColor(config.readEntry("wallpapercolor", m_color->color()));
    KConfigGroup iconConfig(&globalConfig, "DesktopIcons");
    bool showIcons = iconConfig.readEntry("showIcons",true);
    m_showIcons->setCheckState(showIcons ? Qt::Checked : Qt::Unchecked);
    bool alignToGrid = iconConfig.readEntry("alignToGrid", true);
    m_alignToGrid->setCheckState(alignToGrid ? Qt::Checked : Qt::Unchecked);

    m_theme->setCurrentIndex(m_themeModel->indexOf(Plasma::Theme::defaultTheme()->themeName()));

    if (mode == kSlideshowBackground) {
        updateSlideshow();
    }
    else {
        update();
    }
}

void BackgroundDialog::saveConfig(KConfigGroup config, KConfigGroup globalConfig)
{
    int mode = m_mode->currentIndex();
    config.writeEntry("backgroundmode", mode);
    if (mode == kStaticBackground) {
        config.writeEntry("wallpaper", m_img);
        config.writeEntry("wallpapercolor", m_color->color());
        config.writeEntry("wallpaperposition", 
            m_resizeMethod->itemData(m_resizeMethod->currentIndex()).toInt());
        config.writeEntry("selected", m_selected);
    } else if (mode == kNoBackground) {
        config.writeEntry("wallpaper", QString());
        config.writeEntry("wallpapercolor", m_color->color());
    } else {
        QStringList dirs;
        for (int i = 0; i < m_dirlist->count(); i++) {
            dirs << m_dirlist->item(i)->text();
        }
        config.writeEntry("slidepaths", dirs);
        int seconds = QTime(0, 0, 0).secsTo(m_slideshowDelay->time());
        config.writeEntry("slideTimer", seconds);
    }

    KConfigGroup iconConfig(&globalConfig, "DesktopIcons");
    iconConfig.writeEntry("showIcons", (m_showIcons->checkState() == Qt::Checked ? true : false));
    iconConfig.writeEntry("alignToGrid", (m_alignToGrid->checkState() == Qt::Checked ? true : false));

    Plasma::Theme::defaultTheme()->setThemeName(m_theme->currentText());
}

void BackgroundDialog::getNewWallpaper()
{
    KNS::Engine engine(0);
    if (engine.init("wallpaper.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(this);

        if (entries.size() > 0) {
            m_model->reload();
        }
    }
}

void BackgroundDialog::getNewThemes()
{
    KNS::Engine engine(0);
    if (engine.init("plasma-themes.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(this);

        if (entries.size() > 0) {
            // FIXME: do something to reload the list of themes
            m_themeModel->reload();
        }
    }
}

void BackgroundDialog::showFileDialog()
{
    m_dialog = new KFileDialog(KUrl(), "*.png *.jpeg *.jpg *.svg *.svgz", this);
    KImageFilePreview *previewWidget = new KImageFilePreview(m_dialog);
    m_dialog->setPreviewWidget(previewWidget);
    m_dialog->setOperationMode(KFileDialog::Opening);
    m_dialog->setCaption(i18n("Select Wallpaper Image File"));
    m_dialog->setModal(false);
    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();

    connect(m_dialog, SIGNAL(okClicked()), this, SLOT(browse()));
}

void BackgroundDialog::browse()
{
    QString wallpaper = m_dialog->selectedFile();
    disconnect(m_dialog, SIGNAL(okClicked()), this, SLOT(browse()));

    m_dialog->deleteLater();

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
                                   const QString &text)
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

void BackgroundDialog::update()
{
    if (m_mode->currentIndex() == kNoBackground) {
        m_img.clear();
        setPreview(m_img, Background::Scale);
        return;
    }
    int index = m_view->currentIndex();
    if (index == -1) {
        return;
    }
    Background *b = m_model->package(index);
    if (!b) {
        return;
    }

    // FIXME the second parameter is not used, get rid of it.
    bool someMetadata = setMetadata(m_authorLine, b->author());
    someMetadata = setMetadata(m_licenseLine, b->license()) || someMetadata;
    someMetadata = setMetadata(m_emailLine, b->email()) || someMetadata;
    //m_authorLabel->setVisible(someMetadata);
    //m_emailLabel->setVisible(someMetadata);
    //m_licenseLabel->setVisible(someMetadata);
//     m_metadataSeparator->setVisible(someMetadata);

    
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
    KUrl empty;
    KDirSelectDialog dialog(empty, true, this);
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
        stackedWidget->setCurrentIndex(0);
        enableButtons(true);
        update();
        break;
    case kNoBackground:
        m_preview_timer.stop();
        stackedWidget->setCurrentIndex(0);
        enableButtons(false);
        update();
        break;
    case kSlideshowBackground:
        stackedWidget->setCurrentIndex(1);
        updateSlideshow();
        enableButtons(true);
        break;
    }
}

void BackgroundDialog::enableButtons(bool enabled)
{
    m_view->setEnabled(enabled);
    m_resizeMethod->setEnabled(enabled);
    m_pictureUrlButton->setEnabled(enabled);
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

void BackgroundDialog::removeBackground(const QString &path)
{
    m_model->removeBackground(path);
}
