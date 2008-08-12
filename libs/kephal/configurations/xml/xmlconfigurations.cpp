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
#include "outputs/outputs.h"

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
    
    bool XMLConfiguration::modifiable()
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
            factory->save(m_configXml, m_configPath);
        }
        delete factory;
        
        qDebug() << "configurations.configurations.size =>" << m_configXml->configurations()->size();
        QList<ConfigurationXML *> * configs = m_configXml->configurations();
        for (int i = 0; i < configs->size(); i++) {
            ConfigurationXML * config = (* configs)[i];
            qDebug() << "configurations.configurations[" << i << "].name =>" << config->name();
            qDebug() << "configurations.configurations[" << i << "].modifiable =>" << config->modifiable();
            
            XMLConfiguration * c = new XMLConfiguration(this, config);
            m_configurations.insert(config->name(), c);
            connect(c, SIGNAL(activate(XMLConfiguration *)), this, SLOT(activate(XMLConfiguration *)));
            
            int j = 0;
            foreach (kephal::ScreenXML * screen, * (config->screens())) {
                qDebug() << "configurations.configurations[" << i << "].screens[" << j << "].id =>" << screen->id();
                qDebug() << "configurations.configurations[" << i << "].screens[" << j << "].privacy =>" << screen->privacy();
                ++j;
            }
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
    
    void XMLConfigurations::activate(XMLConfiguration * configuration) {
        qDebug() << "activate configuration:" << configuration->name();
        if (configuration == m_activeConfiguration) {
            return;
        }
        
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
            
            ConfigurationsXMLFactory * factory = new ConfigurationsXMLFactory();
            factory->save(m_configXml, m_configPath);
            
            m_currentOutputs = known;
            m_currentOutputsKnown = true;
        }
        
        QMap<int, QSize> screenSizes;
        foreach (ScreenXML * screen, * (configuration->configuration()->screens())) {
            screenSizes.insert(screen->id(), QSize());
        }
        
        foreach (OutputXML * output, * (m_currentOutputs->outputs())) {
            if (output->screen() < 0) {
                continue;
            }
            
            if (! screenSizes.contains(output->screen())) {
                qDebug() << "outputs and configuration dont match";
                return;
            }
            screenSizes[output->screen()] = screenSizes[output->screen()].expandedTo(QSize(output->width(), output->height()));
        }
        
        QMap<int, QRect> screens;
        QPoint origin;
        QMap<int, ScreenXML *> remaining;
        foreach (ScreenXML * screen, * (configuration->configuration()->screens())) {
            remaining.insert(screen->id(), screen);
        }
        
        bool changed;
        do {
            changed = false;
            QSet<ScreenXML *> added;
            foreach (ScreenXML * screen, remaining) {
                QPoint pos;
                bool found = false;
                if (screens.empty()) {
                    pos = QPoint(0, 0);
                    found = true;
                } else if (screens.contains(screen->rightOf())) {
                    pos = screens[screen->rightOf()].topRight();
                    pos.rx()++;
                    found = true;
                } else if (screens.contains(screen->bottomOf())) {
                    pos = screens[screen->bottomOf()].bottomLeft();
                    pos.ry()++;
                    found = true;
                }
                
                if (found) {
                    screens.insert(screen->id(), QRect(pos, screenSizes[screen->id()]));
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
                    if (remaining.contains(s->rightOf()) && ! screens.contains(s->rightOf())) {
                        ScreenXML * toAdd = remaining[s->rightOf()];
                        QPoint pos = screens[s->id()].topLeft();
                        pos.rx() -= screenSizes[toAdd->id()].width();
                        if (pos.x() < origin.x()) {
                            origin.setX(pos.x());
                        }
                        
                        screens.insert(toAdd->id(), QRect(pos, screenSizes[toAdd->id()]));
                        added.insert(toAdd);
                        remaining.remove(toAdd->id());
                        break;
                    }
                    if (remaining.contains(s->bottomOf()) && ! screens.contains(s->bottomOf())) {
                        ScreenXML * toAdd = remaining[s->bottomOf()];
                        QPoint pos = screens[s->id()].topLeft();
                        pos.ry() -= screenSizes[toAdd->id()].height();
                        if (pos.y() < origin.y()) {
                            origin.setY(pos.y());
                        }
                        
                        screens.insert(toAdd->id(), QRect(pos, screenSizes[toAdd->id()]));
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
            qDebug() << "invalid configuration (remaining):" << configuration->name() << remaining;
            return;
        }
        
        for (QMap<int, QRect>::const_iterator i = screens.constBegin(); i != screens.constEnd(); ++i) {
            for (QMap<int, QRect>::const_iterator j = (i + 1); j != screens.constEnd(); ++j) {
                if (i.value().intersects(j.value())) {
                    qDebug() << "invalid configuration (overlapping):" << configuration->name() << screens;
                    return;
                }
            }
        }
        
        QMap<QString, Output *> outputMap;
        QList<Output *> outputs = Outputs::instance()->outputs();
        foreach (Output * output, outputs) {
            outputMap.insert(output->id(), output);
        }
        
        QMap<Output *, QRect> layout;
        for (QMap<int, QRect>::const_iterator i = screens.constBegin(); i != screens.constEnd(); ++i) {
            foreach (OutputXML * output, * (m_currentOutputs->outputs())) {
                if (output->screen() == i.key()) {
                    if (! outputMap.contains(output->name())) {
                        qDebug() << "missing output:" << output->name();
                        return;
                    }
                    layout.insert(outputMap[output->name()], i.value());
                }
            }
        }
        
        qDebug() << "layout:" << layout;
        Outputs::instance()->activateLayout(layout);
        
        m_activeConfiguration = configuration;
    }
    
    Configuration * XMLConfigurations::activeConfiguration() {
        return m_activeConfiguration;
    }
    
}

