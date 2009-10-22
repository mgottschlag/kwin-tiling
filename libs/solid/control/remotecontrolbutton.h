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

#ifndef REMOTECONTROLBUTTON_H
#define REMOTECONTROLBUTTON_H

#include <QString>
#include <QSharedDataPointer>

#include "solid_control_export.h"

namespace Solid
{
namespace Control
{
     
class RemoteControlButtonPrivate;

class SOLIDCONTROL_EXPORT RemoteControlButton
{
public:
    enum ButtonId {
        Unknown = -1,
        // Numbers
        Number0,
        Number1,
        Number2,
        Number3,
        Number4,
        Number5,
        Number6,
        Number7,
        Number8,
        Number9,

        // Media control
        Play,
        Pause,
        PlayPause,
        Stop,
        Forward,
        Backward,
        FastForward,
        Rewind,
        ChannelDown,
        ChannelUp,
        VolumeDown,
        VolumeUp,
        Mute,
        Info,
        Eject,
        Power,
        
        // Navigation
        Up,
        Down,
        Left,
        Right,
        Select,
        Back,
        Menu,
        
        // Jump points
        Aux,
        CD,
        DVD,
        EPG,
        Favorites,
        Help,
        Home,
        Music,
        Text,
        TV,
        
        // Colors
        Blue,
        Green,
        Red,
        Yellow
    };

    /**
    * Creates a new RemoteControlButton object.
    * The buttons name and description are set according to the id.
    *
    * @param remoteName the name of the RemoteControl this button comes from
    * @param id the ID of the button
    * @param repeatCounter the repeat counter. Increase this if a button is held pressed down. Reset it to zero if the button is released
    */
    RemoteControlButton(const QString &remoteName, ButtonId id, int repeatCounter = 0);

    /**
    * Creates a new RemoteControlButton object.
    * If name is a known buttonname, the id will be set accordingly. Else the ID will be Unknown.
    *
    * @param remoteName the name of the RemoteControl this button comes from
    * @param name the name of the button
    * @param repeatCounter the repeat counter. Increase this if a button is held pressed down. Reset it to zero if the button is released
    */
    RemoteControlButton(const QString &remoteName, const QString &name, int repeatCounter = 0);

	/**
	* Copies a RemoteControlButton
	*/
	RemoteControlButton(const RemoteControlButton &other);
	
    /**
    * Destroys a RemoteControlButton object.
    */
    ~RemoteControlButton();

    /**
    * Retrieves the name of the RemoteControl this button comes from.
    *
    * @returns Returns the name of the RemoteControl this button comes from
    */
    QString remoteName() const;

    /**
    * Retrieves the ID of the Button.
    * If the ID is ButtonID::Unknown name() will still contain the name of the button.
    *
    * @returns Returns the the ID of the button
    */
    ButtonId id() const;

    /**
    * Retrieves the name of the Button. If the button ID is a valid ButtonID ( not ButtonID::Unknown ) this will
    * contain a the buttons name string. If the id() is Unknown this name contains the button name
    * defined by the backend. Use this if you need to store buttons in config files or compare buttons with ButtonID::Unknown
    *
    * @returns Returns the name of the button
    */
    QString name() const;

    /**
    * Retrieves the description of the Button.
	* If the button ID is a valid ButtonID ( not ButtonID::Unknown ) this will contain a human 
	* readable, nice formatted and translated string. If the id() is Unknown this name contains the button name
    * defined by the backend. Use this to display button names to to the user.
    *
    * @returns Returns the name of the button
    */
    QString description() const;

    /**
    * Retrieves the repeat count for the buttonpress. If this is greater 0 the button is held down. This can be
    * useful to prevent accidental navigation through menus while still being able to navigate fast through long playlists.
    *
    * @returns Returns the the ID of the Button
    */
    int repeatCounter() const;

private:
	QSharedDataPointer<RemoteControlButtonPrivate> d;
};

class RemoteControlButtonPrivate: public QSharedData
{
    public:
        RemoteControlButtonPrivate() {
            id = RemoteControlButton::Unknown;
            remoteName.clear();
            name.clear();
            repeatCounter = -1;
        };
        
        RemoteControlButtonPrivate(const RemoteControlButtonPrivate &other) : QSharedData(other)
        , remoteName(other.remoteName), id(other.id), name(other.name), repeatCounter(other.repeatCounter) {};
        
        QString remoteName;
        Solid::Control::RemoteControlButton::ButtonId id;
        QString name;
        int repeatCounter;
};

} // Control
} // Solid
#endif // REMOTECONTROLBUTTON_H
