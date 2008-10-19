/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
  Copyright (c) 2008 by Petri Damsten <damu@iki.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgrounddialog.h"

#include <QPainter>
#include <QFile>
#include <QAbstractItemView>
#include <QStandardItemModel>

#include <KStandardDirs>
#include <KDesktopFile>
#include <KColorScheme>
#include <KNS/Engine>

#include <plasma/containment.h>
#include <plasma/panelsvg.h>
#include <plasma/theme.h>
#include <plasma/wallpaper.h>
#include <plasma/view.h>
#include <plasma/corona.h>

#include "wallpaperpreview.h"

typedef QPair<QString, QString> WallpaperInfo;
Q_DECLARE_METATYPE(WallpaperInfo)

class ThemeInfo
{
public:
    QString package;
    Plasma::PanelSvg *svg;
};

class ThemeModel : public QAbstractListModel
{
public:
    enum { PackageNameRole = Qt::UserRole,
           SvgRole = Qt::UserRole + 1
         };

    ThemeModel(QObject *parent = 0);
    virtual ~ThemeModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int indexOf(const QString &path) const;
    void reload();
private:
    QMap<QString, ThemeInfo> m_themes;
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
    reset();
    foreach (const QString& key, m_themes.keys()) {
        delete m_themes[key].svg;
    }
    m_themes.clear();

    // get all desktop themes
    KStandardDirs dirs;
    QStringList themes = dirs.findAllResources("data", "desktoptheme/*/metadata.desktop",
                                               KStandardDirs::NoDuplicates);
    foreach (const QString &theme, themes) {
        kDebug() << theme;
        int themeSepIndex = theme.lastIndexOf('/', -1);
        QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf('/', -1);
        QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);

        KDesktopFile df(theme);
        QString name = df.readName();
        if (name.isEmpty()) {
            name = packageName;
        }

        Plasma::PanelSvg *svg = new Plasma::PanelSvg(this);
        QString svgFile = themeRoot + "/widgets/background.svg";
        if (QFile::exists(svgFile)) {
            svg->setImagePath(svgFile);
        } else {
            svg->setImagePath(svgFile + "z");
        }
        svg->setEnabledBorders(Plasma::PanelSvg::AllBorders);
        ThemeInfo info;
        info.package = packageName;
        info.svg = svg;
        m_themes[name] = info;
    }

    beginInsertRows(QModelIndex(), 0, m_themes.size());
    endInsertRows();
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

    QMap<QString, ThemeInfo>::const_iterator it = m_themes.constBegin();
    for (int i = 0; i < index.row(); ++i) {
        ++it;
    }

    switch (role) {
        case Qt::DisplayRole:
            return it.key();
        case PackageNameRole:
            return (*it).package;
        case SvgRole:
            return qVariantFromValue((void*)(*it).svg);
        default:
            return QVariant();
    }
}

int ThemeModel::indexOf(const QString &name) const
{
    QMapIterator<QString, ThemeInfo> it(m_themes);
    int i = -1;
    while (it.hasNext()) {
        ++i;
        if (it.next().value().package == name) {
            return i;
        }
    }

    return -1;
}


class ThemeDelegate : public QAbstractItemDelegate
{
public:
    ThemeDelegate(QObject * parent = 0);

    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
private:
    static const int MARGIN = 5;
};

ThemeDelegate::ThemeDelegate(QObject* parent)
: QAbstractItemDelegate(parent)
{
}

void ThemeDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QString title = index.model()->data(index, Qt::DisplayRole).toString();
    QString package = index.model()->data(index, ThemeModel::PackageNameRole).toString();

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
    Plasma::PanelSvg *svg = static_cast<Plasma::PanelSvg *>(
            index.model()->data(index, ThemeModel::SvgRole).value<void *>());
    svg->resizePanel(QSize(option.rect.width() - (2 * MARGIN), 100 - (2 * MARGIN)));
    QRect imgRect = QRect(option.rect.topLeft(),
            QSize(option.rect.width() - (2 * MARGIN), 100 - (2 * MARGIN)))
            .translated(MARGIN, MARGIN);
    svg->paintPanel(painter, QPoint(option.rect.left() + MARGIN, option.rect.top() + MARGIN));

    // draw text
    painter->save();
    QFont font = painter->font();
    font.setWeight(QFont::Bold);
    QString colorFile = KStandardDirs::locate("data", "desktoptheme/" + package + "/colors");
    if (!colorFile.isEmpty()) {
        KSharedConfigPtr colors = KSharedConfig::openConfig(colorFile);
        KColorScheme colorScheme(QPalette::Active, KColorScheme::Window, colors);
        painter->setPen(colorScheme.foreground(KColorScheme::NormalText).color());
    }
    painter->setFont(font);
    painter->drawText(option.rect, Qt::AlignCenter | Qt::TextWordWrap, title);
    painter->restore();
}

