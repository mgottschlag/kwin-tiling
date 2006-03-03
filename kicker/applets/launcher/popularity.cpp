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

#include "popularity.h"
#include "prefs.h"
#include <assert.h>
#include <algorithm>
#include <iterator>
#include <kconfig.h>
#include <kdebug.h>
#include <list>
#include <set>
#include <map>
#include <cmath>
#include <vector>

using namespace std;

class PopularityStatisticsImpl
{
public:
    struct SingleFalloffHistory
    {
    public:
        // fallof is a number between 0 and 1. The popularity of
        // each service is multiplied with the falloff value
        // every time a service is used. Only the used service
        // gets also  1-falloff added to its popularity.
        double falloff;
        // popularity values for each service
        map<QString, double> vals;
        // accumulated popularity of the unknown programs
        // started before the statistic started
        double iniVal;
    };
    
    struct Popularity
    {
        QString service;
        double popularity;
        bool operator<(const Popularity& p) const
        { 
            return popularity > p.popularity;
        }
    };
    
    PopularityStatisticsImpl();
    void normalizeHistory(SingleFalloffHistory& h);
    void updateServiceRanks();

    vector<SingleFalloffHistory> m_stats;
    vector<Popularity> m_servicesByPopularity;
    map<QString, int> m_serviceRanks;
    double m_historyHorizon;
};

// ---- Public methods ----

PopularityStatistics::PopularityStatistics() :
    d(new PopularityStatisticsImpl())
{
}

PopularityStatistics::~PopularityStatistics() 
{
    delete d;
}

void PopularityStatistics::useService(const QString& service)
{
    vector<PopularityStatisticsImpl::SingleFalloffHistory>::iterator 
        it(d->m_stats.begin()), end(d->m_stats.end());
    for (; it != end; ++it)
    {
        map<QString, double>::iterator valIt;
        bool found(false);
        for (valIt = it->vals.begin(); valIt != it->vals.end(); ++valIt)
        {
            valIt->second = valIt->second * it->falloff;
            if (valIt->first == service)
            {
                found = true;
                valIt->second += 1-it->falloff;
            }
        }
        it->iniVal = it->iniVal * it->falloff;
        if (found == false)
        {
           it->vals[service] = 1-it->falloff;
        }
        d->normalizeHistory(*it);
    }
    d->updateServiceRanks();
}

void PopularityStatistics::moveToTop(const QStringList& newTopServiceList)
{
    vector<PopularityStatisticsImpl::SingleFalloffHistory>::iterator 
    histIt(d->m_stats.begin()), histEnd(d->m_stats.end());
    for (; histIt != histEnd; ++histIt)
    {
        set<QString> newTopServices;
        for (int n=0; n<newTopServiceList.size(); ++n)
            newTopServices.insert(newTopServiceList[n]);

        // Sort by popularity
        vector<PopularityStatisticsImpl::Popularity> ranking;
        map<QString, double>::iterator valIt;
        for (valIt = histIt->vals.begin(); valIt != histIt->vals.end(); ++valIt)
        {
            PopularityStatisticsImpl::Popularity pop;
            pop.service = valIt->first;
            pop.popularity = valIt->second;
            ranking.push_back(pop);
        }
        stable_sort(ranking.begin(), ranking.end());
        
        // Get the new positions of each service in the ranking.
        // We don't touch the popularity values in the ranking.
        list<QString> topServiceList, bottomServiceList;
        vector<PopularityStatisticsImpl::Popularity>:: iterator rankIt;
        for (rankIt = ranking.begin(); rankIt != ranking.end(); ++rankIt)
        {
            if (newTopServices.find(rankIt->service) != newTopServices.end())
            {
                topServiceList.push_back(rankIt->service);
                //kDebug() << "top service: " << valIt->first << endl;
                newTopServices.erase(rankIt->service);
            }
            else
            {
                //kDebug() << "bottom service: " << valIt->first << endl;
                bottomServiceList.push_back(rankIt->service);
            }
        }
        // Append remaining new services to the topServices list
        while (newTopServices.size() > 0)
        {
            topServiceList.push_back(*newTopServices.begin());
            newTopServices.erase(newTopServices.begin());
        }
        
        list<QString> newServiceList;
        copy(topServiceList.begin(), topServiceList.end(),
            back_insert_iterator<list<QString> >(newServiceList)); 
        copy(bottomServiceList.begin(), bottomServiceList.end(),
            back_insert_iterator<list<QString> >(newServiceList)); 
        
        // Merge the old list of service popularities
        // with the new ordering of the services
        histIt->vals.clear();
        list<QString>::iterator servIt;
        uint serviceIndex = 0;
        //kDebug() << endl;
        
        for (servIt = newServiceList.begin(); servIt != newServiceList.end();
             ++servIt)
        {
            if (serviceIndex < ranking.size())
            {
                histIt->vals[*servIt] = ranking[serviceIndex].popularity;
                //kDebug() << "->Re-Added service " << 
                //ranking[serviceIndex].popularity 
                //    << " " << *servIt << endl;
                //kDebug() << "...was replaced by " << *servIt << endl;
            }
            else
            {
                //kDebug() << "Service " << *servIt << endl;
                //kDebug() << "...was set to popularity=0" << endl;
                histIt->vals[*servIt] = 0.00001;
            }
            // Make sure that the topServices are actually bigger than
            // the bottomServices and not just bigger or equal
            // and also that no services have popularity==0
            if (serviceIndex >= topServiceList.size())
            {
                histIt->vals[*servIt] *= histIt->falloff;
            }
            ++serviceIndex;
        }
        d->normalizeHistory(*histIt);
    }
    d->updateServiceRanks();
}

