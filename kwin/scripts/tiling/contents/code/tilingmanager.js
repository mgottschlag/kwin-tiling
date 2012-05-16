/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2012 Mathias Gottschlag <mgottschlag@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

Qt.include("signal.js");
Qt.include("tile.js");
Qt.include("tilelist.js");
Qt.include("layout.js");
Qt.include("tests.js");

function SpiralLayout() {
    // TODO: Just to make this thing compile
}

/**
 * Class which manages all layouts, connects the various signals and handlers
 * and implements all keyboard shortcuts.
 * @class
 */
function TilingManager() {
    /**
     * Default layout type which is selected for new layouts.
     */
    this.defaultLayout = SpiralLayout;
    /**
     * List of all available layout types.
     */
    this.availableLayouts = [
        SpiralLayout/*,
        ZigZagLayout,
        ColumnLayout,
        RowLayout,
        GridLayout,
        MaximizedLayout,
        FloatingLayout*/
    ];
    for (var i = 0; i < this.availableLayouts.length; i++) {
        this.availableLayouts[i].index = i;
    }
    /**
     * Number of desktops in the system.
     */
    this.desktopCount = workspace.desktopGridWidth
                      * workspace.desktopGridHeight;
    /**
     * Number of screens in the system.
     */
    this.screenCount = workspace.numScreens;
    /**
     * Array containing a list of layouts for every desktop. Each of the lists
     * has one element per screen.
     */
    this.layouts = [];
    /**
     * List of all tiles in the system.
     */
    this.tiles = new TileList();

    var self = this;
    // Read the script settings
    // TODO (this is currently not supported by kwin)
    // Create the various layouts, one for every desktop
    for (var i = 0; i < this.desktopCount; i++) {
        this._createDefaultLayouts(i);
    }
    // Connect the tile list signals so that new tiles are added to the layouts
    this.tiles.tileAdded.connect(function(tile) {
        self._onTileAdded(tile);
    });
    this.tiles.tileRemoved.connect(function(tile) {
        self._onTileRemoved(tile);
    });
    // We need to reset custom client properties first because this might not be
    // the first execution of the script
    var existingClients = workspace.clientList();
    existingClients.forEach(function(client) {
        client.tiling_tileIndex = null;
        client.tiling_floating = null;
    });
    // Create the initial list of tiles
    existingClients.forEach(function(client) {
        self.tiles.addClient(client);
    });
    // Register global callbacks
    workspace.numberDesktopsChanged.connect(function() {
        self._onNumberDesktopsChanged();
    });
    workspace.numberScreensChanged.connect(function() {
        self._onNumberScreensChanged();
    });
    workspace.currentDesktopChanged.connect(function() {
        self._onCurrentDesktopChanged();
    });
    // Register keyboard shortcuts
    registerShortcut("Next Tiling Layout",
                     "Next Tiling Layout",
                     "Meta+PgDown",
                     function() {
        var currentLayout = getCurrentLayout();
        var nextIndex = (currentLayout.index + 1) & availableLayouts.length;
        self._switchLayout(workspace.currentDesktop,
                     workspace.activeScreen,
                     nextIndex);
    });
    registerShortcut("Previous Tiling Layout",
                     "Previous Tiling Layout",
                     "Meta+PgUp",
                     function() {
        var currentLayout = getCurrentLayout();
        var nextIndex = currentLayout.index - 1;
        if (nextIndex < 0) {
            nextIndex += availableLayouts.length;
        }
        self._switchLayout(workspace.currentDesktop,
                           workspace.activeScreen,
                           nextIndex);
    });
    registerShortcut("Toggle Floating",
                     "Toggle Floating",
                     "Meta+F",
                     function() {
        if (!workspace.activeClient) {
            return;
        }
        var tile = tiles.getTile(workspace.activeClient);
        if (tile == null) {
            return;
        }
        self.toggleFloating(tile);
    });
    registerShortcut("Switch Focus Left",
                     "Switch Focus Left",
                     "Meta+H",
                     function() {
        this._switchFocus(Direction.Left);
    });
    registerShortcut("Switch Focus Right",
                     "Switch Focus Right",
                     "Meta+L",
                     function() {
        this._switchFocus(Direction.Right);
    });
    registerShortcut("Switch Focus Up",
                     "Switch Focus Up",
                     "Meta+K",
                     function() {
        this._switchFocus(Direction.Up);
    });
    registerShortcut("Switch Focus Down",
                     "Switch Focus Down",
                     "Meta+J",
                     function() {
        this._switchFocus(Direction.Down);
    });
    registerShortcut("Move Window Left",
                     "Move Window Left",
                     "Meta+Shift+H",
                     function() {
        this._moveTile(Direction.Left);
    });
    registerShortcut("Move Window Right",
                     "Move Window Right",
                     "Meta+Shift+L",
                     function() {
        this._moveTile(Direction.Right);
    });
    registerShortcut("Move Window Up",
                     "Move Window Up",
                     "Meta+Shift+K",
                     function() {
        this._moveTile(Direction.Up);
    });
    registerShortcut("Move Window Down",
                     "Move Window Down",
                     "Meta+Shift+J",
                     function() {
        this._moveTile(Direction.Down);
    });
}

