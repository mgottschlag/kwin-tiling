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
        const char* m_widgetType;
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

    connect(m_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(resetThemeDetails()));
    connect(m_enableAdvanced, SIGNAL(toggled(bool)), this, SLOT(toggleAdvancedVisible()));
    connect(m_removeThemeButton, SIGNAL(clicked()), this, SLOT(removeTheme()));
    connect(m_exportThemeButton, SIGNAL(clicked()), this, SLOT(exportTheme()));
    resetThemeDetails();
    m_themeCustomized = false;
    m_baseTheme = "default";
    reloadConfig();
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
    QString theme;
    if (m_newThemeName->text().isEmpty()) {
        theme = ".customized";
    } else {
        theme = m_newThemeName->text().replace(' ',"_").remove(QRegExp("[^A-Za-z0-9_]"));
    }

    //Customized Theme
    bool newThemeExists = false;
    KStandardDirs dirs;
    QFile customSettingsFile;
    bool customSettingsFileOpen = false;
    if (m_themeCustomized || !m_newThemeName->text().isEmpty()) {
        //Toggle theme directory name to ensure theme reload
        if (QDir(dirs.locateLocal("data", "desktoptheme/" + theme + '/', false)).exists()) {
            theme = theme + '1';
        }
        clearCustomized();

        //Copy all files from the base theme
        QString baseSource = dirs.locate("data", "desktoptheme/" + m_baseTheme + '/');
        KIO::CopyJob *copyBaseTheme = KIO::copyAs(KUrl(baseSource), KUrl(dirs.locateLocal("data", "desktoptheme/" + theme, true)), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(copyBaseTheme, this);

        //Prepare settings file for customized theme
        if (isCustomized(theme)) {
            customSettingsFile.setFileName(dirs.locateLocal("data", "desktoptheme/" + theme + "/settings"));
            customSettingsFileOpen = customSettingsFile.open(QFile::WriteOnly);
            if (customSettingsFileOpen) {
                QTextStream out(&customSettingsFile);
                out << "baseTheme=" + m_baseTheme + "\r\n";;
            }
        }


        //Copy each theme file to new theme folder
        QHashIterator<QString, QString> i(m_themeReplacements);
        while (i.hasNext()) {
            i.next();
            QString source = i.value();
            QString itemDir = "desktoptheme/" + theme + '/' + m_themeItems[i.key()];
            if (source.right(4).toLower() == ".svg") {
                itemDir.append(".svg");
            } else if (source.right(5).toLower() == ".svgz") {
                itemDir.append(".svgz");
            }
            QString dest = dirs.locateLocal("data", itemDir, true);
            if (QFile::exists(source)) {
               QFile::remove(dirs.locateLocal("data", "desktoptheme/" + theme + '/' + m_themeItems[i.key()] + ".svg"));
               QFile::remove(dirs.locateLocal("data", "desktoptheme/" + theme + '/' + m_themeItems[i.key()] + ".svgz"));
               KIO::file_copy(KUrl(source), KUrl(dest), -1, KIO::HideProgressInfo);
            }
            //Save setting for this theme item
            if (customSettingsFileOpen) {
                QTextStream out(&customSettingsFile);
                if (m_dropListFiles.key(source).startsWith("File:") || m_dropListFiles.key(source).startsWith(i18n("(Customized)"))) {
                    out << i.key() + "=" + dest +"\r\n";
                } else {
                    out << i.key() + "=" + source +"\r\n";
                }
            }

        }
        if (customSettingsFileOpen) customSettingsFile.close();

        // Create new theme FDO desktop file
        QFile::remove(dirs.locateLocal("data", "desktoptheme/" + theme + "/metadata.desktop", false));
        QFile desktopFile(dirs.locateLocal("data", "desktoptheme/" + theme +"/metadata.desktop"));
        QString desktopFileData;
        if (isCustomized(theme)) {
            desktopFileData = QString("Name=%1 \r\nComment=%2 \r\nX-KDE-PluginInfo-Name=%3\r\n").arg(i18n("(Customized)")).arg(i18n("User customized theme")).arg(theme);
        } else {
            desktopFileData = "Name=" + m_newThemeName->text() + " \r\n Comment=" + m_newThemeDescription->text() + " \r\n X-KDE-PluginInfo-Author=" + m_newThemeAuthor->text() + " \r\n X-KDE-PluginInfo-Name=" + theme + " \r\n X-KDE-PluginInfo-Version=" + m_newThemeVersion->text();
        }
        if (desktopFile.open(QFile::WriteOnly)) {
            QTextStream out(&desktopFile);
            out << "[Desktop Entry] \r\n" + desktopFileData +" \r\n";
            desktopFile.close();
            newThemeExists = true;
        } else {
            KMessageBox::error(this, i18n("Unable to save theme."), i18n("Desktop Theme Details"));
        }
        m_themeCustomized = false;
    }

    // Plasma Theme
    //Plasma::Theme::defaultTheme()->setThemeName(theme);

    if (newThemeExists) {
        m_themeModel->reload();
        m_theme->setCurrentIndex(m_themeModel->indexOf(theme));
        //FIXME: should say "Appearance Settings" instead of "Desktop Settings"
        KMessageBox::information(this,i18n("To change your desktop theme to \"%1\", open\nDesktop Settings and select \"%2\" from the droplist.",m_theme->currentText(),m_theme->currentText() ), i18n("How to Change Desktop Theme"), "HowToChangeDesktopTheme");
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
    if (m_themeCustomized ||
        (m_theme->currentText() == i18n("(Customized)") && m_newThemeName->text() == "")) {
        KMessageBox::information(this, i18n("Please apply theme item changes (with a new theme name) before attempting to export theme."), i18n("Export Desktop Theme"));
    } else {
        QString themeStoragePath = m_theme->itemData(m_theme->currentIndex(),
                                                     ThemeModel::PackageNameRole).toString();
        KStandardDirs dirs;
        kDebug()<<" themeStoragePath "<<themeStoragePath;

        if ( themeStoragePath == "default")
        {
            KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig("plasmarc"), "Theme");
            themeStoragePath = cfg.readEntry("name", "default");
        }

        QString themePath = dirs.findResource("data", "desktoptheme/" + themeStoragePath + "/metadata.desktop");
        if (!themePath.isEmpty())
        {
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
    QStringList themeItemList;
    QStringList themeItemIconList;
    m_themeItems.clear();
    m_themeReplacements.clear();
    m_themeItemList->clear();
    m_dropListFiles.clear();


    for (int i = 0; themeCollectionName[i].m_type; ++i) {
        m_themeItems[themeCollectionName[i].m_type] = themeCollectionName[i].m_widgetType;
        themeItemList.append(i18nc("plasma name", themeCollectionName[i].m_displayItemName));
        m_themeReplacements[i18nc("plasma name", themeCollectionName[i].m_displayItemName)] = "";
        themeItemIconList.append(themeCollectionName[i].m_iconName);
    }


    m_themeItemList->setRowCount(themeItemList.size());
    m_themeItemList->setColumnCount(2);
    m_themeItemList->setHorizontalHeaderLabels(QStringList()<< i18n("Theme Item")<<i18n("Source"));
    QString item;
    QStringListIterator i(themeItemList);
    int row = 0;
    while (i.hasNext()) {
        item = i.next();
        m_themeItemList->setItem(row, 0, new QTableWidgetItem(item));
        m_themeItemList->item(row,0)->setIcon(KIcon(themeItemIconList.at(row)));
        m_themeItemList->setCellWidget(row, 1, new QComboBox());
        updateReplaceItemList(item);
        m_themeItemList->resizeColumnToContents(1);
        row++;
    }
    m_themeItemList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_themeItemList->verticalHeader()->hide();
    m_themeItemList->horizontalHeader()->setStretchLastSection(true);
    m_themeItemList->horizontalHeader()->setMinimumSectionSize(120);
    m_themeItemList->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);;
    m_themeItemList->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);;
    m_themeItemList->setCurrentCell(0, 1);
}