QSize ThemeDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(200, 100);
}

// From kcategorizeditemsviewdelegate by Ivan Cukic
#define EMBLEM_ICON_SIZE 16
#define UNIVERSAL_PADDING 6
#define FADE_LENGTH 32
#define MAIN_ICON_SIZE 48

class AppletDelegate : public QAbstractItemDelegate
{
public:
    enum { DescriptionRole = Qt::UserRole + 1, PluginNameRole };

    AppletDelegate(QObject * parent = 0);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    int calcItemHeight(const QStyleOptionViewItem& option) const;
};

AppletDelegate::AppletDelegate(QObject* parent)
: QAbstractItemDelegate(parent)
{
}

void AppletDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                           const QModelIndex& index) const
{
    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    const int left = option.rect.left();
    const int top = option.rect.top();
    const int width = option.rect.width();
    const int height = calcItemHeight(option);

    bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);
    QIcon::Mode iconMode = QIcon::Normal;

    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected)) ?
        option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text);

    // Painting main column
    QFont titleFont = option.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);

    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());

    QLinearGradient gradient;

    QString title = index.model()->data(index, Qt::DisplayRole).toString();
    QString description = index.model()->data(index, AppletDelegate::DescriptionRole).toString();

    // Painting

    // Text
    int textInner = 2 * UNIVERSAL_PADDING + MAIN_ICON_SIZE;

    p.setPen(foregroundColor);
    p.setFont(titleFont);
    p.drawText(left + (leftToRight ? textInner : 0),
               top, width - textInner, height / 2,
               Qt::AlignBottom | Qt::AlignLeft, title);
    p.setFont(option.font);
    p.drawText(left + (leftToRight ? textInner : 0),
               top + height / 2,
               width - textInner, height / 2,
               Qt::AlignTop | Qt::AlignLeft, description);

    // Main icon
    const QIcon& icon = qVariantValue<QIcon>(index.model()->data(index, Qt::DecorationRole));
    icon.paint(&p,
        leftToRight ? left + UNIVERSAL_PADDING : left + width - UNIVERSAL_PADDING - MAIN_ICON_SIZE,
        top + UNIVERSAL_PADDING, MAIN_ICON_SIZE, MAIN_ICON_SIZE, Qt::AlignCenter, iconMode);

    // Gradient part of the background - fading of the text at the end
    if (leftToRight) {
        gradient = QLinearGradient(left + width - UNIVERSAL_PADDING - FADE_LENGTH, 0,
                left + width - UNIVERSAL_PADDING, 0);
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, Qt::transparent);
    } else {
        gradient = QLinearGradient(left + UNIVERSAL_PADDING, 0,
                left + UNIVERSAL_PADDING + FADE_LENGTH, 0);
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, Qt::white);
    }

    QRect paintRect = option.rect;
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(paintRect, gradient);

    if (leftToRight) {
        gradient.setStart(left + width - FADE_LENGTH, 0);
        gradient.setFinalStop(left + width, 0);
    } else {
        gradient.setStart(left + UNIVERSAL_PADDING, 0);
        gradient.setFinalStop(left + UNIVERSAL_PADDING + FADE_LENGTH, 0);
    }
    paintRect.setHeight(UNIVERSAL_PADDING + MAIN_ICON_SIZE / 2);
    p.fillRect(paintRect, gradient);
    p.end();

    painter->drawPixmap(option.rect.topLeft(), pixmap);
}

int AppletDelegate::calcItemHeight(const QStyleOptionViewItem& option) const
{
    // Painting main column
    QFont titleFont = option.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);

    int textHeight = QFontInfo(titleFont).pixelSize() + QFontInfo(option.font).pixelSize();
    //kDebug() << textHeight << qMax(textHeight, MAIN_ICON_SIZE) + 2 * UNIVERSAL_PADDING;
    return qMax(textHeight, MAIN_ICON_SIZE) + 2 * UNIVERSAL_PADDING;
}

QSize AppletDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)
    return QSize(200, calcItemHeight(option));
}

BackgroundDialog::BackgroundDialog(const QSize& res, Plasma::Containment *c, Plasma::View* view, QWidget* parent)
    : KDialog(parent),
      m_themeModel(0),
      m_containmentModel(0),
      m_wallpaper(0),
      m_view(view),
      m_containment(c),
      m_preview(0)
{
    setWindowIcon(KIcon("preferences-desktop-wallpaper"));
    setCaption(i18n("Desktop Settings"));
    showButtonSeparator(true);
    setButtons(Ok | Cancel | Apply);

    QWidget * main = new QWidget(this);
    setupUi(main);

    // Size of monitor image: 200x186
    // Geometry of "display" part of monitor image: (23,14)-[151x115]
    qreal previewRatio = (qreal)res.height() / (qreal)res.width();
    QSize monitorSize(200, int(200 * previewRatio));

    Plasma::PanelSvg *svg = new Plasma::PanelSvg(this);
    svg->setImagePath("widgets/monitor");
    svg->resizePanel(monitorSize);
    QPixmap monitorPix(monitorSize + QSize(0, svg->elementSize("base").height() - svg->marginSize(Plasma::BottomMargin)));
    monitorPix.fill(Qt::transparent);

    QPainter painter(&monitorPix);
    QPoint standPosition(monitorSize.width()/2 - svg->elementSize("base").width()/2, svg->contentsRect().bottom());
    svg->paint(&painter, QRect(standPosition, svg->elementSize("base")), "base");
    svg->paintPanel(&painter);
    painter.end();

    m_monitor->setPixmap(monitorPix);
    m_monitor->setWhatsThis(i18n(
        "This picture of a monitor contains a preview of "
        "what the current settings will look like on your desktop."));
    m_preview = new WallpaperPreview(m_monitor);
    m_preview->setGeometry(svg->contentsRect().toRect());

    connect(m_newThemeButton, SIGNAL(clicked()), this, SLOT(getNewThemes()));

    connect(this, SIGNAL(finished(int)), this, SLOT(cleanup()));
    connect(this, SIGNAL(okClicked()), this, SLOT(saveConfig()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(saveConfig()));

    m_themeModel = new ThemeModel(this);
    m_theme->setModel(m_themeModel);
    m_theme->setItemDelegate(new ThemeDelegate(m_theme->view()));
    m_theme->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_containmentModel = new QStandardItemModel(this);
    m_containmentComboBox->setModel(m_containmentModel);
    m_containmentComboBox->setItemDelegate(new AppletDelegate());

    setMainWidget(main);
    reloadConfig();
    adjustSize();
}

BackgroundDialog::~BackgroundDialog()
{
    cleanup();
}

void BackgroundDialog::cleanup()
{
    delete m_wallpaper;
    m_wallpaper = 0;
}

void BackgroundDialog::getNewThemes()
{
    KNS::Engine engine(this);
    if (engine.init("plasma-themes.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(this);

        if (entries.size() > 0) {
            m_themeModel->reload();
            m_theme->setCurrentIndex(m_themeModel->indexOf(
                                     Plasma::Theme::defaultTheme()->themeName()));
        }
    }
}

void BackgroundDialog::reloadConfig()
{
    disconnect(m_wallpaperMode, SIGNAL(currentIndexChanged(int)), this, SLOT(changeBackgroundMode(int)));
    int containmentIndex = 0;
    int wallpaperIndex = 0;

    // Containment
    KPluginInfo::List plugins = Plasma::Containment::listContainments();
    m_containmentModel->clear();
    int i = 0;
    foreach (KPluginInfo info, plugins) {
        QStandardItem* item = new QStandardItem(KIcon(info.icon()), info.name());
        item->setData(info.comment(), AppletDelegate::DescriptionRole);
        item->setData(info.pluginName(), AppletDelegate::PluginNameRole);
        m_containmentModel->appendRow(item);
        if (info.pluginName() == m_containment->pluginName()) {
            containmentIndex = i;
        }
        ++i;
    }
    m_containmentComboBox->setCurrentIndex(containmentIndex);
    m_activityName->setText(m_containment->activity());


    // Wallpaper
    bool doWallpaper = m_containment->drawWallpaper();
    m_wallpaperLabel->setVisible(doWallpaper);
    m_wallpaperGroup->setVisible(doWallpaper);
    if (doWallpaper) {
        // Load wallpaper plugins
        QString currentPlugin;
        QString currentMode;

        Plasma::Wallpaper *currentWallpaper = m_containment->wallpaper();
        if (currentWallpaper) {
            currentPlugin = currentWallpaper->pluginName();
            currentMode = currentWallpaper->renderingMode().name();
        }

        plugins = Plasma::Wallpaper::listWallpaperInfo();
        m_wallpaperMode->clear();
        i = 0;
        foreach (KPluginInfo info, plugins) {
            bool matches = info.pluginName() == currentPlugin;
            const QList<KServiceAction>& modes = info.service()->actions();
            if (modes.count() > 0) {
                foreach (const KServiceAction& mode, modes) {
                    m_wallpaperMode->addItem(KIcon(mode.icon()), mode.text(),
                                    QVariant::fromValue(WallpaperInfo(info.pluginName(), mode.name())));
                    if (matches && mode.name() == currentMode) {
                        wallpaperIndex = i;
                    }
                    ++i;
                }
            } else {
                m_wallpaperMode->addItem(KIcon(info.icon()), info.name(),
                                QVariant::fromValue(WallpaperInfo(info.pluginName(), QString())));
                if (matches) {
                    wallpaperIndex = i;
                }
                ++i;
            }
        }
        m_wallpaperMode->setCurrentIndex(wallpaperIndex);
        changeBackgroundMode(wallpaperIndex);
    }

    // Theme
    m_themeModel->reload();
    m_theme->setCurrentIndex(m_themeModel->indexOf(Plasma::Theme::defaultTheme()->themeName()));

    connect(m_wallpaperMode, SIGNAL(currentIndexChanged(int)), this, SLOT(changeBackgroundMode(int)));
}

void BackgroundDialog::changeBackgroundMode(int mode)
{
    kDebug();
    QWidget* w = 0;
    WallpaperInfo wallpaperInfo = m_wallpaperMode->itemData(mode).value<WallpaperInfo>();

    if (m_wallpaperGroup->layout()->count() > 1) {
        delete dynamic_cast<QWidgetItem*>(m_wallpaperGroup->layout()->takeAt(1))->widget();
    }

    if (m_wallpaper && m_wallpaper->pluginName() != wallpaperInfo.first) {
        delete m_wallpaper;
        m_wallpaper = 0;
    }

    if (!m_wallpaper) {
        m_wallpaper = Plasma::Wallpaper::load(wallpaperInfo.first);
        m_preview->setWallpaper(m_wallpaper);
    }

    if (m_wallpaper) {
        m_wallpaper->setRenderingMode(wallpaperInfo.second);
        KConfigGroup cfg = wallpaperConfig(wallpaperInfo.first);
        kDebug() << "making a" << wallpaperInfo.first << "in mode" << wallpaperInfo.second;
        m_wallpaper->restore(cfg);
        w = m_wallpaper->createConfigurationInterface(m_wallpaperGroup);
    }

    if (!w) {
        w = new QWidget(m_wallpaperGroup);
    }

    m_wallpaperGroup->layout()->addWidget(w);
}

KConfigGroup BackgroundDialog::wallpaperConfig(const QString &plugin)
{
    Q_ASSERT(m_containment);

    //FIXME: we have details about the structure of the containment config duplicated here!
    KConfigGroup cfg = m_containment->config();
    cfg = KConfigGroup(&cfg, "Wallpaper");
    return KConfigGroup(&cfg, plugin);
}

void BackgroundDialog::saveConfig()
{
    QString theme = m_theme->itemData(m_theme->currentIndex(),
                                      ThemeModel::PackageNameRole).toString();
    QString wallpaperPlugin = m_wallpaperMode->itemData(m_wallpaperMode->currentIndex()).value<WallpaperInfo>().first;
    QString wallpaperMode = m_wallpaperMode->itemData(m_wallpaperMode->currentIndex()).value<WallpaperInfo>().second;
    QString containment = m_containmentComboBox->itemData(m_containmentComboBox->currentIndex(),
                                                          AppletDelegate::PluginNameRole).toString();

    // Containment
    if (m_containment->pluginName() != containment) {
        m_containment = m_view->swapContainment(m_containment, containment);
    }

    m_containment->setActivity(m_activityName->text());

    // Wallpaper
    Plasma::Wallpaper *currentWallpaper = m_containment->wallpaper();
    if (currentWallpaper) {
        KConfigGroup cfg = wallpaperConfig(currentWallpaper->pluginName());
        currentWallpaper->save(cfg);
    }

    if (m_wallpaper) {
        KConfigGroup cfg = wallpaperConfig(m_wallpaper->pluginName());
        m_wallpaper->save(cfg);
    }

    m_containment->setWallpaper(wallpaperPlugin, wallpaperMode);

    // Plasma Theme
    Plasma::Theme::defaultTheme()->setThemeName(theme);
}
