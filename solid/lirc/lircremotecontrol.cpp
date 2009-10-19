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

#include "lircremotecontrol.h"
#include "lircclient.h"

#include <kdebug.h>

using namespace Solid::Control;

class LircRemoteControlPrivate
{
public:
    LircRemoteControlPrivate(const QString &name);

    QString name;
    LircClient *m_client;


};

LircRemoteControlPrivate::LircRemoteControlPrivate(const QString &n)
        : name(n)
{
    m_client = LircClient::self();
}

LircRemoteControl::LircRemoteControl(const QString &name)
        : RemoteControl(), d(new LircRemoteControlPrivate(name))
{
    connect(d->m_client, SIGNAL(commandReceived(const QString &, const QString &, int)), this, SLOT(commandReceived(const QString &, const QString &, int)));
}

LircRemoteControl::~LircRemoteControl()
{
    kDebug() << "deleting remote" << d->name;
    delete d;
}

QString LircRemoteControl::name() const
{
    return d->name;
}

QList<RemoteControlButton> LircRemoteControl::buttons() const
{
    QList<RemoteControlButton> retList;
    foreach(const QString &buttonName, d->m_client->buttons(d->name)){
        if(lircButtonToSolid(buttonName) != RemoteControlButton::Unknown){
            retList.append(RemoteControlButton(d->name, lircButtonToSolid(buttonName)));
        } else {
            retList.append(RemoteControlButton(d->name, formatNamespaceButton(buttonName)));
        }
    }
    return retList;
}

void LircRemoteControl::commandReceived(const QString &remote, const QString &button, int repeatCounter){
    if(remote == d->name){
        if(lircButtonToSolid(button) != RemoteControlButton::Unknown){
            emit buttonPressed(RemoteControlButton(remote, lircButtonToSolid(button), repeatCounter));
        } else {
            emit buttonPressed(RemoteControlButton(remote, formatNamespaceButton(button), repeatCounter));
        }
    }
}
    
Solid::Control::RemoteControlButton::ButtonId LircRemoteControl::lircButtonToSolid(const QString &buttonName) const{
    // Numbers
    if(buttonName == "KEY_0"){
        return RemoteControlButton::Number0;
    } else if(buttonName == "KEY_1"){
        return RemoteControlButton::Number1;
    } else if(buttonName == "KEY_2"){
        return RemoteControlButton::Number2;
    } else if(buttonName == "KEY_3"){
        return RemoteControlButton::Number3;
    } else if(buttonName == "KEY_4"){
        return RemoteControlButton::Number4;
    } else if(buttonName == "KEY_5"){
        return RemoteControlButton::Number5;
    } else if(buttonName == "KEY_6"){
        return RemoteControlButton::Number6;
    } else if(buttonName == "KEY_7"){
        return RemoteControlButton::Number7;
    } else if(buttonName == "KEY_8"){
        return RemoteControlButton::Number8;
    } else if(buttonName == "KEY_9"){
        return RemoteControlButton::Number9;
        
    // Media control
    } else if(buttonName == "KEY_PLAY"){
        return RemoteControlButton::Play;
    } else if(buttonName == "KEY_PAUSE"){
        return RemoteControlButton::Pause;
    } else if(buttonName == "KEY_PLAYPAUSE"){
        return RemoteControlButton::PlayPause;
    } else if(buttonName == "KEY_STOP"){
        return RemoteControlButton::Stop;
    } else if(buttonName == "KEY_FORWARD"){
        return RemoteControlButton::Forward;
    } else if(buttonName == "KEY_BACK"){
        return RemoteControlButton::Backward;
    } else if(buttonName == "KEY_FASTFORWARD"){
        return RemoteControlButton::FastForward;
    } else if(buttonName == "KEY_REWIND"){
        return RemoteControlButton::Rewind;
    } else if(buttonName == "KEY_CHANNELDOWN"){
        return RemoteControlButton::ChannelDown;
    } else if(buttonName == "KEY_CHANNELUP"){
        return RemoteControlButton::ChannelUp;
    } else if(buttonName == "KEY_VOLUMEDOWN"){
        return RemoteControlButton::VolumeDown;
    } else if(buttonName == "KEY_VOLUMEUP"){
        return RemoteControlButton::VolumeUp;
    } else if(buttonName == "KEY_MUTE"){
        return RemoteControlButton::Mute;
    } else if(buttonName == "KEY_INFO"){
        return RemoteControlButton::Info;
    } else if(buttonName == "KEY_EJECTCD"){
        return RemoteControlButton::Eject;
    } else if(buttonName == "KEY_POWER"){
        return RemoteControlButton::Power;
        
    // Navigation
    } else if(buttonName == "KEY_UP"){
        return RemoteControlButton::Up;
    } else if(buttonName == "KEY_DOWN"){
        return RemoteControlButton::Down;
    } else if(buttonName == "KEY_LEFT"){
        return RemoteControlButton::Left;
    } else if(buttonName == "KEY_RIGHT"){
        return RemoteControlButton::Right;
    } else if(buttonName == "KEY_SELECT"){
        return RemoteControlButton::Select;
    } else if(buttonName == "KEY_BACK"){
        return RemoteControlButton::Back;
    } else if(buttonName == "KEY_MENU"){
        return RemoteControlButton::Menu;
        
    // Jump points
    } else if(buttonName == "KEY_AUX"){
        return RemoteControlButton::Aux;
    } else if(buttonName == "KEY_CD"){
        return RemoteControlButton::CD;
    } else if(buttonName == "KEY_DVD"){
        return RemoteControlButton::DVD;
    } else if(buttonName == "KEY_EPG"){
        return RemoteControlButton::EPG;
    } else if(buttonName == "KEY_FAVORITES"){
        return RemoteControlButton::Favorites;
    } else if(buttonName == "KEY_HELP"){
        return RemoteControlButton::Help;
    } else if(buttonName == "KEY_HOME"){
        return RemoteControlButton::Home;
// Not defined in LIRC (yet)
/*        } else if(buttonName == "KEY_MUSIC"){
        return RemoteControlButton::Music;*/
    } else if(buttonName == "KEY_TEXT"){
        return RemoteControlButton::Text;
    } else if(buttonName == "KEY_TV"){
        return RemoteControlButton::TV;
        
    // Colors
    } else if(buttonName == "KEY_BLUE"){
        return RemoteControlButton::Blue;
    } else if(buttonName == "KEY_GREEN"){
        return RemoteControlButton::Green;
    } else if(buttonName == "KEY_RED"){
        return RemoteControlButton::Red;
    } else if(buttonName == "KEY_YELLOW"){
        return RemoteControlButton::Yellow;

    // Not in lirc namespace or in RemoteControlButton::ButtonID
    } else {
        return RemoteControlButton::Unknown;
    }
}

QString LircRemoteControl::formatNamespaceButton(const QString &buttonName) const {
    QString newName = buttonName;
    if(buttonName.startsWith("KEY_")){
        newName.remove("KEY_");
        newName = newName.left(1) + newName.mid(1).toLower();
    } else if(buttonName.startsWith("BUTTON_")){
        newName.replace("BUTTON_", "Button");
        newName = newName.left(7) + newName.mid(7).toLower();
    } else {
        newName = buttonName;
    }
    return newName;
}

#include "lircremotecontrol.moc"