void DesktopThemeDetails::updateReplaceItemList(const QString& item)
{
    QString currentReplacement = m_themeReplacements[item];
    QString replacementDropListItem;
    QStringList dropList;
    if ((currentReplacement.isEmpty() && m_theme->currentText() != i18n("(Customized)"))){
        replacementDropListItem = m_theme->currentText() + " " + item;
    }

    // Repopulate combobox droplist
    int itemRow = m_themeItemList->row(m_themeItemList->findItems(item, Qt::MatchExactly).at(0));
    QComboBox *currentComboBox = static_cast<QComboBox*>(m_themeItemList->cellWidget(itemRow,1));
    disconnect(currentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(replacementItemChanged()));
    currentComboBox->clear();
    KStandardDirs dirs;
    QStringList themes = dirs.findAllResources("data", "desktoptheme/*/metadata.desktop",
                                               KStandardDirs::NoDuplicates);
    themes.sort();
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

        QString themeItemFile = themeRoot + '/' + m_themeItems[item];
        //Get correct extension for svg files
        if (QFile::exists(themeItemFile + ".svg")) {
            themeItemFile = themeRoot + '/' + m_themeItems[item] + ".svg";
        }
        if (QFile::exists(themeItemFile + ".svgz")) {
            themeItemFile = themeRoot + '/' + m_themeItems[item] + ".svgz";
        }
        if ((name != i18n("(Customized)")) || (name == i18n("(Customized)") && themeItemFile == currentReplacement)) {
            QString dropListItem = i18n("%1 %2",name,item);
            if (themeItemFile == currentReplacement) {
                replacementDropListItem = dropListItem;
            }
            dropList << dropListItem;
            m_dropListFiles[dropListItem] = themeItemFile;
        }
    }
    if (currentReplacement.isEmpty()) m_themeReplacements[item] = m_dropListFiles[replacementDropListItem];
    currentComboBox->addItems(dropList << i18n("File..."));
    currentComboBox->setCurrentIndex(currentComboBox->findText(replacementDropListItem));
    connect(currentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(replacementItemChanged()));
}

