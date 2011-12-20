/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

/*
   This file includes code from EasyStroke, Copyright (c) 2008-2009,
   Thomas Jaeger <ThJaeger@gmail.com>
   http://easystroke.sourceforge.net

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.
*/

#include <math.h>

#include "triggers/triggers.h"
#include "action_data/action_data.h"
#include "triggers/gestures.h"
#include "windows_handler.h"

#include <QFile>

#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <kstandarddirs.h>

namespace KHotKeys {


GestureTriggerVisitor::~GestureTriggerVisitor()
    {}


GestureTrigger::GestureTrigger( ActionData* data_P, const StrokePoints &pointdata_P )
    : Trigger( data_P ), _pointdata( pointdata_P )
    {
    }


GestureTrigger::~GestureTrigger()
    {
    gesture_handler->unregister_handler( this, SLOT(handle_gesture(StrokePoints)));
    }


void GestureTrigger::accept(TriggerVisitor& visitor)
    {
    if (GestureTriggerVisitor *v = dynamic_cast<GestureTriggerVisitor*>(&visitor))
        {
        v->visit(*this);
        }
    else
        {
        kDebug() << "Visitor error";
        }
    }


void GestureTrigger::activate( bool activate_P )
    {
    if( activate_P )
        gesture_handler->register_handler( this, SLOT(handle_gesture(StrokePoints)));
    else
        gesture_handler->unregister_handler( this, SLOT(handle_gesture(StrokePoints)));
    }


void GestureTrigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    // we want to write using KConfig, so we'll need strings -
    // one for each attribute of each point.
    QStringList strings;

    int n = _pointdata.size();

    for(int i=0; i<n; i++)
        {
        strings.append(QString::number(_pointdata[i].s));
        strings.append(QString::number(_pointdata[i].delta_s));
        strings.append(QString::number(_pointdata[i].angle));
        strings.append(QString::number(_pointdata[i].x));
        strings.append(QString::number(_pointdata[i].y));
        }

    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "GesturePointData", strings);
    cfg_P.writeEntry( "Type", "GESTURE" ); // overwrites value set in base::cfg_write()

    }



Trigger* GestureTrigger::copy( ActionData* data_P ) const
    {
    kDebug() << "GestureTrigger::copy()";
    return new GestureTrigger( data_P ? data_P : data, pointData());
    }


const QString GestureTrigger::description() const
    {
    return i18n( "Gesture trigger" );
    }


const StrokePoints& GestureTrigger::pointData() const
    {
    return _pointdata;
    }


void GestureTrigger::setPointData( const StrokePoints &data )
    {
    _pointdata = data;
    }


void GestureTrigger::setPointData( const QStringList &strings )
    {
    // number of points that can be read
    // (each string is one of 5 coordinates)
    int n = strings.length()/5;
    _pointdata.resize(n);

    for(int i=0; i<n; i++)
        {
        _pointdata[i].s = strings[i*5 + 0].toDouble();
        _pointdata[i].delta_s = strings[i*5 + 1].toDouble();
        _pointdata[i].angle = strings[i*5 + 2].toDouble();
        _pointdata[i].x = strings[i*5 + 3].toDouble();
        _pointdata[i].y = strings[i*5 + 4].toDouble();
        }

#pragma CHECKME
#if 0
    if(n < 5)
        {
        _pointdata.clear();
        importKde3Gesture(*_config);
        }
#endif
    }

void GestureTrigger::handle_gesture( const StrokePoints &pointdata_P )
    {
    qreal score;
    score = comparePointData(pointdata_P, _pointdata);

    if( score > 0.7 )
        {
        emit gotScore(data, score);
        }
    }



