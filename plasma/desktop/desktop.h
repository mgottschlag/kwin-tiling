#ifndef DESKTOP_H
#define DESKTOP_H

#include "kdelibs_export.h"

class QWidget;

namespace Plasma
{

class KDE_EXPORT Desktop
{
    public:
        Desktop();
        ~Desktop();

        void manage();
    private:
        void initializeDisplay();

    private:
        QWidget *m_desktopWidget;
};

}

#endif
