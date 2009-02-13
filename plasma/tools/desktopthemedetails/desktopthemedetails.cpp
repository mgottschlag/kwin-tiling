/*
  Copyright (c) 2008 Andrew Lake <jamboarder@yahoo.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "desktopthemedetails.h"

#include <QPainter>
#include <QFile>
#include <QAbstractItemView>
#include <QtGui/QHeaderView>

#include <KIcon>
#include <KAboutData>
#include <KFileDialog>
#include <KMessageBox>
#include <KStandardDirs>
#include <KDesktopFile>
#include <KColorScheme>
#include <KNS/Engine>
#include <KUrl>
#include <KZip>
#include <kio/netaccess.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/job.h>
#include <kgenericfactory.h>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>

//Theme selector code by Andre Duffeck (modified to add package description)
class ThemeInfo
{
public:
    QString package;
    Plasma::FrameSvg *svg;
    QString description;
    QString author;
    QString version;
    QString themeRoot;
};

class ThemeModel : public QAbstractListModel
{
public:
    enum { PackageNameRole = Qt::UserRole,
           SvgRole = Qt::UserRole + 1,
           PackageDescriptionRole = Qt::UserRole + 2,
           PackageAuthorRole = Qt::UserRole + 3,
           PackageVersionRole = Qt::UserRole + 4
         };

    ThemeModel(QObject *parent = 0);
    virtual ~ThemeModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int indexOf(const QString &path) const;
    void reload();
    void clearThemeList();
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
    clearThemeList();
}

void ThemeModel::clearThemeList()
{
    foreach (const QString& key, m_themes.keys()) {
        delete m_themes[key].svg;
    }
    m_themes.clear();
}

void ThemeModel::reload()
{
    reset();
    clearThemeList();

    // get all desktop themes
    KStandardDirs dirs;
    QStringList themes = dirs.findAllResources("data", "desktoptheme/*/metadata.desktop",
                                               KStandardDirs::NoDuplicates);
    foreach (const QString &theme, themes) {
        int themeSepIndex = theme.lastIndexOf('/', -1);
        QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf('/', -1);
        QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);

        KDesktopFile df(theme);
        QString name = df.readName();
        if (name.isEmpty()) {
            name = packageName;
        }
        QString comment = df.readComment();
        QString author = df.desktopGroup().readEntry("X-KDE-PluginInfo-Author",QString());
        QString version = df.desktopGroup().readEntry("X-KDE-PluginInfo-Version",QString());


        Plasma::FrameSvg *svg = new Plasma::FrameSvg(this);
        QString svgFile = themeRoot + "/widgets/background.svg";
        if (QFile::exists(svgFile)) {
            svg->setImagePath(svgFile);
        } else {
            svg->setImagePath(svgFile + "z");
        }
        svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        ThemeInfo info;
        info.package = packageName;
        info.description = comment;
        info.author = author;
        info.version = version;
        info.svg = svg;
        info.themeRoot = themeRoot;
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
        case PackageDescriptionRole:
            return (*it).description;
        case PackageAuthorRole:
            return (*it).author;
        case PackageVersionRole:
            return (*it).version;
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
    Plasma::FrameSvg *svg = static_cast<Plasma::FrameSvg *>(
            index.model()->data(index, ThemeModel::SvgRole).value<void *>());
    svg->resizeFrame(QSize(option.rect.width() - (2 * MARGIN), 100 - (2 * MARGIN)));
    QRect imgRect = QRect(option.rect.topLeft(),
            QSize(option.rect.width() - (2 * MARGIN), 100 - (2 * MARGIN)))
            .translated(MARGIN, MARGIN);
    svg->paintFrame(painter, QPoint(option.rect.left() + MARGIN, option.rect.top() + MARGIN));

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

K_PLUGIN_FACTORY(DesktopThemeDetailsFactory, registerPlugin<DesktopThemeDetails>();)
K_EXPORT_PLUGIN(DesktopThemeDetailsFactory("desktopthemedetails", "kcm_desktopthemedetails"))



struct ThemeItemNameType {
        const char* m_type;
        const char* m_displayItemName;
        const char* m_themeItemPath;
        const char* m_iconName;
};

const ThemeItemNameType themeCollectionName[] = {
    { "Color Scheme", I18N_NOOP2("plasma name", "Color Scheme"),"colors", "preferences-desktop-color"},
    { "Panel Background", I18N_NOOP2("plasma name", "Panel Background"),"widgets/panel-background", "plasma"},
    { "Kickoff", I18N_NOOP2("plasma name", "Kickoff"), "dialogs/kickoff", "kde"},
    { "Task Items", I18N_NOOP2("plasma name", "Task Items"), "widgets/tasks", "preferences-system-windows"},
    { "Widget Background", I18N_NOOP2("plasma name", "Widget Background"), "widgets/background", "plasma"},
    { "Translucent Background", I18N_NOOP2("plasma name", "Translucent Background"), "widgets/translucentbackground", "plasma"},
    { "Dialog Background", I18N_NOOP2("plasma name", "Dialog Background"), "dialogs/background", "plasma"},
    { "Analog Clock", I18N_NOOP2("plasma name", "Analog Clock"), "widgets/clock", "chronometer"},
    { "Notes", I18N_NOOP2("plasma name", "Notes"), "widgets/notes", "view-pim-notes"},
    { "Tooltip", I18N_NOOP2("plasma name", "Tooltip"), "widgets/tooltip", "plasma"},
    { "Pager", I18N_NOOP2("plasma name", "Pager"), "widgets/pager", "plasma"},
    { "Run Command Dialog", I18N_NOOP2("plasma name", "Run Command Dialog"), "dialogs/krunner", "system-run"},
    { "Shutdown Dialog", I18N_NOOP2("plasma name", "Shutdown Dialog"), "dialogs/shutdowndialog", "system-shutdown"},
    { 0, 0,0,0 } // end of data
};


DesktopThemeDetails::DesktopThemeDetails(QWidget* parent, const QVariantList &args)
    : KCModule(DesktopThemeDetailsFactory::componentData(), parent, args),
      m_themeModel(0)

{
    KAboutData *about = new KAboutData("kcm_desktopthemedetails", 0, ki18n("Desktop Theme Details"), "1.0");
    setAboutData(about);
    setButtons(Apply);
    setWindowIcon(KIcon("preferences-desktop"));
    setupUi(this);
    m_newThemeButton->setIcon(KIcon("get-hot-new-stuff"));

    connect(m_newThemeButton, SIGNAL(clicked()), this, SLOT(getNewThemes()));
    //connect(this, SIGNAL(finished(int)), this, SLOT(cleanup()));

    m_themeModel = new ThemeModel(this);
    m_theme->setModel(m_themeModel);
    m_theme->setItemDelegate(new ThemeDelegate(m_theme->view()));
    m_theme->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    reloadConfig();

    connect(m_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(resetThemeDetails()));
    connect(m_enableAdvanced, SIGNAL(toggled(bool)), this, SLOT(toggleAdvancedVisible()));
    connect(m_removeThemeButton, SIGNAL(clicked()), this, SLOT(removeTheme()));
    connect(m_exportThemeButton, SIGNAL(clicked()), this, SLOT(exportTheme()));

    connect(m_newThemeName, SIGNAL(editingFinished()), this, SLOT(newThemeInfoChanged()));

    m_baseTheme = "default";
    m_themeCustomized = false;
    resetThemeDetails();
    adjustSize();
}

DesktopThemeDetails::~DesktopThemeDetails()
{
    cleanup();
}

void DesktopThemeDetails::cleanup()
{
}

void DesktopThemeDetails::getNewThemes()
{
    KNS::Engine engine(this);
    if (engine.init("plasma-themes.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(this);

        if (entries.size() > 0) {
            m_themeModel->reload();
            reloadConfig();
        }
    }
}

void DesktopThemeDetails::reloadConfig()
{
    // Theme
    //QString theme = Plasma::Theme::defaultTheme()->themeName();
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig("plasmarc"), "Theme");
    QString theme = cfg.readEntry("name", "default");
    m_theme->setCurrentIndex(m_themeModel->indexOf(theme));

}

