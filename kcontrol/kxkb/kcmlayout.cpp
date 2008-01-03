/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QLabel>
#include <QComboBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QHeaderView>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QWidget>
#include <QtGui/QtGui>
#include <QKeySequence>

#include <kicon.h>
#include <kdialog.h>
#include <kglobalaccel.h>
#include <kglobalsettings.h>
#include <kactioncollection.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <ktoolinvocation.h>
#include <kkeysequencewidget.h>

#include "extension.h"
#include "kxkbconfig.h"
#include "rules.h"
#include "pixmap.h"
#include "ui_kcmlayoutwidget.h"

#include "kcmlayout.h"
#include <KPluginFactory>
#include <KPluginLoader>
#include "kcmlayout.moc"

#ifdef HAVE_XKLAVIER
#include "xklavier_adaptor.h"
#endif


K_PLUGIN_FACTORY(KeyboardLayoutFactory,
        registerPlugin<LayoutConfig>("keyboard_layout");
        )
K_EXPORT_PLUGIN(KeyboardLayoutFactory("kxkb"))


static
bool localeAwareLessThan(const QString &s1, const QString &s2)
{
    return QString::localeAwareCompare(s1, s2) < 0;
}

// sort by locale-aware value string
static QList<QString> getKeysSortedByVaue(const QHash<QString, QString>& map)
{
    QList<QString> outList;

    QMap<QString, QString> reverseMap;
    // we have to add nums as translations can be dups and them reverse map will miss items
    int i=0;
    QString fmt("%1%2");
    foreach (QString str, map.keys())
    	reverseMap.insert(fmt.arg(map[str], i++), str);

    QList<QString> values = reverseMap.keys();
    qSort(values.begin(), values.end(), localeAwareLessThan);

    foreach (QString value, values)
        outList << reverseMap[value];
/*        
    int diff = map.keys().count() - reverseMap.keys().count();
    if( diff > 0 ) {
        kDebug() << "original keys" << map.keys().count() << "reverse map" << reverseMap.keys().count() 
            << "- translation encoding must have been messed up - padding layouts...";
        for(int i=0; i<diff; i++)
            reverseMap.insert(QString("%1%2").arg("nocrash", i), "nocrash");
    }
*/
    return outList;
}

enum {
 LAYOUT_COLUMN_FLAG = 0,
 LAYOUT_COLUMN_NAME = 1,
 LAYOUT_COLUMN_MAP = 2,
 LAYOUT_COLUMN_VARIANT = 3,
 LAYOUT_COLUMN_DISPLAY_NAME = 4,
 SRC_LAYOUT_COLUMN_COUNT = 3,
 DST_LAYOUT_COLUMN_COUNT = 5
};

enum { TAB_LAYOUTS=0, TAB_OPTIONS=1, TAB_XKB=2 };
enum { BTN_XKB_ENABLE=0, BTN_XKB_INDICATOR=1, BTN_XKB_DISABLE=2 };


class SrcLayoutModel: public QAbstractTableModel {
public:
    SrcLayoutModel(XkbRules* rules, QObject *parent)
	: QAbstractTableModel(parent)
	{ setRules(rules); }
//    bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const { return false; }
    int columnCount(const QModelIndex& parent) const { return !parent.isValid() ? SRC_LAYOUT_COLUMN_COUNT : 0; }
    int rowCount(const QModelIndex&) const { return m_rules->layouts().keys().count(); }
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
				       
    void setRules(XkbRules* rules) { 
        m_rules = rules; 
	m_layoutKeys = getKeysSortedByVaue( m_rules->layouts() );
    }
    QString getLayoutAt(int row) { return m_layoutKeys[row]; }

private:
    XkbRules* m_rules;
    QStringList m_layoutKeys;
};

QVariant SrcLayoutModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (role != Qt::DisplayRole)
              return QVariant();
	      
    QString colNames[] = {"", i18n("Layout Name"), i18n("Map")};
    if (orientation == Qt::Horizontal) {
	return colNames[section];
    }
              return QVariant();
}

