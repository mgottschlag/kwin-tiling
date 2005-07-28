#ifndef SAVERLIST_H
#define SAVERLIST_H

#include <q3ptrlist.h>

#include "saverconfig.h"

class SaverList : public Q3PtrList<SaverConfig>
{
protected:
    virtual int compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2);
};

#endif