void DesktopThemeDetails::save()
{
    QString themeRoot;
    KStandardDirs dirs;
    if (m_newThemeName->text().isEmpty()) {
        themeRoot = ".customized";
        //Toggle customized theme directory name to ensure theme reload
        if (QDir(dirs.locateLocal("data", "desktoptheme/" + themeRoot + '/', false)).exists()) {
            themeRoot = themeRoot + '1';
        }
    } else {
        themeRoot = m_newThemeName->text().replace(' ',"_").remove(QRegExp("[^A-Za-z0-9_]"));
    }

    //Save theme items
    QFile customSettingsFile;
    bool customSettingsFileOpen = false;

    if (m_themeCustomized || !m_newThemeName->text().isEmpty()) {
        
        clearCustomized(themeRoot);

        //Copy all files from the base theme
        QString baseSource = dirs.locate("data", "desktoptheme/" + m_baseTheme + "/metadata.desktop");
        baseSource = baseSource.left(baseSource.lastIndexOf('/', -1));
        KIO::CopyJob *copyBaseTheme = KIO::copyAs(KUrl(baseSource), KUrl(dirs.locateLocal("data", "desktoptheme/" + themeRoot, true)), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(copyBaseTheme, this);

        //Prepare settings file for customized theme
        if (isCustomized(themeRoot)) {
            customSettingsFile.setFileName(dirs.locateLocal("data", "desktoptheme/" + themeRoot + "/settings"));
            customSettingsFileOpen = customSettingsFile.open(QFile::WriteOnly);
            if (customSettingsFileOpen) {
                QTextStream out(&customSettingsFile);
                out << "baseTheme=" + m_baseTheme + "\r\n";;
            }
        }
        QString settingsSource;

        //Copy each item theme file to new theme folder
        QHashIterator<int, int> i(m_itemThemeReplacements);
        while (i.hasNext()) {
            i.next();
            //Get root directory where item should reside (relative to theme root)
            QString itemRoot = "";
            if (m_itemPaths[i.key()].lastIndexOf('/', -1) != -1) {
                itemRoot= m_itemPaths[i.key()].left(m_itemPaths[i.key()].lastIndexOf('/', -1));
            }
            //Setup source and destination
            QString source;
            QString dest;
            if (i.value() != -1) {
                //Source is a theme
                source = "desktoptheme/" + m_themeRoots[i.value()] + '/' + m_itemPaths[i.key()] + '*';
                dest = "desktoptheme/" + themeRoot + '/' + itemRoot + '/';
                settingsSource = m_themeRoots[i.value()];
            } else {
               //Source is a file
                source = m_itemFileReplacements[i.key()];
                dest = "desktoptheme/" + themeRoot + '/' + itemRoot + '/';
                settingsSource = m_itemFileReplacements[i.key()];
            }


            //Delete item files at destination before copying (possibly there from base theme copy)
            QStringList deleteFiles = dirs.findAllResources("data", "desktoptheme/" + themeRoot + '/' + m_itemPaths[i.key()] + '*',
                                            KStandardDirs::NoDuplicates);
            for (int j = 0; j < deleteFiles.size(); ++j) {
                KIO::DeleteJob *dj = KIO::del(KUrl(deleteFiles.at(j)), KIO::HideProgressInfo);
                KIO::NetAccess::synchronousRun(dj, this);
            }
            
            //Copy item(s)
            dest = dirs.locateLocal("data", dest, true);
            QStringList copyFiles;
            if (i.value() != -1) {
                copyFiles = dirs.findAllResources("data", source, KStandardDirs::NoDuplicates); //copy from theme
            } else {
                copyFiles << source; //copy from file
            }
            for (int j = 0; j < copyFiles.size(); ++j) {
                KIO::CopyJob *cj = KIO::copy(KUrl(copyFiles.at(j)), KUrl(dest), KIO::HideProgressInfo);
                KIO::NetAccess::synchronousRun(cj, this);
            }
            
            //Record settings file
            if (customSettingsFileOpen) {
                QTextStream out(&customSettingsFile);
                out << m_items.key(i.key()) + "=" + settingsSource +"\r\n";
            }
        }
        if (customSettingsFileOpen) customSettingsFile.close();

        // Create new theme FDO desktop file
        QFile::remove(dirs.locateLocal("data", "desktoptheme/" + themeRoot + "/metadata.desktop", false));
        KDesktopFile df(dirs.locateLocal("data", "desktoptheme/" + themeRoot + "/metadata.desktop"));
        KConfigGroup cg = df.desktopGroup();
        if (isCustomized(themeRoot)) {
            cg.writeEntry("Name",i18n("(Customized)"));
            cg.writeEntry("Comment", i18n("User customized theme"));
            cg.writeEntry("X-KDE-PluginInfo-Name", themeRoot);
        } else {
            cg.writeEntry("Name", m_newThemeName->text());
            cg.writeEntry("Comment", m_newThemeDescription->text());
            cg.writeEntry("X-KDE-PluginInfo-Author", m_newThemeAuthor->text());
            cg.writeEntry("X-KDE-PluginInfo-Version", m_newThemeVersion->text());
        }
        cg.sync();

        m_themeCustomized = false;
    }

    m_themeModel->reload();
    if (m_themeModel->indexOf(themeRoot) != -1) {
        m_theme->setCurrentIndex(m_themeModel->indexOf(themeRoot));
        //FIXME: should say "Appearance Settings" instead of "Desktop Settings"
        KMessageBox::information(this,i18n("To change your desktop theme to \"%1\", open\n the desktop Appearance Settings and select \"%2\" from the droplist.",m_theme->currentText(),m_theme->currentText() ), i18n("How to Change Desktop Theme"), "HowToChangeDesktopTheme");
    }
    resetThemeDetails();
}

void DesktopThemeDetails::removeTheme()
{
    bool removeTheme = true;
    QString theme = m_theme->itemData(m_theme->currentIndex(),
                                      ThemeModel::PackageNameRole).toString();
    if (m_themeCustomized) {
        if(KMessageBox::questionYesNo(this, i18n("Theme items have been changed.  Do you still wish remove the \"%1\" theme?", m_theme->currentText()), i18n("Remove Desktop Theme")) == KMessageBox::No) {
            removeTheme = false;
        }
    } else {
        if (theme == "default") {
            KMessageBox::information(this, i18n("Removal of the default KDE theme is not allowed."), i18n("Remove Desktop Theme"));
            removeTheme = false;
        } else {
            if(KMessageBox::questionYesNo(this, i18n("Are you sure you wish remove the \"%1\" theme?", m_theme->currentText()), i18n("Remove Desktop Theme")) == KMessageBox::No) {
                removeTheme = false;
            }
        }

    }
    KStandardDirs dirs;
    if (removeTheme) {
        if (QDir(dirs.locateLocal("data", "desktoptheme/" + theme, false)).exists()) {
            KIO::DeleteJob *deleteTheme = KIO::del(KUrl(dirs.locateLocal("data", "desktoptheme/" + theme, false)), KIO::HideProgressInfo);
            KIO::NetAccess::synchronousRun(deleteTheme, this);
        }
    }
    m_themeModel->reload();
    reloadConfig();
}

void DesktopThemeDetails::exportTheme()
{
    QString theme = m_theme->itemData(m_theme->currentIndex(),
                                      ThemeModel::PackageNameRole).toString();
    if (m_themeCustomized ||
        (isCustomized(theme) && m_newThemeName->text() == "")) {
        KMessageBox::information(this, i18n("Please apply theme item changes (with a new theme name) before attempting to export theme."), i18n("Export Desktop Theme"));
    } else {
        QString themeStoragePath = theme;

        KStandardDirs dirs;
        QString themePath = dirs.findResource("data", "desktoptheme/" + themeStoragePath + "/metadata.desktop");
        if (!themePath.isEmpty()){
            QString expFileName = KFileDialog::getSaveFileName(KUrl(), "*.zip", this, i18n("Export theme to file"));
            if (!expFileName.endsWith(".zip"))
                expFileName = expFileName + ".zip";
            if (!expFileName.isEmpty()) {
                KUrl path(themePath);
                KZip expFile(expFileName);
                expFile.open(QIODevice::WriteOnly);
                expFile.addLocalDirectory(path.directory (), themeStoragePath);
                expFile.close();
            }
        }
    }

}

void DesktopThemeDetails::loadThemeItems()
{
    // Set up items, item paths and  icons
    m_items.clear(); // clear theme items
    m_itemPaths.clear(); // clear theme item paths
    m_itemIcons.clear();
    for (int i = 0; themeCollectionName[i].m_type; ++i) {
        m_items[themeCollectionName[i].m_type] = i;
        m_itemPaths[i] = themeCollectionName[i].m_themeItemPath;
        m_itemIcons[i] = themeCollectionName[i].m_iconName;
    }

    // Get installed themes
    m_themes.clear(); // clear installed theme list
    m_themeRoots.clear(); // clear installed theme root paths
    KStandardDirs dirs;
    QStringList themes = dirs.findAllResources("data", "desktoptheme/*/metadata.desktop",
                                               KStandardDirs::NoDuplicates);
    themes.sort();
    int j=0;
    for (int i = 0; i < themes.size(); ++i) {
        QString theme = themes.at(i);
        int themeSepIndex = theme.lastIndexOf('/', -1);
        QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf('/', -1);
        QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);
        KDesktopFile df(theme);
        QString name = df.readName();
        if (name.isEmpty()) {
            name = packageName;
        }
        
        if (!isCustomized(packageName) && (m_themeRoots.key(packageName, -1) == -1)) {
            m_themes[name] = j;
            m_themeRoots[j] = packageName;
            ++j;
        }
    }

    // Set up default item replacements
    m_itemThemeReplacements.clear();
    m_itemFileReplacements.clear();
    QString currentTheme = m_theme->itemData(m_theme->currentIndex(),
                                      ThemeModel::PackageNameRole).toString();
    if (!isCustomized(currentTheme)) {
        // Set default replacements to current theme
        QHashIterator<QString, int> i(m_items);
        while (i.hasNext()) {
            i.next();
            m_itemThemeReplacements[i.value()] = m_themeRoots.key(currentTheme); 
        }
        m_baseTheme = currentTheme;
    } else {
        // Set default replacements to customized theme settings
        QFile customSettingsFile(dirs.locateLocal("data", "desktoptheme/" + currentTheme +"/settings"));
        if (customSettingsFile.open(QFile::ReadOnly)) {
            QTextStream in(&customSettingsFile);
            QString line;
            QStringList settingsPair;
            while (!in.atEnd()) {
                line = in.readLine();
                settingsPair = line.split('=');
                if (settingsPair.at(0) == "baseTheme") {
                    m_baseTheme = settingsPair.at(1);
                } else {
                    if (m_themeRoots.key(settingsPair.at(1), -1) != -1) { // theme for current item exists
                        m_itemThemeReplacements[m_items[settingsPair.at(0)]] = m_themeRoots.key(settingsPair.at(1));
                    } else if (QFile::exists(settingsPair.at(1))) {
                        m_itemThemeReplacements[m_items[settingsPair.at(0)]] = -1;
                        m_itemFileReplacements[m_items[settingsPair.at(0)]] = settingsPair.at(1);
                    }
                }
            }
            customSettingsFile.close();
        }
    }

    // Build displayed list of theme items
    m_themeItemList->setRowCount(m_items.size());
    m_themeItemList->setColumnCount(2);
    m_themeItemList->setHorizontalHeaderLabels(QStringList()<< i18n("Theme Item")<<i18n("Source"));
    QString displayedItem;
    QHashIterator<QString, int> i(m_items);
    while (i.hasNext()) {
        i.next();
        displayedItem = displayedItemText(i.value());
        m_themeItemList->setItem(i.value(), 0, new QTableWidgetItem(displayedItem));
        m_themeItemList->item(i.value(),0)->setIcon(KIcon(m_itemIcons[i.value()]));
        m_themeItemList->setCellWidget(i.value(), 1, new QComboBox());
        updateReplaceItemList(i.value());
        m_themeItemList->resizeColumnToContents(1);
    }
    m_themeItemList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_themeItemList->verticalHeader()->hide();
    m_themeItemList->horizontalHeader()->setStretchLastSection(true);
    m_themeItemList->horizontalHeader()->setMinimumSectionSize(120);
    m_themeItemList->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);;
    m_themeItemList->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);;
    m_themeItemList->setCurrentCell(0, 1);
}

