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
 * Class which implements tiling for a single screen.
 * @class
 */
function Tiling(screenRectangle, layoutType) {
    /**
     * Tiles which have been added to the layout
     */
    this.tiles = [];
    /**
     * Layout type which provided the current layout.
     */
    this.layoutType = layoutType;
    /**
     * Layout which specifies window sizes/positions.
     */
    this.layout = new layoutType(screenRectangle);
    /**
     * True if the layout is active.
     */
    this.active = false;

    // TODO
}

Tiling.prototype.setLayoutType = function(layoutType) {
    // TODO
}

Tiling.prototype.setLayoutArea = function(area) {
    this.layout.setLayoutArea(area);
    this._updateAllTiles();
}

Tiling.prototype.addTile = function(tile) {
    this.layout.addTile();
    this.tiles.push(tile);
    // TODO: Set "below all" state
    if (this.active) {
        this._updateAllTiles();
        // TODO: Register tile callbacks
    }
}

Tiling.prototype.removeTile = function(tile) {
    var tileIndex = this.tiles.indexOf(tile);
    this.tiles.splice(tileIndex, 1);
    this.layout.removeTile(tileIndex);
    if (this.active) {
        // TODO: Unregister tile callbacks
        this._updateAllTiles();
    }
}

Tiling.prototype.swapTiles = function(tile1, tile2) {
    // TODO
}

Tiling.prototype.activate = function() {
    this.active = true;
    // Resize the tiles like specified by the layout
    this._updateAllTiles();
    // If no tile geometry was specified, just restore the saved geometry
    // TODO
    // Register callbacks for all tiles
    // TODO
}

Tiling.prototype.deactivate = function() {
    this.active = false;
    // Unregister callbacks for all tiles
    // TODO
}

/**
 * Resets tile sizes to their initial size (in case they were resized by the
 * user).
 */
Tiling.prototype.resetTileSizes = function() {
    this.layout.resetTileSizes();
    this._updateAllTiles();
}

Tiling.prototype.getTile = function(x, y) {
    // TODO
}

Tiling.prototype.getTiles = function() {
    // TODO
}

Tiling.prototype._updateAllTiles = function() {
    // Set the position/size of all tiles
    for (var i = 0; i < this.layout.tiles.length; i++) {
        var currentRect = this.tiles[i].clients[0].geometry;
        var newRect = this.layout.tiles[i].rectangle;
        if (currentRect.x != newRect.x
                || currentRect.y != newRect.y
                || currentRect.width != newRect.width
                || currentRect.height != newRect.height) {
            this.tiles[i].setGeometry(newRect);
        }
    }
}
