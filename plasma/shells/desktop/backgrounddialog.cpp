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
#include "mouseplugins.h"

#include <QPainter>
#include <QFile>
#include <QAbstractItemView>
#include <QStandardItemModel>

#include <KStandardDirs>
#include <KDesktopFile>
#include <KColorScheme>
#include <KNS/Engine>

#include <Plasma/Containment>
#include <Plasma/FrameSvg>
#include <Plasma/Wallpaper>
#include <Plasma/View>
#include <Plasma/Corona>

#include "kworkspace/screenpreviewwidget.h"

typedef QPair<QString, QString> WallpaperInfo;
Q_DECLARE_METATYPE(WallpaperInfo)

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

void WallpaperWidget::settingsChanged(bool isModified)
{
    emit modified(isModified);
}

BackgroundDialog::BackgroundDialog(const QSize& res, Plasma::Containment *c, Plasma::View* view,
                                   QWidget* parent, const QString &id, KConfigSkeleton *s)
    : KConfigDialog(parent, id, s),
      m_containmentModel(0),
      m_wallpaper(0),
      m_view(view),
      m_containment(c),
      m_preview(0),
      m_modified(false)
{
    setWindowIcon(KIcon("preferences-desktop-wallpaper"));
    setCaption(i18n("Desktop Settings"));
    showButtonSeparator(true);
    setButtons(Ok | Cancel | Apply);

    QWidget *main= new QWidget(this);
    setupUi(main);
    m_appearanceItem = addPage(main, i18n("Appearance"), "preferences-desktop-wallpaper");
    
    QWidget *activity = new QWidget(this);
    activityUi.setupUi(activity);
    addPage(activity, i18n("Activity"), "activity");
    
    qreal previewRatio = (qreal)res.width() / (qreal)res.height();
    QSize monitorSize(200, int(200 * previewRatio));

    m_monitor->setFixedSize(200, 200);
    m_monitor->setText(QString());
    m_monitor->setWhatsThis(i18n(
        "This picture of a monitor contains a preview of "
        "what the current settings will look like on your desktop."));
    m_preview = new ScreenPreviewWidget(m_monitor);
    m_preview->setRatio(previewRatio);
    m_preview->resize(200, 200);

    connect(this, SIGNAL(finished(int)), this, SLOT(cleanup()));
    connect(this, SIGNAL(okClicked()), this, SLOT(saveConfig()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(saveConfig()));

    if (m_containment) {
        connect(m_containment, SIGNAL(destroyed()), this, SLOT(close()));
    }

    m_containmentModel = new QStandardItemModel(this);
    activityUi.m_containmentComboBox->setModel(m_containmentModel);
    activityUi.m_containmentComboBox->setItemDelegate(new AppletDelegate());

    MousePlugins *m = new MousePlugins(m_containment, this);
    connect(m, SIGNAL(modified(bool)), this, SLOT(settingsModified(bool)));
    m_mouseItem = addPage(m, i18n("Mouse Plugins"), "mouseplugins");

    if (m_containment && m_containment->hasConfigurationInterface()) {
        m_containment->createConfigurationInterface(this);
    }

    reloadConfig();
    adjustSize();

    connect(activityUi.m_containmentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsModified()));
    connect(activityUi.m_activityName, SIGNAL(textChanged(const QString&)), this, SLOT(settingsModified()));
    connect(activityUi.m_activityName, SIGNAL(editingFinished()), this, SLOT(checkActivityName()));
    
    connect(m_wallpaperMode, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsModified()));

    settingsModified(false);
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

void BackgroundDialog::checkActivityName()
{
    if (m_containment && activityUi.m_activityName->text().isEmpty()) {
        activityUi.m_activityName->setText(m_containment->activity());
    }
}

