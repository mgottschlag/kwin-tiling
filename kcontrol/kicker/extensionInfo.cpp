/*
 *  Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>
 *  Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
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

#include <qapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <klocale.h>
 
#include "extensionInfo.h"


extensionInfo::extensionInfo(const QString& desktopFile, const QString& configFile)
    : _configFile(configFile),
      _desktopFile(desktopFile)
{
    load();
}

void extensionInfo::load()
{
    setDefaults();

    if (_desktopFile == QString::null)
    {
        _name = i18n("Main Panel");
        _resizeable = true;
        _useStdSizes = true;
        _customSizeMin = 24;
        _customSizeMax = 128;
        _customSize = 58;
    }
    else
    {
        KDesktopFile df(_desktopFile);
        _name = df.readName();
        _resizeable = df.readBoolEntry("X-KDE-PanelExt-Resizeable", _resizeable);
        
        if (_resizeable)
        {
            _useStdSizes = df.readBoolEntry("X-KDE-PanelExt-StdSizes", _useStdSizes); 
            _size = df.readNumEntry("X-KDE-PanelExt-StdSizeDefault", _size);
            _customSizeMin = df.readNumEntry("X-KDE-PanelExt-CustomSizeMin", _customSizeMin);
            _customSizeMax = df.readNumEntry("X-KDE-PanelExt-CustomSizeMax", _customSizeMax);
            _customSize = df.readNumEntry("X-KDE-PanelExt-CustomSizeDefault", _customSize);
        }
    }

    // sanitize
    if (_customSizeMin < 0) _customSizeMin = 0;
    if (_customSizeMax < _customSizeMin) _customSizeMax = _customSizeMin;
    if (_customSize < _customSizeMin) _customSize = _customSizeMin;

    KConfig c(_configFile);
    c.setGroup("General");

    _position       = c.readNumEntry ("Position",            _position);
    _alignment      = c.readNumEntry ("Alignment",           _alignment);
    _showLeftHB     = c.readBoolEntry("ShowLeftHideButton",  _showLeftHB);
    _showRightHB    = c.readBoolEntry("ShowRightHideButton", _showRightHB);
    _autohidePanel  = c.readBoolEntry("AutoHidePanel",       _autohidePanel);
    _backgroundHide = c.readBoolEntry("BackgroundHide",      _backgroundHide);
    _autoHideSwitch = c.readBoolEntry("AutoHideSwitch",      _autoHideSwitch);
    _autoHideDelay  = c.readNumEntry ("AutoHideDelay",       _autoHideDelay);
    _hideAnim       = c.readBoolEntry("HideAnimation",       _hideAnim);
    _hideAnimSpeed  = c.readNumEntry ("HideAnimationSpeed",  _hideAnimSpeed);
    _unhideLocation = c.readNumEntry ("UnhideLocation",      _unhideLocation);
    _sizePercentage = c.readNumEntry ("SizePercentage",      _sizePercentage);
    _expandSize     = c.readBoolEntry("ExpandSize",          _expandSize);

    if (_resizeable)
    {
        _size           = c.readNumEntry ("Size",       _size);
        _customSize     = c.readNumEntry ("CustomSize", _customSize);
    }
    
    // sanitize
    if (_sizePercentage < 1) _sizePercentage = 1;
    if (_sizePercentage > 100) _sizePercentage = 100;
}

void extensionInfo::setDefaults()
{
    // defaults
    _position       = 3;
    _alignment      = QApplication::reverseLayout() ? 2 : 0;
    _size           = 2;
    _showLeftHB     = true;
    _showRightHB    = false;
    _autohidePanel  = false;
    _backgroundHide = false;
    _autoHideSwitch = false;
    _autoHideDelay  = 3;
    _hideAnim       = true;
    _hideAnimSpeed  = 40;
    _unhideLocation = 6;
    _sizePercentage = 1;
    _expandSize     = true;
    _customSize     = 0;
    _resizeable     = false;
    _useStdSizes    = false;
    _customSizeMin  = 0;
    _customSizeMax  = 0;
}

void extensionInfo::save()
{
    KConfig c(_configFile);
    c.setGroup("General");

    c.writeEntry("Position",            _position);
    c.writeEntry("Alignment",           _alignment);
    c.writeEntry("ShowLeftHideButton",  _showLeftHB);
    c.writeEntry("ShowRightHideButton", _showRightHB);
    c.writeEntry("AutoHidePanel",       _autohidePanel);
    c.writeEntry("BackgroundHide",      _backgroundHide);
    c.writeEntry("AutoHideSwitch",      _autoHideSwitch);
    c.writeEntry("AutoHideDelay",       _autoHideDelay);
    c.writeEntry("HideAnimation",       _hideAnim);
    c.writeEntry("HideAnimationSpeed",  _hideAnimSpeed);
    c.writeEntry("UnhideLocation",      _unhideLocation);
    c.writeEntry("SizePercentage",      _sizePercentage );
    c.writeEntry("ExpandSize",          _expandSize );
    
    if (_resizeable)
    {
        c.writeEntry("Size",       _size);
        c.writeEntry("CustomSize", _customSize);
    }
    
    c.sync();
}

extensionInfoItem::extensionInfoItem(extensionInfo* info, QListView* parent, QListViewItem* after)
    : QListViewItem(parent, after),
      m_info(info)
{
    if (info)
    {
        setText(0, info->_name);
    }
}

extensionInfo* extensionInfoItem::info()
{
   return m_info;
}

