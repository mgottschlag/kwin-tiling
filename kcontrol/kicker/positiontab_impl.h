/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Aaron Seigo <aseigo@olympusproject.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */


#ifndef __positiontab_impl_h__
#define __positiontab_impl_h__

#include "positiontab.h"

class QFrame;
class KBackgroundRenderer;
class KickerConfig;
class ExtensionInfo;

class PositionTab : public PositionTabBase
{
    Q_OBJECT

public:
    PositionTab(QWidget *parent, const char* name = 0);
    ~PositionTab();

    enum positions { PosLeft = 0, PosRight, PosTop, PosBottom };
    enum allignments { AlignLeft = 0, AlignCenter, AlignRight };

    void load();
    void save();
    void defaults();

signals:
    void changed();
    void panelPositionChanged(int);

protected slots:
    void movePanel(int);
    void lengthenPanel(int);
    void panelDimensionsChanged();
    void slotBGPreviewReady(int);
    void infoUpdated();
    void storeInfo();
    void showIdentify();
    void extensionAdded(ExtensionInfo*);
    void extensionRemoved(ExtensionInfo* info);
    void extensionChanged(const QString&);
    void extensionAboutToChange(const QString&);
    void sizeChanged(int);
    void switchPanel(int);
    void jumpToPanel(int);

private:
    QFrame* m_pretendPanel;
    QWidget* m_pretendDesktop;
    KBackgroundRenderer* m_desktopPreview;
    ExtensionInfo* m_panelInfo;

    unsigned int m_panelPos;
    unsigned int m_panelAlign;
    void setPositionButtons();
};

#endif

