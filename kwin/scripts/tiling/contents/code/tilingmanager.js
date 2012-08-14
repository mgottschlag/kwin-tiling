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
Qt.include("tiling.js");
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
    /**
     * Current screen, needed to be able to track screen changes.
     */
    this._currentScreen = workspace.activeScreen;
    /**
     * Current desktop, needed to be able to track screen changes.
     */
    this._currentDesktop = workspace.currentDesktop - 1;
    /**
     * True if a user moving operation is in progress.
     */
    this._moving = false;

    var self = this;
    // Read the script settings
    // TODO (this is currently not supported by kwin)
    // Create the various layouts, one for every desktop
    for (var i = 0; i < this.desktopCount; i++) {
        this._createDefaultLayouts(i);
    }
    this.layouts[this._currentDesktop][this._currentScreen].activate();
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
        var currentLayout = self._getCurrentLayoutType();
        var nextIndex = (currentLayout.index + 1) % self.availableLayouts.length;
        self._switchLayout(workspace.currentDesktop - 1,
                     workspace.activeScreen,
                     nextIndex);
    });
    registerShortcut("Previous Tiling Layout",
                     "Previous Tiling Layout",
                     "Meta+PgUp",
                     function() {
        var currentLayout = self._getCurrentLayoutType();
        var nextIndex = currentLayout.index - 1;
        if (nextIndex < 0) {
            nextIndex += self.availableLayouts.length;
        }
        self._switchLayout(workspace.currentDesktop - 1,
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
        screenLayouts[j] = new Tiling(area, this.defaultLayout);
    }
    this.layouts[desktop] = screenLayouts;
};

TilingManager.prototype._getCurrentLayoutType = function() {
    var currentLayout = this.layouts[this._currentDesktop][this._currentScreen];
    return currentLayout.layoutType;
};

TilingManager.prototype._onTileAdded = function(tile) {
    // Add tile callbacks which are needed to move the tile between different
    // screens/desktops
    var self = this;
    tile.screenChanged.connect(function(oldScreen, newScreen) {
        self._onTileScreenChanged(tile, oldScreen, newScreen);
    });
    tile.desktopChanged.connect(function(oldDesktop, newDesktop) {
        self._onTileDesktopChanged(tile, oldDesktop, newDesktop);
    });
    tile.movingStarted.connect(function() {
        self._onTileMovingStarted(tile);
    });
    tile.movingEnded.connect(function() {
        self._onTileMovingEnded(tile);
    });
    tile.movingStep.connect(function() {
        self._onTileMovingStep(tile);
    });
    // Add the tile to the layouts
    var client = tile.clients[0];
    var tileLayouts = this._getLayouts(client.desktop, client.screen);
    tileLayouts.forEach(function(layout) {
        layout.addTile(tile);
    });
};

TilingManager.prototype._onTileRemoved = function(tile) {
    var client = tile.clients[0];
    var tileLayouts = this._getLayouts(client.desktop, client.screen);
    tileLayouts.forEach(function(layout) {
        layout.removeTile(tile);
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
                this.layouts[i][j] = new Tiling(area, this.defaultLayout);
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

TilingManager.prototype._onTileScreenChanged =
        function(tile, oldScreen, newScreen) {
    // If a tile is moved by the user, screen changes are handled in the move
    // callbacks below
    if (this._moving) {
        return;
    }
    var client = tile.clients[0];
    var oldLayouts = this._getLayouts(client.desktop, oldScreen);
    var newLayouts = this._getLayouts(client.desktop, newScreen);
    this._changeTileLayouts(tile, oldLayouts, newLayouts);
};

TilingManager.prototype._onTileDesktopChanged =
        function(tile, oldDesktop, newDesktop) {
    var client = tile.clients[0];
    var oldLayouts = this._getLayouts(oldDesktop, client.screen);
    var newLayouts = this._getLayouts(newDesktop, client.screen);
    this._changeTileLayouts(tile, oldLayouts, newLayouts);
};

TilingManager.prototype._onTileMovingStarted = function(tile) {
    // TODO
}

TilingManager.prototype._onTileMovingEnded = function(tile) {
    // TODO
}

TilingManager.prototype._onTileMovingStep = function(tile) {
    // TODO
}

TilingManager.prototype._changeTileLayouts =
        function(tile, oldLayouts, newLayouts) {
    oldLayouts.forEach(function(layout) {
        if (newLayouts.indexOf(layout) == -1) {
            layout.removeTile(tile);
        }
    });
    newLayouts.forEach(function(layout) {
        if (oldLayouts.indexOf(layout) == -1) {
            layout.addTile(tile);
        }
    });
};

TilingManager.prototype._onCurrentDesktopChanged = function() {
    print("TODO: onCurrentDesktopChanged.");
    // TODO: We need the same for active screen changes
    this.layouts[this._currentDesktop][this._currentScreen].deactivate();
    this._currentDesktop = workspace.currentDesktop - 1;
    this.layouts[this._currentDesktop][this._currentScreen].activate();
};

TilingManager.prototype._switchLayout = function(desktop, screen, layoutIndex) {
    // TODO: Show the layout switcher dialog
    var layoutType = this.availableLayouts[layoutIndex];
    this.layouts[desktop][screen].setLayoutType(layoutType);
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
