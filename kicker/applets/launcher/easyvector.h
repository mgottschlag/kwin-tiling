/*  Copyright 2004, Daniel Woods Bullok <dan.devel@bullok.com>
    distributed under the terms of the
    GNU GENERAL PUBLIC LICENSE Version 2 -
    See the file kdebase/COPYING for details
*/

#ifndef __easyvector_h__
#define __easyvector_h__
#include <vector>
#include <algorithm>
#include <assert.h>

template < class VALUE >
class __Valtype {
public:
    typedef const VALUE& CVALUE;
};


template < class VALUE >
class __Valtype< VALUE* > {
public:
    typedef const VALUE* CVALUE;
};


template <class VALUE, bool CHECKINDEX=true>
class EasyVector: public std::vector< VALUE > {
public:
    typedef int Index;
    typedef std::vector< Index > Indices;
    typedef typename __Valtype< VALUE >::CVALUE CVALUE;

    static const Index NotFound=-2;
    static const Index Append=-1;

    template < class PTYPE, class PROP_FUNC >
    Index findProperty(const PTYPE &property,
                       PROP_FUNC prop_func) const;
    Index findValue(CVALUE value) const;

    Index lastIndex() const {return this->size()-1;}

    void eraseAt(Index index);

    VALUE takeFrom(Index index);

    void insertAt(Index index,const VALUE &value);
    void insertAt(Index index,const EasyVector &values);

    bool isValidIndex(Index index) const;
    bool isValidInsertIndex(Index index) const;
    virtual ~EasyVector(){}


protected:
    void _checkInsertIndex(Index index) const;
    void _checkIndex(Index index) const;
    Index _convertInsertIndex(Index index) const;
};


template < class VALUE, bool CHECKINDEX >
template < class PTYPE, class PROP_FUNC >
typename EasyVector< VALUE, CHECKINDEX >::Index
    EasyVector< VALUE, CHECKINDEX >::findProperty(const PTYPE &property,
                                                  PROP_FUNC prop_func) const
{   typename EasyVector< VALUE, CHECKINDEX >::const_iterator i;
    for (i=this->begin();i!=this->end();++i) {
        if (prop_func(*i)==property)
            return i-this->begin();
    }
    return NotFound;
}


template < class VALUE, bool CHECKINDEX >
typename EasyVector< VALUE, CHECKINDEX >::Index
    EasyVector< VALUE, CHECKINDEX >::findValue(CVALUE value) const
{   typename EasyVector< VALUE, CHECKINDEX >::const_iterator i;
    i=std::find(this->begin(),this->end(),value);
    if (i==this->end()) return NotFound;
    return i-this->begin();
}


template < class VALUE, bool CHECKINDEX >
void EasyVector< VALUE, CHECKINDEX >::eraseAt(Index index)
{   _checkIndex(index);
    erase(this->begin()+index);
}


template < class VALUE, bool CHECKINDEX >
VALUE EasyVector< VALUE, CHECKINDEX >::takeFrom(Index index)
{   _checkIndex(index);
    VALUE result=(*this)[index];
    eraseAt(index);
    return result;
}


template < class VALUE, bool CHECKINDEX >
void EasyVector< VALUE, CHECKINDEX >::insertAt(EasyVector< VALUE, CHECKINDEX >::Index index,const VALUE &value)
{   index=_convertInsertIndex(index);
    _checkInsertIndex(index);
    if (index==int(this->size())) {
        this->push_back(value);
        return;
    }
    insert(this->begin()+index,value);
}


template < class VALUE, bool CHECKINDEX >
void EasyVector< VALUE, CHECKINDEX >::insertAt(EasyVector< VALUE, CHECKINDEX >::Index index,const EasyVector< VALUE, CHECKINDEX > &v)
{   index=_convertInsertIndex(index);
    _checkInsertIndex(index);
    insert(this->begin()+index,v.begin(),v.end());
}


template < class VALUE, bool CHECKINDEX >
bool EasyVector< VALUE, CHECKINDEX >::isValidIndex(EasyVector< VALUE, CHECKINDEX >::Index index) const
{   return(0<=index && index<int(this->size()));}

template < class VALUE, bool CHECKINDEX >
bool EasyVector< VALUE, CHECKINDEX >::isValidInsertIndex(EasyVector< VALUE, CHECKINDEX >::Index index) const
{   return(index==Append)||(0<=index && index<=int(this->size()));}

template < class VALUE, bool CHECKINDEX >
inline typename EasyVector< VALUE, CHECKINDEX >::Index EasyVector< VALUE, CHECKINDEX >::_convertInsertIndex(Index index) const
{   if (index==Append) return this->size();
    return index;
}

template < class VALUE, bool CHECKINDEX >
void EasyVector< VALUE, CHECKINDEX >::_checkInsertIndex(Index index) const
{   if (CHECKINDEX) assert (isValidInsertIndex(index));}

template < class VALUE, bool CHECKINDEX >
void EasyVector< VALUE, CHECKINDEX >::_checkIndex(Index index) const
{   if (CHECKINDEX) assert (isValidIndex(index));}


#endif