// try to import a gesture from KDE3 times which is composed of a string of
// numbers
void GestureTrigger::setKDE3Gesture(const QString &gestureCode)
    {
    if(gestureCode.isEmpty())
        {
        _pointdata.clear();
        return;
        }

    Stroke stroke;

    // the old format has very little data, so we'll interpolate a little
    // to make it work with the new format.
    // as the stroke expects the data in integer format we'll use large numbers.
    int oldx = -1;
    int oldy = -1;
    int newx = 0;
    int newy = 0;

    for(int i=0; i < gestureCode.length(); i++)
        {
        switch(gestureCode[i].toAscii())
            {
            case '1':
                newx = 0;
                newy = 2000;
                break;
            case '2':
                newx = 1000;
                newy = 2000;
                break;
            case '3':
                newx = 2000;
                newy = 2000;
                break;
            case '4':
                newx = 0;
                newy = 1000;
                break;
            case '5':
                newx = 1000;
                newy = 1000;
                break;
            case '6':
                newx = 2000;
                newy = 1000;
                break;
            case '7':
                newx = 0;
                newy = 0;
                break;
            case '8':
                newx = 1000;
                newy = 0;
                break;
            case '9':
                newx = 2000;
                newy = 0;
                break;

            default: return;
            }

        // interpolate
        if(oldx != -1)
            {
            stroke.record( oldx + 1 * (newx-oldx)/4.0 , oldy + 1 * (newy-oldy)/4.0 );
            stroke.record( oldx + 2 * (newx-oldx)/4.0 , oldy + 2 * (newy-oldy)/4.0 );
            stroke.record( oldx + 3 * (newx-oldx)/4.0 , oldy + 3 * (newy-oldy)/4.0 );
            }

        // add the one point that is really known
        stroke.record(newx, newy);

        oldx = newx;
        oldy = newy;

        }

    // the calculations for the new format chop off some points at the end.
    // that's usually no problem, but here we'll want to compensate
    stroke.record(newx,  newy);
    stroke.record(newx,  newy);
    stroke.record(newx,  newy);

    _pointdata = stroke.processData();
    }


