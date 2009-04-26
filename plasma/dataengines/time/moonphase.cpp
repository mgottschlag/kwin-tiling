/***************************************************************************
 *   Copyright 1998,2000  Stephan Kulow <coolo@kde.org>                    *
 *   Copyright 2009 by Davide Bettio <davide.bettio@kdemail.net>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "moonphase.h"

#include <QDateTime>
#include <KDebug>

#include "phases.cpp"

void appendMoonphase(Plasma::DataEngine::Data &data)
{
    time_t time;
    int counter;

    uint lun = 0;
    time_t last_new = 0;
    time_t next_new = 0;

    do {
        double JDE = moonphasebylunation(lun, 0);
        last_new = next_new;
        next_new = JDtoDate(JDE, 0);
        lun++;
    } while (next_new < time);

    lun -= 2;

    QDateTime ln;
    ln.setTime_t( last_new );
    kDebug() << "last new " << KGlobal::locale()->formatDateTime( ln );

    time_t first_quarter = JDtoDate( moonphasebylunation( lun, 1 ), 0 );
    QDateTime fq;
    fq.setTime_t( first_quarter );
    kDebug() << "first quarter " << KGlobal::locale()->formatDateTime( fq );

    time_t full_moon = JDtoDate( moonphasebylunation( lun, 2 ), 0 );
    QDateTime fm;
    fm.setTime_t( full_moon );
    kDebug() << "full moon " << KGlobal::locale()->formatDateTime( fm );

    time_t third_quarter = JDtoDate( moonphasebylunation( lun, 3 ), 0 );
    QDateTime tq;
    tq.setTime_t( third_quarter );
    kDebug() << "third quarter " << KGlobal::locale()->formatDateTime( tq );

    QDateTime nn;
    nn.setTime_t( next_new );
    kDebug() << "next new " << KGlobal::locale()->formatDateTime( nn );

    QDateTime now;
    now.setTime_t( time );
    kDebug() << "now " << KGlobal::locale()->formatDateTime( now );

    counter = ln.daysTo( now );
    kDebug() << "counter " << counter << " " << fm.daysTo( now );

    if ( fm.daysTo( now ) == 0 ) {
        counter = 14;
        ///toolTipData.setMainText( i18n( "Full Moon" ) );
        return;
    } else if ( counter <= 15 && counter >= 13 ) {
        counter = 14 + fm.daysTo( now );
        kDebug() << "around full moon " << counter;
    }

    int diff = fq.daysTo( now );
    if ( diff  == 0 )
        counter = 7;
    else if ( counter <= 8 && counter >= 6 ) {
        counter = 7 + diff;
         kDebug() << "around first quarter " << counter;
    }

    diff = ln.daysTo( now );
    if ( diff == 0 )
        counter = 0;
    else if ( counter <= 1 || counter >= 28 )
    {
        counter = ( 29 + diff ) % 29;
        diff = -nn.daysTo( now );
        if ( diff == 0 )
            counter = 0;
        else if ( diff < 3 )
            counter = 29 - diff;
        kDebug() << "around new " << counter << " " << diff;
    }

    if ( tq.daysTo( now ) == 0 )
        counter = 21;
    else if ( counter <= 22 && counter >= 20 )
    {
        counter = 21 + tq.daysTo( now );
        kDebug() << "around third quarter " << counter;
    }

    kDebug() << "counter " << counter;
    data["Moon" "Phase"] = counter;
}
