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
#include <Q3ListView>
#include <Q3ListViewItem>
#include <Q3CheckListItem>
#include <QHeaderView>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QtGui>

#include <kshortcutsdialog.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
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

static const QString DEFAULT_VARIANT_NAME("<default>");

class OptionListItem : public Q3CheckListItem
{
	public:

		OptionListItem(  OptionListItem *parent, const QString &text, Type tt,
						 const QString &optionName );
		OptionListItem(  Q3ListView *parent, const QString &text, Type tt,
						 const QString &optionName );
		~OptionListItem() {}

		QString optionName() const { return m_OptionName; }
		OptionListItem *findChildItem(  const QString& text );

	protected:
		QString m_OptionName;
};


OptionListItem::OptionListItem( OptionListItem *parent, const QString &text,
								Type tt, const QString &optionName )
	: Q3CheckListItem( parent, text, tt ), m_OptionName( optionName )
{
}

OptionListItem::OptionListItem( Q3ListView *parent, const QString &text,
								Type tt, const QString &optionName )
	: Q3CheckListItem( parent, text, tt ), m_OptionName( optionName )
{
}

OptionListItem * OptionListItem::findChildItem( const QString& optionName )
{
	OptionListItem *child = static_cast<OptionListItem *>( firstChild() );

	while ( child )
	{
		if ( child->optionName() == optionName )
			break;
		child = static_cast<OptionListItem *>( child->nextSibling() );
	}

	return child;
}

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
    int columnCount(const QModelIndex& parent) const { return DST_LAYOUT_COLUMN_COUNT; }
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
	      
    QString colNames[] = {"", i18n("Layout Name"), i18n("Map"), i18n("Variant"), i18n("Display Name")};
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


//K_PLUGIN_FACTORY_DECLARATION(KeyboardLayoutFactory)

LayoutConfig::LayoutConfig(QWidget *parent, const QVariantList &)
  : KCModule(KeyboardLayoutFactory::componentData(), parent),
    m_rules(NULL),
    m_srcModel(NULL),
    m_dstModel(NULL)
{
    //Read rules - we _must_ read _before_ creating UIs
    loadRules();

    widget = new Ui_LayoutConfigWidget();
    widget->setupUi(this);
//  main->addWidget(widget);

    m_srcModel = new SrcLayoutModel(m_rules, NULL);
    m_srcModel->setHeaderData(LAYOUT_COLUMN_FLAG, Qt::Horizontal, "");
    m_srcModel->setHeaderData(LAYOUT_COLUMN_NAME, Qt::Horizontal, "Layout name", Qt::DisplayRole);
    m_srcModel->setHeaderData(LAYOUT_COLUMN_MAP, Qt::Horizontal, "Map", Qt::DisplayRole);

    widget->srcTableView->setModel(m_srcModel);
//    widget->srcTableView->setSortingEnabled(true);
    widget->srcTableView->setColumnWidth(LAYOUT_COLUMN_FLAG, 26);
    widget->srcTableView->verticalHeader()->hide();
    widget->srcTableView->setShowGrid(false);
    widget->srcTableView->resizeRowsToContents();

    m_dstModel = new DstLayoutModel(m_rules, &m_kxkbConfig, NULL);
    widget->dstTableView->setModel(m_dstModel);
    widget->dstTableView->setColumnWidth(LAYOUT_COLUMN_FLAG, 26);
    widget->dstTableView->verticalHeader()->hide();


  connect( widget->chkEnable, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->chkIndicatorOnly, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->chkShowSingle, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->chkShowFlag, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->comboModel, SIGNAL(activated(int)), this, SLOT(changed()));

  connect( widget->srcTableView, SIGNAL(doubleClicked(const QModelIndex &)),
									this, SLOT(add()));
  connect( widget->btnAdd, SIGNAL(clicked()), this, SLOT(add()));
  connect( widget->btnRemove, SIGNAL(clicked()), this, SLOT(remove()));

//  connect( widget->comboVariant, SIGNAL(activated(int)), this, SLOT(changed()));
  connect( widget->comboVariant, SIGNAL(activated(int)), this, SLOT(variantChanged()));
  connect( widget->dstTableView->selectionModel(), 
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this, SLOT(layoutSelChanged()) );

//  connect( widget->editDisplayName, SIGNAL(textChanged(const QString&)), this, SLOT(displayNameChanged(const QString&)));

  widget->btnUp->setIconSet(KIcon("arrow-up"));
//  connect( widget->btnUp, SIGNAL(clicked()), this, SLOT(changed()));
  connect( widget->btnUp, SIGNAL(clicked()), this, SLOT(moveUp()));
  widget->btnDown->setIconSet(KIcon("arrow-down"));
//  connect( widget->btnDown, SIGNAL(clicked()), this, SLOT(changed()));
  connect( widget->btnDown, SIGNAL(clicked()), this, SLOT(moveDown()));

  connect( widget->grpSwitching, SIGNAL( clicked( int ) ), SLOT(changed()));

  connect( widget->chkEnableSticky, SIGNAL(toggled(bool)), this, SLOT(changed()));
  connect( widget->spinStickyDepth, SIGNAL(valueChanged(int)), this, SLOT(changed()));

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

	widget->comboModel->setCurrentText(i18n(modelName));

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
			widget->grpSwitching->setButton(0);
			break;
		case SWITCH_POLICY_DESKTOP:
			widget->grpSwitching->setButton(1);
			break;
		case SWITCH_POLICY_WIN_CLASS:
			widget->grpSwitching->setButton(2);
			break;
		case SWITCH_POLICY_WINDOW:
			widget->grpSwitching->setButton(3);
			break;
	}

	widget->chkEnableSticky->setChecked(m_kxkbConfig.m_stickySwitching);
	widget->spinStickyDepth->setEnabled(m_kxkbConfig.m_stickySwitching);
	widget->spinStickyDepth->setValue( m_kxkbConfig.m_stickySwitchingDepth);

	updateStickyLimit();

	widget->chkEnable->setChecked( m_kxkbConfig.m_useKxkb );
	widget->grpLayouts->setEnabled( m_kxkbConfig.m_useKxkb );
	widget->grpOptions->setEnabled( m_kxkbConfig.m_useKxkb );

	// display xkb options
	QStringList options = m_kxkbConfig.m_options.split(',');
	for (QListIterator<QString> it(options); it.hasNext(); )
	{
		QString optionName = it.next();
		if( optionName.trimmed().isEmpty() ) {
			kWarning() << "skipping empty option name" ;
  			continue;
		}

		const XkbOption& option = m_rules->options()[optionName];
		OptionListItem *item = m_optionGroups[ option.group->name ];

		if (item != NULL) {
			OptionListItem *child = item->findChildItem( option.name );

			if ( child )
				child->setState( Q3CheckListItem::On );
			else
				kDebug() << "load: Unknown option: " << optionName;
		}
		else {
			kDebug() << "load: Unknown option group: " << option.group->name << " of " << optionName;
		}
	}

	updateLayoutCommand();
	updateOptionsCommand();
	emit KCModule::changed( false );
}