QVariant
SrcLayoutModel::data(const QModelIndex& index, int role) const
{ 
    if (!index.isValid())
	return QVariant();

    int col = index.column();
    int row = index.row();
    QHash<QString, QString> layouts = m_rules->layouts();
    QString layout = m_layoutKeys[row];
	
    if (role == Qt::TextAlignmentRole) {
	return int(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::DecorationRole) {
	switch(col) {
	    case LAYOUT_COLUMN_FLAG: return LayoutIcon::getInstance().findPixmap(layout, true);
	}
    } else if (role == Qt::DisplayRole) {
	switch(col) {
	    case LAYOUT_COLUMN_NAME: return layouts[layout];
	    case LAYOUT_COLUMN_MAP: return layout;
	    break;
	    default: ;
	}
    }
    return QVariant();
}

class DstLayoutModel: public QAbstractTableModel {
public:
    DstLayoutModel(XkbRules* rules, KxkbConfig* kxkbConfig, QObject *parent)
	: QAbstractTableModel(parent),
	m_kxkbConfig(kxkbConfig)
	{ setRules(rules); }
    int columnCount(const QModelIndex& parent) const { return !parent.isValid() ? DST_LAYOUT_COLUMN_COUNT : 0; }
    int rowCount(const QModelIndex&) const { return m_kxkbConfig->m_layouts.count(); }
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setRules(XkbRules* rules) { m_rules = rules; }
    void reset() { QAbstractTableModel::reset(); }
    void emitDataChange(int row, int col) { emit dataChanged(createIndex(row,col),createIndex(row,col)); }

private:
    XkbRules* m_rules;
    KxkbConfig* m_kxkbConfig;
};

QVariant DstLayoutModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
	      
    QString colNames[] = {"", i18n("Layout Name"), i18n("Map"), i18n("Variant"), i18n("Label")};
    if (orientation == Qt::Horizontal) {
	return colNames[section];
    }
    return QVariant();
}

