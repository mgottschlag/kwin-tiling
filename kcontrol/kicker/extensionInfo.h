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

#ifndef __PANELINFO_H
#define __PANELINFO_H

#include <qvaluelist.h>
#include <qlistview.h>
#include <kpanelextension.h>

class ExtensionInfo;

typedef QValueList<ExtensionInfo*> ExtensionInfoList;

class ExtensionInfo
{
    public:
        ExtensionInfo(const QString& destopFile,
                      const QString& configFile,
                      const QString& configPath);
        ~ExtensionInfo() {};

        void setDefaults();
        void save();
        void load();
        void configChanged();

        QString _configFile;
        QString _configPath;
        QString _desktopFile;

        // Configuration settings
        QString  _name;
        int      _position;
        int      _alignment;
        int      _xineramaScreen;
        int      _size;
        int      _customSize;
        bool     _showLeftHB;
        bool     _showRightHB;
        int      _hideButtonSize;
        bool     _autohidePanel;
        bool     _backgroundHide;
        bool     _autoHideSwitch;
        int      _autoHideDelay;
        bool     _hideAnim;
        int      _hideAnimSpeed;
        int      _unhideLocation;
        int      _sizePercentage;
        bool     _expandSize;

        // Original settings to ensure that we can figure out
        // what has changed externally to the panel vs within the panel
        int      _orig_position;
        int      _orig_alignment;
        int      _orig_size;
        int      _orig_customSize;

        // Size info
        bool    _resizeable;
        bool    _useStdSizes;
        int     _customSizeMin;
        int     _customSizeMax;

	// position handling
	bool	_allowedPosition[4];
};

#endif