void LayoutConfig::save()
{
//	QString model = lookupLocalized(m_rules->models(), widget->comboModel->currentText());
	QString model = widget->comboModel->itemData(widget->comboModel->currentIndex()).toString();
	m_kxkbConfig.m_model = model;

	m_kxkbConfig.m_enableXkbOptions = widget->chkEnableOptions->isChecked();
	m_kxkbConfig.m_resetOldOptions = widget->checkResetOld->isChecked();
	m_kxkbConfig.m_options = createOptionString();

	if( m_kxkbConfig.m_layouts.count() == 0 ) {
		m_kxkbConfig.m_layouts.append(LayoutUnit(DEFAULT_LAYOUT_UNIT));
 		widget->chkEnable->setChecked(false);
 	}

	m_kxkbConfig.m_useKxkb = widget->chkEnable->isChecked();
	m_kxkbConfig.m_indicatorOnly = widget->chkIndicatorOnly->isChecked();
	m_kxkbConfig.m_showSingle = widget->chkShowSingle->isChecked();
	m_kxkbConfig.m_showFlag = widget->chkShowFlag->isChecked();

	int modeId = widget->grpSwitching->id(widget->grpSwitching->selected());
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
	return - 1;

    QModelIndexList selected = selectionModel->selectedRows();
    int row = selected[0].row();
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
			widget->comboVariant->setCurrentText(variant);
		}
		else {
			widget->comboVariant->setCurrentIndex(0);
		}
	}
//	updateDisplayName();
}