/*v
Old version - moves everything else one position up
and 'service' to the bottom
void PopularityStatistics::moveToBottom(const QString& service)
{
    // Moves a service to the bottom of the ranking
    // by moving everything else to the top
    d->updateServiceRanks();
    QStringList allButOneServices;
    vector<PopularityStatisticsImpl::Popularity>::iterator 
        it(d->m_servicesByPopularity.begin()),
        end(d->m_servicesByPopularity.end());
    for (; it != end; ++it)
    {
        if (it->service != service)
            allButOneServices << it->service;
    }
    moveToTop(allButOneServices);
}*/

void PopularityStatistics::moveToBottom(const QString& service)
{
    vector<PopularityStatisticsImpl::SingleFalloffHistory>::iterator 
        it(d->m_stats.begin()), end(d->m_stats.end());
    for (; it != end; ++it)
    {
        it->iniVal += it->vals[service];
        it->vals[service] = 0;
        d->normalizeHistory(*it);
    }
    d->updateServiceRanks();
}

QString PopularityStatistics::serviceByRank(int n) const
{
    if (n >= 0 && n < int(d->m_servicesByPopularity.size()))
        return d->m_servicesByPopularity[n].service;
    else
        return QString();
}

double PopularityStatistics::popularityByRank(int n) const
{
    if (n >= 0 && n < int(d->m_servicesByPopularity.size()))
        return d->m_servicesByPopularity[n].popularity;
    else
        return 0.0;
}

int PopularityStatistics::rankByService(const QString service)
{
    if (d->m_serviceRanks.find(service) != d->m_serviceRanks.end())
    {
        return d->m_serviceRanks[service];
    }
    return -1;
}

void PopularityStatistics::writeConfig(Prefs* prefs) const
{
    QStringList serviceNames, serviceHistories;
    int limit = prefs->serviceCacheSize();
    //kDebug() << "popularityData: writeConfig" << endl;
    for (int n=0; n<int(d->m_servicesByPopularity.size()) && n<limit; ++n)
    {
        PopularityStatisticsImpl::Popularity pop = d->m_servicesByPopularity[n];
        QStringList historyData;
        for (int i=0; i<int(d->m_stats.size()); ++i)
        {
            historyData << QString::number(d->m_stats[i].vals[pop.service]);
        }
        serviceNames << pop.service;
        serviceHistories << historyData.join("/");
        //kDebug() << "popularityData: writeConfig -- " << pop.service << endl;
    }
    prefs->setServiceNames(serviceNames);
    prefs->setServiceHistories(serviceHistories);
}

