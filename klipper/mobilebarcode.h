/*
    Copyright (c) 2010 Sune Vuorela <sune@vuorela.dk>

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

#ifndef MOBILEBARCODE_H
#define MOBILEBARCODE_H

#include <QWidget>

namespace MobileBarcode {

class DataMatrixWidget : public QWidget {
  Q_OBJECT
  public:
    DataMatrixWidget(QWidget* parent=0);
  public Q_SLOTS:
    /**
     * @param QString sets the data to be shown in the barcode
     */
    void setData(const QString& data);
  protected:
    virtual void paintEvent(QPaintEvent* );
    virtual void resizeEvent(QResizeEvent* );
    
  private:
    void redoImage(); //this function is where the interaction with dmtx library happens
    QString m_data; //the data to be shown
    int m_size; //it's quadratic so a int is okay.
    QImage m_image; //caching, no need to redo the image on each repaint
    bool m_dirty; //if the image should be redone in paintEvent.
};

}

#endif // MOBILEBARCODE_H
