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

/**
 * Class which keeps track of all tiles in the system. The class automatically
 * puts tab groups in one single tile. Tracking of new and removed clients is
 * done here as well.
 * @class
 */
function TileList() {
    /**
     * List of currently existing tiles.
     */
    this.tiles = [];
    /**
     * Signal which is triggered whenever a new tile is added to the list.
     */
    this.tileAdded = new Signal();
    /**
     * Signal which is triggered whenever a tile is removed from the list.
     */
    this.tileRemoved = new Signal();
    /**
     * Signal which is triggered every time the windows in a tile change.
     */
    this.tileChanged = new Signal();

    // We connect to the global workspace callbacks which are triggered when
    // clients are added/removed in order to be able to keep track of the
    // new/deleted tiles
    var self = this;
    workspace.clientAdded.connect(function(client) {
        self._onClientAdded(client);
    });
    workspace.clientRemoved.connect(function(client) {
        self._onClientRemoved(client);
    });
}

/**
 * Adds another client to the tile list. When this is called, the tile list also
 * adds callback functions to the relevant client signals to trigger tile change
 * events when necessary. This function might trigger a tileAdded event.
 *
 * @param client Client which is added to the tile list.
 */
TileList.prototype.addClient = function(client) {
    // Check whether the client is part of an existing tile
    var tileIndex = client.tiling_tileIndex;
    if (tileIndex) {
        // TODO
    } else {
        // If not, create a new tile
        // TODO
    }
};

/**
 * Returns the tile in which a certain client is located.
 *
 * @param client Client for which the tile shall be returned.
 * @return Tile in which the client is located.
 */
TileList.prototype.getTile = function(client) {
    // TODO
};

/**
 * TODO: What was this supposed to do?
 */
TileList.prototype.updateTabGroups = function(client) {
    // TODO
}

TileList.prototype._onClientAdded = function(client) {
    if (TileList.isIgnored(client)) {
        return;
    }
    this._identifyNewTiles();
    this.addClient(client);
}

TileList.prototype._onClientRemoved = function(client) {
    var tileIndex = client.tiling_tileIndex;
    if (!tileIndex) {
        return;
    }
    // TODO
}

TileList.prototype._onClientTabGroupChanged = function(client) {
    // TODO
}

/**
 * Updates the tile index on all clients in all existing tiles by synchronizing
 * the tiling_tileIndex property of the group. Clients which do not belong to
 * any existing tile will have this property set to null afterwards, while
 * clients which belong to a tile have the correct tile index.
 *
 * This can only detect clients which are not in any tile, it does not detect
 * client tab group changes! These shall be handled by removing the client from
 * any tile in _onClientTabGroupChanged() first.
 */
TileList.prototype._identifyNewTiles = function() {
    for (var i = 0; i < this.tiles.length; i++) {
        var firstClient = this.tiles[i].clients[0];
        firstClient.tiling_tileIndex = i;
        firstClient.syncTabGroupFor("tiling_tileIndex", true);
        firstClient.syncTabGroupFor("tiling_floating", true);
    });
}

/**
 * Returns false for clients which shall not be handled by the tiling script at
 * all, e.g. the panel.
 */
TileList._isIgnored = function(client) {
    // TODO
    return false;
}
