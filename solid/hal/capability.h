/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef CAPABILITY_H
#define CAPABILITY_H

#include <QObject>
#include <QStringList>

#include <solid/ifaces/capability.h>

#include "haldevice.h"

class Capability : public QObject, virtual public Solid::Ifaces::Capability
{
    Q_OBJECT
    Q_INTERFACES( Solid::Ifaces::Capability )
public:
    Capability( HalDevice *device );
    virtual ~Capability();

protected:
    HalDevice *m_device;

public:
    inline static QStringList toStringList( Solid::Capability::Type capability )
    {
        QStringList list;

        switch( capability )
        {
        case Solid::Capability::GenericInterface:
            // Doesn't exist with HAL
            break;
        case Solid::Capability::Processor:
            list << "processor";
            break;
        case Solid::Capability::Block:
            list << "block";
            break;
        case Solid::Capability::Storage:
            list << "storage";
            break;
        case Solid::Capability::Cdrom:
            list << "storage.cdrom";
            break;
        case Solid::Capability::Volume:
            list << "volume";
            break;
        case Solid::Capability::OpticalDisc:
            list << "volume.disc";
            break;
        case Solid::Capability::Camera:
            list << "camera";
            break;
        case Solid::Capability::PortableMediaPlayer:
            list << "portable_audio_player";
            break;
        case Solid::Capability::NetworkHw:
            list << "net";
            break;
        case Solid::Capability::AcAdapter:
            list << "ac_adapter";
            break;
        case Solid::Capability::Battery:
            list << "battery";
            break;
        case Solid::Capability::Button:
            list << "button";
            break;
        case Solid::Capability::Display:
            list << "display_device";
            break;
        case Solid::Capability::AudioHw:
            list << "alsa" << "oss";
            break;
        case Solid::Capability::DvbHw:
            list << "dvb";
            break;
        case Solid::Capability::Unknown:
            break;
        }

        return list;
    }

    inline static Solid::Capability::Type fromString( const QString &capability )
    {
        if ( capability == "processor" )
            return Solid::Capability::Processor;
        else if ( capability == "block" )
            return Solid::Capability::Block;
        else if ( capability == "storage" )
            return Solid::Capability::Storage;
        else if ( capability == "storage.cdrom" )
            return Solid::Capability::Cdrom;
        else if ( capability == "volume" )
            return Solid::Capability::Volume;
        else if ( capability == "volume.disc" )
            return Solid::Capability::OpticalDisc;
        else if ( capability == "camera" )
            return Solid::Capability::Camera;
        else if ( capability == "portable_audio_player" )
            return Solid::Capability::PortableMediaPlayer;
        else if ( capability == "net" )
            return Solid::Capability::NetworkHw;
        else if ( capability == "ac_adapter" )
            return Solid::Capability::AcAdapter;
        else if ( capability == "battery" )
            return Solid::Capability::Battery;
        else if ( capability == "button" )
            return Solid::Capability::Button;
        else if ( capability == "display_device" )
            return Solid::Capability::Display;
        else if ( capability == "alsa" || capability == "oss" )
            return Solid::Capability::AudioHw;
        else if ( capability == "dvb" )
            return Solid::Capability::DvbHw;
        else
            return Solid::Capability::Unknown;
    }
};

#endif
