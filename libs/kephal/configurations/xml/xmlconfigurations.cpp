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


#include "xmlconfigurations.h"

#include "../../xml/configurations_xml.h"
#include "outputs/backendoutputs.h"
#include "outputs/simpleoutput.h"
#include "screens/screens.h"

#include <QDir>


namespace kephal {

    XMLConfiguration::XMLConfiguration(XMLConfigurations * parent, ConfigurationXML * config)
        : Configuration(parent),
        m_configuration(config)
    {
    }
            
    ConfigurationXML * XMLConfiguration::configuration()
    {
        return m_configuration;
    }
    
    QString XMLConfiguration::name()
    {
        return m_configuration->name();
    }
    
    bool XMLConfiguration::isModifiable()
    {
        return m_configuration->modifiable();
    }
    
    bool XMLConfiguration::isActivated()
    {
        return this == m_parent->activeConfiguration();
    }
    
    void XMLConfiguration::activate()
    {
        emit activate(this);
    }
    
    void XMLConfiguration::setLayout(const QMap<int, QPoint> & layout) {
    }
    
    int XMLConfiguration::primaryScreen() {
        return m_configuration->primaryScreen();
    }
    
    QMap<int, QPoint> XMLConfiguration::layout() {
        if (! m_layout.empty()) {
            return m_layout;
        }
        
        QMap<int, ScreenXML *> remaining;
        foreach (ScreenXML * screen, * (m_configuration->screens())) {
            remaining.insert(screen->id(), screen);
        }
        
        QMap<int, QPoint> layout;
        bool changed;
        do {
            changed = false;
            QSet<ScreenXML *> added;
            foreach (ScreenXML * screen, remaining) {
                QPoint pos;
                bool found = false;
                if (layout.empty()) {
                    pos = QPoint(0, 0);
                    found = true;
                } else if (layout.contains(screen->rightOf())) {
                    pos = QPoint(layout[screen->rightOf()]);
                    pos.rx()++;
                    found = true;
                } else if (layout.contains(screen->bottomOf())) {
                    pos = QPoint(layout[screen->bottomOf()]);
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
            
            while (! added.empty()) {
                QSet<ScreenXML *>::iterator i = added.begin();
                while (i != added.end()) {
                    ScreenXML * s = *i;
                    if (remaining.contains(s->rightOf()) && ! layout.contains(s->rightOf())) {
                        ScreenXML * toAdd = remaining[s->rightOf()];
                        QPoint pos = QPoint(layout[s->id()]);
                        pos.rx()--;
                        
                        layout.insert(toAdd->id(), pos);
                        added.insert(toAdd);
                        remaining.remove(toAdd->id());
                        break;
                    }
                    if (remaining.contains(s->bottomOf()) && ! layout.contains(s->bottomOf())) {
                        ScreenXML * toAdd = remaining[s->bottomOf()];
                        QPoint pos = QPoint(layout[s->id()]);
                        pos.ry()--;
                        
                        layout.insert(toAdd->id(), pos);
                        added.insert(toAdd);
                        remaining.remove(toAdd->id());
                        break;
                    }
                    i = added.erase(i);
                    //++i;
                }
            }
        } while (changed);
        
        if (! remaining.empty()) {
            qDebug() << "invalid configuration (remaining):" << name() << remaining;
            layout.clear();
        }
        
        Configurations::translateOrigin(layout);
        m_layout = layout;
        
        return layout;
    }


    XMLConfigurations::XMLConfigurations(QObject * parent)
        : Configurations(parent),
        m_activeConfiguration(0),
        m_currentOutputs(0),
        m_currentOutputsKnown(false)
    {
        QDir dir = QDir::home();
        dir.cd(".local");
        m_configPath = dir.filePath("screen-configurations.xml");
        
        init();
    }
    
    QMap<QString, Configuration *> XMLConfigurations::configurations()
    {
        QMap<QString, Configuration *> result;
        for (QMap<QString, XMLConfiguration *>::const_iterator i = m_configurations.constBegin();
                i != m_configurations.constEnd(); ++i) {
            result.insert(i.key(), i.value());
        }
        
        return result;
    }
    
    void XMLConfigurations::init()
    {
        ConfigurationsXMLFactory * factory = new ConfigurationsXMLFactory();
        m_configXml = (ConfigurationsXML *) factory->load(m_configPath);
        delete factory;
        
        if (! m_configXml) {
            m_configXml = new ConfigurationsXML();
            
            /**
             * Create default single layout
             */
            ConfigurationXML * config = new ConfigurationXML();
            config->setParent(m_configXml);
            m_configXml->configurations()->append(config);
            
            config->setName("single");
            config->setModifiable(false);
            
            ScreenXML * screen = new ScreenXML();
            screen->setParent(config);
            config->screens()->append(screen);
            
            screen->setId(0);
            screen->setPrivacy(false);


            /**
             * Create default extended-right layout
             */
            config = new ConfigurationXML();
            config->setParent(m_configXml);
            m_configXml->configurations()->append(config);
            
            config->setName("extended-right");
            config->setModifiable(false);
            
            screen = new ScreenXML();
            screen->setParent(config);
            config->screens()->append(screen);
            
            screen->setId(0);
            screen->setPrivacy(false);
        
            screen = new ScreenXML();
            screen->setParent(config);
            config->screens()->append(screen);
            
            screen->setId(1);
            screen->setPrivacy(false);
            screen->setRightOf(0);
            
            
            /**
             * Create default extended-left layout
             */
            config = new ConfigurationXML();
            config->setParent(m_configXml);
            m_configXml->configurations()->append(config);
            
            config->setName("extended-left");
            config->setModifiable(false);
            
            screen = new ScreenXML();
            screen->setParent(config);
            config->screens()->append(screen);
            
            screen->setId(0);
            screen->setPrivacy(false);
            screen->setRightOf(1);
        
            screen = new ScreenXML();
            screen->setParent(config);
            config->screens()->append(screen);
            
            screen->setId(1);
            screen->setPrivacy(false);
            
            
            /**
             * Create outputs section for single output
             */
            OutputsXML * outputs = new OutputsXML();
            outputs->setParent(m_configXml);
            m_configXml->outputs()->append(outputs);
            
            outputs->setConfiguration("single");
            
            OutputXML * output = new OutputXML();
            output->setParent(outputs);
            outputs->outputs()->append(output);
            
            output->setName("*");
            output->setScreen(0);
            output->setVendor("*");
            

            /**
             * Create outputs section for 2 screens
             */
            outputs = new OutputsXML();
            outputs->setParent(m_configXml);
            m_configXml->outputs()->append(outputs);
            
            outputs->setConfiguration("extended-right");
            
            output = new OutputXML();
            output->setParent(outputs);
            outputs->outputs()->append(output);
            
            output->setName("*");
            output->setScreen(0);
            output->setVendor("*");
            
            output = new OutputXML();
            output->setParent(outputs);
            outputs->outputs()->append(output);
            
            output->setName("*");
            output->setScreen(1);
            output->setVendor("*");
            

            /**
             * Save the default xml
             */
            saveXml();
        }
        
        QList<ConfigurationXML *> * configs = m_configXml->configurations();
        for (int i = 0; i < configs->size(); i++) {
            ConfigurationXML * config = (* configs)[i];
            
            XMLConfiguration * c = new XMLConfiguration(this, config);
            m_configurations.insert(config->name(), c);
            connect(c, SIGNAL(activate(XMLConfiguration *)), this, SLOT(activate(XMLConfiguration *)));
        }
        
        findOutputs();
    }
    
    Configuration * XMLConfigurations::findConfiguration()
    {
        qDebug() << "looking for a matching configuration...";
        findOutputs();
        if (! m_currentOutputs) {
            return 0;
        }
        qDebug() << "found outputs, known:" << m_currentOutputsKnown;
        
        XMLConfiguration * config = m_configurations[m_currentOutputs->configuration()];
        if (! config) {
            qDebug() << "config" << m_currentOutputs->configuration() << "does not exist!!";
            return 0;
        }
        
        return config;
    }
    
    void XMLConfigurations::findOutputs()
    {
        m_currentOutputsKnown = true;
        m_currentOutputs = findKnownOutputs();
        if (! m_currentOutputs) {
            m_currentOutputsKnown = false;
            m_currentOutputs = findBestOutputs();
        }
    }
    
    OutputsXML * XMLConfigurations::findKnownOutputs()
    {
        QList<Output *> currentOutputs = Outputs::instance()->outputs();
        int connected = 0;
        foreach (Output * output, currentOutputs) {
            if (output->isConnected()) {
                ++connected;
            }
        }
        
        foreach (OutputsXML * knownOutputs, * (m_configXml->outputs())) {
            if (knownOutputs->outputs()->size() != connected) {
                continue;
            }
            
            bool matchedAll = true;
            foreach (Output * current, currentOutputs) {
                if (! current->isConnected()) {
                    continue;
                }

                bool matched = false;
                foreach (OutputXML * known, * (knownOutputs->outputs())) {
                    if (known->name() != current->id()) {
                        continue;
                    }
                    
                    if ((current->vendor() == known->vendor())
                            && (current->productId() == known->product())
                            && (current->serialNumber() == known->serial())) {
                        matched = true;
                        break;
                    }
                }
                
                if (! matched) {
                    matchedAll = false;
                    break;
                }
            }
            
            if (matchedAll) {
                return knownOutputs;
            }
        }
        
        return 0;
    }
    
    OutputsXML * XMLConfigurations::findBestOutputs()
    {
        QList<Output *> currentOutputs = Outputs::instance()->outputs();
        int connected = 0;
        foreach (Output * output, currentOutputs) {
            if (output->isConnected()) {
                ++connected;
            }
        }
        
        qDebug() << "connected:" << connected;
        
        qreal scoreAllMax = 0.01;
        OutputsXML * knownAllMax = 0;
        foreach (OutputsXML * knownOutputs, * (m_configXml->outputs())) {
            if (knownOutputs->outputs()->size() != connected) {
                continue;
            }
            
            qreal scoreAll = 1;
            QSet<OutputXML *> knownTaken;
            foreach (Output * current, currentOutputs) {
                if (! current->isConnected()) {
                    continue;
                }
                
                qDebug() << "looking for current" << current->id();
                
                qreal scoreMax = 0.01;
                OutputXML * knownMax = 0;
                foreach (OutputXML * known, * (knownOutputs->outputs())) {
                    if (knownTaken.contains(known)) {
                        continue;
                    }
                    
                    qreal score = 1;
                    score *= match(known->name(), current->id());
                    score *= match(known->vendor(), current->vendor());
                    score *= match(known->product(), current->productId());
                    
                    qDebug() << "known" << known->name() << "has score:" << score;
                    if (score > scoreMax) {
                        knownMax = known;
                        scoreMax = score;
                    }
                }
                
                if (knownMax) {
                    scoreAll *= scoreMax;
                    knownTaken.insert(knownMax);
                    knownMax->setActualOutput(current->id());
                } else {
                    scoreAll = 0;
                    break;
                }
            }
            
            if (scoreAll > scoreAllMax) {
                scoreAllMax = scoreAll;
                knownAllMax = knownOutputs;
            }
        }
        
        return knownAllMax;
    }
    
    qreal XMLConfigurations::match(QString known, QString current) {
        if (known == current) {
            return 1;
        } else if (known == "*") {
            return 0.5;
        } else {
            return 0;
        }
    }
    
    qreal XMLConfigurations::match(int known, int current) {
        if (known == current) {
            return 1;
        } else if (known == -1) {
            return 0.5;
        } else {
            return 0;
        }
    }
    
    QList<Configuration *> XMLConfigurations::alternateConfigurations() {
        QList<Configuration *> configs;
        foreach (XMLConfiguration * config, m_configurations) {
            if (config->layout().size() <= m_currentOutputs->outputs()->size()) {
                configs.append(config);
            }
        }
        
        return configs;
    }
    
    QList<XMLConfiguration *> XMLConfigurations::equivalentConfigurations(int numScreens) {
        qDebug() << "looking for equivalent configurations with" << numScreens << "screens";
        
        QList<XMLConfiguration *> result;
        foreach (XMLConfiguration * config, m_configurations) {
            if ((! config->isModifiable()) && (config->layout().size() == numScreens)) {
                qDebug() << "found:" << config->name();
                result.append(config);
            }
        }
        
        return result;
    }
    
    QList<QPoint> XMLConfigurations::possiblePositions(Output * output) {
        QList<QPoint> result;
        QSet<QPoint> unique;
        if (! output->isConnected()) {
            return result;
        }
        
        QMap<XMLConfiguration *, QPoint> positions;
        if (! m_activeConfiguration->isModifiable()) {
            positions = equivalentConfigurationsPositions(output);
            foreach (QPoint point, positions) {
                unique.insert(point);
            }
            
            positions = simpleConfigurationsPositions(output, true);
            foreach (QPoint point, positions) {
                unique.insert(point);
            }
        } else {
            positions = sameConfigurationsPositions(output, false);
            foreach (QPoint point, positions) {
                unique.insert(point);
            }
            
            positions = simpleConfigurationsPositions(output, false);
            foreach (QPoint point, positions) {
                unique.insert(point);
            }
        }
        
        foreach (QPoint p, unique) {
            result.append(p);
        }
        return result;
    }
    
    void XMLConfigurations::move(Output * output, const QPoint & position) {
        if (! output->isConnected()) {
            return;
        }
        if (position == output->position()) {
            return;
        }
        
        QMap<XMLConfiguration *, QPoint> positions;
        if (! m_activeConfiguration->isModifiable()) {
            positions = equivalentConfigurationsPositions(output);
            for (QMap<XMLConfiguration *, QPoint>::const_iterator i = positions.constBegin(); i != positions.constEnd(); ++i) {
                if (i.value() == position) {
                    activate(i.key());
                    return;
                }
            }
            
            positions = simpleConfigurationsPositions(output, true);
            foreach (QPoint point, positions) {
                qDebug() << "TODO: handle moving of output";
            }
        } else {
            positions = sameConfigurationsPositions(output, false);
            foreach (QPoint point, positions) {
                qDebug() << "TODO: handle moving of output";
            }
            
            positions = simpleConfigurationsPositions(output, false);
            foreach (QPoint point, positions) {
                qDebug() << "TODO: handle moving of output";
            }
        }
    }
    
    void XMLConfigurations::resize(Output * output, const QSize & size) {
        qDebug() << "XMLConfigurations::resize() called" << output->id() << size;
        if ((! output->isConnected()) || (! output->isActivated())) {
            return;
        }
        if (size == output->size()) {
            return;
        }
        
        SimpleOutput * resized = new SimpleOutput(this, output);
        resized->_setSize(size);
        QMap<Output *, int> outputScreens;
        foreach (Output * o, Outputs::instance()->outputs()) {
            if (o == output) {
                outputScreens.insert(resized, output->screen()->id());
            } else if (o->isActivated()) {
                outputScreens.insert(o, o->screen()->id());
            }
        }
        
        QMap<int, QRect> layout = m_activeConfiguration->realLayout(outputScreens);
        activateLayout(layout, outputScreens);
        delete resized;
    }
    
    QMap<XMLConfiguration *, QMap<int, QPoint> > XMLConfigurations::matchingConfigurationsLayouts(const QMap<int, QPoint> & currentLayout, int removedOutputs) {
        //qDebug() << "searching matching layouts for" << currentLayout;
        QMap<XMLConfiguration *, QMap<int, QPoint> > result;
        QList<XMLConfiguration *> configurations = equivalentConfigurations(currentLayout.size() + removedOutputs);
        foreach (XMLConfiguration * configuration, configurations) {
            QMap<int, QPoint> layout = configuration->layout();
            QMap<int, int> match = matchLayouts(currentLayout, layout);
            if (! match.empty()) {
                result.insert(configuration, layout);
            }
        }
        
        return result;
    }
    
    QMap<int, int> XMLConfigurations::matchLayouts(const QMap<int, QPoint> & currentLayout, QMap<int, QPoint> layout) {
        int removed = layout.size() - currentLayout.size();
        QList<int> indexes = layout.keys();
        QPoint origin = currentLayout.begin().value();
        QMap<int, int> result;
        foreach (int i, indexes) {
            translateOrigin(layout, layout[i] - origin);
            for (QMap<int, QPoint>::const_iterator j = currentLayout.constBegin(); j != currentLayout.constEnd(); ++j) {
                bool found = false;
                for (QMap<int, QPoint>::iterator k = layout.begin(); k != layout.end(); ++k) {
                    if (j.value() == k.value()) {
                        found = true;
                        result.insert(j.key(), k.key());
                        layout.erase(k);
                        break;
                    }
                }
                if (! found) {
                    result.clear();
                }
            }
            if (! result.empty()) {
                int j = -1;
                for (QMap<int, QPoint>::const_iterator k = layout.constBegin(); k != layout.constEnd(); ++k) {
                    result.insert(j, k.key());
                    --j;
                }
                return result;
            }
        }
        
        return result;
    }
    
    QMap<int, QRect> XMLConfigurations::calcMatchingLayout(const QMap<int, QPoint> & currentLayout, XMLConfiguration * configuration, QMap<int, QPoint> layout, Output * output, int * outputScreen) {
        QMap<int, int> match = matchLayouts(currentLayout, layout);
        QMap<Output *, int> outputs;
        Output * add = (match.contains(-1) ? output : 0);
        Output * remove = (add ? 0 : output);
        foreach (Output * o, Outputs::instance()->outputs()) {
            Screen * screen = o->screen();
            if (remove && (remove == o)) {
                outputs.insert(o, -1);
                remove = 0;
            } else if (screen && match.contains(screen->id())) {
                outputs.insert(o, match[screen->id()]);
            } else if (add && (add == o)) {
                outputs.insert(o, match[-1]);
                add = 0;
                if (outputScreen) {
                    *outputScreen = match[-1];
                }
            }
        }
        
        QMap<int, QRect> realLayout = configuration->realLayout(layout, outputs);
        translateToOther(realLayout, output, match);
        
        return realLayout;
    }
    
    void XMLConfigurations::translateToOther(QMap<int, QRect> & layout, Output * output, QMap<int, int> match) {
        foreach (Output * o, Outputs::instance()->outputs()) {
            if (o == output) {
                continue;
            }
            
            Screen * screen = o->screen();
            if (screen && (match.empty() || match.contains(screen->id()))) {
                QPoint offset = layout[match.empty() ? screen->id() : match[screen->id()]].topLeft() - o->position();
                translateOrigin(layout, offset);
                break;
            }
        }
    }
    
    QMap<XMLConfiguration *, QPoint> XMLConfigurations::equivalentConfigurationsPositions(Output * output) {
        bool cloned = false;
        if (! output->isActivated()) {
            cloned = true;
        } else {
            foreach (Output * o, Outputs::instance()->outputs()) {
                if (o == output) {
                    continue;
                }
                if (o->screen() == output->screen()) {
                    cloned = true;
                    break;
                }
            }
        }
        
        QMap<XMLConfiguration *, QPoint> positions;
        QMap<int, QPoint> currentLayout = m_activeConfiguration->layout();
        QMap<XMLConfiguration *, QMap<int, QPoint> > layouts;
        QMap<int, QPoint> cloneLayout;
        XMLConfiguration * cloneConfig = 0;
        if (! cloned) {
            currentLayout.remove(output->screen()->id());
            translateOrigin(currentLayout);
            layouts = matchingConfigurationsLayouts(currentLayout, 0);
            if (! layouts.empty()) {
                QMap<XMLConfiguration *, QMap<int, QPoint> >::const_iterator i = layouts.constBegin();
                cloneLayout = i.value();
                cloneConfig = i.key();
            }
        } else {
            cloneLayout = currentLayout;
            cloneConfig = m_activeConfiguration;
        }
        
        if (cloneConfig) {
            QMap<int, QRect> layout = calcMatchingLayout(currentLayout, cloneConfig, cloneLayout, output);
            foreach (QRect geom, layout) {
                positions.insertMulti(cloneConfig, geom.topLeft());
            }
        }
        
        layouts = matchingConfigurationsLayouts(currentLayout, 1);
        for (QMap<XMLConfiguration *, QMap<int, QPoint> >::const_iterator i = layouts.constBegin(); i != layouts.constEnd(); ++i) {
            //qDebug() << "matching layout:" << i.value();
            int outputScreen = -1;
            QMap<int, QRect> layout = calcMatchingLayout(currentLayout, i.key(), i.value(), output, & outputScreen);
            if (layout.contains(outputScreen)) {
                positions.insertMulti(i.key(), layout[outputScreen].topLeft());
            }
        }
        
        for (QMap<XMLConfiguration *, QPoint>::iterator i = positions.begin(); i != positions.end();) {
            /*if (i.value() == output->position()) {
                i = positions.erase(i);
            } else {*/
                QPoint pos = i.value();
                for (QMap<XMLConfiguration *, QPoint>::iterator j = i + 1; j != positions.end();) {
                    if (j.value() == pos) {
                        j = positions.erase(j);
                    } else {
                        ++j;
                    }
                }
                ++i;
            //}
        }
        
        return positions;
    }
    
    QMap<XMLConfiguration *, QPoint> XMLConfigurations::simpleConfigurationsPositions(Output * output, bool sameCount) {
        Screen * screen = output->screen();
        bool cloned = false;
        if (! output->isActivated()) {
            cloned = true;
        } else {
            int count = 0;
            foreach (Output * o, Outputs::instance()->outputs()) {
                if (o->isActivated()) {
                    ++count;
                }
                if (o == output) {
                    continue;
                }
                if (o->screen() == screen) {
                    cloned = true;
                    break;
                }
            }
            
            if (count <= 1) {
                return QMap<XMLConfiguration *, QPoint>();
            }
        }
        
        QMap<XMLConfiguration *, QPoint> positions;
        QMap<int, QPoint> currentLayout = m_activeConfiguration->layout();
        QMap<XMLConfiguration *, QMap<int, QPoint> > layouts;
        QMap<int, QPoint> cloneLayout = currentLayout;
        XMLConfiguration * cloneConfig = 0;
        QMap<int, QPoint> noCloneLayout = currentLayout;
        XMLConfiguration * noCloneConfig = 0;
        
        if (! cloned) {
            cloneLayout = m_activeConfiguration->cloneLayout(screen->id());
            cloneConfig = simpleConfiguration(currentLayout.size() - 1);
            cloneConfig->setLayout(cloneLayout);
            
            if (sameCount) {
                noCloneConfig = simpleConfiguration(currentLayout.size());
            }
        } else {
            if (sameCount) {
                cloneConfig = m_activeConfiguration;
            }
            
            noCloneConfig = simpleConfiguration(currentLayout.size() + 1);
        }
        
        if (cloneConfig) {
            QMap<int, QRect> layout = calcMatchingLayout(currentLayout, cloneConfig, cloneLayout, output);
            foreach (QRect geom, layout) {
                positions.insertMulti(cloneConfig, geom.topLeft());
            }
        }
        
        if (noCloneConfig) {
            noCloneConfig->setLayout(noCloneLayout);
            int screenId = 0;
            if (cloned) {
                while (noCloneLayout.contains(screenId)) {
                    ++screenId;
                }
            } else {
                screenId = screen->id();
            }
            QSet<QPoint> possible = noCloneConfig->possiblePositions(screenId);
            
            //qDebug() << "trying" << possible << "as" << screenId << "in layout" << noCloneConfig->name() << noCloneLayout;
            
            QMap<Output *, int> outputIndexes;
            foreach (Output * o, Outputs::instance()->outputs()) {
                Screen * s = o->screen();
                outputIndexes.insert(o, (s ? s->id() : -1));
            }
            outputIndexes.insert(output, screenId);
            
            /*for (QMap<Output *, int>::const_iterator i = outputIndexes.constBegin(); i != outputIndexes.constEnd(); ++i) {
                qDebug() << "output:" << i.value() << i.key()->id();
            }*/
            
            foreach (QPoint p, possible) {
                noCloneLayout.insert(screenId, p);
                //qDebug() << "layout:" << noCloneLayout;
                QMap<int, QRect> layout = noCloneConfig->realLayout(noCloneLayout, outputIndexes);
                if (layout.contains(screenId)) {
                    translateToOther(layout, output);
                    //qDebug() << "results in:" << layout;
                    positions.insertMulti(noCloneConfig, layout[screenId].topLeft());
                }
            }
        }
        
        return positions;
    }
    
    QMap<XMLConfiguration *, QPoint> XMLConfigurations::sameConfigurationsPositions(Output * output, bool sameCount) {
        Screen * screen = output->screen();
        bool cloned = false;
        if (! output->isActivated()) {
            cloned = true;
        } else {
            foreach (Output * o, Outputs::instance()->outputs()) {
                if (o == output) {
                    continue;
                }
                if (o->screen() == screen) {
                    cloned = true;
                    break;
                }
            }
        }
        
        QMap<XMLConfiguration *, QPoint> positions;
        QMap<int, QPoint> currentLayout = m_activeConfiguration->layout();
        QMap<XMLConfiguration *, QMap<int, QPoint> > layouts;
        int screenId = (screen ? screen->id() : currentLayout.keys()[0]);
        
        QSet<QPoint> possible = (cloned ? m_activeConfiguration->positions() : m_activeConfiguration->possiblePositions(screenId));
        
        //qDebug() << "trying" << possible << "as" << screenId << "in layout" << m_activeConfiguration->name() << currentLayout;
        
        QMap<Output *, int> outputIndexes;
        foreach (Output * o, Outputs::instance()->outputs()) {
            Screen * s = o->screen();
            outputIndexes.insert(o, (s ? s->id() : -1));
        }
        outputIndexes.insert(output, screenId);
        
        /*for (QMap<Output *, int>::const_iterator i = outputIndexes.constBegin(); i != outputIndexes.constEnd(); ++i) {
            qDebug() << "output:" << i.value() << i.key()->id();
        }*/
        
        foreach (QPoint p, possible) {
            currentLayout.insert(screenId, p);
            //qDebug() << "layout:" << currentLayout;
            QMap<int, QRect> layout = m_activeConfiguration->realLayout(currentLayout, outputIndexes);
            if (layout.contains(screenId)) {
                translateToOther(layout, output);
                //qDebug() << "results in:" << layout;
                positions.insertMulti(m_activeConfiguration, layout[screenId].topLeft());
            }
        }
        
        return positions;
    }
    
    void XMLConfigurations::activate(XMLConfiguration * configuration) {
        qDebug() << "activate configuration:" << configuration->name();
        if (configuration == m_activeConfiguration) {
            return;
        }
        QMap<int, QPoint> layout = configuration->layout();
        
        //bool saveWhenDone = false;
        if (! m_currentOutputsKnown) {
            qDebug() << "saving xml for current outputs...";
            
            OutputsXML * known = new OutputsXML();
            known->setParent(m_configXml);
            m_configXml->outputs()->append(known);
            
            known->setConfiguration(configuration->name());

            QMap<QString, OutputXML *> currentMap;
            foreach (OutputXML * o, * (m_currentOutputs->outputs())) {
                currentMap.insert(o->actualOutput(), o);
            }
            
            QList<Output *> outputs = Outputs::instance()->outputs();
            foreach (Output * output, outputs) {
                if (! output->isConnected()) {
                    continue;
                }
                if (! currentMap.contains(output->id())) {
                    qDebug() << "m_currentOutputs not up to date!!";
                    return;
                }
                
                OutputXML * outputXml = new OutputXML();
                outputXml->setParent(known);
                known->outputs()->append(outputXml);
                
                outputXml->setName(output->id());
                outputXml->setScreen(currentMap[output->id()]->screen());
                outputXml->setVendor(output->vendor());
                outputXml->setProduct(output->productId());
                outputXml->setSerial(output->serialNumber());
                outputXml->setWidth(output->size().width());
                outputXml->setHeight(output->size().height());
            }
            
            m_currentOutputs = known;
            m_currentOutputsKnown = true;
        } else {
            m_currentOutputs->setConfiguration(configuration->name());
            matchOutputScreens(layout);
        }
        saveXml();
        
        QMap<Output *, int> outputScreens = currentOutputScreens(layout);
        QMap<int, QRect> screens = configuration->realLayout(layout, outputScreens);
        if (activateLayout(screens, outputScreens)) {
            m_activeConfiguration = configuration;
            emit configurationActivated(configuration);
        }
    }
    
    void XMLConfigurations::matchOutputScreens(const QMap<int, QPoint> & layout) {
        QMap<QString, int> outputScreens;
        QSet<int> screens;
        QSet<int> unknownScreens;
        QSet<int> takenScreens;
        QSet<QString> cloned;
        foreach (int screen, layout.keys()) {
            screens << screen;
        }
        
        if (screens.size() > m_currentOutputs->outputs()->size()) {
            qDebug() << "configuration and outputs dont match!";
        }
        
        foreach (OutputXML * output, * (m_currentOutputs->outputs())) {
            if (output->screen() >= 0) {
                if (screens.contains(output->screen())) {
                    outputScreens.insert(output->name(), output->screen());
                    if (takenScreens.contains(output->screen())) {
                        cloned << output->name();
                    }
                    takenScreens << output->screen();
                } else {
                    unknownScreens << output->screen();
                }
            }
        }
        
        foreach (int taken, outputScreens) {
            screens.remove(taken);
        }
        
        while (! (screens.empty() || unknownScreens.empty())) {
            int from = * unknownScreens.begin();
            unknownScreens.remove(from);
            int to = * screens.begin();
            screens.remove(to);
            
            foreach (OutputXML * output, * (m_currentOutputs->outputs())) {
                if (output->screen() == from) {
                    outputScreens.insert(output->name(), to);
                }
            }
        }
        
        while (! (screens.empty() || cloned.empty())) {
            QString o = * cloned.begin();
            cloned.remove(o);
            int to = * screens.begin();
            screens.remove(to);
            outputScreens.insert(o, to);
        }
        
        foreach (OutputXML * output, * (m_currentOutputs->outputs())) {
            if (outputScreens.contains(output->name())) {
                output->setScreen(outputScreens[output->name()]);
            } else {
                output->setScreen(-1);
            }
        }
    }
    
    QMap<Output *, int> XMLConfigurations::currentOutputScreens(const QMap<int, QPoint> & layout) {
        QMap<Output *, int> outputScreens;
        foreach (OutputXML * output, * (m_currentOutputs->outputs())) {
            if (output->screen() >= 0) {
                Output * o = Outputs::instance()->output(output->name());
                if (! o) {
                    qDebug() << "missing output:" << output->name();
                    outputScreens.clear();
                    return outputScreens;
                }
                outputScreens.insert(o, output->screen());
            }
        }
        return outputScreens;
    }
    
    bool XMLConfigurations::activateLayout(const QMap<int, QRect> & screensLayout, const QMap<Output *, int> & outputScreens) {
        if (screensLayout.empty()) {
            qDebug() << "layout is empty!!";
            return false;
        }
        
        QMap<Output *, QRect> layout;
        for (QMap<int, QRect>::const_iterator i = screensLayout.constBegin(); i != screensLayout.constEnd(); ++i) {
            for (QMap<Output *, int>::const_iterator j = outputScreens.constBegin(); j != outputScreens.constEnd(); ++j) {
                if (j.value() == i.key()) {
                    layout.insert(j.key(), QRect(i.value().topLeft(), j.key()->size()));
                }
            }
        }
        
        qDebug() << "layout:" << layout;
        BackendOutputs::instance()->activateLayout(layout);
        
        return true;
    }
    
    Configuration * XMLConfigurations::activeConfiguration() {
        return m_activeConfiguration;
    }
    
    XMLConfiguration * XMLConfigurations::simpleConfiguration(int numScreens) {
        QString name = "simple-" + QString::number(numScreens);
        if (m_configurations.contains(name)) {
            return m_configurations[name];
        }
        
        ConfigurationXML * config = new ConfigurationXML();
        config->setParent(m_configXml);
        m_configXml->configurations()->append(config);
        
        config->setName(name);
        config->setModifiable(true);
        
        for (int i = 0; i < numScreens; ++i) {
            ScreenXML * screen = new ScreenXML();
            screen->setParent(config);
            config->screens()->append(screen);
            
            screen->setId(i);
            screen->setPrivacy(false);
            screen->setRightOf(i - 1);
        }
        
        saveXml();
        
        m_configurations.insert(name, new XMLConfiguration(this, config));
        return m_configurations[name];
    }
    
    void XMLConfigurations::saveXml() {
        ConfigurationsXMLFactory * factory = new ConfigurationsXMLFactory();
        factory->save(m_configXml, m_configPath);
        delete factory;
    }

}

