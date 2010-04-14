/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "xmlconfiguration.h"

#include <QSet>
#include <KDebug>

#include "xmlconfigurations.h"
#include "xml/configurations_xml.h"

namespace Kephal {

    /* The layout initialisation code in the ctor came from XMLConfiguration::layout()
     * It tries to describe the relative screen layout of this Configuration
     * where each screen is a point in 2d space.
     * Used by BackendConfigurations::partition() and
     * XMLConfigurations::alternateConfigurations()
     * Broken because it only works if screens are laid out relative to one another, not
     * absolutely.
     */
    XMLConfiguration::XMLConfiguration(XMLConfigurations * parent, ConfigurationXML * config)
        : BackendConfiguration(parent),
        m_configuration(config)
    {
        // set up remaining screens list, add all ScreenXMLs
        QMap<int, ScreenXML *> remaining;
        foreach (ScreenXML * screen, m_configuration->screens()) {
            remaining.insert(screen->id(), screen);
        }

        QMap<int, QPoint> layout;
        bool changed;
        do {
            changed = false;
            QSet<ScreenXML *> added;
            // find the next remaining ScreenXML with known location relative to
            // an already known ScreenXML
            // starting with the first ScreenXML found at 0,0
            foreach (ScreenXML * screen, remaining) {
                QPoint pos;
                bool found = false;
                if (layout.empty()) {
                    pos = QPoint(0, 0);
                    found = true;
                } else if (layout.contains(screen->rightOf())) {
                    pos = layout[screen->rightOf()];
                    pos.rx()++;
                    found = true;
                } else if (layout.contains(screen->bottomOf())) {
                    pos = layout[screen->bottomOf()];
                    pos.ry()++;
                    found = true;
                }

                if (found) {
                    layout.insert(screen->id(), pos);
                    changed = true;
                    remaining.remove(screen->id());
                    added.insert(screen);
                    break;
                }
            }

            // for each added screenXml, move all of its left and above neighbours
            // to layout
            // left neighbours are placed 1 unit left of added
            // above neighbours are placed 1 unit above added
            //
            // note that added is changed inside this loop, so it probably moves all remaining
            // screenXmls out from remaining to layout, since the inner loop doesn't exit if they
            // are not neighbours.
            while (! added.empty()) {
                QSet<ScreenXML *>::iterator i = added.begin();

                while (i != added.end()) {
                    // locate its left neighbour
                    ScreenXML * s = *i;
                    if (remaining.contains(s->rightOf()) &&
                            /* this seems like the wrong place to perform the following
                               consistency check since screenxmls are always removed from
                               remaining when added to layout */
                            ! layout.contains(s->rightOf())) {
                        ScreenXML * toAdd = remaining[s->rightOf()];
                        QPoint pos = QPoint(layout[s->id()]);
                        pos.rx()--;

                        layout.insert(toAdd->id(), pos);
                        added.insert(toAdd);
                        remaining.remove(toAdd->id());
                        break;
                    }
                    // locate its above neighbour
                    if (remaining.contains(s->bottomOf()) &&
                            /* as above */
                            ! layout.contains(s->bottomOf())) {
                        ScreenXML * toAdd = remaining[s->bottomOf()];
                        QPoint pos = QPoint(layout[s->id()]);
                        pos.ry()--;

                        layout.insert(toAdd->id(), pos);
                        added.insert(toAdd);
                        remaining.remove(toAdd->id());
                        break;
                    }
                    i = added.erase(i);
                    // WILL well this is fucked... only exits if remaining contains a rightOf or
                    // bottomOf relative screen. what if screens are not contiguous or relatively
                    // located.
                    //++i;
                }
            }
        } while (changed);

        if (! remaining.empty()) {
            //kDebug() << "invalid configuration (remaining):" << name() << remaining;
            INVALID_CONFIGURATION("remaining screens")
            layout.clear();
        }

        Configurations::translateOrigin(layout);
        m_layout = layout;
    }

    ConfigurationXML * XMLConfiguration::configuration() const
    {
        return m_configuration;
    }

    QString XMLConfiguration::name() const
    {
        return m_configuration->name();
    }

    bool XMLConfiguration::isModifiable() const
    {
        return m_configuration->modifiable();
    }

    bool XMLConfiguration::isActivated() const
    {
        return this == m_parent->activeConfiguration();
    }

    void XMLConfiguration::activate()
    {
        emit configurationActivated(this);
    }

    void XMLConfiguration::setLayout(const QMap<int, QPoint> & layout) {
        m_layout = layout;
    }

    int XMLConfiguration::primaryScreen() const
    {
        return m_configuration->primaryScreen();
    }

    QMap<int, QPoint> XMLConfiguration::layout() const
    {
        return m_layout;
    }

} // namespace Kephal
// vim: sw=4 sts=4 et tw=100
