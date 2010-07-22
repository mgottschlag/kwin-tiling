/*
    Copyright (c) Sune Vuorela <sune@vuorela.dk>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

*/

#include "mobilebarcode.h"
#include <dmtx.h> //there is a libdmtx(3) that documents this header

#include <QDebug>
#include <QPainter>

#include <QResizeEvent>


MobileBarcode::DataMatrixWidget::DataMatrixWidget(QWidget* parent): QWidget(parent),m_size(100) {
  m_dirty=true;
  setMinimumSize(10,10); //we need to ensure that there is something to be painted by paintevent.
}

void MobileBarcode::DataMatrixWidget::setData(const QString& data) {
  if(m_data!= data) {
    m_data = data;
    m_dirty=true;
    repaint();
  }
}

void MobileBarcode::DataMatrixWidget::paintEvent(QPaintEvent* event ) {
  if(m_dirty) {
    redoImage();
  }
  QPainter painter(this);
  QSize currentsize = size();
  int xoffset = (currentsize.width()-m_image.size().width())/2;
  int yoffset = (currentsize.height()-m_image.size().height())/2;
  painter.drawImage(QPoint(xoffset,yoffset),m_image,m_image.rect());
  QWidget::paintEvent(event);
}

void MobileBarcode::DataMatrixWidget::resizeEvent(QResizeEvent* event ) {
  QSize currentsize = event->size();
  int newsize = qMin(currentsize.height(),currentsize.width());
  if(newsize!=m_size) {
    m_size=newsize;
    m_dirty=true;
  }
  QWidget::resizeEvent(event);
}


void MobileBarcode::DataMatrixWidget::redoImage() {
  if(m_data.size()==0) {
    return;
  }

  DmtxEncode * enc = dmtxEncodeCreate();
  dmtxEncodeSetProp( enc, DmtxPropPixelPacking, DmtxPack32bppRGBX );
  dmtxEncodeSetProp( enc, DmtxPropWidth, m_size );
  dmtxEncodeSetProp( enc, DmtxPropHeight, m_size );

  char* raw_string = qstrdup( m_data.toUtf8().trimmed().constData() );
  dmtxEncodeDataMatrix(enc,strlen(raw_string),(unsigned char*) raw_string);
  free(raw_string);
  
  m_image = QImage(enc->image->pxl,enc->image->width,enc->image->height, QImage::Format_RGB32);
  QSize minimum_size = QSize(enc->image->width,enc->image->height);
  if(minimum_size!= minimumSize()) {
    setMinimumSize(minimum_size);
    updateGeometry();
  }
  
  dmtxEncodeDestroy(&enc); 
}

#include "mobilebarcode.moc"
