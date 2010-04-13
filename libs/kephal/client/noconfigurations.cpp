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


#include "noconfigurations.h"


namespace Kephal {

    SimpleConfiguration::SimpleConfiguration(NoConfigurations * parent)
        : Configuration(parent)
    {
    }

    QString SimpleConfiguration::name() {
        return "simple";
    }

    bool SimpleConfiguration::isModifiable() {
        return false;
    }

    bool SimpleConfiguration::isActivated() {
        return true;
    }

    QMap<int, QPoint> SimpleConfiguration::layout() {
        return QMap<int, QPoint>();
    }

    void SimpleConfiguration::activate() {
    }

    int SimpleConfiguration::primaryScreen() {
        return 0;
    }



    NoConfigurations::NoConfigurations(QObject * parent)
        : Configurations(parent)
    {
        m_config = new SimpleConfiguration(this);
    }

    QMap<QString, Configuration *> NoConfigurations::configurations() {
        QMap<QString, Configuration *> result;
        result.insert(m_config->name(), m_config);
        return result;
    }

    Configuration * NoConfigurations::findConfiguration() {
        return m_config;
    }

    Configuration * NoConfigurations::activeConfiguration() {
        return m_config;
    }

    QList<Configuration *> NoConfigurations::alternateConfigurations() {
        return QList<Configuration *>();
    }

    QList<QPoint> NoConfigurations::possiblePositions(const Output * output) {
        Q_UNUSED(output)
        return QList<QPoint>();
    }

    bool NoConfigurations::move(Output * output, const QPoint & position) {
        Q_UNUSED(output)
        Q_UNUSED(position)
        return false;
    }

    bool NoConfigurations::resize(Output * output, const QSize & size) {
        Q_UNUSED(output)
        Q_UNUSED(size)
        return false;
    }

    int NoConfigurations::screen(Output * output) {
        Q_UNUSED(output)
        return -1;
    }

    void NoConfigurations::applyOutputSettings() {
    }

    bool NoConfigurations::rotate(Output * output, Rotation rotation) {
        Q_UNUSED(output)
        Q_UNUSED(rotation)
        return false;
    }

    bool NoConfigurations::changeRate(Output * output, float rate) {
        Q_UNUSED(output)
        Q_UNUSED(rate)
        return false;
    }

    bool NoConfigurations::reflectX(Output * output, bool reflect) {
        Q_UNUSED(output)
        Q_UNUSED(reflect)
        return false;
    }

    bool NoConfigurations::reflectY(Output * output, bool reflect) {
        Q_UNUSED(output)
        Q_UNUSED(reflect)
        return false;
    }

    void NoConfigurations::setPolling(bool polling) {
        Q_UNUSED(polling)
    }

    bool NoConfigurations::polling() {
        return false;
    }

    void NoConfigurations::confirm() {
    }

    void NoConfigurations::revert() {
    }

}


