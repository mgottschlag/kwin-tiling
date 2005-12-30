#ifndef LAYER_H
#define LAYER_H

namespace Plasma
{

class KDE_EXPORT Layer
{
    public:
        Layer();

    protected:
        virtual void paintEvent(QPaintEvent *e);
        virtual void mousePressEvent(QMouseEvent *e);

    private:
        class Private;
        Private *d;
};

}

#endif
