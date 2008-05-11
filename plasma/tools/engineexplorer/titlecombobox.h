
#ifndef TITLECOMBOBOX_H
#define TITLECOMBOBOX_H

#include <QComboBox>
#include <QPainter>

#include <KDebug>
#include <KLocale>

class TitleComboBox : public QComboBox
{
public:
    TitleComboBox(QWidget *parent = 0)
        : QComboBox(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        QComboBox::paintEvent(event);

        if (currentIndex() > 0) {
            return;
        }

        QPainter p(this);
        /*QFont bold = p.font();
        bold.setBold(true);
        p.setFont(bold);*/
        p.setPen(palette().color(QPalette::Disabled, QPalette::WindowText));
        int frameWidth = style()->pixelMetric(QStyle::PM_ComboBoxFrameWidth);
        QRect r = rect().adjusted(frameWidth, frameWidth, frameWidth, frameWidth);
        p.drawText(QStyle::visualRect(layoutDirection(), rect(), r), i18n("Data Engines"));
    }
};

#endif

