/*
 *  Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>
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
 */

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qlistview.h>
#include <qfileinfo.h>
#include <qradiobutton.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kdebug.h>

#include "extensionstab_impl.h"
#include "extensionstab_impl.moc"


extern int kickerconfig_screen_number;


ExtensionsTab::ExtensionsTab( QWidget *parent, const char* name )
  : ExtensionsTabBase (parent, name)
{
    m_extensionsListView->clear();

    connect(m_locationGroup, SIGNAL(clicked(int)), SLOT(locationChanged()));
    connect(m_alignGroup, SIGNAL(clicked(int)), SLOT(slotChanged()));
    connect(m_manual,SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_automatic,SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_background,SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_autoHideSwitch, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(m_delaySpinBox, SIGNAL(valueChanged(int)), SLOT(slotChanged()));
    connect(m_lHB, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(m_rHB, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(m_hideButtonSlider, SIGNAL(valueChanged(int)), SLOT(slotChanged()));
    connect(m_none, SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_top, SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_topRight, SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_right, SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_bottomRight, SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_bottomLeft, SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_left, SIGNAL(toggled(bool)), SLOT(slotChanged()));
    connect(m_topLeft, SIGNAL(toggled(bool)), SLOT(slotChanged()));

    connect(m_extensionsListView, SIGNAL(selectionChanged(QListViewItem*)), SLOT(loadConfig(QListViewItem*)));

    // whats this help
    QWhatsThis::add(m_locationGroup, i18n("This sets the position of the panel extension"
                                          " i.e. the screen border it is attached to. You can also change this"
                                          " position by left-clicking on some free space on the panel extension and"
                                          " dragging it to a screen border."));

    QWhatsThis::add(m_alignGroup, i18n("This setting determines how the panel is aligned, i.e."
                                      " how it's positioned on the panel edge."
                                      " Note that in order for this setting to have any effect,"
                                      " the panel size has to be set to a value of less than 100%"));

    QWhatsThis::add(m_manual, i18n(
        "If this option is selected, the only way to hide the panel extension "
        "will be via the hide buttons at its side."));
    QWhatsThis::add(m_automatic, i18n(
        "If this option is selected, the panel extension will automatically hide "
        "after some time and reappear when you move the mouse to the "
        "screen edge where the panel extension is hidden. "
        "This is particularly useful for small screen resolutions, "
        "for example, on laptops."));
    QWhatsThis::add(m_background, i18n(
        "If this option is selected, the panel extension will allow itself to "
        "be covered by other windows. Raise the panel extension to the top by "
        "moving the mouse to the screen edge specified by the unhide location "
        "below."));
    QWhatsThis::add(m_autoHideSwitch, i18n("If this option is enabled, the panel extension will automatically show "
					   "itself for a brief period of time when the desktop is switched "
					   "so you can see which desktop you are on.") );

    QString delaystr = i18n("Here you can change the delay after which the panel extension will disappear"
                            " if not used.");

    QWhatsThis::add(m_hideButtonSlider, i18n("Here you can change the size of the hide buttons."));

    QWhatsThis::add(m_delaySpinBox, delaystr);

    QWhatsThis::add(m_backgroundGroup, i18n(
        "Here you can set the location on the screen's edge that will "
        "bring the panel to front."));
    load();
}

void ExtensionsTab::load()
{
    m_extensionsListView->clear();

    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    QStringList elist = c->readListEntry("Extensions2");
    QStringList::Iterator it = elist.begin();
    for ( ; it != elist.end(); it++ )
    {
        // extension id
        QString extensionId(*it);
        QString group = extensionId;

        // is there a config group for this extension?
        if(!c->hasGroup(group))
            continue;

        // create a matching applet container
        if (!extensionId.contains("Extension") > 0)
            continue;

        // set config group
        c->setGroup(group);

        QString df = KGlobal::dirs()->findResource("extensions", c->readEntry("DesktopFile"));
        QString cf = c->readEntry("ConfigFile");

        (void) new ExtensionInfo( df, cf, m_extensionsListView );
    }

    QListViewItem* item = m_extensionsListView->firstChild();
    if( item ) {
        m_extensionsListView->setSelected( item, true );
    }

    loadConfig( m_extensionsListView->selectedItem() );

    delete c;
}

void ExtensionsTab::loadConfig( QListViewItem* item )
{
    ExtensionInfo* info = (ExtensionInfo*)item;

    m_mainFrame->setEnabled( info != 0 );

    if( info ) {
	m_locationGroup->setButton(info->_position);
       m_alignGroup->setButton(info->_alignment);

       // if the panel is horizontal...
       if (m_locationGroup->id(m_locationGroup->selected()) > 1) {
           m_alignLeftTop->setText(i18n("Le&ft"));
           m_alignRightBottom->setText(i18n("R&ight"));
       } else {
           m_alignLeftTop->setText(i18n("T&op"));
           m_alignRightBottom->setText(i18n("Bottom"));
       }
       m_modeGroup->setButton(info->_hideMode);

       m_autoHideSwitch->setChecked(info->_autoHideSwitch);

	// disconnect so that slotChanged doesn't get called here
	disconnect(m_delaySpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));

       m_delaySpinBox->setValue(info->_autoHideDelay);

	// reconnect
	connect(m_delaySpinBox, SIGNAL(valueChanged(int)), SLOT(slotChanged()));
       m_automaticGroup->setEnabled(m_automatic->isChecked());

//	int sizepercentage = info->_sizePercentage;
//	m_percentSlider->setValue( sizepercentage );
//	m_percentSpinBox->setValue( sizepercentage );

//	m_expandCheckBox->setChecked( info->_expandSize );

//	m_showToolTips->setChecked( info->_showToolTips );

//	bool hideanim = info->_hideAnim;
//	bool autohideanim = info->_autoHideAnim;

//	m_manualHideSlider->setValue( info->_hideAnimSpeed );
//	m_autoHideSlider->setValue( info->_autoHideAnimSpeed );

//	m_manualHideSlider->setEnabled(hideanim);
//	m_autoHideSlider->setEnabled(autohideanim);

//	m_manualHideAnimation->setChecked(hideanim);
//	m_autoHideAnimation->setChecked(autohideanim);

	bool showLHB = info->_showLeftHB;
	bool showRHB = info->_showRightHB;

	m_lHB->setChecked( showLHB );
	m_rHB->setChecked( showRHB );

	m_hideButtonSlider->setValue( info->_HBwidth );
	m_hideButtonSlider->setEnabled(showLHB || showRHB);

       m_backgroundGroup->setButton( info->_unhideLocation );
       m_backgroundGroup->setEnabled(m_background->isChecked());
    }
}

void ExtensionsTab::locationChanged()
{
    // if the panel is horizontal...
    if ( m_locationGroup->id(m_locationGroup->selected()) > 1) {
       m_alignLeftTop->setText(i18n("Le&ft"));
       m_alignRightBottom->setText(i18n("R&ight"));
    } else {
       m_alignLeftTop->setText(i18n("T&op"));
       m_alignRightBottom->setText(i18n("Bottom"));
    }

    slotChanged();
}

void ExtensionsTab::slotChanged()
{
    m_hideButtonSlider->setEnabled( m_lHB->isChecked() || m_rHB->isChecked() );

    ExtensionInfo* info = (ExtensionInfo*)m_extensionsListView->selectedItem();

    if( info ) {
	info->_position          = m_locationGroup->id(m_locationGroup->selected());
       info->_alignment         = m_alignGroup->id(m_alignGroup->selected());
	info->_HBwidth           = m_hideButtonSlider->value();
	info->_showLeftHB        = m_lHB->isChecked();
	info->_showRightHB       = m_rHB->isChecked();
       info->_hideMode          = m_modeGroup->id(m_modeGroup->selected());
	info->_autoHideSwitch    = m_autoHideSwitch->isChecked();
       info->_autoHideDelay     = m_delaySpinBox->value();
       info->_unhideLocation    = m_backgroundGroup->id(m_backgroundGroup->selected());
//	info->_hideAnim          = m_manualHideAnimation->isChecked();
//	info->_autoHideAnim      = m_autoHideAnimation->isChecked();
//	info->_hideAnimSpeed     = m_manualHideSlider->value();
//	info->_autoHideAnimSpeed = m_autoHideSlider->value();
//	info->_showToolTips      = m_showToolTips->isChecked();
//	info->_sizePercentage    = m_percentSlider->value();
//	info->_expandSize        = m_expandCheckBox->isChecked();
    }

    emit changed();
}

void ExtensionsTab::save()
{
    ExtensionInfo* info = (ExtensionInfo*)m_extensionsListView->firstChild();
    for( ; info; info = (ExtensionInfo*)info->nextSibling() ) {
	info->save();
    }
}

void ExtensionsTab::defaults()
{
    ExtensionInfo* info = (ExtensionInfo*)m_extensionsListView->selectedItem();

    if( info ) {
	info->setDefaults();
	loadConfig( info );
   }
}

ExtensionInfo::ExtensionInfo( const QString& desktopFile, const QString& configFile, QListView* parent )
  : QListViewItem( parent )
{
    KDesktopFile df(desktopFile);

    setText( 0, df.readName() );
    _configFile = configFile;

    KConfig *c = new KConfig(_configFile);
    c->setGroup("General");

    setDefaults();

    _position          = c->readNumEntry(  "Position",               _position);
    _alignment         = c->readNumEntry(  "Alignment",              _alignment);
    _HBwidth           = c->readNumEntry(  "HideButtonSize",         _HBwidth);
    _showLeftHB        = c->readBoolEntry( "ShowLeftHideButton",     _showLeftHB);
    _showRightHB       = c->readBoolEntry( "ShowRightHideButton",    _showRightHB);
    // For backwards compatibility, we don't just store the enum.
    if( c->readBoolEntry( "AutoHidePanel", _hideMode == 1 ) ) {
       _hideMode = 1;
    } else if( c->readBoolEntry( "BackgroundHide", _hideMode == 2 )) {
       _hideMode = 2;
    } else {
       _hideMode = 0;
    }
    _autoHideSwitch    = c->readBoolEntry( "AutoHideSwitch",         _autoHideSwitch);
    _autoHideDelay     = c->readNumEntry(  "AutoHideDelay",          _autoHideDelay);
    _hideAnim          = c->readBoolEntry( "HideAnimation",          _hideAnim);
    _autoHideAnim      = c->readBoolEntry( "AutoHideAnimation",      _autoHideAnim);
    _hideAnimSpeed     = c->readNumEntry(  "HideAnimationSpeed",     _hideAnimSpeed);
    _autoHideAnimSpeed = c->readNumEntry(  "AutoHideAnimationSpeed", _autoHideAnimSpeed);
    _unhideLocation    = c->readNumEntry(  "UnhideLocation",         _unhideLocation);
    _showToolTips      = c->readBoolEntry( "ShowToolTips",           _showToolTips );
    _sizePercentage    = c->readNumEntry(  "SizePercentage",         _sizePercentage );
    _expandSize        = c->readBoolEntry( "ExpandSize",             _expandSize );

    // sanitize
    if (_HBwidth < 3) _HBwidth = 3;
    if (_HBwidth > 24) _HBwidth = 24;

    if ( _sizePercentage < 1 ) _sizePercentage = 1;
    if ( _sizePercentage > 100 ) _sizePercentage = 100;

    delete c;
}

void ExtensionInfo::setDefaults()
{
    // defaults
    _position          = 3;
    _alignment         = QApplication::reverseLayout() ? 2 : 0;
    _HBwidth           = 14;
    _showLeftHB        = true;
    _showRightHB       = false;
    _hideMode          = 0;
    _autoHideSwitch    = false;
    _autoHideDelay     = 3;
    _hideAnim          = true;
    _autoHideAnim      = true;
    _hideAnimSpeed     = 40;
    _autoHideAnimSpeed = 40;
    _unhideLocation    = 6;
    _showToolTips      = true;
    _sizePercentage    = 1;
    _expandSize        = true;
}

void ExtensionInfo::save()
{
    KConfig *c = new KConfig(_configFile);
    c->setGroup("General");

    c->writeEntry( "Position", _position);
    c->writeEntry( "Alignment", _alignment);
    c->writeEntry( "HideButtonSize",         _HBwidth);
    c->writeEntry( "ShowLeftHideButton",     _showLeftHB);
    c->writeEntry( "ShowRightHideButton",    _showRightHB);
    c->writeEntry( "AutoHidePanel",          _hideMode == 1 );
    c->writeEntry( "BackgroundHide",         _hideMode == 2 );
    c->writeEntry( "AutoHideSwitch",         _autoHideSwitch);
    c->writeEntry( "AutoHideDelay",          _autoHideDelay);
    c->writeEntry( "HideAnimation",          _hideAnim);
    c->writeEntry( "AutoHideAnimation",      _autoHideAnim);
    c->writeEntry( "HideAnimationSpeed",     _hideAnimSpeed);
    c->writeEntry( "AutoHideAnimationSpeed", _autoHideAnimSpeed);
    c->writeEntry( "UnhideLocation",         _unhideLocation);
    c->writeEntry( "ShowToolTips",           _showToolTips );
    c->writeEntry( "SizePercentage",         _sizePercentage );
    c->writeEntry( "ExpandSize",             _expandSize );

    delete c;
}