void DesktopThemeDetails::updateReplaceItemList(const int& item)
{
    QString currentTheme = m_theme->itemData(m_theme->currentIndex(),
                                      ThemeModel::PackageNameRole).toString();

    // Repopulate combobox droplist
    QComboBox *itemComboBox = static_cast<QComboBox*>(m_themeItemList->cellWidget(item,1));
    disconnect(itemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(replacementItemChanged()));
    itemComboBox->clear();
    for (int i = 0; i < m_themes.size(); ++i) {
       QString displayedDropListItem = i18n("%1 %2", m_themes.key(i), displayedItemText(item));
       itemComboBox->addItem(displayedDropListItem);
    }
    itemComboBox->addItem(i18n("File..."));

    // Set combobox value to current replacement
    if (m_itemThemeReplacements[item] != -1) {
        itemComboBox->setCurrentIndex(m_itemThemeReplacements[item]);
    } else {
        itemComboBox->addItem(m_itemFileReplacements[item]);
        itemComboBox->setCurrentIndex(itemComboBox->findText(m_itemFileReplacements[item]));
    }
    connect(itemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(replacementItemChanged()));
}

void DesktopThemeDetails::replacementItemChanged()
{
    //Check items to see if theme has been customized
    m_themeCustomized = true;
    QHashIterator<QString, int> i(m_items);
    while (i.hasNext()) {
        i.next();
        QComboBox *itemComboBox = static_cast<QComboBox*>(m_themeItemList->cellWidget(i.value(), 1));
        int replacement = itemComboBox->currentIndex();
        if (replacement <= (m_themes.size() - 1)) {
            // Item replacement source is a theme
            m_itemThemeReplacements[i.value()] = itemComboBox->currentIndex();
        } else if (replacement > (m_themes.size() - 1)) {
            // Item replacement source is a file
            if (itemComboBox->currentText() == i18n("File...")) {
                //Get the filename for the replacement item
                QString fileReplacement = KFileDialog::getOpenFileName(KUrl(), QString(), this, i18n("Select File to Use for %1",i.key()));  
                if (!fileReplacement.isEmpty()) {
                    m_itemFileReplacements[i.value()] = fileReplacement;
                    int index = itemComboBox->findText(fileReplacement);
                    if (index == -1) itemComboBox->addItem(fileReplacement);
                    disconnect(itemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(replacementItemChanged()));
                    itemComboBox->setCurrentIndex(itemComboBox->findText(fileReplacement));
                    connect(itemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(replacementItemChanged()));
                    m_itemThemeReplacements[i.value()] = -1; //source is not a theme
                    m_itemFileReplacements[i.value()] = itemComboBox->currentText();
                } else {
                    // Reset combobox to previous value if no file is selected
                    if (m_itemThemeReplacements[i.value()] != -1) {
                        itemComboBox->setCurrentIndex(m_itemThemeReplacements[i.value()]);
                    } else {
                        itemComboBox->setCurrentIndex(itemComboBox->findText(m_itemFileReplacements[i.value()]));
                    }
                    m_themeCustomized = false;
                }
            } else {
                m_itemThemeReplacements[i.value()] = -1; //source is not a theme
                m_itemFileReplacements[i.value()] = itemComboBox->currentText();
            }
        }
    }

    if (m_themeCustomized) emit changed();
}