QVariant
DstLayoutModel::data(const QModelIndex& index, int role) const
{ 
    if (!index.isValid())
	return QVariant();

    int col = index.column();
    int row = index.row();
    QHash<QString, QString> layouts = m_rules->layouts();
    LayoutUnit lu = m_kxkbConfig->m_layouts[row];
	
    if (role == Qt::TextAlignmentRole) {
	return int(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::DecorationRole) {
	switch(col) {
	    case LAYOUT_COLUMN_FLAG: 
                return LayoutIcon::getInstance().findPixmap(lu.layout, m_kxkbConfig->m_showFlag, lu.getDisplayName());
	}
    } else if (role == Qt::DisplayRole) {
	switch(col) {
	    case LAYOUT_COLUMN_NAME: return layouts[lu.layout];
	    case LAYOUT_COLUMN_MAP: return lu.layout;
	    case LAYOUT_COLUMN_VARIANT: return lu.variant;
	    case LAYOUT_COLUMN_DISPLAY_NAME: return lu.getDisplayName();
	    break;
	    default: ;
	}
    }
    return QVariant();
}


class XkbOptionsModel: public QAbstractItemModel {
public:
    XkbOptionsModel(XkbRules* rules, KxkbConfig* kxkbConfig, QObject *parent)
	: QAbstractItemModel(parent),
	m_kxkbConfig(kxkbConfig)
	{ setRules(rules); }

    int columnCount(const QModelIndex& /*parent*/) const { return 1; }
    int rowCount(const QModelIndex& parent) const { 
        if( ! parent.isValid() )
            return m_rules->optionGroups().count();
        if( ! parent.parent().isValid() )
            return m_rules->optionGroups().values()[parent.row()].options.count();
        return 0; 
    }
    QModelIndex parent(const QModelIndex& index) const {
        if (!index.isValid() )
            return QModelIndex();
        if( index.internalId() < 100 )
            return QModelIndex();
        return createIndex(((index.internalId() - index.row())/100) - 1, index.column());
    }
    QModelIndex index(int row, int column, const QModelIndex& parent) const {
        if(!parent.isValid()) return createIndex(row, column);
        return createIndex(row, column, (100 * (parent.row()+1)) + row);
    }
    Qt::ItemFlags flags ( const QModelIndex & index ) const {
        if( ! index.isValid() )
            return 0;
        
        if( !index.parent().isValid() )
            return Qt::ItemIsEnabled;

        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) {
        int groupRow = index.parent().row();
        if( groupRow < 0 ) return false;
        
        QString xkbGroupNm = m_rules->optionGroups().keys()[groupRow];
        const XkbOptionGroup& xkbGroup = m_rules->optionGroups()[xkbGroupNm];
	const XkbOption& option = xkbGroup.options[index.row()];

        if( value.toInt() == Qt::Checked ) {
            if( xkbGroup.exclusive ) {
                // clear if exclusive (TODO: radiobutton)
                int idx = m_kxkbConfig->m_options.indexOf(QRegExp(xkbGroupNm+".*"));
                if( idx >= 0 ) {
                    for(int i=0; i<xkbGroup.options.count(); i++)
                        if( xkbGroup.options[i].name == m_kxkbConfig->m_options[idx] ) {
                            setData(createIndex(i, index.column(), (quint32)index.internalId()-index.row()+i), Qt::Unchecked, role);
                            break;
                        }
                //    m_kxkbConfig->m_options.removeAt(idx);
                //    idx = m_kxkbConfig->m_options.indexOf(QRegExp(xkbGroupNm+".*"));
                }
            }
            if( m_kxkbConfig->m_options.indexOf(option.name) < 0 ) {
                m_kxkbConfig->m_options.append(option.name);
            }
        }
        else {
            m_kxkbConfig->m_options.removeAll(option.name);
        }
        emit dataChanged(index, index);
        return true;
    }
    
    QVariant data(const QModelIndex& index, int role) const;

    void setRules(XkbRules* rules) { m_rules = rules; }
    void reset() { QAbstractItemModel::reset(); }

    void gotoGroup(const QString& group, QTreeView* view) {
        int index = m_rules->optionGroups().keys().indexOf(group);
//        kDebug() << "scrolling to group" << index << "-" << group;
        if( index != -1 ) {
            QModelIndex modelIdx = createIndex(index,0);
//            view->selectionModel()->setCurrentIndex(createIndex(index,0), QItemSelectionModel::NoUpdate);
            view->setExpanded(modelIdx, true);
            view->scrollTo(modelIdx, QAbstractItemView::PositionAtTop);
            view->selectionModel()->setCurrentIndex(modelIdx, QItemSelectionModel::Current);
            view->setFocus(Qt::OtherFocusReason);
//            kDebug() << "wdg:" << view->itemDelegate(createIndex(index, 0).child(0,0));
        }
        else
            kDebug() << "can't scroll to group" << group;
    }
private:
    XkbRules* m_rules;
    KxkbConfig* m_kxkbConfig;
};

QVariant
XkbOptionsModel::data(const QModelIndex& index, int role) const
{ 
    if (!index.isValid())
	return QVariant();

    int row = index.row();
	
    if (role == Qt::DisplayRole) {
        if( ! index.parent().isValid() )
	    return m_rules->optionGroups().values()[row].description;
        else {
            int groupRow = index.parent().row();
            QString xkbGroupNm = m_rules->optionGroups().keys()[groupRow];
            const XkbOptionGroup& xkbGroup = m_rules->optionGroups()[xkbGroupNm];
	    return xkbGroup.options[row].description;
        }
    }
    else if( role==Qt::CheckStateRole && index.parent().isValid() ) {
        int groupRow = index.parent().row();
        QString xkbGroupNm = m_rules->optionGroups().keys()[groupRow];
        const XkbOptionGroup& xkbGroup = m_rules->optionGroups()[xkbGroupNm];

        return m_kxkbConfig->m_options.indexOf(xkbGroup.options[row].name) == -1 ? Qt::Unchecked : Qt::Checked;
    }
    return QVariant();
}


//K_PLUGIN_FACTORY_DECLARATION(KeyboardLayoutFactory)

LayoutConfig::LayoutConfig(QWidget *parent, const QVariantList &)
  : KCModule(KeyboardLayoutFactory::componentData(), parent),
    DEFAULT_VARIANT_NAME(i18nc("Default variant", "Default")),
    m_rules(NULL),
    m_srcModel(NULL),
    m_dstModel(NULL),
    m_xkbOptModel(NULL)
{
//kDebug() << "Qt  locale" << QLocale::system().name();
//kDebug() << "KDE locale" << KGlobal::locale()->language() << KGlobal::locale()->country();
//kDebug() << "OS  locale" << setlocale(LC_ALL, NULL);
    //Read rules - we _must_ read _before_ creating UIs
    loadRules();

    widget = new Ui_LayoutConfigWidget();
    widget->setupUi(this);
    layout()->setMargin(0);
//  main->addWidget(widget);

    m_srcModel = new SrcLayoutModel(m_rules, NULL);

    widget->srcTableView->setModel(m_srcModel);
//    widget->srcTableView->setSortingEnabled(true);
    widget->srcTableView->setColumnWidth(LAYOUT_COLUMN_FLAG, 26);
    widget->srcTableView->setColumnWidth(LAYOUT_COLUMN_MAP, 30);
//(QTableView)    widget->srcTableView->verticalHeader()->hide();
//(QTableView)    widget->srcTableView->setShowGrid(false);
//(QTableView)    widget->srcTableView->resizeRowsToContents();
    widget->srcTableView->setRootIsDecorated(false);

    m_dstModel = new DstLayoutModel(m_rules, &m_kxkbConfig, NULL);
    widget->dstTableView->setModel(m_dstModel);
    widget->dstTableView->setColumnWidth(LAYOUT_COLUMN_FLAG, 26);
    widget->dstTableView->setColumnWidth(LAYOUT_COLUMN_MAP, 30);
//(QTableView)    widget->dstTableView->verticalHeader()->hide();
    widget->dstTableView->setRootIsDecorated(false);

    m_xkbOptModel = new XkbOptionsModel(m_rules, &m_kxkbConfig, NULL);
    widget->xkbOptionsTreeView->setModel(m_xkbOptModel);
    widget->xkbOptionsTreeView->header()->hide();
    widget->xkbOptionsTreeView->expandAll();

    connect( m_xkbOptModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), 
        this, SLOT(xkbOptionsChanged(const QModelIndex &, const QModelIndex &)));

    connect( widget->grpEnableKxkb, SIGNAL( clicked( int ) ), SLOT(enableChanged()));
    connect( widget->grpEnableKxkb, SIGNAL( clicked( int ) ), SLOT(updateGroupsFromServer()));
//    connect( widget->chkEnable, SIGNAL( toggled( bool )), this, SLOT(changed()));
//    connect( widget->chkIndicatorOnly, SIGNAL( toggled( bool )), this, SLOT(changed()));
    connect( widget->chkShowSingle, SIGNAL( toggled( bool )), this, SLOT(changed()));
//    connect( widget->chkShowFlag, SIGNAL( toggled( bool )), this, SLOT(changed()));
    connect( widget->chkShowFlag, SIGNAL(toggled(bool)), this, SLOT(showFlagChanged(bool)));
    connect( widget->comboModel, SIGNAL(activated(int)), this, SLOT(changed()));

    connect( widget->srcTableView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(add()));
    connect( widget->btnAdd, SIGNAL(clicked()), this, SLOT(add()));
    connect( widget->btnRemove, SIGNAL(clicked()), this, SLOT(remove()));

//  connect( widget->comboVariant, SIGNAL(activated(int)), this, SLOT(changed()));
    connect( widget->comboVariant, SIGNAL(activated(int)), this, SLOT(variantChanged()));
    connect( widget->dstTableView->selectionModel(), 
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this, SLOT(layoutSelChanged()) );

    connect( widget->btnXkbShortcut, SIGNAL(clicked()), this, SLOT(xkbShortcutPressed()));
    connect( widget->btnXkbShortcut3d, SIGNAL(clicked()), this, SLOT(xkbShortcut3dPressed()));

    connect( widget->editDisplayName, SIGNAL(textChanged(const QString&)), this, SLOT(displayNameChanged(const QString&)));

    widget->btnUp->setIcon(KIcon("arrow-up"));
    widget->btnAdd->setIcon(KIcon("arrow-right"));
    widget->btnRemove->setIcon(KIcon("arrow-left"));
    connect( widget->btnUp, SIGNAL(clicked()), this, SLOT(moveUp()));
    widget->btnDown->setIcon(KIcon("arrow-down"));
    connect( widget->btnDown, SIGNAL(clicked()), this, SLOT(moveDown()));

    connect( widget->grpSwitching, SIGNAL( clicked( int ) ), SLOT(changed()));
#ifdef STICKY_SWITCHING
    connect( widget->chkEnableSticky, SIGNAL(toggled(bool)), this, SLOT(changed()));
#endif

    KIcon clearIcon = qApp->isLeftToRight() ? KIcon("edit-clear-locationbar-rtl") : KIcon("edit-clear-locationbar-ltr");
    widget->xkbClearButton->setIcon(clearIcon);
    widget->xkb3dClearButton->setIcon(clearIcon);
    connect(widget->xkbClearButton, SIGNAL(clicked()), this, SLOT(clearXkbSequence()));
    connect(widget->xkb3dClearButton, SIGNAL(clicked()), this, SLOT(clearXkb3dSequence()));

#ifdef STICKY_SWITCHING
    connect( widget->spinStickyDepth, SIGNAL(valueChanged(int)), this, SLOT(changed()));
#endif

    KGlobalAccel::self()->overrideMainComponentData(componentData());
    actionCollection = new KActionCollection( this, KComponentData("kxkb") );
//    actionCollection->setConfigGlobal(true);
    KAction* a = NULL;
#include "kxkbbindings.cpp"
    kDebug() << "getting shortcut" << a->globalShortcut().toString();

    widget->kdeKeySequence->setModifierlessAllowed(false);
    widget->kdeKeySequence->setKeySequence( a->globalShortcut().primary() );

    connect(widget->kdeKeySequence, SIGNAL(keySequenceChanged (const QKeySequence &)), this, SLOT(changed()));

    refreshRulesUI();

    makeOptionsTab();

    load();
}