QWidget* LayoutConfig::makeOptionsTab()
{
  Q3ListView *listView = widget->listOptions;

  listView->setMinimumHeight(150);
  listView->setSortColumn( -1 );
  listView->setColumnText( 0, i18n( "Options" ) );
  listView->clear();

  connect(listView, SIGNAL(clicked(Q3ListViewItem *)), SLOT(changed()));
  connect(listView, SIGNAL(clicked(Q3ListViewItem *)), SLOT(updateOptionsCommand()));

  connect(widget->chkEnableOptions, SIGNAL(toggled(bool)), SLOT(changed()));

  connect(widget->checkResetOld, SIGNAL(toggled(bool)), SLOT(changed()));
  connect(widget->checkResetOld, SIGNAL(toggled(bool)), SLOT(updateOptionsCommand()));

  //Create controllers for all options
  QHashIterator<QString, XkbOptionGroup> it( m_rules->optionGroups() );
  for (; it.hasNext(); )
  {
	  OptionListItem *parent;
	  const XkbOptionGroup& optionGroup = it.next().value();

      if( optionGroup.exclusive ) {
        parent = new OptionListItem(listView, i18n( optionGroup.description ),
          		Q3CheckListItem::RadioButtonController, optionGroup.name);
        OptionListItem *item = new OptionListItem(parent, i18n( "None" ),
          		Q3CheckListItem::RadioButton, "none");
        item->setState(Q3CheckListItem::On);
      }
      else {
        parent = new OptionListItem(listView, i18n( optionGroup.description ),
            Q3CheckListItem::CheckBoxController, optionGroup.name);
      }

      parent->setOpen(true);
      m_optionGroups.insert( optionGroup.name, parent);
	  kDebug() << "optionGroup insert: " << optionGroup.name;
  }


  QHashIterator<QString, XkbOption> it2( m_rules->options() );
  for (; it2.hasNext(); )
  {
	  const XkbOption& option = it2.next().value();

	  OptionListItem *parent = m_optionGroups[option.group->name];
	  if( parent == NULL ) {
		kError() << "no option group item for group: " << option.group->name
			   << " for option " << option.name << endl;
		exit(1);
	  }

     if( parent->type() == Q3CheckListItem::RadioButtonController )
		new OptionListItem(parent, i18n( option.description ),
             Q3CheckListItem::RadioButton, option.name);
     else
	 	new OptionListItem(parent, i18n( option.description ),
            Q3CheckListItem::CheckBox, option.name);

//	  kDebug() << "option insert: " << option.name;
  }

  //scroll->setMinimumSize(450, 330);

  return listView;
}

void LayoutConfig::updateOptionsCommand()
{
  QString setxkbmap;
  QString options = createOptionString();

  if( !options.isEmpty() ) {
    setxkbmap = "setxkbmap -option "; //-rules " + m_rule
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
  bool enabled = widget->chkEnable->isChecked();

  widget->chkIndicatorOnly->setEnabled(enabled);
  if( ! enabled )
	widget->chkIndicatorOnly->setChecked(false);

  widget->grpLayouts->setEnabled(enabled);
  widget->grpSwitching->setEnabled(enabled);
//  widget->grpStickySwitching->setEnabled(enabled);

//  bool indicatorOnly = widget->chkIndicatorOnly->isChecked();
//  widget->grpIndicator->setEnabled(indicatorOnly);
  
  emit KCModule::changed( true );
}


void LayoutConfig::loadRules()
{
    // do we need this ?
    // this could obly be used if rules are changed and 'Defaults' is pressed
    delete m_rules;
    m_rules = new XkbRules();
    if( m_srcModel )
        m_srcModel->setRules(m_rules);
    if( m_dstModel )
	m_dstModel->setRules(m_rules);
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
  QString options;
  for (QHashIterator<QString, XkbOption> it(m_rules->options()); it.hasNext(); )
  {
    const XkbOption& option = it.next().value();

      OptionListItem *item = m_optionGroups[ option.group->name ];

      if( !item ) {
        kDebug() << "WARNING: skipping empty group for " << option.name
          << " - could not found group: " << option.group->name << endl;
        continue;
      }

      OptionListItem *child = item->findChildItem( option.name );

      if ( child ) {
        if ( child->state() == Q3CheckListItem::On ) {
          QString selectedName = child->optionName();
          if ( !selectedName.isEmpty() && selectedName != "none" ) {
            if (!options.isEmpty())
              options.append(',');
            options.append(selectedName);
          }
        }
      }
      else
        kDebug() << "Empty option button for group " << it.key();
  }
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
				if( !XKBExtension::setXkbOptions(m_kxkbConfig.m_options, m_kxkbConfig.m_resetOldOptions) ) {
					kDebug() << "Setting XKB options failed!";
				}
			}
		}
	}
}
