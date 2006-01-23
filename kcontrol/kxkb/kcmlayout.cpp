#include <stdlib.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <q3listview.h>
#include <q3header.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <q3buttongroup.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kkeydialog.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>

#include "extension.h"
#include "rules.h"
#include "kcmlayout.h"
#include "kcmlayout.moc"
#include "pixmap.h"
#include "kcmmisc.h"
#include <kiconloader.h>
#include "kcmlayoutwidget.h"

#include <X11/Xlib.h>
#include <QX11Info>
#include <ktoolinvocation.h>


static const char* switchModes[] = {
  "Global", "WinClass", "Window"
};

static QString lookupLocalized(const Q3Dict<char> &dict, const QString& text)
{
  Q3DictIterator<char> it(dict);
  while (it.current())
    {
      if ( i18n(it.current()) == text )
        return it.currentKey();
      ++it;
    }

  return QString();
}

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

LayoutConfig::LayoutConfig(QWidget *parent, const char *name)
  : KCModule(parent, name), 
    m_rules(NULL)
{
  QVBoxLayout *main = new QVBoxLayout(this, 0, KDialog::spacingHint());

  widget = new LayoutConfigWidget(this, "widget");
  main->addWidget(widget);

  connect( widget->chkEnable, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->chkShowSingle, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->chkShowFlag, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->comboModel, SIGNAL(activated(int)), this, SLOT(changed()));

  connect( widget->listLayoutsSrc, SIGNAL(doubleClicked(Q3ListViewItem*,const QPoint&, int)),
									this, SLOT(add()));
  connect( widget->btnAdd, SIGNAL(clicked()), this, SLOT(add()));
  connect( widget->btnRemove, SIGNAL(clicked()), this, SLOT(remove()));

  connect( widget->comboVariant, SIGNAL(activated(int)), this, SLOT(changed()));
  connect( widget->comboVariant, SIGNAL(activated(int)), this, SLOT(variantChanged()));
  connect( widget->listLayoutsDst, SIGNAL(selectionChanged(Q3ListViewItem *)),
		this, SLOT(layoutSelChanged(Q3ListViewItem *)));

  connect( widget->chkLatin, SIGNAL(clicked()), this, SLOT(changed()));
  connect( widget->chkLatin, SIGNAL(clicked()), this, SLOT(latinChanged()));

  widget->btnUp->setIconSet(SmallIconSet("1uparrow"));
  connect( widget->btnUp, SIGNAL(clicked()), this, SLOT(changed()));
  connect( widget->btnUp, SIGNAL(clicked()), this, SLOT(moveUp()));
  widget->btnDown->setIconSet(SmallIconSet("1downarrow"));
  connect( widget->btnDown, SIGNAL(clicked()), this, SLOT(changed()));
  connect( widget->btnDown, SIGNAL(clicked()), this, SLOT(moveDown()));

  connect( widget->grpSwitching, SIGNAL( clicked( int ) ), SLOT(changed()));

  connect( widget->chkEnableSticky, SIGNAL(toggled(bool)), this, SLOT(changed()));
  connect( widget->spinStickyDepth, SIGNAL(valueChanged(int)), this, SLOT(changed()));

  widget->listLayoutsSrc->setColumnWidth(0, 28);
  widget->listLayoutsDst->setColumnWidth(0, 28);
  widget->listLayoutsDst->setSorting(-1);
#if 0
  widget->listLayoutsDst->setResizeMode(Q3ListView::LastColumn);
  widget->listLayoutsSrc->setResizeMode(Q3ListView::LastColumn);
#endif

  //Read rules - we _must_ read _before_ creating xkb-options comboboxes
  ruleChanged();

  makeOptionsTab();

  load();
}


LayoutConfig::~LayoutConfig()
{
  delete m_rules;
}

void LayoutConfig::updateStickyLimit()
{
    int layoutsCnt = widget->listLayoutsDst->childCount();
    widget->spinStickyDepth->setMaxValue( (layoutsCnt<=2) ? 2 : layoutsCnt - 1);
}

