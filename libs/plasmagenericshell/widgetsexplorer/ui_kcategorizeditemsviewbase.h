/********************************************************************************
** Form generated from reading ui file 'kcategorizeditemsviewbase.ui'
**
** Created: Mon Jun 22 20:40:40 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_KCATEGORIZEDITEMSVIEWBASE_H
#define UI_KCATEGORIZEDITEMSVIEWBASE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "customdragtreeview_p.h"

QT_BEGIN_NAMESPACE

class Ui_KCategorizedItemsViewBase
{
public:
    QVBoxLayout *vboxLayout;
    QFrame *frame;
    QVBoxLayout *vboxLayout1;
    QFrame *line;
    CustomDragTreeView *itemsView;

    void setupUi(QWidget *KCategorizedItemsViewBase)
    {
        if (KCategorizedItemsViewBase->objectName().isEmpty())
            KCategorizedItemsViewBase->setObjectName(QString::fromUtf8("KCategorizedItemsViewBase"));
        KCategorizedItemsViewBase->resize(487, 528);
        vboxLayout = new QVBoxLayout(KCategorizedItemsViewBase);
        vboxLayout->setMargin(0);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        frame = new QFrame(KCategorizedItemsViewBase);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Sunken);
        vboxLayout1 = new QVBoxLayout(frame);
        vboxLayout1->setSpacing(0);
        vboxLayout1->setMargin(0);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        line = new QFrame(frame);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShadow(QFrame::Plain);
        line->setFrameShape(QFrame::HLine);

        vboxLayout1->addWidget(line);

        itemsView = new CustomDragTreeView(frame);
        itemsView->setObjectName(QString::fromUtf8("itemsView"));
        itemsView->setFrameShape(QFrame::NoFrame);
        itemsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        itemsView->setDragEnabled(true);
        itemsView->setDragDropMode(QAbstractItemView::DragOnly);
        itemsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        itemsView->setIconSize(QSize(64, 64));
        itemsView->setIndentation(0);
        itemsView->setRootIsDecorated(false);

        vboxLayout1->addWidget(itemsView);


        vboxLayout->addWidget(frame);


        retranslateUi(KCategorizedItemsViewBase);

        QMetaObject::connectSlotsByName(KCategorizedItemsViewBase);
    } // setupUi

    void retranslateUi(QWidget *KCategorizedItemsViewBase)
    {
        Q_UNUSED(KCategorizedItemsViewBase);
    } // retranslateUi

};

namespace Ui {
    class KCategorizedItemsViewBase: public Ui_KCategorizedItemsViewBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KCATEGORIZEDITEMSVIEWBASE_H
