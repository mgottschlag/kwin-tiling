/*
    localdomainurifilter.h

    This file is part of the KDE project
    Copyright (C) 2002 Lubos Lunak <llunak@suse.cz>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _LOCALDOMAINURIFILTER_H_
#define _LOCALDOMAINURIFILTER_H_

#include <time.h>

#include <dcopobject.h>
#include <kgenericfactory.h>
#include <kurifilter.h>

class KInstance;

class LocalDomainURIFilter
    : public KURIFilterPlugin, public DCOPObject
    {
    K_DCOP
    Q_OBJECT
    public:
	LocalDomainURIFilter( QObject* parent, const char* name, const QStringList& args );
        virtual bool filterURI( KURIFilterData &data ) const;
    k_dcop:
        virtual void configure();
    private:
	bool isLocalDomainHost( const QString& cmd ) const;
	mutable QString last_host;
	mutable bool last_result;
	mutable time_t last_time;
    };

#endif