LayoutConfig::~LayoutConfig()
{
    delete m_rules;
}


void LayoutConfig::load()
{
    m_kxkbConfig.load(KxkbConfig::LOAD_ALL);
    initUI();
}

void LayoutConfig::initUI()
{
	QString modelName = m_rules->models()[m_kxkbConfig.m_model];
	widget->comboModel->setCurrentIndex( widget->comboModel->findText(modelName) );

	m_dstModel->reset();
	widget->dstTableView->update();

	// display KXKB switching options
	widget->chkShowSingle->setChecked(m_kxkbConfig.m_showSingle);
	widget->chkShowFlag->setChecked(m_kxkbConfig.m_showFlag);

//	widget->chkEnableOptions->setChecked( m_kxkbConfig.m_enableXkbOptions );
	widget->checkResetOld->setChecked(m_kxkbConfig.m_resetOldOptions);

	widget->grpSwitching->setSelected( m_kxkbConfig.m_switchingPolicy );

#ifdef STICKY_SWITCHING
    widget->chkEnableSticky->setChecked(m_kxkbConfig.m_stickySwitching);
    widget->spinStickyDepth->setEnabled(m_kxkbConfig.m_stickySwitching);
    widget->spinStickyDepth->setValue( m_kxkbConfig.m_stickySwitchingDepth);
    updateStickyLimit();
#endif

    int enableKxkb = BTN_XKB_DISABLE;
    if( m_kxkbConfig.m_indicatorOnly ) enableKxkb = BTN_XKB_INDICATOR;
    else
        if( m_kxkbConfig.m_useKxkb ) enableKxkb = BTN_XKB_ENABLE;
    widget->grpEnableKxkb->setSelected(enableKxkb);
    enableChanged();

    updateShortcutsLabels();

    updateLayoutCommand();
    updateOptionsCommand();
        
    widget->tabWidget->setCurrentIndex(TAB_LAYOUTS);
    
    emit KCModule::changed( false );
}