void BackgroundDialog::reloadConfig()
{
    disconnect(m_wallpaperMode, SIGNAL(currentIndexChanged(int)), this, SLOT(changeBackgroundMode(int)));
    int containmentIndex = 0;
    int wallpaperIndex = 0;

    // Containment
    KPluginInfo::List plugins = Plasma::Containment::listContainmentsOfType("desktop");
    m_containmentModel->clear();
    int i = 0;
    foreach (const KPluginInfo& info, plugins) {
        QStandardItem* item = new QStandardItem(KIcon(info.icon()), info.name());
        item->setData(info.comment(), AppletDelegate::DescriptionRole);
        item->setData(info.pluginName(), AppletDelegate::PluginNameRole);
        m_containmentModel->appendRow(item);

        if (m_containment && info.pluginName() == m_containment->pluginName()) {
            containmentIndex = i;
        }

        ++i;
    }

    activityUi.m_containmentComboBox->setCurrentIndex(containmentIndex);
    
    if (m_containment) {
        activityUi.m_activityName->setText(m_containment->activity());
    }

    // Wallpaper
    bool doWallpaper = !m_containment || m_containment->drawWallpaper();
    #if 0
    m_wallpaperLabel->setVisible(doWallpaper);
    #endif
    m_wallpaperGroup->setVisible(doWallpaper);
    m_monitor->setVisible(doWallpaper);
    m_preview->setVisible(doWallpaper);
    //kDebug() << "do wallpapers?!" << doWallpaper;
    if (doWallpaper) {
        // Load wallpaper plugins
        QString currentPlugin;
        QString currentMode;

        Plasma::Wallpaper *currentWallpaper = m_containment ? m_containment->wallpaper() : 0;
        if (currentWallpaper) {
            currentPlugin = currentWallpaper->pluginName();
            currentMode = currentWallpaper->renderingMode().name();
            KConfigGroup cg = wallpaperConfig(currentPlugin);
            currentWallpaper->save(cg);
        }

        plugins = Plasma::Wallpaper::listWallpaperInfo();
        m_wallpaperMode->clear();
        int i = 0;
        foreach (const KPluginInfo& info, plugins) {
            //kDebug() << "doing wallpaper" << info.pluginName() << currentPlugin;
            bool matches = info.pluginName() == currentPlugin;
            const QList<KServiceAction>& modes = info.service()->actions();
            if (modes.count() > 0) {
                if (matches) {
                    wallpaperIndex = i;
                    //kDebug() << "matches at" << wallpaperIndex;
                }

                foreach (const KServiceAction& mode, modes) {
                    m_wallpaperMode->addItem(KIcon(mode.icon()), mode.text(),
                                    QVariant::fromValue(WallpaperInfo(info.pluginName(), mode.name())));
                    //kDebug() << matches << mode.name() << currentMode;
                    if (matches && mode.name() == currentMode) {
                        wallpaperIndex = i;
                        //kDebug() << "matches at" << wallpaperIndex;
                    }
                    ++i;
                }
            } else {
                m_wallpaperMode->addItem(KIcon(info.icon()), info.name(),
                                QVariant::fromValue(WallpaperInfo(info.pluginName(), QString())));
                if (matches) {
                    wallpaperIndex = i;
                    //kDebug() << "matches at" << wallpaperIndex;
                }

                ++i;
            }
        }

        //kDebug() << "match is said to be" << wallpaperIndex << "out of" << m_wallpaperMode->count();
        m_wallpaperMode->setCurrentIndex(wallpaperIndex);
        changeBackgroundMode(wallpaperIndex);
    }

    connect(m_wallpaperMode, SIGNAL(currentIndexChanged(int)), this, SLOT(changeBackgroundMode(int)));
    settingsModified(false);
}

