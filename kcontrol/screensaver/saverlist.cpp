
#include "saverlist.h"

class SaverConfig;
class Q3PtrCollection;

int SaverList::compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2)
{
    SaverConfig *s1 = (SaverConfig *)item1;
    SaverConfig *s2 = (SaverConfig *)item2;

    return s1->name().localeAwareCompare(s2->name());
}