void LayoutConfig::save()
{
    KCModule::save();

	QString model = widget->comboModel->itemData(widget->comboModel->currentIndex()).toString();
	m_kxkbConfig.m_model = model;

//	m_kxkbConfig.m_enableXkbOptions = widget->chkEnableOptions->isChecked();
	m_kxkbConfig.m_resetOldOptions = widget->checkResetOld->isChecked();

	if( m_kxkbConfig.m_layouts.count() == 0 ) {
	    m_kxkbConfig.m_layouts.append(LayoutUnit(DEFAULT_LAYOUT_UNIT));
 	    widget->grpEnableKxkb->setSelected(BTN_XKB_DISABLE);
 	}

	m_kxkbConfig.m_useKxkb = widget->grpEnableKxkb->selected() <= BTN_XKB_INDICATOR;
	m_kxkbConfig.m_indicatorOnly = widget->grpEnableKxkb->selected() == BTN_XKB_INDICATOR;
	m_kxkbConfig.m_showSingle = widget->chkShowSingle->isChecked();
	m_kxkbConfig.m_showFlag = widget->chkShowFlag->isChecked();

	int modeId = widget->grpSwitching->selected();
	m_kxkbConfig.m_switchingPolicy = (SwitchingPolicy)modeId;

#ifdef STICKY_SWITCHING
    m_kxkbConfig.m_stickySwitching = widget->chkEnableSticky->isChecked();
    m_kxkbConfig.m_stickySwitchingDepth = widget->spinStickyDepth->value();
#endif
    m_kxkbConfig.save();

    KAction* action = static_cast<KAction*>(actionCollection->action(0));
    KShortcut shortcut(widget->kdeKeySequence->keySequence());
    action->setGlobalShortcut(shortcut, KAction::ActiveShortcut, KAction::NoAutoloading);
    kDebug() << "saving kxkb shortcut" << shortcut.toString();
//    actionCollection->writeSettings();

    KGlobalSettings::emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_SHORTCUTS);

    KToolInvocation::kdeinitExec("kxkb");
    emit KCModule::changed( false );
}

void LayoutConfig::xkbOptionsChanged(const QModelIndex & /*topLeft*/, const QModelIndex & /*bottomRight*/)
{
    updateOptionsCommand();
    updateShortcutsLabels();
    changed();
//    widget->xkbOptionsTreeView->update(topLeft);
}