void DesktopThemeDetails::replacementItemChanged()
{
    //Check all items to see if theme has been customized
    m_themeCustomized=false;
    int i;
    for (i = 0; i < m_themeItemList->rowCount(); i++) {
        QComboBox *currentComboBox = static_cast<QComboBox*>(m_themeItemList->cellWidget(i, 1));
        QString currentReplacement = currentComboBox->currentText();

        QString currentItem = m_themeItemList->item(i, 0)->text();
        QString originalValue = m_dropListFiles.key(m_themeReplacements[currentItem]);
        QString changedValue;

        if (currentReplacement == i18n("File...")) {
            //Get the filename for the replacement item
            changedValue = KFileDialog::getOpenFileName(KUrl(), QString(), this, i18n("Select File to Use for %1",currentItem));
            if (!changedValue.isEmpty()) {
                //TODO need a i18n ?
                currentReplacement = "File:" + changedValue;
                m_dropListFiles[currentReplacement]=changedValue;
                int index = currentComboBox->findText("File:",Qt::MatchStartsWith);
                if (index != -1) currentComboBox->removeItem(index);
                currentComboBox->addItem(currentReplacement);
            } else {
                currentReplacement = originalValue;
            }
            disconnect(currentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(replacementItemChanged()));
            int index = currentComboBox->findText(currentReplacement);
            if (index != -1) currentComboBox->setCurrentIndex(index);
            connect(currentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(replacementItemChanged()));
        } else {
            //Get the filename for the drop list replacement item
            changedValue = m_dropListFiles[currentReplacement];
        }

        kDebug() << changedValue;
        if (changedValue != originalValue) {
            m_themeCustomized = true;
            m_themeReplacements[currentItem] = changedValue;
        }
    }
    if (m_themeCustomized) emit changed();
}

void DesktopThemeDetails::resetThemeDetails()
{
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
    KStandardDirs dirs;
    QString theme = m_theme->itemData(m_theme->currentIndex(),
                                      ThemeModel::PackageNameRole).toString();
    m_baseTheme = theme;
    loadThemeItems();
    // Load customized theme settings
    if (isCustomized(theme)) {
        QFile customSettingsFile(dirs.locateLocal("data", "desktoptheme/" + theme +"/settings"));
        if (customSettingsFile.open(QFile::ReadOnly)) {
            QTextStream in(&customSettingsFile);
            QString line;
            QStringList settingsPair;
            QMap<QString, QString>lst;
            //cache it
            for (int i = 0; themeCollectionName[i].m_type; ++i) {
                lst.insert(themeCollectionName[i].m_type, i18nc("plasma name", themeCollectionName[i].m_displayItemName));
            }

            while (!in.atEnd()) {
                line = in.readLine();
                settingsPair = line.split('=');
                if (settingsPair.at(0) == "baseTheme") {
                    m_baseTheme = settingsPair.at(1);
                } else {
                    m_themeReplacements[lst[settingsPair.at(0)]] = settingsPair.at(1);
                    updateReplaceItemList(lst[settingsPair.at(0)]);
                }
            }
            customSettingsFile.close();
        }
    }

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
}

bool DesktopThemeDetails::isCustomized(const QString& theme) {
    if (theme == ".customized" || theme == ".customized1") {
        return true;
    } else {
        return false;
    }
}

void DesktopThemeDetails::clearCustomized() {
    KStandardDirs dirs;
    if (QDir(dirs.locateLocal("data", "desktoptheme/.customized", false)).exists()) {
        KIO::DeleteJob *clearCustom = KIO::del(KUrl(dirs.locateLocal("data", "desktoptheme/.customized", false)), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(clearCustom, this);
    }
    if (QDir(dirs.locateLocal("data", "desktoptheme/.customized1", false)).exists()) {
        KIO::DeleteJob *clearCustom1 = KIO::del(KUrl(dirs.locateLocal("data", "desktoptheme/.customized1", false)), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(clearCustom1, this);
    }
}