void DesktopThemeDetails::newThemeInfoChanged()
{
    emit changed();
}

void DesktopThemeDetails::resetThemeDetails()
{
    QString theme = m_theme->itemData(m_theme->currentIndex(),
                                      ThemeModel::PackageNameRole).toString();
    m_themeInfoName->setText(m_theme->currentText());
    m_themeInfoDescription->setText(m_theme->itemData(m_theme->currentIndex(),
                                ThemeModel::PackageDescriptionRole).toString());
    QString author = m_theme->itemData(m_theme->currentIndex(),
                                ThemeModel::PackageAuthorRole).toString();
    if (!author.isEmpty()) {
        m_themeInfoAuthor->setText(i18n(" Author: %1",author));
    } else {
        m_themeInfoAuthor->setText("");
    }
    QString version = m_theme->itemData(m_theme->currentIndex(),
                                ThemeModel::PackageVersionRole).toString();
    if (!version.isEmpty()) {
       m_themeInfoVersion->setText(i18n("Version: %1",version));
    } else {
       m_themeInfoVersion->setText("");
    }

    loadThemeItems();

    m_newThemeName->clear();
    m_newThemeAuthor->clear();
    m_newThemeVersion->clear();
    m_newThemeDescription->clear();
    m_enableAdvanced->setChecked(false);
    toggleAdvancedVisible();
    m_themeCustomized = false;
 }

