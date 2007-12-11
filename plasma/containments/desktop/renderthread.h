/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include "backgroundpackage.h"
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

class RenderThread : public QThread
{
Q_OBJECT
public:
    RenderThread(const QSize &size, float ratio);
    virtual ~RenderThread();
    
    int render(const QString &file,
               const QColor &color,
               Background::ResizeMethod method,
               Qt::TransformationMode mode);
               
    void setSize(const QSize &size);
protected:
    virtual void run();
private:
    QMutex m_mutex; // to protect parameters
    QWaitCondition m_condition;
    
    // protected by mutex
    int m_current_token;
    QString m_file;
    QColor m_color;
    QSize m_size;
    float m_ratio;
    Background::ResizeMethod m_method;
    Qt::TransformationMode m_mode;
    
    bool m_abort;
    bool m_restart;
signals:
    void done(int token, const QImage &pixmap);
};

#endif // RENDERTHREAD_H