static QStringList getGroupOptionList(const QStringList& options, const QString& grp)
{
    QRegExp grpRegExp("^" + grp + ".*");
    return options.filter(grpRegExp);
}

void LayoutConfig::clearXkbSequence()
{
    QStringList grpOptions = getGroupOptionList(m_kxkbConfig.m_options, "grp");
    foreach(QString opt, grpOptions)
        m_kxkbConfig.m_options.removeAll(opt);
    m_xkbOptModel->reset();
    widget->xkbOptionsTreeView->update();
    updateShortcutsLabels();
    changed();
}

void LayoutConfig::clearXkb3dSequence()
{
    QStringList grpOptions = getGroupOptionList(m_kxkbConfig.m_options, "lv3");
    foreach(QString opt, grpOptions)
        m_kxkbConfig.m_options.removeAll(opt);
    m_xkbOptModel->reset();
    widget->xkbOptionsTreeView->update();
    updateShortcutsLabels();
    changed();
}

void LayoutConfig::xkbShortcutPressed()
{
    widget->tabWidget->setCurrentIndex(TAB_XKB);
    m_xkbOptModel->gotoGroup("grp", widget->xkbOptionsTreeView);
}

void LayoutConfig::xkbShortcut3dPressed()
{
    widget->tabWidget->setCurrentIndex(TAB_XKB);
    m_xkbOptModel->gotoGroup("lv3", widget->xkbOptionsTreeView);
}

static QString getShortcutText(const QStringList& options, const QString& grp)
{
    QStringList grpOptions = getGroupOptionList(options, grp);
    
    if( grpOptions.count() > 1 )
        return i18n("Multiple keys");
    else
    if( grpOptions.count() == 1 )
        return i18n("Defined");         //TODO: show shortcut
    else
        return i18n("None");
}

void LayoutConfig::updateShortcutsLabels()
{
    QString txt = getShortcutText( m_kxkbConfig.m_options, "grp" );
    widget->btnXkbShortcut->setText(txt);
    widget->btnXkbShortcut->setToolTip("");
    txt = getShortcutText( m_kxkbConfig.m_options, "lv3" );
    widget->btnXkbShortcut3d->setText(txt);
    widget->btnXkbShortcut3d->setToolTip("");
}

void LayoutConfig::showFlagChanged(bool on)
{
    m_kxkbConfig.m_showFlag = on;
    m_dstModel->reset();
    widget->dstTableView->update();
    
    changed();
}

void LayoutConfig::updateStickyLimit()
{
#ifdef STICKY_SWITCHING
    int layoutsCnt = m_kxkbConfig.m_layouts.count();
    int maxDepth = layoutsCnt - 1;

    if( maxDepth < 2 ) {
        maxDepth = 2;
    }

    widget->spinStickyDepth->setMaximum(maxDepth);
/*	if( value > maxDepth )
		setValue(maxDepth);*/
#endif
}


void LayoutConfig::updateGroupsFromServer()
{
    bool enabled = widget->grpEnableKxkb->selected() == BTN_XKB_ENABLE;
    //kDebug() << "enabled:" << enabled << m_kxkbConfig.m_layouts.count();
    if( enabled ) {
#ifdef HAVE_XKLAVIER
        XkbConfig xkbConfig = XKlavierAdaptor::getInstance(QX11Info::display())->getGroupNames();
#else
        XkbConfig xkbConfig = X11Helper::getGroupNames(QX11Info::display());
#endif
        xkbConfig.model = m_kxkbConfig.m_model;
        //TODO: update model
        if( m_kxkbConfig.m_layouts.count() > 1 || xkbConfig.layouts.count() == 0 ) {
            xkbConfig.layouts = m_kxkbConfig.m_layouts;
        }
        kDebug() << m_kxkbConfig.m_options.join(",") << xkbConfig.options.join(",");
        if( !m_kxkbConfig.m_resetOldOptions || m_kxkbConfig.m_options.count() > 0 || xkbConfig.options.count() == 0 ) {
            xkbConfig.options = m_kxkbConfig.m_options;
        }

        m_kxkbConfig.setConfiguredLayouts(xkbConfig);

        m_dstModel->reset();
        widget->dstTableView->update();
        updateLayoutCommand();

        m_xkbOptModel->reset();
        widget->xkbOptionsTreeView->update();
        updateOptionsCommand();
    }
}

