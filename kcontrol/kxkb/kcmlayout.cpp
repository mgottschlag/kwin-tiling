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
#include <QSpinBox>
#include <QWidget>
#include <QtGui>

#include <kicon.h>
#include <kshortcutsdialog.h>
#include <kactioncollection.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <ktoolinvocation.h>

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


static inline QString i18n(const QString& str) { return i18n( str.toLatin1().constData() ); }

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

static const QString DEFAULT_VARIANT_NAME("<default>");

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
				       
    void setRules(XkbRules* rules) { m_rules = rules; 
	QHash<QString, QString> layouts = m_rules->layouts();
	QList<QString> keys = layouts.keys();
	// sort by i18n string
	QMap<QString, QString> map;
	foreach (QString str, keys)
    	    map.insert(i18n(layouts[str]), str);
        m_layoutKeys = map.values();
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
	    case LAYOUT_COLUMN_NAME: return i18n(layouts[layout]);
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
    int columnCount(const QModelIndex& /*parent*/) const { return DST_LAYOUT_COLUMN_COUNT; }
    int rowCount(const QModelIndex&) const { return m_kxkbConfig->m_layouts.count(); }
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setRules(XkbRules* rules) { m_rules = rules; }
    void reset() { QAbstractTableModel::reset(); }

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
    QString layout = m_kxkbConfig->m_layouts[row].layout;
	
    if (role == Qt::TextAlignmentRole) {
	return int(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::DecorationRole) {
	switch(col) {
	    case LAYOUT_COLUMN_FLAG: return LayoutIcon::getInstance().findPixmap(layout, true);
	}
    } else if (role == Qt::DisplayRole) {
	switch(col) {
	    case LAYOUT_COLUMN_NAME: return i18n(layouts[layout]);
	    case LAYOUT_COLUMN_MAP: return layout;
	    case LAYOUT_COLUMN_VARIANT: return m_kxkbConfig->m_layouts[row].variant;
	    case LAYOUT_COLUMN_DISPLAY_NAME: return m_kxkbConfig->m_layouts[row].displayName;
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
//        kDebug() << index;
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
//    kDebug() << index << value;
        int groupRow = index.parent().row();
        if( groupRow < 0 ) return false;
        
        QString xkbGroupNm = m_rules->optionGroups().keys()[groupRow];
        const XkbOptionGroup& xkbGroup = m_rules->optionGroups()[xkbGroupNm];
	const XkbOption& option = xkbGroup.options[index.row()];

        if( value.toInt() == Qt::Checked ) {
                if( xkbGroup.exclusive ) {
                // clear if exclusive (TODO: radiobutton)
                    int idx = m_kxkbConfig->m_options.indexOf(QRegExp(xkbGroupNm+".*"));
//                    kDebug() << "other idx to clear" << idx;
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

    void gotoGroup(const QString& group, QAbstractItemView* view) {
        int index = m_rules->optionGroups().keys().indexOf(group);
        kDebug() << "scrolling to group" << index << "-" << group;
        if( index != -1 ) {
//            view->selectionModel()->setCurrentIndex(createIndex(index,0), QItemSelectionModel::NoUpdate);
            view->scrollTo(createIndex(index,0), QAbstractItemView::PositionAtTop);
            view->selectionModel()->setCurrentIndex(createIndex(index,0), QItemSelectionModel::Current);
            view->setFocus(Qt::OtherFocusReason);
            kDebug() << "wdg:" << view->itemDelegate(createIndex(index, 0).child(0,0));
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
    m_rules(NULL),
    m_srcModel(NULL),
    m_dstModel(NULL),
    m_xkbOptModel(NULL)
{
    //Read rules - we _must_ read _before_ creating UIs
    loadRules();

    widget = new Ui_LayoutConfigWidget();
    widget->setupUi(this);
//  main->addWidget(widget);

    m_srcModel = new SrcLayoutModel(m_rules, NULL);
    m_srcModel->setHeaderData(LAYOUT_COLUMN_FLAG, Qt::Horizontal, "");
    m_srcModel->setHeaderData(LAYOUT_COLUMN_NAME, Qt::Horizontal, i18n("Layout name"), Qt::DisplayRole);
    m_srcModel->setHeaderData(LAYOUT_COLUMN_MAP, Qt::Horizontal, i18n("Map"), Qt::DisplayRole);

    widget->srcTableView->setModel(m_srcModel);
//    widget->srcTableView->setSortingEnabled(true);
    widget->srcTableView->setColumnWidth(LAYOUT_COLUMN_FLAG, 26);
    widget->srcTableView->setColumnWidth(LAYOUT_COLUMN_MAP, 30);
    widget->srcTableView->verticalHeader()->hide();
    widget->srcTableView->setShowGrid(false);
    widget->srcTableView->resizeRowsToContents();

    m_dstModel = new DstLayoutModel(m_rules, &m_kxkbConfig, NULL);
    widget->dstTableView->setModel(m_dstModel);
    widget->dstTableView->setColumnWidth(LAYOUT_COLUMN_FLAG, 26);
    widget->dstTableView->setColumnWidth(LAYOUT_COLUMN_MAP, 30);
    widget->dstTableView->verticalHeader()->hide();

    m_xkbOptModel = new XkbOptionsModel(m_rules, &m_kxkbConfig, NULL);
    widget->xkbOptionsTreeView->setModel(m_xkbOptModel);
    widget->xkbOptionsTreeView->header()->hide();
    widget->xkbOptionsTreeView->expandAll();

    connect( m_xkbOptModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), 
        this, SLOT(xkbOptionsChanged(const QModelIndex &, const QModelIndex &)));

    connect( widget->grpEnableKxkb, SIGNAL( clicked( int ) ), SLOT(enableChanged()));
//    connect( widget->chkEnable, SIGNAL( toggled( bool )), this, SLOT(changed()));
//    connect( widget->chkIndicatorOnly, SIGNAL( toggled( bool )), this, SLOT(changed()));
    connect( widget->chkShowSingle, SIGNAL( toggled( bool )), this, SLOT(changed()));
    connect( widget->chkShowFlag, SIGNAL( toggled( bool )), this, SLOT(changed()));
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
    connect( widget->btnKdeShortcut, SIGNAL(clicked()), this, SLOT(kdeShortcutPressed()));

//  connect( widget->editDisplayName, SIGNAL(textChanged(const QString&)), this, SLOT(displayNameChanged(const QString&)));

    widget->btnUp->setIcon(KIcon("arrow-up"));
    connect( widget->btnUp, SIGNAL(clicked()), this, SLOT(moveUp()));
    widget->btnDown->setIcon(KIcon("arrow-down"));
    connect( widget->btnDown, SIGNAL(clicked()), this, SLOT(moveDown()));

    connect( widget->grpSwitching, SIGNAL( clicked( int ) ), SLOT(changed()));
    connect( widget->chkEnableSticky, SIGNAL(toggled(bool)), this, SLOT(changed()));

#ifdef STICKY_SWITCHING
    connect( widget->spinStickyDepth, SIGNAL(valueChanged(int)), this, SLOT(changed()));
#else
    widget->grpBoxStickySwitching->setVisible(false);
#endif
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
	if( modelName.isEmpty() )
		modelName = DEFAULT_MODEL;

        QString modelName_ = i18n(modelName);
	widget->comboModel->setCurrentIndex( widget->comboModel->findText(modelName_) );

	m_dstModel->reset();
	widget->dstTableView->update();

	// display KXKB switching options
	widget->chkShowSingle->setChecked(m_kxkbConfig.m_showSingle);
	widget->chkShowFlag->setChecked(m_kxkbConfig.m_showFlag);

	widget->chkEnableOptions->setChecked( m_kxkbConfig.m_enableXkbOptions );
	widget->checkResetOld->setChecked(m_kxkbConfig.m_resetOldOptions);

	switch( m_kxkbConfig.m_switchingPolicy ) {
		default:
		case SWITCH_POLICY_GLOBAL:
			widget->grpSwitching->setSelected(0);
			break;
		case SWITCH_POLICY_DESKTOP:
			widget->grpSwitching->setSelected(1);
			break;
		case SWITCH_POLICY_WIN_CLASS:
			widget->grpSwitching->setSelected(2);
			break;
		case SWITCH_POLICY_WINDOW:
			widget->grpSwitching->setSelected(3);
			break;
	}

	widget->chkEnableSticky->setChecked(m_kxkbConfig.m_stickySwitching);
	widget->spinStickyDepth->setEnabled(m_kxkbConfig.m_stickySwitching);
	widget->spinStickyDepth->setValue( m_kxkbConfig.m_stickySwitchingDepth);

	updateStickyLimit();

        int enableKxkb = 2;
        if( m_kxkbConfig.m_indicatorOnly ) enableKxkb = 1;
        if( m_kxkbConfig.m_useKxkb ) enableKxkb = 0;
        widget->grpEnableKxkb->setSelected(enableKxkb);
//	widget->chkEnable->setChecked( m_kxkbConfig.m_useKxkb );
	widget->grpLayouts->setEnabled( m_kxkbConfig.m_useKxkb );
	widget->grpOptions->setEnabled( m_kxkbConfig.m_useKxkb );

        updateShortcutsLabels();

	updateLayoutCommand();
	updateOptionsCommand();
	emit KCModule::changed( false );
}


void LayoutConfig::save()
{
	QString model = widget->comboModel->itemData(widget->comboModel->currentIndex()).toString();
	m_kxkbConfig.m_model = model;

	m_kxkbConfig.m_enableXkbOptions = widget->chkEnableOptions->isChecked();
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
	switch( modeId ) {
		default:
		case 0:
			m_kxkbConfig.m_switchingPolicy = SWITCH_POLICY_GLOBAL;
			break;
		case 1:
			m_kxkbConfig.m_switchingPolicy = SWITCH_POLICY_DESKTOP;
			break;
		case 2:
			m_kxkbConfig.m_switchingPolicy = SWITCH_POLICY_WIN_CLASS;
			break;
		case 3:
			m_kxkbConfig.m_switchingPolicy = SWITCH_POLICY_WINDOW;
			break;
	}

	m_kxkbConfig.m_stickySwitching = widget->chkEnableSticky->isChecked();
	m_kxkbConfig.m_stickySwitchingDepth = widget->spinStickyDepth->value();

	m_kxkbConfig.save();

	KToolInvocation::kdeinitExec("kxkb");
	emit KCModule::changed( false );
}

void LayoutConfig::xkbOptionsChanged(const QModelIndex & /*topLeft*/, const QModelIndex & /*bottomRight*/)
{
//    kDebug() << "chked" << topLeft << bottomRight;
    updateOptionsCommand();
    changed();
//    widget->xkbOptionsTreeView->update(topLeft);
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

void LayoutConfig::kdeShortcutPressed()
{
    QStringList args;
    args << "keys";
    int res = KToolInvocation::kdeinitExecWait( "kcmshell", args );
    if( res )
        updateShortcutsLabels();
    else
        kError() << "failed to start 'kcmshell keys'";
}

static QString getShortcutText(const QStringList& options, const QString& grp)
{
    if( options.indexOf(QRegExp("^" + grp + ".*")) != -1 )
        return i18n("Defined");
    else
        return i18n("Not defined");
}

void LayoutConfig::updateShortcutsLabels()
{
    QString txt = getShortcutText( m_kxkbConfig.m_options, "grp" );
    widget->xkbShortcut->setText(txt);
    txt = getShortcutText( m_kxkbConfig.m_options, "lv3" );
    widget->xkbShortcut3d->setText(txt);

    KActionCollection actions(this);
    actions.readSettings();
    QAction* action = actions.action(I18N_NOOP("Switch to Next Keyboard Layout"));
    if( action != NULL ) {
        widget->kdeShortcut->setText( action->shortcut().toString(QKeySequence::NativeText) );
    }
    else {
        widget->kdeShortcut->setText(i18n("Not defined"));
    }
}

void LayoutConfig::updateStickyLimit()
{
    int layoutsCnt = m_kxkbConfig.m_layouts.count();
    int maxDepth = layoutsCnt - 1;

    if( maxDepth < 2 ) {
        maxDepth = 2;
    }

    widget->spinStickyDepth->setMaximum(maxDepth);
/*	if( value > maxDepth )
		setValue(maxDepth);*/
}


void LayoutConfig::enableChanged()
{
    bool enabled = widget->grpEnableKxkb->selected() == 0;
    if( enabled && m_kxkbConfig.m_layouts.count() == 0 ) {
#ifdef HAVE_XKLAVIER
	QList<LayoutUnit> lus = XKlavierAdaptor::getInstance(QX11Info::display())->getGroupNames();
	if( lus.count() > 0 ) {
	    m_kxkbConfig.setConfiguredLayouts(lus);
            m_dstModel->reset();
            widget->dstTableView->update();
	}
#endif
    }
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
    kDebug() << "selected to add" << layout;
    m_kxkbConfig.m_layouts << LayoutUnit(layout, "");

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

    QModelIndexList selected = selectionModel->selectedRows();
    kDebug() << "removing" << selected;
    m_kxkbConfig.m_layouts.removeAt(selected[0].row());

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
    int row = selected[0].row();
    int new_row = row + shift;
    
    if( new_row >= 0 && new_row < GROUP_LIMIT )
	m_kxkbConfig.m_layouts.move(row, new_row);

    m_dstModel->reset();
    widget->dstTableView->update();
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

    QString selectedVariant = widget->comboVariant->currentText();
    if( selectedVariant == DEFAULT_VARIANT_NAME )
        selectedVariant = "";
    
    m_kxkbConfig.m_layouts[row].variant = selectedVariant;
    m_dstModel->reset();
    widget->dstTableView->update();

    updateLayoutCommand();
    changed();
}

void LayoutConfig::displayNameChanged(const QString& newDisplayName)
{
    int row = getSelectedDstLayout();

	if( row == -1 )
		return;

	LayoutUnit& layoutUnit = m_kxkbConfig.m_layouts[row];

	QString oldName = layoutUnit.displayName;

	if( oldName.isEmpty() )
		oldName = KxkbConfig::getDefaultDisplayName( layoutUnit );

	if( oldName != newDisplayName ) {
		kDebug() << "setting label for " << layoutUnit.toPair() << " : " << newDisplayName;
		layoutUnit.displayName = newDisplayName;

		updateIndicator();
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

/** will update flag with label if layout label has been edited
*/
void LayoutConfig::updateIndicator()
{
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

	QStringList vars = m_rules->getAvailableVariants(kbdLayout);
	kDebug() << "layout " << kbdLayout << " has " << vars.count() << " variants";

	if( vars.count() > 0 ) {
		vars.prepend(DEFAULT_VARIANT_NAME);
		widget->comboVariant->addItems(vars);

		QString variant = m_kxkbConfig.m_layouts[row].variant;
		if( variant != NULL && variant.isEmpty() == false ) {
                    int idx = widget->comboVariant->findText(variant);
		    widget->comboVariant->setCurrentIndex(idx);
		}
		else {
		    widget->comboVariant->setCurrentIndex(0);
		}
	}
//	updateDisplayName();
}

void LayoutConfig::makeOptionsTab()
{
    connect(widget->chkEnableOptions, SIGNAL(toggled(bool)), SLOT(changed()));

    connect(widget->checkResetOld, SIGNAL(toggled(bool)), SLOT(changed()));
    connect(widget->checkResetOld, SIGNAL(toggled(bool)), SLOT(updateOptionsCommand()));
}

void LayoutConfig::updateOptionsCommand()
{
  QString setxkbmap;
  QString options = createOptionString();

  if( !options.isEmpty() ) {
    setxkbmap = "setxkbmap -option ";
    if( widget->checkResetOld->isChecked() )
      setxkbmap += "-option ";
    setxkbmap += options;
  }
  widget->editCmdLineOpt->setText(setxkbmap);
}

void LayoutConfig::updateLayoutCommand()
{
	QString kbdLayouts;
	QString kbdVariants;

	QList<LayoutUnit> layouts = m_kxkbConfig.m_layouts;
	for(int i=0; i<layouts.count(); i++) {
		QString layout = layouts[i].layout;
		QString variant = layouts[i].variant;
		QString displayName = layouts[i].displayName;

		if( variant == DEFAULT_VARIANT_NAME )
			variant = "";

		if( kbdLayouts.length() > 0 ) {
			kbdLayouts += ',';
			kbdVariants += ',';
		}

		kbdLayouts += layout;
		kbdVariants += variant;
	}

    QString setxkbmap = "setxkbmap";
    setxkbmap += " -model " + widget->comboModel->itemData(widget->comboModel->currentIndex()).toString();
    setxkbmap += " -layout " + kbdLayouts;
    setxkbmap += " -variant " + kbdVariants;

    widget->editCmdLine->setText(setxkbmap);
}

/*
void LayoutConfig::updateDisplayName()
{
	Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();

	QString layoutDisplayName;
		kDebug() << "sel: '" << sel << "'";
	if( sel != NULL ) {
		LayoutUnit layoutUnitKey = getLayoutUnitKey(sel);
		QString kbdLayout = layoutUnitKey.layout;
		QString variant = layoutUnitKey.variant;
	//	QString layoutDisplayName = m_kxkbConfig.getLayoutDisplayName( *m_kxkbConfig.m_layouts.find(layoutUnitKey) );
		layoutDisplayName = sel->text(LAYOUT_COLUMN_DISPLAY_NAME);
		if( layoutDisplayName.isEmpty() ) {
			int count = 0;
			Q3ListViewItem *item = widget->listLayoutsDst->firstChild();
			while (item) {
				QString layout_ = item->text(LAYOUT_COLUMN_MAP);
				if( layout_ == kbdLayout )
					++count;
				item = item->nextSibling();
			}
			bool single = count < 2;
			layoutDisplayName = m_kxkbConfig.getDefaultDisplayName(LayoutUnit(kbdLayout, variant), single);
		}
		kDebug() << "disp: '" << layoutDisplayName << "'";
	}

	widget->editDisplayName->setEnabled( sel != NULL );
	widget->editDisplayName->setText(layoutDisplayName);
}
*/

void LayoutConfig::changed()
{
  bool enabled = widget->grpEnableKxkb->selected() == BTN_XKB_ENABLE;

//  widget->chkIndicatorOnly->setEnabled(enabled);
//  if( ! enabled )
//	widget->chkIndicatorOnly->setChecked(false);

  widget->grpLayouts->setEnabled(enabled);
  widget->tabWidget->widget(TAB_OPTIONS)->setEnabled(enabled);

//  bool indicatorOnly = widget->chkIndicatorOnly->isChecked();
//  widget->grpIndicator->setEnabled(indicatorOnly);
  
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
//    QStringList modelsList;
    widget->comboModel->clear();
    QHashIterator<QString, QString> it(m_rules->models());
    while (it.hasNext()) {
//		modelsList.append(i18n(it.next().value()));
	const QString key = it.next().key();
	widget->comboModel->addItem(i18n(m_rules->models()[key]), key);
    }
//    modelsList.sort();
// TODO: sort

//	widget->comboModel->addItems(modelsList);
    widget->comboModel->setCurrentIndex(0);
	//TODO: reset options and xkb options
}


QString LayoutConfig::createOptionString()
{
    QString options = m_kxkbConfig.m_options.join(",");
    return options;
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
    KDE_EXPORT void kcminit_keyboard()
    {
	KxkbConfig m_kxkbConfig;
	m_kxkbConfig.load(KxkbConfig::LOAD_INIT_OPTIONS);

	if( m_kxkbConfig.m_useKxkb == true ) {
	    KToolInvocation::startServiceByDesktopName("kxkb");
	}
	else {
	    // Even if the layouts have been disabled we still want to set Xkb options
	    // user can always switch them off now in the "Options" tab
	    if( m_kxkbConfig.m_enableXkbOptions ) {
		if( !XKBExtension::setXkbOptions(m_kxkbConfig.m_options.join(","), m_kxkbConfig.m_resetOldOptions) ) {
		    kDebug() << "Setting XKB options failed!";
		}
	    }
	}
    }
}
