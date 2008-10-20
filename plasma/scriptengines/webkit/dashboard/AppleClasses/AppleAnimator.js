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

function AppleAnimator(duration, interval, start, finish, handler)
{
    //properties specified by Apple
    this.duration = duration;
    this.interval = interval;
    this.animations = [];
    this.timer = 0;
    this.oncomplete; //no documentation exists for this.

    //internal members:
    this.intervalPointer = undefined;

    var that = this;

    if (arguments.length === 5) {
        var animation = new AppleAnimation(start, finish, handler);
        that.addAnimation(animation); //I hope I can use this already
    }
};

AppleAnimator.prototype.addAnimation = function (animation) {
    //I'm unsure what the behaviour of this function should be if
    //AppleAnimator.start() has already been called.
    animation.initialize(this.duration, this.interval);
    this.animations.push(animation);
};

AppleAnimator.prototype.start = function ()
{
    if (this.intervalPointer == undefined) {

        var that = this; //for the closure (may not be necessary?)
        function intervalHandler() {
            //setInterval doesn't pass 'this', so we need a closure instead.
            that.timer += that.interval;
            if (that.timer > that.duration) {
                that.stop();
            } else {
                for (var i=0; i<that.animations.length; i++) {
                    var animation = that.animations[i];
                    animation.step();
                    animation.callback(animation, animation.now,
                            animation.from, animation.to);
                }
            }
        }

        this.intervalPointer = setInterval(intervalHandler, this.interval);
    }
};

AppleAnimator.prototype.stop = function ()
{
    if (this.intervalPointer != undefined) {
        clearInterval(this.intervalPointer);
        delete this.intervalPointer;
        //Maybe this.timer should be set to 0, and animations re-initialized
    }
};

function AppleAnimation(start, finish, handler)
{
    this.from = start;
    this.to = finish;
    this.now = undefined;
    this.callback = handler;

    //internal members:
    //prototype.initialize
    //this.step
};

AppleAnimation.prototype.initialize = function(duration, interval)
{
    this.now = this.from;

    var frac = interval / duration;
    var diff = this.to - this.from; //Oh memory/speed trade-off (premature optimization?)

    this.step = function () {
        //This is a closure, and don't forget it.
        this.now += diff * frac;
    }
};

function AppleRect(left, top, right, bottom)
{
    this.left = left;
    this.top = top;
    this.right = right;
    this.bottom = bottom;
};

function AppleRectAnimation(start, finish, handler)
{
    this.from = start;
    this.to = finish;
    this.now = undefined;
    this.callback = handler;
};

AppleRectAnimation.prototype.initialize = function(duration, interval)
{
    this.now = this.from;

    var frac = interval / duration;
    var diff = {
            left: this.to.left - this.from.left,
            top: this.to.top - this.from.top,
            right: this.to.right - this.from.right,
            bottom: this.to.bottom - this.from.bottom
    }

    this.step = function () {
        //This is a closure, and don't forget it.
        this.now = new AppleRect(this.now.left + frac * diff.left,
                                 this.now.top + frac * diff.top,
                                 this.now.right + frac * diff.right,
                                 this.now.bottom + frac * diff.right)
    }
};