void LayoutConfig::add()
{
    Q3ListViewItem* sel = widget->listLayoutsSrc->selectedItem();
    if( sel == 0 )
	return;
    
    widget->listLayoutsSrc->takeItem(sel);
    widget->listLayoutsDst->insertItem(sel);
    if( widget->listLayoutsDst->childCount() > 1 )
	sel->moveItem(widget->listLayoutsDst->lastItem());
// disabling temporary: does not work reliable in Qt :(
//    widget->listLayoutsSrc->setSelected(sel, true);
//    layoutSelChanged(sel);

    updateStickyLimit();
    changed();
}

void LayoutConfig::remove()
{
    Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();
    Q3ListViewItem* newSel = 0;

    if( sel == 0 )
	return;

    if( sel->itemBelow() )
	newSel = sel->itemBelow();
    else
    if( sel->itemAbove() )
	newSel = sel->itemAbove();

    widget->listLayoutsDst->takeItem(sel);
    widget->listLayoutsSrc->insertItem(sel);
    if( newSel )
	widget->listLayoutsSrc->setSelected(newSel, true);
    layoutSelChanged(newSel);

    updateStickyLimit();
    changed();
}

void LayoutConfig::moveUp()
{
    Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();
    if( sel == 0 || sel->itemAbove() == 0 )
	return;

    if( sel->itemAbove()->itemAbove() == 0 ) {
	widget->listLayoutsDst->takeItem(sel);
	widget->listLayoutsDst->insertItem(sel);
	widget->listLayoutsDst->setSelected(sel, true);
    }
    else
	sel->moveItem(sel->itemAbove()->itemAbove());
}

void LayoutConfig::moveDown()
{
    Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();
    if( sel == 0 || sel->itemBelow() == 0 )
	return;

    sel->moveItem(sel->itemBelow());
}

void LayoutConfig::variantChanged()
{
    Q3ListViewItem* selLayout = widget->listLayoutsDst->selectedItem();
    if( !selLayout ) {
      widget->comboVariant->clear();
      widget->comboVariant->setEnabled(false);
      return;
    }

    QString kbdLayout = lookupLocalized( m_rules->layouts(), selLayout->text(1) );
    m_variants.replace(kbdLayout, widget->comboVariant->currentText().latin1());
}

void LayoutConfig::latinChanged()
{
    Q3ListViewItem* selLayout = widget->listLayoutsDst->selectedItem();
    if (  !selLayout ) {
      widget->chkLatin->setChecked( false );
      widget->chkLatin->setEnabled( false );
      return;
    }

    QString kbdLayout = lookupLocalized(  m_rules->layouts(), selLayout->text( 1 ) );
    if (  widget->chkLatin->isChecked() )
        m_includes.replace( kbdLayout, "us" );
    else
        m_includes.replace( kbdLayout, "" );
}