// Create a score for how well two strokes match.
// Algorithm taken from EasyStroke 0.4.1, modified to work in C++ and commented
// for better maintainability. The algorithm logic should still be the same.
//
// The basic idea of the algorithm is to find points that are similar to each
// other by iterating over both strokes and trying to match points so that the
// cost (that is mostly defined by angle differences and the cost of the
// cheapest predecessor) becomes minimal.
// When the matching has arrived at the last point of both strokes the cost for
// this match is taken as the overall cost and then transformed into a score
// between 0 and 1.
//
//
// If you're really interested in why this works you can have a look at the
// small mathematical tex-document that is distributed with EasyStroke.
// Probably won't help much though.
qreal GestureTrigger::comparePointData(const StrokePoints &a, const StrokePoints &b) const
    {
    // 0.2 will be a very large value in these computations
    const qreal stroke_infinity=0.2;

    int m = a.size() - 1;
    int n = b.size() - 1;

    // if there's too little data: return score of 0
    if(m < 1 || n < 1)
        return 0.0;

    // array "cost" to save results of the cost calculations.
    // set all cells of cost to infinity.
    // we use nested vectors instead of a real array because the array size is
    // determined at runtime.
    QVector< QVector<qreal> > cost(m+1, QVector<qreal>(n+1, stroke_infinity));

    // we start at the beginnings of both strokes - with cost 0.
    cost[0][0] = 0.0;

    // iterate over all cols and rows except for the last
    // (last one would be aborted anyway)
    // Attention: We don' use absolute coordinates in this method! x and y are
    // the indices of the current row and column in the cost-matrix.
    for (int x = 0; x < m; x++)
        {
        for (int y = 0; y < n; y++)
            {

            // cost already too high -> forget this one, it won't get better
            if (cost[x][y] >= stroke_infinity)
                continue;

            qreal sx  = a[x].s;
            qreal sy  = b[y].s;

            // these will get incremented later in the function.
            // they define how large the area is we'll jump around in to look
            // for cheaper ways
            int max_x = x;
            int max_y = y;

            // this will be used to limit the number of iterations
            int k = 0;

            // we'll have to control the repeated execution of code which
            // _should_ be a function, but it needs to access too
            // many of the variables of this function here.
            // rewriting all of this as a class makes it even more complicated,
            // so I'll define some variables that decide how often the chunk of
            // code should be run.

            // jumpToEnd: make one last jump to cell m,n and end this
            bool jumpToEnd = false;
            bool iterateOverX2=false, iterateOverY2=false;

            // variables that give the position in the matrix we want to jump
            // to. we'll iterate over these.
            int x2, y2;


            // artificially limit the number of iterations. the changing of
            // max_x and max_y ensures we don't do the same all the time
            while (k < 4)
                {
                // first we set up our logic controllers

                // if difference between s at max_x+1 and s at x is bigger
                // than difference between s at max_y+1 and y...
                if (a[max_x+1].s - sx > b[max_y+1].s - sy)
                    {
                    // widen our search in y-direction
                    max_y++;

                    // if we're at the side of the matrix make the step to
                    // the end and break
                    if (max_y == n)
                        {
                        jumpToEnd = true;
                        x2 = m;
                        y2 = n;
                        }
                    else // check the cells with the new max_y
                        {
                        iterateOverX2 = true;
                        x2 = x+1;
                        y2 = max_y;
                        }

                    }

                // differences bigger or equal in y-direction
                //  -> same thing with y and x switched
                else
                    {
                    // widen our search in x-direction
                    max_x++;

                    // if we're at the side -> step to the end and break
                    if (max_x == m)
                        {
                        jumpToEnd = true;
                        x2 = m;
                        y2 = n;
                        }
                    else // check the cells with the new max_x
                        {
                        iterateOverY2 = true;
                        x2 = max_x;
                        y2 = y+1;
                        }

                    }

                // contained in this loop we have the code that
                // performs the step from current x,y to x2,y2 and
                // calculates the cost for it. if a new minimal cost for a cell
                // is found this new cost is written to it.
                while ( jumpToEnd
                        || ( iterateOverX2 && x2 <= max_x )
                        || ( iterateOverY2 && y2 <= max_y )
                        )
                    {

                    // very small value for s
                    const qreal epsilon = 0.000001;

                    qreal delta_sx = a[x2].s - a[x].s;
                    qreal delta_sy = b[y2].s - b[y].s;


                    // only consider doing this if the change of positions in x
                    // and y is comparable and not too small.
                    if (delta_sx < delta_sy * 2.2 && delta_sy < delta_sx * 2.2
                        && delta_sx >= epsilon && delta_sy >= epsilon)
                        {

                        k++;

                        // we compute the additional cost for this step
                        // by computing the square of the angle difference of
                        // the origin points and weighing it with how long the
                        // angles remain like this...
                        qreal c = (a[x].delta_s + b[y].delta_s) * angleSquareDifference(a[x].angle, b[y].angle);

                        // and adding similar values for all cells that need to
                        // be crossed to get to the step target (by traversing
                        // x and y separately)
                        for (int ix = x+1; ix < x2; ix++)
                            c += a[ix].delta_s * angleSquareDifference(a[ix].angle, b[y].angle);
                        for (int iy = y+1; iy < y2; iy++)
                            c += b[iy].delta_s * angleSquareDifference(a[x].angle, b[iy].angle);

                        // now add that to the cost of our starting point
                        qreal new_cost = cost[x][y] + c;

                        // if we found a cheaper origin for the target than
                        // before: save new minimal cost
                        if (new_cost < cost[x2][y2])
                            cost[x2][y2] = new_cost;

                        }

                    // control logic

                    if(jumpToEnd)
                        break;
                    if(iterateOverX2)
                        x2++;
                    if(iterateOverY2)
                        y2++;
                    }

                // if we jumped to the end we're finished with this combination
                // of x and y
                if(jumpToEnd)
                    break;

                // reset the logic controllers
                iterateOverX2 = false;
                iterateOverY2 = false;


                }


            }
        }

    // only task remaining is returning the results

    // target cell (m, n) now hopefully has minimal cost.
    // we compute our score from that.
    qreal score = (1.0 - 5.0*cost[m][n]);

    return score;
    }



// gives us the square of the difference of two angles
inline qreal GestureTrigger::angleSquareDifference(qreal alpha, qreal beta) const
    {
    qreal d = alpha - beta;

    if (d < -1.0)
        d += 2.0;
    else if (d > 1.0)
        d -= 2.0;

    return (d*d);
    }

} // namespace KHotKeys

