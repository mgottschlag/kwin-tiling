/*
 *   Copyright 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kidenticongenerator.h"

#include <QHash>
#include <QPainter>
#include <QDebug>
#include <QCryptographicHash>
#include <Plasma/Svg>

KIdenticonGenerator * KIdenticonGenerator::m_instance = NULL;

KIdenticonGenerator * KIdenticonGenerator::self()
{
    if (!m_instance) {
        m_instance = new KIdenticonGenerator();
    }

    return m_instance;
}

KIdenticonGenerator::KIdenticonGenerator()
{
    // loading SVG
    m_shapes.setImagePath("widgets/identiconshapes");
    m_shapes.setContainsMultipleImages(true);
}

QPixmap KIdenticonGenerator::generate(int size, QString id)
{
    // qHash function doesn't give random enough results
    // and gives similar hashes for similar strings.

    QByteArray bytes = QCryptographicHash::hash(id.toUtf8(), QCryptographicHash::Md5);

    // Generating hash
    quint32 hash = 0;

    char * hashBytes = (char *) & hash;
    for (int i = 0; i < bytes.size(); i++) {
        // Using XOR for mixing the bytes because
        // it is fast and cryptographically safe
        // (more than enough for our use-case)
        hashBytes[i % 4] ^= bytes.at(i);
    }

    return generate(size, hash);
}

QPixmap KIdenticonGenerator::generate(int size, quint32 hash)
{
    // We are dividing the pixmap into 9 blocks - 3 x 3
    int blockSize = size / 3;

    // pulling parts of the hash
    quint32 tmp = hash;

    quint8 block[4];
    block[0] = tmp & 31; tmp >>= 5;
    block[1] = tmp & 31; tmp >>= 5;
    block[2] = tmp & 31; tmp >>= 5;

    // Painting alpha channel
    QPixmap pixmapAlpha(size, size);
    pixmapAlpha.fill(Qt::black);

    QPainter painterAlpha(& pixmapAlpha);

    QRectF rect(0, 0, blockSize + 0.5, blockSize + 0.5);

    for (int i = 0; i < 4; i++) {
        // Painting the corner item
        rect.moveTopLeft(QPoint(0, 0));
        m_shapes.paint(& painterAlpha, rect, "shape" + QString::number(block[0] + 1));

        // Painting side item
        rect.moveTopLeft(QPoint(blockSize, 0));
        m_shapes.paint(& painterAlpha, rect, "shape" + QString::number(block[1] + 1));

        // Rotating the canvas to paint other edges
        painterAlpha.translate(size, 0);
        painterAlpha.rotate(90);
    }

    // Painting center item
    rect.moveTopLeft(QPoint(blockSize, blockSize));
    m_shapes.paint(& painterAlpha, rect, "shape" + QString::number(block[2] + 1));

    painterAlpha.end();

    // Painting final pixmap
    QPixmap pixmapResult(size, size);

    // Color is chosen according to hash
    // TODO: use a color from the theme as a base (for value)
    QColor color;

    color.setHsv(hash % 359 + 1, 250, 200);
    pixmapResult.fill(color);

    QRadialGradient gradient(50, 50, 100);
    gradient.setColorAt(0, color.lighter());
    gradient.setColorAt(1, color.darker());

    QPainter resultPainter(& pixmapResult);
    resultPainter.fillRect(0, 0, size, size, gradient);

    resultPainter.end();

    pixmapResult.setAlphaChannel(pixmapAlpha);

    return pixmapResult;

}

