/*  Copyright 2004, Daniel Woods Bullok <dan.devel@bullok.com>
    distributed under the terms of the
    GNU GENERAL PUBLIC LICENSE Version 2 -
    See the file kdebase/COPYING for details
*/

#include "flowgridmanager.h"
#include <kdebug.h>
#ifdef DEBUG
   #define DEBUGSTR kDebug()
#else
   #define DEBUGSTR kndDebug()
#endif


FlowGridManager::FlowGridManager(QSize p_item_size,
                                 QSize p_space_size,
                                 QSize p_border_size,
                                 QSize p_frame_size,
                                 Qt::Orientation orient,
                                 int num_items,
                                 Slack slack_x,Slack slack_y)
{
    _pItemSize=p_item_size;
    _pSpaceSize=p_space_size;
    _pBorderSize=p_border_size;
    _pFrameSize=p_frame_size;
    _orientation=orient;
    _numItems=num_items;
    _slackX=slack_x;
    _slackY=slack_y;
    _conserveSpace=false;

    _dirty=true;
    _valid=false;
}

// set members.
// These all set the _dirty flag if the new value is different.
void FlowGridManager::setNumItems(int num_items)
{   if (_numItems==num_items)
        return;
    _numItems=num_items; _dirty=true;
}
void FlowGridManager::setItemSize(QSize p_item_size)
{   if (_pItemSize==p_item_size)
        return;
    _pItemSize=p_item_size; _dirty=true;
}

void FlowGridManager::setSpaceSize(QSize p_space_size)
{   if (_pSpaceSize==p_space_size)
        return;
    _pSpaceSize=p_space_size; _dirty=true;
}

void FlowGridManager::setBorderSize(QSize p_border_size)
{   if (_pBorderSize==p_border_size)
        return;
    _pBorderSize=p_border_size; _dirty=true;
}

void FlowGridManager::setFrameSize(QSize p_frame_size)
{   if (_pFrameSize==p_frame_size)
        return;
    _pFrameSize=p_frame_size;
    if (_pFrameSize.width()<=0) {
        _orientation=Qt::Vertical;
    }
    if (_pFrameSize.height()<=0) {
        _orientation=Qt::Horizontal;
    }
    _dirty=true;
}

void FlowGridManager::setOrientation(Qt::Orientation orient)
{   if (orient==_orientation)
        return;
    _orientation=orient; _dirty=true;
}

void FlowGridManager::setSlack(Slack slack_x, Slack slack_y)
{   if (slack_x==_slackX && slack_y==_slackY) return;
     _slackX=slack_x; _slackY=slack_y; _dirty=true;}


void FlowGridManager::setConserveSpace(bool conserve)
{   if (_conserveSpace==conserve)
        return;
    _conserveSpace=conserve; _dirty=true;
}



// get members
QSize  FlowGridManager::itemSize() const
{   _checkReconfigure();  return _itemSize;}

QSize  FlowGridManager::spaceSize() const
{   _checkReconfigure();  return _spaceSize;}

QSize  FlowGridManager::borderSize() const
{   _checkReconfigure();  return _borderSize;}

QSize  FlowGridManager::gridDim() const
{   _checkReconfigure();  return _gridDim;}

QSize  FlowGridManager::gridSpacing() const
{   _checkReconfigure();  return _gridSpacing;}

QSize  FlowGridManager::frameSize() const
{   _checkReconfigure();  return _frameSize;}

QPoint FlowGridManager::origin() const
{   _checkReconfigure();  return _origin;}

Qt::Orientation FlowGridManager::orientation() const
{   _checkReconfigure();  return _orientation;}

/*Slack FlowGridManager::slackX() const
{   return _slackY;}

Slack FlowGridManager::slackY() const
{   return _slackY;}
*/

bool FlowGridManager::conserveSpace() const
{        return _conserveSpace; }


bool FlowGridManager::isValid() const
{   _checkReconfigure();  return _valid;}

QPoint FlowGridManager::posAtCell(int x,int y) const
{  _checkReconfigure();
    return _origin+QPoint(_gridSpacing.width()*x,_gridSpacing.height()*y);
}

QPoint FlowGridManager::pos(int i) const
{   return posAtCell(cell(i).x(),cell(i).y());
}

QPoint FlowGridManager::cell(int index) const
{   _checkReconfigure();
    //assert((index>=0) && (index<_gridDim.width()*_gridDim.height()));
    int x=index % _gridDim.width(),
        y=index / _gridDim.width();
    return QPoint(x,y);
}




// return height if orientation is Horizontal
// return width if orientation is Vertical
int FlowGridManager::_getHH(QSize size) const
{  if (_orientation==Qt::Horizontal)
        return size.height();
    return size.width();
}

// return height if orientation is Vertical
// return width if orientation is Horizontal
int FlowGridManager::_getWH(QSize size) const
{  if (_orientation==Qt::Horizontal)
        return size.width();
    return size.height();
}

// swap horizontal and vertical if orientation is Vertical, otherwise return arg
QSize FlowGridManager::_swapHV(QSize hv) const
{  if (_orientation==Qt::Horizontal)
        return hv;
    QSize temp=hv;
    temp.transpose();
    return temp;
}


// return the amount of slack when:
//    nitems = # of items
//    length = total length of space where items will be placed
//    item, space, border = length of respective entities
int FlowGridManager::_slack(int nitems,int length,int item,int space,int border) const
{  return length-(2*border)-(nitems-1)*space-nitems*item;}


