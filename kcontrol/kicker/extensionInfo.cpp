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

#include <kdebug.h>
#include <kdesktopfile.h>
#include <klocale.h>

#include "extensionInfo.h"


ExtensionInfo::ExtensionInfo(const QString& desktopFile,
                             const QString& configFile,
                             const QString& configPath)
    : _configFile(configFile),
      _configPath(configPath),
      _desktopFile(desktopFile)
{
    load();
}

void ExtensionInfo::load()
{
    setDefaults();

    if (_desktopFile.isNull())
    {
        _name = i18n("Main Panel");
        _resizeable = true;
        _useStdSizes = true;
        _customSizeMin = 24;
        _customSizeMax = 256;
        _customSize = 56;
        _showLeftHB     = false;
        _showRightHB    = true;
	for (int i=0;i<4;i++) _allowedPosition[i]=true;
    }
    else
    {
        KDesktopFile df(_desktopFile);
        _name = df.readName();
        _resizeable = df.readEntry("X-KDE-PanelExt-Resizeable", _resizeable);

        if (_resizeable)
        {
            _useStdSizes = df.readEntry("X-KDE-PanelExt-StdSizes", _useStdSizes);
            _size = df.readEntry("X-KDE-PanelExt-StdSizeDefault", _size);
            _customSizeMin = df.readEntry("X-KDE-PanelExt-CustomSizeMin", _customSizeMin);
            _customSizeMax = df.readEntry("X-KDE-PanelExt-CustomSizeMax", _customSizeMax);
            _customSize = df.readEntry("X-KDE-PanelExt-CustomSizeDefault", _customSize);
        }
	QStringList allowedPos=QStringList::split(",",df.readEntry("X-KDE-PanelExt-Positions","Left,Top,Right,Bottom").toUpper());
	for (int i=0;i<4;i++) _allowedPosition[i]=false;
	kDebug()<<"BEFORE X-KDE-PanelExt-Positions parsing"<<endl;
	for (unsigned int i=0;i<allowedPos.count();i++) {
		kDebug()<<allowedPos[i]<<endl;
		if (allowedPos[i]=="LEFT") _allowedPosition[KPanelExtension::Left]=true;
		if (allowedPos[i]=="RIGHT") _allowedPosition[KPanelExtension::Right]=true;
		if (allowedPos[i]=="TOP") _allowedPosition[KPanelExtension::Top]=true;
		if (allowedPos[i]=="BOTTOM") _allowedPosition[KPanelExtension::Bottom]=true;
	}	

    }

    // sanitize
    if (_customSizeMin < 0) _customSizeMin = 0;
    if (_customSizeMax < _customSizeMin) _customSizeMax = _customSizeMin;
    if (_customSize < _customSizeMin) _customSize = _customSizeMin;

    KConfig c(_configFile);
    c.setGroup("General");

    _position       = c.readEntry ("Position",            _position);
    _alignment      = c.readEntry ("Alignment",           _alignment);
    _xineramaScreen = c.readEntry ("XineramaScreen",      _xineramaScreen);
    _showLeftHB     = c.readEntry ("ShowLeftHideButton",  _showLeftHB);
    _showRightHB    = c.readEntry ("ShowRightHideButton", _showRightHB);
    _hideButtonSize = c.readEntry ("HideButtonSize",      _hideButtonSize);
    _autohidePanel  = c.readEntry ("AutoHidePanel",       _autohidePanel);
    _backgroundHide = c.readEntry ("BackgroundHide",      _backgroundHide);
    _autoHideSwitch = c.readEntry ("AutoHideSwitch",      _autoHideSwitch);
    _autoHideDelay  = c.readEntry ("AutoHideDelay",       _autoHideDelay);
    _hideAnim       = c.readEntry ("HideAnimation",       _hideAnim);
    _hideAnimSpeed  = c.readEntry ("HideAnimationSpeed",  _hideAnimSpeed);
    _unhideLocation = c.readEntry ("UnhideLocation",      _unhideLocation);
    _sizePercentage = c.readEntry ("SizePercentage",      _sizePercentage);
    _expandSize     = c.readEntry ("ExpandSize",          _expandSize);

    if (_resizeable)
    {
        _size           = c.readEntry ("Size",       _size);
        _customSize     = c.readEntry ("CustomSize", _customSize);
    }

    _orig_position = _position;
    _orig_alignment = _alignment;
    _orig_size = _size;
    _orig_customSize = _customSize;

    // sanitize
    if (_sizePercentage < 1) _sizePercentage = 1;
    if (_sizePercentage > 100) _sizePercentage = 100;
}

void ExtensionInfo::configChanged()
{
    KConfig c(_configFile);
    c.setGroup("General");

    // check to see if the new value is different from both
    // the original value and the currently set value, then it
    // must be a newly set value, external to the panel!
    int position  = c.readEntry ("Position",  _position);
    if (position != _position && position != _orig_position)
    {
        _orig_position = _position = position;
    }

    int alignment = c.readEntry ("Alignment", _alignment);
    if (alignment != _alignment && alignment != _orig_alignment)
    {
        _orig_alignment = _alignment = alignment;
    }

    if (_resizeable)
    {
        int size = c.readEntry ("Size", _size);
        if (size != _size && size != _orig_size)
        {
            _orig_size = _size = size;
        }

        int customSize = c.readEntry ("CustomSize", _customSize);
        if (customSize != _customSize && customSize != _orig_customSize)
        {
            _orig_customSize = _customSize = customSize;
        }

    }
}

void ExtensionInfo::setDefaults()
{
    // defaults
    _position       = 3;
    _alignment      = QApplication::isRightToLeft() ? 2 : 0;
    _xineramaScreen = QApplication::desktop()->primaryScreen();
    _size           = 2;
    _showLeftHB     = false;
    _showRightHB    = true;
    _hideButtonSize = 14;
    _autohidePanel  = false;
    _backgroundHide = false;
    _autoHideSwitch = false;
    _autoHideDelay  = 3;
    _hideAnim       = true;
    _hideAnimSpeed  = 40;
    _unhideLocation = 0;
    _sizePercentage = 100;
    _expandSize     = true;
    _customSize     = 0;
    _resizeable     = false;
    _useStdSizes    = false;
    _customSizeMin  = 0;
    _customSizeMax  = 0;
}

void ExtensionInfo::save()
{
    KConfig c(_configFile);
    c.setGroup("General");

    c.writeEntry("Position",            _position);
    c.writeEntry("Alignment",           _alignment);
    c.writeEntry("XineramaScreen",      _xineramaScreen);
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

    // FIXME: this is set only for the main panel and only in the
    // look 'n feel (aka appearance) tab. so we can't save it here
    // this should be implemented properly. - AJS
    //c.writeEntry("HideButtonSize",      _hideButtonSize);

    if (_resizeable)
    {
        c.writeEntry("Size",       _size);
        c.writeEntry("CustomSize", _customSize);
    }

    _orig_position = _position;
    _orig_alignment = _alignment;
    _orig_size = _size;
    _orig_customSize = _customSize;

    c.sync();
}
