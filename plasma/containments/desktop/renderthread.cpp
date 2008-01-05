/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "renderthread.h"

#include <QPainter>
#include <KDebug>
#include <KSvgRenderer>

RenderThread::RenderThread(const QSize &size, float ratio)
: m_current_token(-1)
, m_size(size)
, m_ratio(ratio)
{
    m_abort = false;
    m_restart = false;
}

RenderThread::~RenderThread()
{
    {
        // abort computation
        QMutexLocker lock(&m_mutex);
        m_abort = true;
        m_condition.wakeOne();
    }
    
    wait();
}

void RenderThread::setSize(const QSize& size)
{
    QMutexLocker lock(&m_mutex);
    m_size = size;
}

int RenderThread::render(const QString &file,
                          const QColor &color,
                          Background::ResizeMethod method,
                          Qt::TransformationMode mode)
{
    int token;
    {
        QMutexLocker lock(&m_mutex);
        m_file = file;
        m_color = color;
        m_method = method;
        m_mode = mode;
        m_restart = true;
        token = ++m_current_token;
    }
    
    if (!isRunning()) {
        start();
    }
    else {
        m_condition.wakeOne();
    }
    
    return token;
}

void RenderThread::run()
{
    QString file;
    QColor color;
    QSize size;
    float ratio;
    Background::ResizeMethod method;
    Qt::TransformationMode mode;
    int token;
    
    forever {
        {
            QMutexLocker lock(&m_mutex);
            while (!m_restart && !m_abort) {
                m_condition.wait(&m_mutex);
            }
            if (m_abort) {
                return;
            }
            m_restart = false;
            
            // load all parameters in nonshared variables
            token = m_current_token;
            file = m_file;
            color = m_color;
            size = m_size;
            ratio = m_ratio;
            method = m_method;
            mode = m_mode;
        }
        
        QPoint pos(0, 0);
        bool tiled = false;
        bool scalable = file.endsWith("svg") || file.endsWith("svgz");
        QSize scaledSize;
        QImage img;
        
        // load nonscalable image
        if (!scalable) {
            img = QImage(file);
        }
        
        // set image size
        QSize imgSize;
        if (scalable) {
            // scalable: image can be of any size
            imgSize = size; 
        }
        else {
            // otherwise, use the natural size of the loaded image
            imgSize = img.size();
        }
        imgSize *= ratio;
        
        // set render parameters according to resize mode
        switch (method)
        {
        case Background::Scale:
            scaledSize = size;
            break;
        case Background::Center:
            scaledSize = imgSize;
            pos = QPoint((size.width() - scaledSize.width()) / 2,
                        (size.height() - scaledSize.height()) / 2);
            break;
        case Background::ScaleCrop: {
            float xratio = (float) size.width() / imgSize.width();
            float yratio = (float) size.height() / imgSize.height();
            if (xratio > yratio) {
                int width = size.width();
                int height = width * imgSize.height() / imgSize.width();
                scaledSize = QSize(width, height);
            }
            else {
                int height = size.height();
                int width = height * imgSize.width() / imgSize.height();
                scaledSize = QSize(width, height);
            }
            pos = QPoint((size.width() - scaledSize.width()) / 2,
                        (size.height() - scaledSize.height()) / 2);
            break;
        }
        case Background::Tiled:
            scaledSize = imgSize;
            tiled = true;
            break;
        case Background::CenterTiled:
            scaledSize = imgSize;
            pos = QPoint(
                -scaledSize.width() + 
                    ((size.width() - scaledSize.width()) / 2) % scaledSize.width(),
                -scaledSize.height() + 
                    ((size.height() - scaledSize.height()) / 2) % scaledSize.height());
            tiled = true;
            break;
        }
        
        QImage result(size, QImage::Format_ARGB32_Premultiplied);
        result.fill(color.rgb());
        
        QPainter p(&result);
        if (scalable) {
            // tiling is ignored for scalable wallpapers
            KSvgRenderer svg(file);
            if (m_restart) {
                continue;
            }
            svg.render(&p);
        }
        else {
            QImage scaled = img.scaled(scaledSize, Qt::IgnoreAspectRatio, mode);
            if (m_restart) {
                continue;
            }
            if (tiled) {
                for (int x = pos.x(); x < size.width(); x += scaledSize.width()) {
                    for (int y = pos.y(); y < size.height(); y += scaledSize.height()) {
                        p.drawImage(QPoint(x, y), scaled);
                        if (m_restart) {
                            goto endLoop;
                        }
                    }
                }
            }
            else {
                p.drawImage(pos, scaled);
            }
        }

        // signal we're done
        emit done(token, result);
        
        endLoop: continue;
    }
}
