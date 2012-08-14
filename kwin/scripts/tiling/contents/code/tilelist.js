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
    if (TileList._isIgnored(client)) {
        return;
    }
    var self = this;
    client.tabGroupChanged.connect(function() {
        self._onClientTabGroupChanged(client);
    });
    // We also have to connect other client signals here instead of in Tile
    // because the tile of a client might change over time
    var getTile = function(client) {
        return self.tiles[client.tiling_tileIndex];
    };
    client.shadeChanged.connect(function() {
        getTile(client).onClientShadeChanged(client);
    });
    client.geometryChanged.connect(function() {
        getTile(client).onClientGeometryChanged(client);
    });
    client.keepAboveChanged.connect(function() {
        getTile(client).onClientKeepAboveChanged(client);
    });
    client.keepBelowChanged.connect(function() {
        getTile(client).onClientKeepBelowChanged(client);
    });
    client.fullScreenChanged.connect(function() {
        getTile(client).onClientFullScreenChanged(client);
    });
    client.minimizedChanged.connect(function() {
        getTile(client).onClientMinimizedChanged(client);
    });
    client['clientMaximizedStateChanged(KWin::Client*,bool,bool)'].connect(
            function(client, h, v) {
        getTile(client).onClientMaximizedStateChanged(client, h, v);
    });
    client.desktopChanged.connect(function() {
        getTile(client).onDesktopChanged(client);
    });
   // Check whether the client is part of an existing tile
    var tileIndex = client.tiling_tileIndex;
    if (tileIndex >= 0 && tileIndex < tiles.length) {
        this.tiles[tileIndex].clients.push(client);
    } else {
        // If not, create a new tile
        this._addTile(client);
    }
};

/**
 * Returns the tile in which a certain client is located.
 *
 * @param client Client for which the tile shall be returned.
 * @return Tile in which the client is located.
 */
TileList.prototype.getTile = function(client) {
    var tileIndex = client.tiling_tileIndex;
    if (tileIndex >= 0 && tileIndex < tiles.length) {
        return this.tiles[tileIndex];
    } else {
        return null;
    }
};

TileList.prototype._onClientAdded = function(client) {
    this._identifyNewTiles();
    this.addClient(client);
};

TileList.prototype._onClientRemoved = function(client) {
    var tileIndex = client.tiling_tileIndex;
    if (!(tileIndex >= 0 && tileIndex < this.tiles.length)) {
        return;
    }
    // Remove the client from its tile
    var tile = this.tiles[tileIndex];
    if (tile.clients.length == 1) {
        // Remove the tile if this was the last client in it
        this._removeTile(tileIndex);
    } else {
        // Remove the client from its tile
        tile.clients.splice(tile.clients.indexOf(client), 1);
    }
};

TileList.prototype._onClientTabGroupChanged = function(client) {
    var tileIndex = client.tiling_tileIndex;
    var tile = this.tiles[tileIndex];
    if (tile.clients.length == 1) {
        // If this is the only client in the tile, the tile either does not
        // change or is destroyed
        this.tiles.forEach(function(otherTile) {
            if (otherTile != tile) {
                otherTile.syncCustomProperties();
            }
        });
        if (client.tiling_tileIndex != tileIndex) {
            this._removeTile(tileIndex);
            this.tiles[client.tiling_tileIndex].clients.push(client);
        }
    } else {
        tile.clients.splice(tile.clients.indexOf(client), 1);
        client.tiling_tileIndex = this.tiles.length;
        // Check whether the client has been added to an existing tile
        this._identifyNewTiles();
        if (client.tiling_tileIndex != this.tiles.length) {
            this.tiles[client.tiling_tileIndex].clients.push(client);
        } else {
            this._addTile(client);
        }
    }
};

TileList.prototype._addTile = function(client) {
    var newTile = new Tile(client, this.tiles.length)
    this.tiles.push(newTile);
    this.tileAdded.emit(newTile);
};

TileList.prototype._removeTile = function(tileIndex) {
    // Remove the tile if this was the last client in it
    this.tileRemoved.emit(this.tiles[tileIndex]);
    this.tiles[tileIndex] = this.tiles[this.tiles.length - 1];
    this.tiles.length--;
    this.tiles[tileIndex].tileIndex = tileIndex;
    this.tiles[tileIndex].syncCustomProperties();
};

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
    this.tiles.forEach(function(tile) {
        tile.syncCustomProperties();
    });
};

/**
 * Returns false for clients which shall not be handled by the tiling script at
 * all, e.g. the panel.
 */
TileList._isIgnored = function(client) {
    // NOTE: Application workarounds should be put here
    return client.specialWindow;
};
