/*****************************************************************

Copyright (c) 2005 Fred Schaettgen <kde.sch@ttgen.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __popularity_h__
#define __popularity_h__

#include <QString>
#include <qstringlist.h>

class PopularityStatisticsImpl;
class Prefs;

/**
 * Tracks the usage of any kind of service to offer recommendations.
 * A service is identified by a string. After calling @useService
 * a few times, you can get a popularity ranking for the used
 * services.
 * The algorithm tries to take both short- and long-term usage
 * into account at the same time.
 * The popularity value can be interpreted as the probability
 * that the given service will be the next one to be used.
 * If some new services are suddenly used a few times, their ranking
 * may grow higher than the ranking of services, which were used very
 * frequently a while ago. But after this short term usage has
 * stopped, its influence gets weaker more quickly then for the old, 
 * more frequently used services.
 * During first time usage, the algorithm needs some time to stabilize, 
 * since there is simply no dependable long term usage data available, 
 * so the ranking is more likely to change at first.
 * But in the long run, the behaviour of the algorithm is 
 * completely stable, unlike simple usage counting for instance.
 */
class PopularityStatistics
{
public:
    PopularityStatistics();
    virtual ~PopularityStatistics();
    
    /**
     * Touch a service. This will increase the usage
     * counters for the given service and decrease the
     * counters for all the others.
     */
    void useService(const QString& service);
    
    /**
     * Exchange all state variables of the most
     * popular service with those from services,
     * moving the given services to the top of the
     * list. Apart from that the order stays the same
     * as before. Order of items in the string list
     * does *not* matter/
     */
    void moveToTop(const QStringList& services);
    
    /**
     * Sets all counters to zero for the given service
     */
    void moveToBottom(const QString& service);
    
    /**
     * Retrieve the name of a service by its position
     * in the current popularity ranking
     */
    QString serviceByRank(int n) const;
    
    /**
     * Retrieve the popularity (0-1) of a service by 
     * its position in the current popularity ranking
     */
    double popularityByRank(int n) const;

    /**
     * Gets the rank of a given service. 
     * Returns -1 if the service is not in the ranking
     */
    int rankByService(const QString service);
 
    /** 
     * Writes the configuration.
     * A section must be set already for config.
     */
    void writeConfig(Prefs* prefs) const;

    /**
     * Reads the configuration.
     * A section must be set already for config.
     */
    void readConfig(Prefs* prefs);

    /**
     * Modify the weighting of the history logs.
     * 0 <= h <= 1. 1 means long term history
     * 0 means short term history - in fact the popularity ranking
     * becomes a recently-used list in that case.
     */
    void setHistoryHorizon(double h);
    double historyHorizon();

protected:
    PopularityStatisticsImpl *d;

private:
    PopularityStatistics(const PopularityStatistics&) {}
};

#endif
