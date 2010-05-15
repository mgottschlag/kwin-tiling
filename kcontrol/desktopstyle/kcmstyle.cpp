/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2009 by Davide Bettio <davide.bettio@kdemail.net>
 
 * Portions Copyright (C) 2007 Paolo Capriotti <p.capriotti@gmail.com>
 * Portions Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 * Portions Copyright (C) 2008 by Petri Damsten <damu@iki.fi>
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kcmstyle.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kstyle.h>
#include <kstandarddirs.h>
#include <kautostart.h>
#include <KDebug>
#include <KColorScheme>
#include <KStandardDirs>
#include <knewstuff3/downloaddialog.h>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>

#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtGui/QAbstractItemView>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>
#include <QtGui/QStyleFactory>
#include <QtGui/QFormLayout>
#include <QtGui/QStandardItemModel>
#include <QtGui/QStyle>
#include <QtDBus/QtDBus>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

// X11 namespace cleanup
#undef Below
#undef KeyPress
#undef KeyRelease


/**** DLL Interface for kcontrol ****/

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KLibLoader>

K_PLUGIN_FACTORY(KCMStyleFactory, registerPlugin<KCMStyle>();)
K_EXPORT_PLUGIN(KCMStyleFactory("kcmdesktopstyle"))

class ThemeInfo
{
public:
    QString package;
    Plasma::FrameSvg *svg;
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
    QModelIndex indexOf(const QString &path) const;
    void reload();
private:
    QMap<QString, ThemeInfo> m_themes;
};

ThemeModel::ThemeModel( QObject *parent )
    : QAbstractListModel( parent )
{
}

ThemeModel::~ThemeModel()
{
}

void ThemeModel::reload()
{
    reset();
    foreach (const ThemeInfo &info, m_themes) {
        delete info.svg;
    }
    m_themes.clear();

    // get all desktop themes
    KPluginInfo::List themeInfos = Plasma::Theme::listThemeInfo();

    foreach (const KPluginInfo &themeInfo, themeInfos) {
        kDebug() << themeInfo.name() << themeInfo.pluginName();
        QString name = themeInfo.name();
        if (name.isEmpty()) {
            name = themeInfo.pluginName();
        }

        Plasma::Theme *theme = new Plasma::Theme(themeInfo.pluginName(), this);
        Plasma::FrameSvg *svg = new Plasma::FrameSvg(theme);
        svg->setUsingRenderingCache(false);
        svg->setTheme(theme);
        svg->setImagePath("widgets/background");
        svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);

        ThemeInfo info;
        info.package = themeInfo.pluginName();
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

QModelIndex ThemeModel::indexOf(const QString &name) const
{
    QMapIterator<QString, ThemeInfo> it(m_themes);
    int i = -1;
    while (it.hasNext()) {
        ++i;
        if (it.next().value().package == name) {
            return index(i, 0);
        }
    }

    return QModelIndex();
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
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

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


KCMStyle::KCMStyle( QWidget* parent, const QVariantList& )
	: KCModule( KCMStyleFactory::componentData(), parent )
{
	m_bDesktopThemeDirty = false;

	KAutostart plasmaNetbookAutoStart("plasma-netbook");
	m_isNetbook = plasmaNetbookAutoStart.autostarts();

	KGlobal::dirs()->addResourceType("themes", "data", "kstyle/themes");

	KAboutData *about =
		new KAboutData( I18N_NOOP("kcmdesktopstyle"), 0,
						ki18n("KDE Style Module"),
						0, KLocalizedString(), KAboutData::License_GPL,
						ki18n("(c) 2002 Karol Szwed, Daniel Molkentin"));

	about->addAuthor(ki18n("Karol Szwed"), KLocalizedString(), "gallium@kde.org");
	about->addAuthor(ki18n("Daniel Molkentin"), KLocalizedString(), "molkentin@kde.org");
	about->addAuthor(ki18n("Ralf Nolden"), KLocalizedString(), "nolden@kde.org");
	setAboutData( about );

	// Setup pages and mainLayout
	mainLayout = new QVBoxLayout( this );
	mainLayout->setMargin(0);

	tabWidget  = new QTabWidget( this );
	mainLayout->addWidget( tabWidget );

	// Add Page0 (Desktop Theme)
	// -------------------
	page0 = new QWidget;
	themeUi.setupUi(page0);

	themeUi.m_newThemeButton->setIcon(KIcon("get-hot-new-stuff"));
	
	m_themeModel = new ThemeModel(this);
	themeUi.m_theme->setModel(m_themeModel);
	themeUi.m_theme->setItemDelegate(new ThemeDelegate(themeUi.m_theme));
	themeUi.m_theme->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	connect(themeUi.m_theme->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(setDesktopThemeDirty()));
	connect(themeUi.m_newThemeButton, SIGNAL(clicked()), this, SLOT(getNewThemes()));

	m_workspaceThemeTabActivated = true;
        loadDesktopTheme();
  
	tabWidget->addTab(page0, i18nc("@title:tab", "&Workspace"));

	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
}


KCMStyle::~KCMStyle()
{
}

void KCMStyle::load()
{
	KConfig config( "kdeglobals", KConfig::FullConfig );

	if (m_workspaceThemeTabActivated) {
		loadDesktopTheme();
	}

	m_bDesktopThemeDirty = false;

	emit changed( false );
}


void KCMStyle::save()
{
	// Don't do anything if we don't need to.
	if ( !m_bDesktopThemeDirty )
		return;

	//Desktop theme
	if ( m_bDesktopThemeDirty )
	{
		QString theme = m_themeModel->data(themeUi.m_theme->currentIndex(), ThemeModel::PackageNameRole).toString();
		if (m_isNetbook) {
			KConfigGroup cg(KSharedConfig::openConfig("plasmarc"), "Theme-plasma-netbook");
			cg.writeEntry("name", theme);
		} else {
			Plasma::Theme::defaultTheme()->setThemeName(theme);
		}
	}

	// Clean up
	m_bDesktopThemeDirty    = false;
	emit changed( false );
}

void KCMStyle::defaults()
{
}

void KCMStyle::setDesktopThemeDirty()
{
	m_bDesktopThemeDirty = true;
	emit changed(true);
}

void KCMStyle::tabChanged(int index)
{
	if (index == 0 && !m_workspaceThemeTabActivated) { //Workspace theme tab (never loaded before)
		m_workspaceThemeTabActivated = true;
		QTimer::singleShot(100, this, SLOT(loadDesktopTheme()));
	}
}

// ----------------------------------------------------------------
// All the Desktop Theme stuff
// ----------------------------------------------------------------

void KCMStyle::getNewThemes()
{
    KNS3::DownloadDialog dialog("plasma-themes.knsrc", this);
    dialog.exec();
    KNS3::Entry::List entries = dialog.changedEntries();

    if (entries.size() > 0) {
        loadDesktopTheme();
    }
}

void KCMStyle::loadDesktopTheme()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_themeModel->reload();
	QString themeName;
	if (m_isNetbook) {
		KConfigGroup cg(KSharedConfig::openConfig("plasmarc"), "Theme-plasma-netbook");
		themeName = cg.readEntry("name", "air-netbook");
	} else {
		themeName = Plasma::Theme::defaultTheme()->themeName();
	}
	themeUi.m_theme->setCurrentIndex(m_themeModel->indexOf(themeName));
	QApplication::restoreOverrideCursor();
}

#include "kcmstyle.moc"

// vim: set noet ts=4:
