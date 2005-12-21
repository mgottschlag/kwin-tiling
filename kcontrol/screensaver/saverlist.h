#ifndef SAVERLIST_H
#define SAVERLIST_H

#include <QList>
#include <Q3PtrCollection>
#include "saverconfig.h"

class SaverList : public QList<SaverConfig*>
{
public:
	virtual ~SaverList(){}
protected:
    virtual int compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2);
};

#endif
