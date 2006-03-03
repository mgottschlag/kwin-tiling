/*  Copyright 2004, Daniel Woods Bullok <dan.devel@bullok.com>
    distributed under the terms of the
    GNU GENERAL PUBLIC LICENSE Version 2 -
    See the file kdebase/COPYING for details
*/

#ifndef __const_space_grid_h__
#define __const_space_grid_h__

#include <qnamespace.h>
#include <qpoint.h>
#include <qsize.h>


class FlowGridManager {
// Determine if two FlowGridManager objs have the same layout.  They may or
// may not have the same input parameters, but the resulting layout is identical.
    friend bool operator== ( const FlowGridManager & gp1, const FlowGridManager & gp2 );

public:
    typedef enum {
        ItemSlack,SpaceSlack,BorderSlack,NoSlack
    } Slack;

    FlowGridManager(QSize p_item_size=QSize(0,0),
                                     QSize p_space_size=QSize(0,0),
                                     QSize p_border_size=QSize(0,0),
                                     QSize frame_size=QSize(0,0),
                                     Qt::Orientation orient=Qt::Horizontal,
                                     int num_items=0,
                                     Slack slack_x=ItemSlack,
                                     Slack slack_y=ItemSlack);


    void setNumItems(int num_items);
    void setItemSize(QSize item_size);
    void setSpaceSize(QSize space_size);
    void setBorderSize(QSize border_size);
    void setOrientation(Qt::Orientation orient);
    void setFrameSize(QSize frame_size);
    void setSlack(Slack slack_x, Slack slack_y);
    void setConserveSpace(bool conserve);


    QSize  itemSize() const;
    QSize  spaceSize() const;
    QSize  borderSize() const;
    QSize  gridDim() const;
    QSize  gridSpacing() const;
    QSize  frameSize() const;
    QPoint origin() const;
    Qt::Orientation orientation() const;
    bool   conserveSpace() const;

//    Slack  slackX() const;
//    Slack  slackY() const;

    QPoint posAtCell(int x,int y) const;
    QPoint pos(int i) const;
    QPoint cell(int index) const;
    bool isValid() const;
    int indexNearest(QPoint p) const;

    void dump();
protected:
    int _getHH(QSize size) const;
    int _getWH(QSize size) const;
    QSize _swapHV(QSize hv) const;
    inline void _checkReconfigure() const;
    int _slack(int nitems,int length,int item,int space,int border) const;
    void _reconfigure() const;
    void _clear() const;

protected:
    // user-definable data
    QSize _pItemSize,_pSpaceSize,_pBorderSize,_pFrameSize;
    Slack _slackX, _slackY;
    bool _conserveSpace;
    Qt::Orientation _orientation;
    int _numItems;

    // results
    mutable QSize _itemSize, _spaceSize, _borderSize, _gridDim, _gridSpacing, _frameSize;
    mutable QPoint _origin;

    // status
    mutable bool _dirty, _valid;

};


// reconfigure the grid if necessary.
inline void FlowGridManager::_checkReconfigure() const
{   if (!_dirty) return;
    _reconfigure();
}

#endif

