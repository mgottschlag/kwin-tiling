/*
 * Copyright 2008 Stefan Buller <hikingpete@cain.afraid.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

function AppleScrollArea(content) {
	for (var i = 0; i < arguments.length; i++) {
		this.addScrollbar(arguments[i]);
	}
	var that=this;
	var handler=function(e) {
		switch(e.which) {
			case 37: //left
				that.horizontalScrollTo(that.content.scrollLeft -
						that.singlepressScrollPixels);
				break;
			case 38: //up
				that.verticalScrollTo(that.content.scrollTop -
						that.singlepressScrollPixels);
				break;
			case 39: //right
				that.horizontalScrollTo(that.content.scrollLeft +
						that.singlepressScrollPixels);
				break;
			case 40: //down
				that.verticalScrollTo(that.content.scrollTop +
						that.singlepressScrollPixels);
				break;
			default:
				//the event may contiue to propagate
				return true;
		}
		//arrest propagation
		return false;
	}
	content.addEventListener('keydown',handler,false);

	//Apple mandated properties that must be reacted to.
	this.scrollsVertically = false;
	this.scrollsHorizontally = false;
	this.singlepressScrollPixels = 10; //Somebody change this. Please.

	//Apple mandated properties that are `read only'.
	this.viewHeight = content.clientHeight;
	this.viewToContentHeightRatio = content.clientHeight / content.scrollHeight;
	this.viewWidth = content.clientWidth;
	this.viewToContentWidthRatio = content.clientWidth / content.scrollWidth;
	//I'm worried that the scrollHeight/Width could change on me. If that turns
	//out to be a problem, then getters would be the way to go.

	//extras
	this.scrollbars = [];
	this.content = content;
}

AppleScrollArea.prototype.addScrollbar = function(scrollbar) {
	this.scrollbars.push(scrollbar);
	scrollbar.setScrollArea(this);
}

AppleScrollArea.prototype.removeScrollbar = function(scrollbar) {
	this.scrollbars.filter(function(element){return (element === scrollbar);});
	scrollbar.setScrollArea(null); //Just a guess. This might not be right.
}

AppleScrollArea.prototype.remove = function() {
	//Remove the div, or remove the effects of AppleScrollArea?
	//Perhaps this can all be replaced with a simple removeChild()
	content.scrollTop = 0;
	content.scrollLeft = 0;
	for (var i = 0; i < this.scrollbars.length; i++) {
		this.scrollbars[i].remove();
		delete this.scrollbars[i];
	}
	delete this;
}

AppleScrollArea.prototype.reveal = function(element) {
	//First we find it
	var distX = 0;
	var distY = 0;
	var el = element;
	while (el !== this.content) {
		distX += (+el.offsetTop);
		distY += (+el.offsetLeft);
		el = el.parentNode;
		if (el == null) {
			throw "Target element not in ScrollArea.";
		}
	}

	this.verticalScrollTo(distY);
	this.horizontalScrollTo(distX);
}

AppleScrollArea.prototype.focus = function() {
	this.content.focus();
}

AppleScrollArea.prototype.blur = function() {
	this.content.blur();
}

AppleScrollArea.prototype.verticalScrollTo = function(position) {
	this.scrollTop = position;
}

AppleScrollArea.prototype.horizontalScrollTo = function(position) {
	this.scrollLeft = position;
}

 