void LayoutConfig::enableChanged()
{
    bool enabled = widget->grpEnableKxkb->selected() == BTN_XKB_ENABLE;
    bool indicatorEnabled = widget->grpEnableKxkb->selected() <= BTN_XKB_INDICATOR;

    widget->grpLayouts->setEnabled(enabled);
    widget->tabWidget->widget(TAB_OPTIONS)->setEnabled(enabled);
    widget->tabWidget->widget(TAB_XKB)->setEnabled(enabled);
    widget->grpIndicatorOptions->setEnabled(indicatorEnabled);

    changed();
}

void LayoutConfig::add()
{
    QItemSelectionModel* selectionModel = widget->srcTableView->selectionModel();
    if( selectionModel == NULL || !selectionModel->hasSelection() 
		|| m_kxkbConfig.m_layouts.count() >= GROUP_LIMIT )
	return;

    QModelIndexList selected = selectionModel->selectedRows();
    QHash<QString, QString> layouts = m_rules->layouts();
    QString layout = m_srcModel->getLayoutAt(selected[0].row());
//    kDebug() << "selected to add" << layout;
    LayoutUnit lu(layout, "");
    m_kxkbConfig.m_layouts << lu;

    m_dstModel->reset();
    widget->dstTableView->update();

    updateAddButton();
    updateLayoutCommand();

    updateStickyLimit();
    changed();
}

void LayoutConfig::updateAddButton()
{
    bool aboveLimit = m_kxkbConfig.m_layouts.count() >= GROUP_LIMIT;
    widget->btnAdd->setEnabled( !aboveLimit );
}

void LayoutConfig::remove()
{
    QItemSelectionModel* selectionModel = widget->dstTableView->selectionModel();
    if( selectionModel == NULL || !selectionModel->hasSelection() )
	return;

    int row = getSelectedDstLayout();
    if( row == -1 )
        return;

    m_kxkbConfig.m_layouts.removeAt(row);

    m_dstModel->reset();
    widget->dstTableView->update();

    layoutSelChanged();
    updateAddButton();
    updateLayoutCommand();
    updateStickyLimit();

    changed();
}

void LayoutConfig::moveSelected(int shift)
{
    QItemSelectionModel* selectionModel = widget->dstTableView->selectionModel();
    if( selectionModel == NULL || !selectionModel->hasSelection() )
	return;

    QModelIndexList selected = selectionModel->selectedRows();
    if( selected.count() < 1 )
        return;
    
    int row = selected[0].row();
    int new_row = row + shift;
    
    if( new_row >= 0 && new_row <= m_kxkbConfig.m_layouts.count()-1 ) {
	m_kxkbConfig.m_layouts.move(row, new_row);
        m_dstModel->reset();
        widget->dstTableView->update();
    }
}

void LayoutConfig::moveUp()
{
    moveSelected(-1);
    updateLayoutCommand();
    changed();
}

void LayoutConfig::moveDown()
{
    moveSelected(1);
    updateLayoutCommand();
    changed();
}

void LayoutConfig::variantChanged()
{
    int row = getSelectedDstLayout();

    if( row == -1 ) {
	widget->comboVariant->clear();
	widget->comboVariant->setEnabled(false);
	return;
    }

    QString selectedVariant = widget->comboVariant->itemData( widget->comboVariant->currentIndex() ).toString();
//    if( selectedVariant == DEFAULT_VARIANT_NAME )
//        selectedVariant = "";
    
    m_kxkbConfig.m_layouts[row].variant = selectedVariant;
    m_dstModel->emitDataChange(row, LAYOUT_COLUMN_VARIANT);

    updateLayoutCommand();
    changed();
}

void LayoutConfig::displayNameChanged(const QString& newDisplayName)
{
    int row = getSelectedDstLayout();
    if( row == -1 )
        return;

    LayoutUnit& layoutUnit = m_kxkbConfig.m_layouts[row];

    QString oldName = layoutUnit.getDisplayName();

    if( oldName != newDisplayName ) {
	layoutUnit.setDisplayName(newDisplayName);

        m_dstModel->emitDataChange(row, LAYOUT_COLUMN_DISPLAY_NAME);
        m_dstModel->emitDataChange(row, LAYOUT_COLUMN_FLAG);

	changed();
    }
}

int LayoutConfig::getSelectedDstLayout()
{
    QItemSelectionModel* selectionModel = widget->dstTableView->selectionModel();
    if( selectionModel == NULL || !selectionModel->hasSelection() )
	return -1;

    QModelIndexList selected = selectionModel->selectedRows();
    int row = selected.count() > 0 ? selected[0].row() : -1;
    return row;
}

