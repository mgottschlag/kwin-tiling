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
     * Layout which specifies window sizes/positions.
     */
    this.layout = new layoutType(screenRectangle);

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
    // TODO
}

Tiling.prototype.removeTile = function(tile) {
    // TODO
}

Tiling.prototype.swapTiles = function(tile1, tile2) {
    // TODO
}

Tiling.prototype.activate = function() {
    // Resize the tiles like specified by the layout
    this._updateAllTiles();
    // If no tile geometry was specified, just restore the saved geometry
    // TODO
    // Register callbacks for all tiles
    // TODO
}

Tiling.prototype.deactivate = function() {
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
    // TODO: Set the position/size of all tiles
}