void LayoutConfig::layoutSelChanged(Q3ListViewItem *sel)
{
    widget->comboVariant->clear();
    widget->comboVariant->setEnabled( sel != 0 );
    widget->chkLatin->setChecked( false );
    widget->chkLatin->setEnabled( sel != 0 );

    if( sel == 0 ) {
        updateLayoutCommand();
        return;
    }

    QString kbdLayout = lookupLocalized( m_rules->layouts(), sel->text(1) );

// need better algorithm here for determining if needs us group
    if (  ! m_rules->isSingleGroup(kbdLayout) 
	    || kbdLayout.startsWith("us") || kbdLayout.startsWith("en") ) {
        widget->chkLatin->setEnabled( false );
    }
    else {
        char* inc = m_includes[ kbdLayout ];
        if ( inc && (strncmp(inc, "us", 2)==0 || strncmp(inc, "en", 2)==0) ) {
            widget->chkLatin->setChecked(true);
        }
        else {
            widget->chkLatin->setChecked(false);
        }
    }

    QStringList vars = m_rules->getVariants(kbdLayout);

    if( vars.count() == 0 ) {	// cowardly running away
        updateLayoutCommand();
        return;
    }

    char* variant = m_variants[kbdLayout];
    widget->comboVariant->insertStringList(vars);

    if( variant ) {
      widget->comboVariant->setCurrentText(variant);
    }
    else {
      widget->comboVariant->setCurrentItem(0);
      m_variants.insert(kbdLayout, widget->comboVariant->currentText().latin1());
    }

    updateLayoutCommand();
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
  Q3DictIterator<char> it(m_rules->options());
  OptionListItem *parent;
  for (; it.current(); ++it)
  {
    if (!it.currentKey().contains(':'))
    {
      if( it.currentKey() == "ctrl" || it.currentKey() == "caps"
          || it.currentKey() == "altwin" ) {
        parent = new OptionListItem(listView, i18n( it.current() ),
            Q3CheckListItem::RadioButtonController, it.currentKey());
        OptionListItem *item = new OptionListItem(parent, i18n( "None" ),
            Q3CheckListItem::RadioButton, "none");
        item->setState(Q3CheckListItem::On);
      }
      else {
        parent = new OptionListItem(listView, i18n( it.current() ),
            Q3CheckListItem::CheckBoxController, it.currentKey());
      }
      parent->setOpen(true);
      m_optionGroups.insert(i18n(it.currentKey().toLocal8Bit()), parent);
    }
  }

  it.toFirst();
  for( ; it.current(); ++it)
  {
    QString key = it.currentKey();
    int pos = key.find(':');
    if (pos >= 0)
    {
      OptionListItem *parent = m_optionGroups[key.left(pos)];
      if (parent == NULL )
        parent = m_optionGroups["misc"];
      if (parent != NULL) {
      // workaroung for mistake in rules file for xkb options in XFree 4.2.0
        QString text(it.current());
        text = text.replace( "Cap$", "Caps." );
        if( parent->type() == Q3CheckListItem::RadioButtonController )
            new OptionListItem(parent, i18n(text.latin1()),
                Q3CheckListItem::RadioButton, key);
        else
            new OptionListItem(parent, i18n(text.latin1()),
                Q3CheckListItem::CheckBox, key);
      }
    }
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
  QString setxkbmap;
  Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();

  if( sel ) {
    QString kbdLayout = lookupLocalized(m_rules->layouts(), sel->text(1));
//    char* variant = m_variants[kbdLayout];
    QString variant = widget->comboVariant->currentText();

    setxkbmap = "setxkbmap"; //-rules " + m_rule
    setxkbmap += " -model " + lookupLocalized(m_rules->models(), widget->comboModel->currentText())
      + " -layout ";
    if( widget->chkLatin->isChecked() )
      setxkbmap += "us,";
    setxkbmap += kbdLayout;

    if( !variant.isEmpty() ) {
      setxkbmap += " -variant ";
      if( widget->chkLatin->isChecked() )
        setxkbmap += ",";
      setxkbmap += variant;
    }
  }
  widget->editCmdLine->setText(setxkbmap);
}

void LayoutConfig::changed()
{
  updateLayoutCommand();
  emit KCModule::changed( true );
}


void LayoutConfig::load()
{
  // open the config file
  KConfig *config = new KConfig("kxkbrc", true);
  config->setGroup("Layout");

  bool use = config->readEntry( "Use", false);

  // find out which rule applies
  //QString rule = "xfree86"; //config->readEntry("Rule", "xfree86");

  // update other files
  ruleChanged();

  // find out about the model
  QString model = config->readEntry("Model", "pc104");

  // TODO: When no model and no layout are known (1st start), we really
  // should ask the X-server about it's settings!

  QString m_name = m_rules->models()[model];
  widget->comboModel->setCurrentText(i18n(m_name.toLocal8Bit()));

  QString layout = config->readEntry("Layout", "us");
  QString l_name = m_rules->layouts()[layout];

  QStringList otherLayouts = config->readEntry("Additional", QStringList());
  if( !l_name.isEmpty() ) {
    otherLayouts.prepend(layout);
  }
// to optimize we should have gone from it.end to it.begin
  for ( QStringList::Iterator it = otherLayouts.begin(); it != otherLayouts.end(); ++it ) {
    Q3ListViewItemIterator src_it( widget->listLayoutsSrc );
    for ( ; src_it.current(); ++src_it ) {
	Q3ListViewItem* srcItem = src_it.current();
	if ( *it == lookupLocalized(m_rules->layouts(), src_it.current()->text(1)) ) {
    	    widget->listLayoutsSrc->takeItem(srcItem);
    	    widget->listLayoutsDst->insertItem(srcItem);
	    srcItem->moveItem(widget->listLayoutsDst->lastItem());
	    break;
	}
    }
  }

// reading variants
  QStringList vars = config->readEntry("Variants", QStringList());
  m_rules->parseVariants(vars, m_variants);
//  m_rules->parseVariants( vars, m_variants );

  QStringList incs = config->readEntry( "Includes", QStringList() );
  m_rules->parseVariants( incs, m_includes, false );

  bool showSingle = config->readEntry( "ShowSingle", false );
  widget->chkShowSingle->setChecked(showSingle);

  bool showFlag = config->readEntry( "ShowFlag", true );
  widget->chkShowFlag->setChecked(showFlag);

  bool enableXkbOptions = config->readEntry( "EnableXkbOptions", true );
  widget->chkEnableOptions->setChecked( enableXkbOptions );
  bool resetOld = config->readEntry("ResetOldOptions", false);
  widget->checkResetOld->setChecked(resetOld);
  QStringList options = config->readEntry("Options", QStringList());

  for (QStringList::Iterator it = options.begin(); it != options.end(); ++it)
    {
      QString option = *it;
      QString optionKey = option.mid(0, option.find(':'));
      QString optionName = m_rules->options()[option];
      OptionListItem *item = m_optionGroups[optionKey];
      if (item != NULL) {
        OptionListItem *child = item->findChildItem( option );

        if ( child )
          child->setState( Q3CheckListItem::On );
	    else
	      kdDebug() << "load: Unknown option " << option << endl;
      }
      else {
	    kdDebug() << "load: Unknown option group " << optionKey << endl;
      }
    }

  QString swMode = config->readEntry("SwitchMode", "Global");
  widget->grpSwitching->setButton(0);

   for(int ii=1; ii<3; ii++)
    if( swMode == switchModes[ii] )
      widget->grpSwitching->setButton(ii);

  bool stickySwitching = config->readEntry("StickySwitching", false);
  widget->chkEnableSticky->setChecked(stickySwitching);
  widget->spinStickyDepth->setEnabled(stickySwitching);
  widget->spinStickyDepth->setValue( config->readEntry("StickySwitchingDepth", 1) + 1);

  updateStickyLimit();

  delete config;

  widget->chkEnable->setChecked( use );
  widget->grpLayouts->setEnabled(use);
  widget->grpSwitching->setEnabled(use);

  updateOptionsCommand();
  emit KCModule::changed( false );
}

void LayoutConfig::ruleChanged()
{
//  if( rule == m_rule )
//    return;

//  m_rule = rule;

  QString model; //, layout;
  if (m_rules)
    {
      model = lookupLocalized(m_rules->models(), widget->comboModel->currentText());
//      layout = lookupLocalized(m_rules->layouts(), layoutCombo->currentText());
    }

  delete m_rules;
  m_rules = new KeyRules();

  QStringList tmp;
  widget->comboModel->clear();
  Q3DictIterator<char> it(m_rules->models());
  while (it.current())
    {
      tmp.append(i18n(it.current()));
      ++it;
    }
  tmp.sort();
  widget->comboModel->insertStringList(tmp);


  // fill in the additional layouts  -- moved from load() by A. Rysin
  widget->listLayoutsSrc->clear();
  widget->listLayoutsDst->clear();
  Q3DictIterator<char> it2(m_rules->layouts());
  while (it2.current())
    {
      Q3CheckListItem *item = new Q3CheckListItem(widget->listLayoutsSrc, "");
      QString addLayout = it2.currentKey();
      item->setPixmap(0, LayoutIcon::findPixmap(addLayout, true));
      item->setText(1, i18n(it2.current()) );
      item->setText(2, "(" + addLayout + ")" );
      ++it2;
    }
  widget->listLayoutsSrc->setSorting(1);	// from Qt3 QListView sorts by language


  if (!model.isEmpty())
  {
    QString m_name = m_rules->models()[model];
    widget->comboModel->setCurrentText(m_name);
  }
}


QString LayoutConfig::createOptionString()
{
  QString options;
  for (Q3DictIterator<char> it(m_rules->options()); it.current(); ++it)
  {
    QString option(it.currentKey());

    if (option.contains(':')) {

      QString optionKey = option.mid(0, option.find(':'));
      OptionListItem *item = m_optionGroups[optionKey];

      if( !item ) {
        kdDebug() << "WARNING: skipping empty group for " << it.currentKey()
          << endl;
        continue;
      }

      OptionListItem *child = item->findChildItem( option );

      if ( child ) {
        if ( child->state() == Q3CheckListItem::On ) {
          QString selectedName = child->optionName();
          if ( !selectedName.isEmpty() && selectedName != "none" ) {
            if (!options.isEmpty())
              options.append(QLatin1Char(','));
            options.append(selectedName);
          }
        }
      }
      else
        kdDebug() << "Empty option button for group " << it.currentKey() << endl;
    }
  }
  return options;
}

void LayoutConfig::save()
{
  KConfig *config = new KConfig("kxkbrc", false);
  config->setGroup("Layout");
  //  config->writeEntry("Rule", ruleCombo->currentText());

  QString model = lookupLocalized(m_rules->models(), widget->comboModel->currentText());
  config->writeEntry("Model", model);

  config->writeEntry("EnableXkbOptions", widget->chkEnableOptions->isChecked() );
  config->writeEntry("ResetOldOptions", widget->checkResetOld->isChecked());
  config->writeEntry("Options", createOptionString() );

  QString layout;
  QStringList otherLayouts;
  Q3ListViewItem *item = widget->listLayoutsDst->firstChild();
  if( item ) {
    layout = lookupLocalized(m_rules->layouts(), item->text(1));
    if( !layout.isEmpty() )
	config->writeEntry("Layout", layout);
    item = item->nextSibling();
  }
  else {
    widget->chkEnable->setChecked(false);
  }

  while (item) {
	QString layout = lookupLocalized(m_rules->layouts(), item->text(1));
	otherLayouts.append(layout);
      item = item->nextSibling();
  }
  config->writeEntry("Additional", otherLayouts);

  QStringList varList;
  QStringList incList;
  item = widget->listLayoutsDst->firstChild();
  while ( item ) {
    QString layout = lookupLocalized( m_rules->layouts(), item->text( 1 ) );
    if( m_includes[ layout ] && m_includes[ layout ][ 0 ] != '\0' )
    {
        QString inc = layout;
        inc += "(";
        inc += m_includes[ layout ];
        inc += ")";
        incList.append(  inc );
    }
    if( m_variants[ layout ] && m_variants[ layout ][ 0 ] != '\0' )
    {
	QString var = layout;
	var += "(";
	var += m_variants[ layout ];
	var += ")";
	varList.append( var );
    }

    item = item->nextSibling();
  }
  config->writeEntry( "Includes", incList );
  config->writeEntry("Variants", varList);

  config->writeEntry("Use", widget->chkEnable->isChecked());
  config->writeEntry("ShowSingle", widget->chkShowSingle->isChecked());
  config->writeEntry("ShowFlag", widget->chkShowFlag->isChecked());

  int modeId = widget->grpSwitching->id(widget->grpSwitching->selected());
  if( modeId < 1 || modeId > 2 )
    modeId = 0;

  config->writeEntry("SwitchMode", switchModes[modeId]);

  config->writeEntry("StickySwitching", widget->chkEnableSticky->isChecked());
  config->writeEntry("StickySwitchingDepth", widget->spinStickyDepth->value() - 1);

  config->sync();

  delete config;

  KToolInvocation::kdeinitExec("kxkb");
  emit KCModule::changed( false );
}


void LayoutConfig::defaults()
{
  widget->chkEnable->setChecked(false);
  ruleChanged();

  widget->comboModel->setCurrentText("pc104");

  widget->chkEnableOptions->setChecked( true );
  widget->checkResetOld->setChecked( false );

  widget->grpSwitching->setButton(0);

  widget->chkEnableSticky->setChecked( false );
  widget->spinStickyDepth->setEnabled( false );

  Q3ListViewItem *item = widget->listLayoutsSrc->firstChild();
  while (item)
    {
      Q3CheckListItem *cli = dynamic_cast<Q3CheckListItem*>(item);
      cli->setOn(false);
      item = item->nextSibling();
    }
  emit KCModule::changed( true );
}

extern "C"
{
  KDE_EXPORT KCModule *create_keyboard_layout(QWidget *parent, const char *)
  {
    return new LayoutConfig(parent, "kcmlayout");
  }

  KDE_EXPORT KCModule *create_keyboard(QWidget *parent, const char *)
  {
    return new KeyboardConfig(parent, "kcmlayout");
  }

  KDE_EXPORT void init_keyboard()
  {
    KConfig *config = new KConfig("kcminputrc", true); // Read-only, no globals
    config->setGroup("Keyboard");

    XKeyboardState   kbd;
    XKeyboardControl kbdc;

    XGetKeyboardControl(QX11Info::display(), &kbd);
    bool key = config->readEntry("KeyboardRepeating", true);
    kbdc.key_click_percent = config->readEntry("ClickVolume", kbd.key_click_percent);
    kbdc.auto_repeat_mode = (key ? AutoRepeatModeOn : AutoRepeatModeOff);

    XChangeKeyboardControl(QX11Info::display(),
                           KBKeyClickPercent | KBAutoRepeatMode,
                           &kbdc);

    if( key ) {
        int delay_ = config->readEntry("RepeatDelay", 250);
        double rate_ = config->readEntry("RepeatRate", 30.0);
        set_repeatrate(delay_, rate_);
    }


    int numlockState = config->readEntry( "NumLock", 2 );
    if( numlockState != 2 )
        numlockx_change_numlock_state( numlockState == 0 );

    delete config;

    config = new KConfig("kxkbrc", true, false);
    config->setGroup("Layout");

// Even if the layouts have been disabled we still want to set Xkb options
// user can always switch them off now in the "Options" tab
    bool enableXkbOptions = config->readEntry("EnableXkbOptions", true);
    if( enableXkbOptions ) {
	bool resetOldOptions = config->readEntry("ResetOldOptions", false);
	QString options = config->readEntry("Options", "");
	if( !XKBExtension::setXkbOptions(options, resetOldOptions) ) {
	    kdDebug() << "Setting XKB options failed!" << endl;
	}
    }

    if ( config->readEntry("Use", true) )
        KToolInvocation::startServiceByDesktopName("kxkb");
    delete config;
  }
}

#if 0// do not remove!
// please don't change/fix messages below
// they're taken from XFree86 as is and should stay the same
   I18N_NOOP("Brazilian ABNT2");
   I18N_NOOP("Dell 101-key PC");
   I18N_NOOP("Everex STEPnote");
   I18N_NOOP("Generic 101-key PC");
   I18N_NOOP("Generic 102-key (Intl) PC");
   I18N_NOOP("Generic 104-key PC");
   I18N_NOOP("Generic 105-key (Intl) PC");
   I18N_NOOP("Japanese 106-key");
   I18N_NOOP("Microsoft Natural");
   I18N_NOOP("Northgate OmniKey 101");
   I18N_NOOP("Keytronic FlexPro");
   I18N_NOOP("Winbook Model XP5");

// These options are from XFree 4.1.0
 I18N_NOOP("Group Shift/Lock behavior");
 I18N_NOOP("R-Alt switches group while pressed");
 I18N_NOOP("Right Alt key changes group");
 I18N_NOOP("Caps Lock key changes group");
 I18N_NOOP("Menu key changes group");
 I18N_NOOP("Both Shift keys together change group");
 I18N_NOOP("Control+Shift changes group");
 I18N_NOOP("Alt+Control changes group");
 I18N_NOOP("Alt+Shift changes group");
 I18N_NOOP("Control Key Position");
 I18N_NOOP("Make CapsLock an additional Control");
 I18N_NOOP("Swap Control and Caps Lock");
 I18N_NOOP("Control key at left of 'A'");
 I18N_NOOP("Control key at bottom left");
 I18N_NOOP("Use keyboard LED to show alternative group");
 I18N_NOOP("Num_Lock LED shows alternative group");
 I18N_NOOP("Caps_Lock LED shows alternative group");
 I18N_NOOP("Scroll_Lock LED shows alternative group");

//these seem to be new in XFree86 4.2.0
 I18N_NOOP("Left Win-key switches group while pressed");
 I18N_NOOP("Right Win-key switches group while pressed");
 I18N_NOOP("Both Win-keys switch group while pressed");
 I18N_NOOP("Left Win-key changes group");
 I18N_NOOP("Right Win-key changes group");
 I18N_NOOP("Third level choosers");
 I18N_NOOP("Press Right Control to choose 3rd level");
 I18N_NOOP("Press Menu key to choose 3rd level");
 I18N_NOOP("Press any of Win-keys to choose 3rd level");
 I18N_NOOP("Press Left Win-key to choose 3rd level");
 I18N_NOOP("Press Right Win-key to choose 3rd level");
 I18N_NOOP("CapsLock key behavior");
 I18N_NOOP("uses internal capitalization. Shift cancels Caps.");
 I18N_NOOP("uses internal capitalization. Shift doesn't cancel Caps.");
 I18N_NOOP("acts as Shift with locking. Shift cancels Caps.");
 I18N_NOOP("acts as Shift with locking. Shift doesn't cancel Caps.");
 I18N_NOOP("Alt/Win key behavior");
 I18N_NOOP("Add the standard behavior to Menu key.");
 I18N_NOOP("Alt and Meta on the Alt keys (default).");
 I18N_NOOP("Meta is mapped to the Win-keys.");
 I18N_NOOP("Meta is mapped to the left Win-key.");
 I18N_NOOP("Super is mapped to the Win-keys (default).");
 I18N_NOOP("Hyper is mapped to the Win-keys.");
 I18N_NOOP("Right Alt is Compose");
 I18N_NOOP("Right Win-key is Compose");
 I18N_NOOP("Menu is Compose");

//these seem to be new in XFree86 4.3.0
 I18N_NOOP( "Both Ctrl keys together change group" );
 I18N_NOOP( "Both Alt keys together change group" );
 I18N_NOOP( "Left Shift key changes group" );
 I18N_NOOP( "Right Shift key changes group" );
 I18N_NOOP( "Right Ctrl key changes group" );
 I18N_NOOP( "Left Alt key changes group" );
 I18N_NOOP( "Left Ctrl key changes group" );
 I18N_NOOP( "Compose Key" );
 
//these seem to be new in XFree86 4.4.0
 I18N_NOOP("Shift with numpad keys works as in MS Windows.");
 I18N_NOOP("Special keys (Ctrl+Alt+<key>) handled in a server.");
 I18N_NOOP("Miscellaneous compatibility options");
 I18N_NOOP("Right Control key works as Right Alt");

//these seem to be in x.org and Debian XFree86 4.3
 I18N_NOOP("Right Alt key switches group while pressed");
 I18N_NOOP("Left Alt key switches group while pressed");
 I18N_NOOP("Press Right Alt-key to choose 3rd level");
#endif