void PopularityStatistics::readConfig(Prefs* prefs)
{
    int n = 0;
    QStringList serviceNames = prefs->serviceNames();
    QStringList histories = prefs->serviceHistories();
    for (n = min(serviceNames.size(), histories.size())-1; n>=0; --n)
    {
        QString serviceName = serviceNames[n];
        QStringList serviceHistory =
            histories[n].split( "/");
        for (int i=std::min(serviceHistory.size(), (int)d->m_stats.size())-1; i>=0; --i)
        {
            d->m_stats[i].vals[serviceName] = serviceHistory[i].toDouble();
        }
    }
    
    for (int i=0; i<int(d->m_stats.size()); ++i)
    {
        map<QString, double>::iterator valIt;
        double valSum = 0;
        for (valIt = d->m_stats[i].vals.begin(); 
             valIt != d->m_stats[i].vals.end(); ++valIt)
        {
            if (valIt->second < 0) valIt->second = 0;
            valSum += valIt->second;
        }
        // Scale down values if their sum is bigger than 1 
        // because of rounding errors or a corrupted config file
        if (valSum > 1)
        {
            for (valIt = d->m_stats[i].vals.begin(); 
                 valIt != d->m_stats[i].vals.end(); ++valIt)
            {
                valIt->second = valIt->second / valSum;
            }
        }
        d->m_stats[i].iniVal = 1-valSum;
    }
    d->updateServiceRanks();
}

void PopularityStatistics::setHistoryHorizon(double h)
{
    d->m_historyHorizon = std::max(std::min(h, 1.0), 0.0);
    d->updateServiceRanks();
}

double PopularityStatistics::historyHorizon()
{
    return d->m_historyHorizon;
}


// ---- Implementation methods ----

PopularityStatisticsImpl::PopularityStatisticsImpl()
{
    static const int rateBaseCount(8);
    
    for (int n=0; n<rateBaseCount; ++n) 
    {
        SingleFalloffHistory h;
        h.falloff = 1.0 - (0.5 / exp(double(n)*1.5));
        m_stats.push_back(h);
    }
}

void PopularityStatisticsImpl::normalizeHistory(SingleFalloffHistory& h)
{
    //kDebug() << "Normalize history" << endl;
    double sum = h.iniVal;
    map<QString, double>::iterator it;
    for (it = h.vals.begin(); it != h.vals.end(); ++it)
    {
        sum += it->second;
    }
    for (it = h.vals.begin(); it != h.vals.end(); ++it)
    {
        it->second = it->second / sum;
    }
    h.iniVal = h.iniVal / sum;
}

void PopularityStatisticsImpl::updateServiceRanks()
{
    // For each service calculate the average over the popularity
    // for all falloff values and then sort by these averaged values

    vector<SingleFalloffHistory>::iterator 
        it(m_stats.begin()), end(m_stats.end());
    map<QString, double> serviceValSum, serviceValWeightSum;
    int numStats = m_stats.size();
    for (int statIndex = 0; it != end; ++it, ++statIndex)
    {
        // Put more weight on the short term history if m_historyHorizon==0
        // and more on the the long term history for m_historyHorizon==1
        double a = 2*(numStats-1)*m_historyHorizon - numStats + 0.5;
        if (statIndex < a || statIndex > a + numStats)
        {
            continue;
        }

        map<QString, double>::iterator valIt;
        /*double valSum = 0;
        for (valIt = it->vals.begin(); valIt != it->vals.end(); ++valIt)
        {
            valSum += valIt->second;
        }
        if (valSum == 0) valSum = 1;*/
        for (valIt = it->vals.begin(); valIt != it->vals.end(); ++valIt)
        {
            serviceValWeightSum[valIt->first] += 1;
            serviceValSum[valIt->first] += valIt->second;
        }
    }
    
    m_servicesByPopularity.clear();
    map<QString, double>::iterator sIt;
    for (sIt = serviceValWeightSum.begin(); 
         sIt != serviceValWeightSum.end(); ++sIt)
    {
        Popularity p;
        p.service = sIt->first;
        assert(sIt->second > 0);
        p.popularity = serviceValSum[sIt->first] / sIt->second;
        m_servicesByPopularity.push_back(p);
    }
    stable_sort(m_servicesByPopularity.begin(), m_servicesByPopularity.end());
    m_serviceRanks.clear();
    for (uint n = 0; n < m_servicesByPopularity.size(); ++n)
    {
        m_serviceRanks[m_servicesByPopularity[n].service] = n;
        /*kDebug() << QString("Rank %1: %2 %3").arg(n)
            .arg(m_servicesByPopularity[n].popularity)
            .arg(m_servicesByPopularity[n].service) << endl;*/
    }
}