void LayoutConfig::layoutSelChanged()
{
    int row = getSelectedDstLayout();

    widget->comboVariant->clear();
    widget->comboVariant->setEnabled( row != -1 );
    if( row == -1 ) {
        return;
    }

    QString kbdLayout = m_kxkbConfig.m_layouts[row].layout;

    QList<XkbVariant> vars = m_rules->getAvailableVariants(kbdLayout);
    kDebug() << "layout " << kbdLayout << " has " << vars.count() << " variants";

    if( vars.count() > 0 ) {
//		vars.prepend(DEFAULT_VARIANT_NAME);
//		widget->comboVariant->addItems(vars);
        widget->comboVariant->addItem(DEFAULT_VARIANT_NAME, "");
        for(int ii=0; ii<vars.count(); ii++) {
	    widget->comboVariant->addItem(vars[ii].description, vars[ii].name);
            widget->comboVariant->setItemData(widget->comboVariant->count()-1, vars[ii].description, Qt::ToolTipRole );
        }
	QString variant = m_kxkbConfig.m_layouts[row].variant;
	if( variant != NULL && variant.isEmpty() == false ) {
            int idx = widget->comboVariant->findData(variant);
	    widget->comboVariant->setCurrentIndex(idx);
	}
	else {
	    widget->comboVariant->setCurrentIndex(0);
	}
    }
    updateDisplayName();
}

void LayoutConfig::makeOptionsTab()
{
//    connect(widget->chkEnableOptions, SIGNAL(toggled(bool)), SLOT(changed()));
    connect(widget->checkResetOld, SIGNAL(toggled(bool)), SLOT(changed()));
    connect(widget->checkResetOld, SIGNAL(toggled(bool)), SLOT(updateOptionsCommand()));
}

void LayoutConfig::updateOptionsCommand()
{
    widget->editCmdLineOpt->setText(createOptionString());
}

void LayoutConfig::updateLayoutCommand()
{
    QStringList layouts;
    QStringList variants;

    QList<LayoutUnit> layoutUnits = m_kxkbConfig.m_layouts;
    for(int i=0; i<layoutUnits.count(); i++) {
	QString layout = layoutUnits[i].layout;
	QString variant = layoutUnits[i].variant;
	//QString displayName = layoutUnits[i].displayName;

	if( variant == DEFAULT_VARIANT_NAME )
	    variant = "";

	layouts << layout;
	variants << variant;
    }

    QString model = widget->comboModel->itemData(widget->comboModel->currentIndex()).toString();
    QString setxkbmap = XKBExtension::getLayoutGroupsCommand(model, layouts, variants);

    widget->editCmdLine->setText(setxkbmap);
}


void LayoutConfig::updateDisplayName()
{
    int row = getSelectedDstLayout();

    widget->editDisplayName->setEnabled( row != -1 );
    if( row == -1 ) {
        widget->editDisplayName->clear();
        return;
    }

    widget->editDisplayName->setText( m_kxkbConfig.m_layouts[row].getDisplayName() );
}

void LayoutConfig::changed()
{
  emit KCModule::changed( true );
}


void LayoutConfig::loadRules()
{
    // TODO: do we need this ?
    // this could obly be used if rules are changed and 'Defaults' is pressed
    delete m_rules;
    m_rules = new XkbRules();
    if( m_srcModel )
        m_srcModel->setRules(m_rules);
    if( m_dstModel )
	m_dstModel->setRules(m_rules);
    if( m_xkbOptModel )
	m_xkbOptModel->setRules(m_rules);
}

void LayoutConfig::refreshRulesUI()
{
    widget->comboModel->clear();
    QList<QString> sortedModels = getKeysSortedByVaue( m_rules->models() );
    foreach( QString model, sortedModels ) {
	widget->comboModel->addItem( m_rules->models()[model], model);
        widget->comboModel->setItemData( widget->comboModel->count()-1, m_rules->models()[model], Qt::ToolTipRole );
    }
    widget->comboModel->setCurrentIndex(0);
	//TODO: reset options and xkb options
}


QString LayoutConfig::createOptionString()
{
    bool reset = widget->checkResetOld->isChecked();
    return XKBExtension::getXkbOptionsCommand(m_kxkbConfig.m_options, reset);
}


void LayoutConfig::defaults()
{
    loadRules();
    refreshRulesUI();
    m_kxkbConfig.setDefaults();

    initUI();

    emit KCModule::changed( true );
}

extern "C"
{
    KDE_EXPORT void kcminit_keyboard_layout()
    {
	KxkbConfig m_kxkbConfig;
	m_kxkbConfig.load(KxkbConfig::LOAD_ACTIVE_OPTIONS);

	if( m_kxkbConfig.m_useKxkb ) {
            KToolInvocation::kdeinitExec("kxkb");
	}
    }
}