void BackgroundDialog::changeBackgroundMode(int mode)
{
    kDebug();
    QWidget* w = 0;
    WallpaperInfo wallpaperInfo = m_wallpaperMode->itemData(mode).value<WallpaperInfo>();

    if (m_wallpaperGroup->layout()->count() > 1) {
        QLayoutItem *item = m_wallpaperGroup->layout()->takeAt(1);
        QWidget *widget = item->widget();
        delete item;
        delete widget;
    }

    if (m_wallpaper && m_wallpaper->pluginName() != wallpaperInfo.first) {
        delete m_wallpaper;
        m_wallpaper = 0;
    }

    if (!m_wallpaper) {
        m_wallpaper = Plasma::Wallpaper::load(wallpaperInfo.first);
        m_preview->setPreview(m_wallpaper);
    }

    if (m_wallpaper) {
        m_wallpaper->setRenderingMode(wallpaperInfo.second);
        KConfigGroup cfg = wallpaperConfig(wallpaperInfo.first);
        //kDebug() << "making a" << wallpaperInfo.first << "in mode" << wallpaperInfo.second;
        if (m_containment) {
            m_wallpaper->setTargetSizeHint(m_containment->size());
        }
        m_wallpaper->restore(cfg);

        WallpaperWidget *wallpaperWidget = new WallpaperWidget(m_wallpaperGroup);
        w = m_wallpaper->createConfigurationInterface(wallpaperWidget);
        connect(wallpaperWidget, SIGNAL(modified(bool)), this, SLOT(settingsModified(bool)));
    }

    if (!w) {
        w = new QWidget(m_wallpaperGroup);
    } else if (w->layout()) {
        QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(w->layout());
        if (gridLayout) {
            gridLayout->setColumnMinimumWidth(0, m_wallpaperTypeLabel->geometry().right());
            gridLayout->setColumnStretch(0, 0);
            gridLayout->setColumnStretch(1, 10);
            gridLayout->setContentsMargins(0, 0, 0, 0);
        }
    }

    m_wallpaperGroup->layout()->addWidget(w);
    settingsModified(true);
}

KConfigGroup BackgroundDialog::wallpaperConfig(const QString &plugin)
{
    //FIXME: we have details about the structure of the containment config duplicated here!
    KConfigGroup cfg = m_containment ? m_containment->config() : KConfigGroup(KGlobal::config(), "Wallpaper");
    cfg = KConfigGroup(&cfg, "Wallpaper");
    return KConfigGroup(&cfg, plugin);
}

void BackgroundDialog::saveConfig()
{
    QString wallpaperPlugin = m_wallpaperMode->itemData(m_wallpaperMode->currentIndex()).value<WallpaperInfo>().first;
    QString wallpaperMode = m_wallpaperMode->itemData(m_wallpaperMode->currentIndex()).value<WallpaperInfo>().second;
    QString containment = activityUi.m_containmentComboBox->itemData(activityUi.m_containmentComboBox->currentIndex(),
                                                          AppletDelegate::PluginNameRole).toString();

    // Containment
    if (m_containment) {
        if (m_containment->pluginName() != containment) {
            disconnect(m_containment, SIGNAL(destroyed()), this, SLOT(close()));
            disconnect(this, 0, m_containment, 0);

            m_containment = m_view->swapContainment(m_containment, containment);
            emit containmentPluginChanged(m_containment);

            //remove all pages but our own
            KPageWidgetModel *m = qobject_cast<KPageWidgetModel *>(pageWidget()->model());
            if (m) {
                int rows = m->rowCount();
                QList<KPageWidgetItem *> itemsToRemove;
                for (int i = 0; i < rows; ++i) {
                    QModelIndex idx = m->index(i, 0);

                    if (!idx.isValid()) {
                        continue;
                    }

                    KPageWidgetItem *item = m->item(idx);

                    if (item && item != m_appearanceItem && item != m_mouseItem) {
                        itemsToRemove.append(item);
                    }
                }

                foreach (KPageWidgetItem *item, itemsToRemove) {
                    removePage(item);
                }
            }

            //add the new containment's config
            if (m_containment->hasConfigurationInterface()) {
                m_containment->createConfigurationInterface(this);
            }
            connect(m_containment, SIGNAL(destroyed()), this, SLOT(close()));
        }

        m_containment->setActivity(activityUi.m_activityName->text());

        // Wallpaper
        Plasma::Wallpaper *currentWallpaper = m_containment->wallpaper();
        if (currentWallpaper) {
            KConfigGroup cfg = wallpaperConfig(currentWallpaper->pluginName());
            currentWallpaper->save(cfg);
        }
    }

    if (m_wallpaper) {
        KConfigGroup cfg = wallpaperConfig(m_wallpaper->pluginName());
        m_wallpaper->save(cfg);
    }

    if (m_containment) {
        m_containment->setWallpaper(wallpaperPlugin, wallpaperMode);
    }

    settingsModified(false);
}

void BackgroundDialog::settingsModified(bool modified)
{
    m_modified = modified;
    updateButtons();
}

bool BackgroundDialog::hasChanged()
{
    return m_modified;
}
