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
 * Class which arranges the windows in a spiral with the largest window filling
 * the left half of the screen.
 */
function SpiralLayout(screenRectangle) {
    Layout.call(this, screenRectangle);
    // TODO
}

SpiralLayout.name = "Spiral";
// TODO: Add an image for the layout switcher
SpiralLayout.image = null;

SpiralLayout.prototype = new Layout();
SpiralLayout.prototype.constructor = SpiralLayout;

SpiralLayout.prototype.onLayoutAreaChange = function(oldArea, newArea) {
    // TODO: Scale all tiles
}

SpiralLayout.prototype.resetTileSizes = function() {
    // Simply erase all tiles and recreate them to recompute the initial sizes
    var tileCount = this.tiles.length;
    this.tiles.length = 0;
    for (var i = 0; i < tileCount; i++) {
        addTile();
    }
}

SpiralLayout.prototype.addTile = function() {
    print("SpiralLayout: addTile");
    if (this.tiles.length == 0) {
        // The first tile fills the whole screen
        var rect = Qt.rect(this.screenRectangle.x,
                           this.screenRectangle.y,
                           this.screenRectangle.width,
                           this.screenRectangle.height);
        this._createTile(rect);
    } else {
        // Divide the last tile into two halves
        var lastRect = this.tiles[this.tiles.length - 1].rectangle;
        var newRect = Qt.rect(lastRect.x,
                              lastRect.y,
                              lastRect.width,
                              lastRect.height);
        var direction = this.tiles.length % 4;
        var splitX = Math.floor(lastRect.width / 2);
        var splitY = Math.floor(lastRect.height / 2);
        switch (direction) {
        case 0:
            lastRect.y = lastRect.y + splitY;
            lastRect.height = lastRect.height - splitY;
            newRect.height = splitY;
            break;
        case 1:
            lastRect.width = splitX;
            newRect.x = newRect.x + splitX;
            newRect.width = newRect.width - splitX;
            break;
        case 2:
            lastRect.height = splitY;
            newRect.y = newRect.y + splitY;
            newRect.height = newRect.height - splitY;
            break;
        case 3:
            lastRect.x = lastRect.x + splitX;
            lastRect.width = lastRect.width - splitX;
            newRect.width = splitX;
            break;
        }
        this._createTile(newRect);
    }
    var lastRect = this.tiles[this.tiles.length - 1].rectangle;
}

SpiralLayout.prototype.removeTile = function(tileIndex) {
    // Increase the size of the last tile
    if (this.tiles.length > 1) {
        var tileCount = this.tiles.length - 1;
        var rects = [
            this.tiles[tileCount - 1].rectangle,
            this.tiles[tileCount].rectangle
        ];
        var left = Math.min(rects[0].x, rects[1].x);
        var top = Math.min(rects[0].y, rects[1].y);
        var right = Math.max(rects[0].x + rects[0].width,
                             rects[1].x + rects[1].width);
        var bottom = Math.max(rects[0].y + rects[0].height,
                              rects[1].y + rects[1].height);
        var lastRect = Qt.rect(left, top, right - left, bottom - top);
        this.tiles[tileCount - 1].rectangle = lastRect;
    }
    // Remove the last array entry
    this.tiles.length--;
    // Fix the neighbour information
    if (this.tiles.length > 0) {
        this.tiles[0].neighbours[Direction.Up] = this.tiles.length - 1;
        var lastTile = this.tiles[this.tiles.length - 1];
        lastTile.neighbours[Direction.Down] = 0;
        lastTile.hasDirectNeighbour[Direction.Down] = false;
    }
}

SpiralLayout.prototype.resizeTile = function(tileIndex, rectangle) {
    // TODO
}

SpiralLayout.prototype._createTile = function(rect) {
    // Create a new tile and add it to the list
    var tile = {};
    tile.rectangle = rect;
    tile.neighbours = [];
    tile.hasDirectNeighbour = [];
    tile.neighbours[Direction.Left] = this.tiles.length;
    tile.hasDirectNeighbour[Direction.Left] = false;
    tile.neighbours[Direction.Right] = this.tiles.length;
    tile.hasDirectNeighbour[Direction.Right] = false;
    tile.neighbours[Direction.Up] = this.tiles.length - 1;
    tile.hasDirectNeighbour[Direction.Up] = true;
    tile.neighbours[Direction.Down] = 0;
    tile.hasDirectNeighbour[Direction.Down] = false;
    this.tiles.push(tile);
    // Update the first tile
    this.tiles[0].neighbours[Direction.Up] = this.tiles.length - 1;
    this.tiles[0].hasDirectNeighbour[Direction.Up] = false;
}