void FlowGridManager::_clear() const
{
    _borderSize=QSize(0,0);
    _spaceSize=QSize(0,0);
    _itemSize=QSize(0,0);
    _gridDim=QSize(0,0);
    _gridSpacing=QSize(0,0);
    _origin=QPoint(0,0);
    _frameSize=QSize(0,0);

    _dirty=false;
    _valid=false;
}


int FlowGridManager::indexNearest(QPoint p) const
{  if (!isValid()) return -1;
   QPoint c=(p-_origin)-QPoint(_spaceSize.width(),_spaceSize.height())/2;
   int x=c.x()/_gridSpacing.width(),
       y=c.y()/_gridSpacing.height();
   int i= x+y*_gridDim.width();
   if (i>_numItems) return -1;
   return i;
}



// Redistribute the boxes
void FlowGridManager::_reconfigure() const
{   if ((!_pFrameSize.isValid()) ||
        (!_pItemSize.isValid()) ||
         _numItems==0 ) {
        _clear();
        return;
    }
    int height=_getHH(_pFrameSize),
        pItemHeight=_getHH(_pItemSize),
        pSpaceHeight=_getHH(_pSpaceSize),
        pBorderHeight=_getHH(_pBorderSize),
        spanlen=(height-2*pBorderHeight+pSpaceHeight)/(pItemHeight+pSpaceHeight);
    int slack,iSlack;

    if (spanlen==0) {
        _dirty=false;
        _valid=false;
        return;
    }
    // figure out the number of spans required for all items
    int numspans=_numItems/spanlen;
    if (numspans*spanlen<_numItems) {
        numspans++;
    }

    slack=_slack(spanlen,height,pItemHeight,pSpaceHeight,pBorderHeight);  // total slack
    iSlack=slack/spanlen;                // slack per item
    // Items pick up extra slack
    if (_slackX==ItemSlack) pItemHeight+=iSlack;
    slack=_slack(spanlen,height,pItemHeight,pSpaceHeight,pBorderHeight);

    // space picks up extra slack
    if (spanlen>1) {
        iSlack=slack/(spanlen+1);
        pSpaceHeight+=iSlack;
    }

    slack=_slack(spanlen,height,pItemHeight,pSpaceHeight,pBorderHeight);
    iSlack=slack/2;
    pBorderHeight+=iSlack;
    if (_conserveSpace) {
        _itemSize=_swapHV(QSize(_getWH(_pItemSize),pItemHeight));
        _spaceSize=_swapHV(QSize(_getWH(_pSpaceSize),pSpaceHeight));
        _borderSize=_swapHV(QSize(_getWH(_pBorderSize),pBorderHeight));
    }
    else {
        _itemSize=_swapHV(QSize(pItemHeight,pItemHeight));
        _spaceSize=_swapHV(QSize(pSpaceHeight,pSpaceHeight));
        _borderSize=_swapHV(QSize(pBorderHeight,pBorderHeight));
    }
    _gridDim=_swapHV(QSize(numspans,spanlen));

    _gridSpacing=_itemSize+_spaceSize;
    _origin=QPoint(_borderSize.width(),_borderSize.height());
    _frameSize=2*_borderSize+QSize(_gridDim.width()*_gridSpacing.width()-_spaceSize.width(),
                              _gridDim.height()*_gridSpacing.height()-_spaceSize.height());

    _dirty=false;
    _valid=true;
}


void FlowGridManager::dump()
{
    DEBUGSTR<<endl<<flush;

    DEBUGSTR<<"_pItemSize=("<<_pItemSize.width()<<","<<_pItemSize.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_pSpaceSize=("<<_pSpaceSize.width()<<","<<_pSpaceSize.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_pBorderSize=("<<_pBorderSize.width()<<","<<_pBorderSize.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_pFrameSize=("<<_pFrameSize.width()<<","<<_pFrameSize.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_borderSize=("<<_borderSize.width()<<","<<_borderSize.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_spaceSize=("<<_spaceSize.width()<<","<<_spaceSize.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_itemSize=("<<_itemSize.width()<<","<<_itemSize.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_gridDim=("<<_gridDim.width()<<","<<_gridDim.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_gridSpacing=("<<_gridSpacing.width()<<","<<_gridSpacing.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_origin=("<<_origin.x()<<","<<_origin.y()<<")"<<endl<<flush;
    DEBUGSTR<<"_frameSize=("<<_frameSize.width()<<","<<_frameSize.height()<<")"<<endl<<flush;
    DEBUGSTR<<"_conserveSpace="<<_conserveSpace<<endl<<flush;

    DEBUGSTR<<"_orientation="<<_orientation<<endl<<flush;
    DEBUGSTR<<"_numItems="<<_numItems<<endl<<flush;
    DEBUGSTR<<"_slackX="<<_slackX<<endl<<flush;
    DEBUGSTR<<"_slackY="<<_slackY<<endl<<flush;
    DEBUGSTR<<"_dirty="<<_dirty<<endl<<flush;
    DEBUGSTR<<"_valid="<<_valid<<endl<<flush;
    DEBUGSTR<<endl<<flush;
}



bool operator== ( const FlowGridManager & csg1, const FlowGridManager & csg2 )
{
    return csg1.gridDim()==csg2.gridDim() &&
    csg1.origin()==csg2.origin() &&
    csg1.gridSpacing()==csg2.gridSpacing() &&
    csg1.frameSize()==csg2.frameSize();
}




