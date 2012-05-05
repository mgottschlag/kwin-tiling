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
Qt.include("tilelist.js");
Qt.include("tests.js");

function SpiralLayout() {
    // TODO: Just to make this thing compile
}

/**
 * Class which manages all layouts, connects the various signals and handlers
 * and implements all keyboard shortcuts.
 * @class
 */
function Tiling() {
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
    this.tiles.tileChanged.connect(function(tile) {
        self._onTileChanged(tile);
    });
    // Create the initial list of tiles
    var existingClients = workspace.clientList();
    var addClient = function(client) {
        self.tiles.addClient(client);
        // Register client callbacks which are not handled by TileList
        // TODO
    }
    existingClients.forEach(addClient);
    // Register global callbacks
    workspace.clientAdded.connect(addClient);
    workspace.clientRemoved.connect(function(client) {
        self.tiles.removeClient(client);
    });
    // TODO
    // Register keyboard shortcuts
    // TODO
}

/**
 * Utility function which returns the area on the selected screen/desktop which
 * is filled by the layout for that screen.
 *
 * @param desktop Desktop for which the area shall be returned.
 * @param screen Screen for which the area shall be returned.
 * @return Rectangle which contains the area which shall be used by layouts.
 */
Tiling.getTilingArea = function(desktop, screen) {
    // TODO: Should this function be moved to Layout?
    return workspace.clientArea(KWin.MaximizeArea, screen, desktop);
};

Tiling.prototype._createDefaultLayouts = function(desktop) {
    var screenLayouts = [];
    for (var j = 0; j < this.screenCount; j++) {
        var area = Tiling.getTilingArea(desktop, j);
        screenLayouts[j] = new this.defaultLayout(area);
    }
    this.layouts[desktop] = screenLayouts;
};

Tiling.prototype._onTileAdded = function(tile) {
    // TODO
};

Tiling.prototype._onTileRemoved = function(tile) {
    // TODO
};

Tiling.prototype._onTileChanged = function(tile) {
    // TODO
};
