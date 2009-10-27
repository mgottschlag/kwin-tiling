/*
    Copyright (C) <2009>  Michael Zanetti <michael_zanetti@gmx.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "remotecontrolbutton.h"

#include <klocale.h>


Solid::Control::RemoteControlButton::RemoteControlButton(const QString &remoteName, ButtonId id, int repeatCounter){
	d = new RemoteControlButtonPrivate;
    d->remoteName = remoteName;
    d->id = id;
    d->repeatCounter = repeatCounter;
    switch(id){
        case Number0:
            d->name = "0";
            break;
        case Number1:
            d->name = "1";
            break;
        case Number2:
            d->name = "2";
            break;
        case Number3:
            d->name = "3";
            break;
        case Number4:
            d->name = "4";
            break;
        case Number5:
            d->name = "5";
            break;
        case Number6:
            d->name = "6";
            break;
        case Number7:
            d->name = "7";
            break;
        case Number8:
            d->name = "8";
            break;
        case Number9:
            d->name = "9";
            break;
            
        // Media control
        case Play:
            d->name = "Play";
            break;
        case Pause:
            d->name = "Pause";
            break;
        case PlayPause:
            d->name = "PlayPause";
            break;
        case Stop:
            d->name = "Stop";
            break;
        case Forward:
            d->name = "SkipForward";
            break;
        case Backward:
            d->name = "SkipBackward";
            break;
        case FastForward:
            d->name = "FastForward";
            break;
        case Rewind:
            d->name = "Rewind";
            break;
        case ChannelDown:
            d->name = "ChannelDown";
            break;
        case ChannelUp:
            d->name = "ChannelUp";
            break;
        case VolumeDown:
            d->name = "VolumeDown";
            break;
        case VolumeUp:
            d->name = "VolumeUp";
            break;
        case Mute:
            d->name = "Mute";
            break;
        case Info:
            d->name = "Info";
            break;
        case Eject:
            d->name = "Eject";
            break;
        case Power:
            d->name = "Power";
            break;
            
        // Navigation
        case Up:
            d->name = "Up";
            break;
        case Down:
            d->name = "Down";
            break;
        case Left:
            d->name = "Left";
            break;
        case Right:
            d->name = "Right";
            break;
        case Select:
            d->name = "Select";
            break;
        case Back:
            d->name = "Back";
            break;
        case Menu:
            d->name = "Menu";
            break;
            
        // Jump points
        case Aux:
            d->name = "Aux";
            break;
        case CD:
            d->name = "CD";
            break;
        case DVD:
            d->name = "DVD";
            break;
        case EPG:
            d->name = "EPG";
            break;
        case Favorites:
            d->name = "Favorites";
            break;
        case Help:
            d->name = "Help";
            break;
        case Home:
            d->name = "Home";
            break;
        case Music:
            d->name = "Music";
            break;
        case Text:
            d->name = "Text";
            break;
        case TV:
            d->name = "TV";
            break;
            
        // Colors
        case Blue:
            d->name = "Blue";
            break;
        case Green:
            d->name = "Green";
            break;
        case Red:
            d->name = "Red";
            break;
        case Yellow:
            d->name = "Yellow";
            break;
        default:
            d->name = "Unknown";
    }
}

Solid::Control::RemoteControlButton::RemoteControlButton(const QString &remoteName, const QString &name, int repeatCounter){
	d = new RemoteControlButtonPrivate;
    d->remoteName = remoteName;
    d->name = name;
    d->repeatCounter = repeatCounter;
    if (name == "0")
        d->id = Number0;
    else if (name == "1")
        d->id = Number1;
    else if (name == "2")
        d->id = Number2;
    else if (name == "3")
        d->id = Number3;
    else if (name == "4")
        d->id = Number4;
    else if (name == "5")
        d->id = Number5;
    else if (name == "6")
        d->id = Number6;
    else if (name == "7")
        d->id = Number7;
    else if (name == "8")
        d->id = Number8;
    else if (name == "9")
        d->id = Number9;

    // Media control
    else if (name == "Play")
        d->id = Play;
    else if (name == "Pause")
        d->id = Pause;
    else if (name == "PlayPause")
        d->id = PlayPause;
    else if (name == "Stop")
        d->id = Stop;
    else if (name == "SkipForward")
        d->id = Forward;
    else if (name == "SkipBackward")
        d->id = Backward;
    else if (name == "FastForward")
        d->id = FastForward;
    else if (name == "Rewind")
        d->id = Rewind;
    else if (name == "ChannelDown")
        d->id = ChannelDown;
    else if (name == "ChannelUp")
        d->id = ChannelUp;
    else if (name == "VolumeDown")
        d->id = VolumeDown;
    else if (name == "VolumeUp")
        d->id = VolumeUp;
    else if (name == "Mute")
        d->id = Mute;
    else if (name == "Info")
        d->id = Info;
    else if (name == "Eject")
        d->id = Eject;
    else if (name == "Power")
        d->id = Power;

    // Navigation
    else if (name == "Up")
        d->id = Up;
    else if (name == "Down")
        d->id = Down;
    else if (name == "Left")
        d->id = Left;
    else if (name == "Right")
        d->id = Right;
    else if (name == "Select")
        d->id = Select;
    else if (name == "Back")
        d->id = Back;
    else if (name == "Menu")
        d->id = Menu;

    // Jump points
    else if (name == "Aux")
        d->id = Aux;
    else if (name == "CD")
        d->id = CD;
    else if (name == "DVD")
        d->id = DVD;
    else if (name == "EPG")
        d->id = EPG;
    else if (name == "Favorites")
        d->id = Favorites;
    else if (name == "Help")
        d->id = Help;
    else if (name == "Home")
        d->id = Home;
    else if (name == "Music")
        d->id = Music;
    else if (name == "Text")
        d->id = Text;
    else if (name == "TV")
        d->id = TV;

    // Colors
    else if (name == "Blue")
        d->id = Blue;
    else if (name == "Green")
        d->id = Green;
    else if (name == "Red")
        d->id = Red;
    else if (name == "Yellow")
        d->id = Yellow;
    else
        d->id = Unknown;
}
Solid::Control::RemoteControlButton::RemoteControlButton(const RemoteControlButton &other): d(other.d){

}

Solid::Control::RemoteControlButton::~RemoteControlButton()
{

}

QString Solid::Control::RemoteControlButton::remoteName() const
{
    return d->remoteName;
}

Solid::Control::RemoteControlButton::ButtonId Solid::Control::RemoteControlButton::id() const
{
    return d->id;
}


QString Solid::Control::RemoteControlButton::name() const
{
    return d->name;
}

QString Solid::Control::RemoteControlButton::description() const
{
    switch(d->id){
        case Unknown:
            return d->name;
            
        // Numbers
        case Number0:
            return i18nc("A button on a Remote Control", "0");
        case Number1:
            return i18nc("A button on a Remote Control", "1");
        case Number2:
            return i18nc("A button on a Remote Control", "2");
        case Number3:
            return i18nc("A button on a Remote Control", "3");
        case Number4:
            return i18nc("A button on a Remote Control", "4");
        case Number5:
            return i18nc("A button on a Remote Control", "5");
        case Number6:
            return i18nc("A button on a Remote Control", "6");
        case Number7:
            return i18nc("A button on a Remote Control", "7");
        case Number8:
            return i18nc("A button on a Remote Control", "8");
        case Number9:
            return i18nc("A button on a Remote Control", "9");
            
            // Media control
        case Play:
            return i18nc("A button on a Remote Control", "Play");
        case Pause:
            return i18nc("A button on a Remote Control", "Pause");
        case PlayPause:
            return i18nc("A button on a Remote Control", "Play Pause");
        case Stop:
            return i18nc("A button on a Remote Control", "Stop");
        case Forward:
            return i18nc("A button on a Remote Control", "Skip Forward");
        case Backward:
            return i18nc("A button on a Remote Control", "Skip Backward");
        case FastForward:
            return i18nc("A button on a Remote Control", "Fast Forward");
        case Rewind:
            return i18nc("A button on a Remote Control", "Rewind");
        case ChannelDown:
            return i18nc("A button on a Remote Control", "Channel Down");
        case ChannelUp:
            return i18nc("A button on a Remote Control", "Channel Up");
        case VolumeDown:
            return i18nc("A button on a Remote Control", "Volume Down");
        case VolumeUp:
            return i18nc("A button on a Remote Control", "Volume Up");
        case Mute:
            return i18nc("A button on a Remote Control", "Mute");
        case Info:
            return i18nc("A button on a Remote Control", "Info");
        case Eject:
            return i18nc("A button on a Remote Control", "Eject");
        case Power:
            return i18nc("A button on a Remote Control", "Power");
            
            // Navigation
        case Up:
            return i18nc("A button on a Remote Control", "Up");
        case Down:
            return i18nc("A button on a Remote Control", "Down");
        case Left:
            return i18nc("A button on a Remote Control", "Left");
        case Right:
            return i18nc("A button on a Remote Control", "Right");
        case Select:
            return i18nc("A button on a Remote Control", "Select");
        case Back:
            return i18nc("A button on a Remote Control", "Back");
        case Menu:
            return i18nc("A button on a Remote Control", "Menu");
            
            // Jump points
        case Aux:
            return i18nc("A button on a Remote Control", "Aux");
        case CD:
            return i18nc("A button on a Remote Control", "CD");
        case DVD:
            return i18nc("A button on a Remote Control", "DVD");
        case EPG:
            return i18nc("A button on a Remote Control", "EPG");
        case Favorites:
            return i18nc("A button on a Remote Control", "Favorites");
        case Help:
            return i18nc("A button on a Remote Control", "Help");
        case Home:
            return i18nc("A button on a Remote Control", "Home");
        case Music:
            return i18nc("A button on a Remote Control", "Music");
        case Text:
            return i18nc("A button on a Remote Control", "Text");
        case TV:
            return i18nc("A button on a Remote Control", "TV");
            
            // Colors
        case Blue:
            return i18nc("A button on a Remote Control", "Blue");
        case Green:
            return i18nc("A button on a Remote Control", "Green");
        case Red:
            return i18nc("A button on a Remote Control", "Red");
        case Yellow:
            return i18nc("A button on a Remote Control", "Yellow");
        default:
            return i18nc("A button on a Remote Control", "Unknown");
    }
}

int Solid::Control::RemoteControlButton::repeatCounter() const
{
    return d->repeatCounter;
}