/**
 * Utility function which returns the area on the selected screen/desktop which
 * is filled by the layout for that screen.
 *
 * @param desktop Desktop for which the area shall be returned.
 * @param screen Screen for which the area shall be returned.
 * @return Rectangle which contains the area which shall be used by layouts.
 */
TilingManager.getTilingArea = function(desktop, screen) {
    // TODO: Should this function be moved to Layout?
    return workspace.clientArea(KWin.MaximizeArea, screen, desktop);
};

TilingManager.prototype._createDefaultLayouts = function(desktop) {
    var screenLayouts = [];
    for (var j = 0; j < this.screenCount; j++) {
        var area = TilingManager.getTilingArea(desktop, j);
        screenLayouts[j] = new this.defaultLayout(area);
    }
    this.layouts[desktop] = screenLayouts;
};

TilingManager.prototype._onTileAdded = function(tile) {
    // Add global callbacks
    // TODO
    // Add the tile to the layouts
    var client = tile.clients[0];
    var tileLayouts = this._getLayouts(client.desktop, client.screen);
    tileLayouts.forEach(function(layout) {
        // TODO: layout.addTile(tile);
    });
};

TilingManager.prototype._onTileRemoved = function(tile) {
    var client = tile.clients[0];
    var tileLayouts = this._getLayouts(client.desktop, client.screen);
    tileLayouts.forEach(function(layout) {
        // TODO: layout.removeTile(tile);
    });
};

TilingManager.prototype._onNumberDesktopsChanged = function() {
    var newDesktopCount =
            workspace.desktopGridWidth * workspace.desktopGridHeight;
    var onAllDesktops = tiles.tiles.filter(function(tile) {
        return tile.desktop == -1;
    });
    // Remove tiles from desktops which do not exist any more (we only have to
    // care about tiles shown on all desktops as all others have been moved away
    // from the desktops by kwin before)
    for (var i = newDesktopCount; i < this.desktopCount; i++) {
        onAllDesktops.forEach(function(tile) {
           this.layouts[i][tile.screen].removeTile(tile);
        });
    }
    // Add new desktops
    for (var i = this.desktopCount; i < newDesktopCount; i++) {
        this._createDefaultLayouts(i);
        onAllDesktops.forEach(function(tile) {
           this.layouts[i][tile.screen].addTile(tile);
        });
    }
    // Remove deleted desktops
    if (this.desktopCount > newDesktopCount) {
        layouts.length = newDesktopCount;
    }
    this.desktopCount = newDesktopCount;
};

TilingManager.prototype._onNumberScreensChanged = function() {
    // Add new screens
    if (this.screenCount < workspace.numScreens) {
        for (var i = 0; i < this.desktopCount; i++) {
            for (var j = this.screenCount; j < workspace.numScreens; j++) {
                var area = TilingManager.getTilingArea(i, j);
                this.layouts[i][j] = new this.defaultLayout(area);
            }
        }
    }
    // Remove deleted screens
    if (this.screenCount > workspace.numScreens) {
        for (var i = 0; i < this.desktopCount; i++) {
            this.layouts[i].length = workspace.numScreens;
        }
    }
    this.screenCount = workspace.numScreens;
};

TilingManager.prototype._onCurrentDesktopChanged = function() {
    print("TODO: onCurrentDesktopChanged.");
    // TODO
};

TilingManager.prototype._switchLayout = function(desktop, screen, layoutIndex) {
    print("TODO: switchLayout.");
    // TODO
};

TilingManager.prototype._toggleFloating = function(tile) {
    print("TODO: toggleFloating.");
    // TODO
};

TilingManager.prototype._switchFocus = function(direction) {
    print("TODO: switchFocus.");
    // TODO
};

TilingManager.prototype._moveTile = function(direction) {
    print("TODO: moveTile.");
    // TODO
};

TilingManager.prototype._getLayouts = function(desktop, screen) {
    if (desktop > 0) {
        return [this.layouts[desktop - 1][screen]];
    } else if (desktop == 0) {
        return [];
    } else if (desktop == -1) {
        var result = [];
        for (var i = 0; i < this.desktopCount; i++) {
            result.push(this.layouts[i][screen]);
        }
        return result;
    }
}
