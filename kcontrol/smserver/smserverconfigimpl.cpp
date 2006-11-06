/***************************************************************************
                          smserverconfigimpl.cpp  -  description
                             -------------------
    begin                : Thu May 17 2001
    copyright            : (C) 2001 by stulle
    email                : stulle@tux
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "smserverconfigimpl.h"
#include "smserverconfigimpl.moc"

SMServerConfigImpl::SMServerConfigImpl(QWidget *parent ) : SMServerConfigDlg(parent) {
    connect(confirmLogoutCheck,SIGNAL(toggled(bool)), SLOT(configChanged()));
    connect(loginGroup,SIGNAL(clicked(int)), SLOT(configChanged()));
    connect(sdGroup,SIGNAL(clicked(int)),SLOT(configChanged()));
    connect(excludeLineedit,SIGNAL(textChanged(QString)),SLOT(configChanged()));
    connect(offerShutdownCheck,SIGNAL(toggled(bool)),SLOT(configChanged()));
}
SMServerConfigImpl::~SMServerConfigImpl(){
}
/** No descriptions */
void SMServerConfigImpl::configChanged(){

 emit changed();

}
