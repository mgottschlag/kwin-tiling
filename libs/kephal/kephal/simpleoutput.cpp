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


#include "simpleoutput.h"


namespace Kephal {

    SimpleOutput::SimpleOutput(QObject * parent, QString id, QSize size, QPoint position, bool connected, bool activated)
        : Output(parent)
    {
        m_id = id;
        m_size = size;
        m_position = position;
        m_connected = connected;
        m_activated = activated;
    }

    SimpleOutput::SimpleOutput(QObject * parent)
        : Output(parent),
        m_size(0, 0),
        m_position(0, 0),
        m_connected(false),
        m_activated(false)
    {
    }

    SimpleOutput::SimpleOutput(QObject * parent, Output * output)
        : Output(parent)
    {
        m_id = output->id();
        m_size = output->size();
        m_position = output->position();
        m_connected = output->isConnected();
        m_activated = output->isActivated();
    }


    QString SimpleOutput::id() const
    {
        return m_id;
    }

    QSize SimpleOutput::size() const {
        return m_size;
    }

    QList<QSize> SimpleOutput::availableSizes() const {
        if (m_availableSizes.empty()) {
            QList<QSize> result;
            result.append(size());
            return result;
        }
        return m_availableSizes;
    }

    QPoint SimpleOutput::position() const {
        return m_position;
    }

#if 0
    void SimpleOutput::_setId(const QString & id) {
        m_id = id;
    }

    void SimpleOutput::_setSize(const QSize & size) {
        m_size = size;
    }

    void SimpleOutput::_setAvailableSizes(const QList<QSize> & sizes) {
        m_availableSizes = sizes;
    }

    void SimpleOutput::_setPosition(const QPoint & position) {
        m_position = position;
    }

    void SimpleOutput::_setConnected(bool connected) {
        m_connected = connected;
    }

    void SimpleOutput::_setActivated(bool activated) {
        m_activated = activated;
    }

    bool SimpleOutput::isConnected() {
        return m_connected;
    }

    bool SimpleOutput::isActivated() {
        return m_connected && m_activated;
    }

    void SimpleOutput::_setVendor(const QString & vendor) {
        m_vendor = vendor;
    }

    QString SimpleOutput::vendor() {
        return m_vendor;
    }

    void SimpleOutput::_setProductId(int productId) {
        m_productId = productId;
    }

    int SimpleOutput::productId() {
        return m_productId;
    }

    void SimpleOutput::_setSerialNumber(unsigned int serialNumber) {
        m_serialNumber = serialNumber;
    }

    unsigned int SimpleOutput::serialNumber() {
        return m_serialNumber;
    }

    QSize SimpleOutput::preferredSize() {
        return m_preferredSize;
    }

    void SimpleOutput::_setPreferredSize(const QSize & size) {
        m_preferredSize = size;
    }

    void SimpleOutput::_setRotation(Rotation rotation) {
        m_rotation = rotation;
    }

    void SimpleOutput::_setReflectX(bool reflect) {
        m_reflectX = reflect;
    }

    void SimpleOutput::_setReflectY(bool reflect) {
        m_reflectY = reflect;
    }

    void SimpleOutput::_setRate(float rate) {
        m_rate = rate;
    }

    void SimpleOutput::_setAvailableRates(const QList<float> & rates) {
        m_rates = rates;
    }
#endif
    Rotation SimpleOutput::rotation() const
    {
        return m_rotation;
    }

    bool SimpleOutput::xReflected() const
    {
        return m_reflectX;
    }

    bool SimpleOutput::yReflected() const
    {
        return m_reflectY;
    }

    float SimpleOutput::rate() const
    {
        return m_rate;
    }

    QList<float> SimpleOutput::availableRates() const
    {
        return m_rates;
    }

}
