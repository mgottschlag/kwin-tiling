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
 * Class which manages the windows in one tile and handles resize/move and
 * property change events.
 * @class
 */
function Tile(firstClient, tileIndex) {
    /**
     * Signal which is triggered whenever the user starts to move the tile.
     */
    this.movingStarted = new Signal();
    /**
     * Signal which is triggered whenever the user stops moving the tile.
     */
    this.movingEnded = new Signal();
    /**
     * Signal which is triggered whenever the geometry changes between
     * movingStarted and movingEnded.
     */
    this.movingStep = new Signal();
    /**
     * Signal which is triggered whenever the user starts to resize the tile.
     */
    this.resizingStarted = new Signal();
    /**
     * Signal which is triggered whenever the user stops resizing the tile.
     */
    this.resizingEnded = new Signal();
    /**
     * Signal which is triggered whenever the geometry changes between
     * resizingStarted and resizingEnded.
     */
    this.resizingStep = new Signal();
    /**
     * Signal which is triggered when the geometry of the tile changes because
     * of something different to a user move or resize action.
     */
    this.geometryChanged = new Signal();
    /**
     * Signal which is triggered whenever the tile forced floating state
     * changes. Two parameters are passed to the handlers, the old and the new
     * forced floating state.
     */
    this.forcedFloatingChanged = new Signal();
    /**
     * Signal which is triggered whenever the tile is moved to a different
     * screen. Two parameters are passed to the handlers, the old and the new
     * screen.
     */
    this.screenChanged = new Signal();
    /**
     * Signal which is triggered whenever the tile is moved to a different
     * desktop. Two parameters are passed to the handlers, the old and the new
     * desktop.
     */
    this.desktopChanged = new Signal();
    /**
     * List of the clients in this tile.
     */
    this.clients = [firstClient];
    /**
     * Index of this tile in the TileList to which the tile belongs.
     */
    this.tileIndex = tileIndex;
    /**
     * True if this tile has been marked as floating by the user.
     */
    this.floating = false;
    /**
     * True if this tile has to be floating because of client properties.
     */
    this.forcedFloating = false;

    // Check whether this tile has to be floating
    // TODO

    this.syncCustomProperties();
}

/**
 * Sets the geometry of the tile. geometryChanged events caused by this function
 * are suppressed.
 *
 * @param geometry New tile geometry.
 */
Tile.prototype.setGeometry = function(geometry) {
    // TODO
};

/**
 * Saves the current geometry so that it can later be restored using
 * restoreGeometry().
 */
Tile.prototype.saveGeometry = function() {
    // TODO
};

/**
 * Restores the previously saved geometry.
 */
Tile.prototype.restoreGeometry = function() {
    // TODO
};

/**
 * Returns the currently active client in the tile.
 */
Tile.prototype.getClient = function() {
    // TODO
};

/**
 * Synchronizes all custom properties (tileIndex, floating between all clients
 * in the tile).
 */
Tile.prototype.syncCustomProperties = function() {
    this.clients[0].tiling_tileIndex = this.tileIndex;
    this.clients[0].tiling_floating = this.floating;
    this.clients[0].syncTabGroupFor("tiling_tileIndex", true);
    this.clients[0].syncTabGroupFor("tiling_floating", true);
};
