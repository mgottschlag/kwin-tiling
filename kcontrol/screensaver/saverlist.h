#ifndef SAVERLIST_H
#define SAVERLIST_H

#include <qptrlist.h>

#include "saverconfig.h"

class SaverList : public QPtrList<SaverConfig>
{
protected:
    virtual int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2);
};

#endif