void DesktopThemeDetails::toggleAdvancedVisible()
{
    m_newThemeNameLabel->setVisible(m_enableAdvanced->isChecked());
    m_newThemeName->setVisible(m_enableAdvanced->isChecked());
    m_newThemeAuthor->setVisible(m_enableAdvanced->isChecked());
    m_newThemeAuthorLabel->setVisible(m_enableAdvanced->isChecked());
    m_newThemeVersion->setVisible(m_enableAdvanced->isChecked());
    m_newThemeVersionLabel->setVisible(m_enableAdvanced->isChecked());
    m_newThemeDescriptionLabel->setVisible(m_enableAdvanced->isChecked());
    m_newThemeDescription->setVisible(m_enableAdvanced->isChecked());
    m_exportThemeButton->setVisible(m_enableAdvanced->isChecked());
    m_removeThemeButton->setVisible(m_enableAdvanced->isChecked());
    m_advancedLine->setVisible(m_enableAdvanced->isChecked());
}

bool DesktopThemeDetails::isCustomized(const QString& theme) {
    if (theme == ".customized" || theme == ".customized1") {
        return true;
    } else {
        return false;
    }
}

void DesktopThemeDetails::clearCustomized(const QString& themeRoot) {
    KStandardDirs dirs;
    
    if ((isCustomized(themeRoot))) {
        // Remove both possible unnamed customized directories
        if (QDir(dirs.locateLocal("data", "desktoptheme/.customized", false)).exists()) {
            KIO::DeleteJob *clearCustom = KIO::del(KUrl(dirs.locateLocal("data", "desktoptheme/.customized", false)), KIO::HideProgressInfo);
            KIO::NetAccess::synchronousRun(clearCustom, this);
        }
        if (QDir(dirs.locateLocal("data", "desktoptheme/.customized1", false)).exists()) {
            KIO::DeleteJob *clearCustom1 = KIO::del(KUrl(dirs.locateLocal("data", "desktoptheme/.customized1", false)), KIO::HideProgressInfo);
            KIO::NetAccess::synchronousRun(clearCustom1, this);
        }
    } else {
        if (QDir(dirs.locateLocal("data", "desktoptheme/" + themeRoot, false)).exists()) {
            KIO::DeleteJob *clearCustom = KIO::del(KUrl(dirs.locateLocal("data", "desktoptheme/" + themeRoot, false)), KIO::HideProgressInfo);
            KIO::NetAccess::synchronousRun(clearCustom, this);
        }
    }   
}

QString DesktopThemeDetails::displayedItemText(int item) {
    QString displayedText = m_items.key(item);  
    for (int i = 0; themeCollectionName[i].m_type; ++i) {
        if (themeCollectionName[i].m_type == m_items.key(item)) {
            displayedText = i18nc("plasma name", themeCollectionName[i].m_displayItemName);
        }
    }
    return displayedText;
}


