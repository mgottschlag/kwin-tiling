/*****************************************************************

Copyright (c) 2001 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef ServiceButton_h
#define ServiceButton_h

#include "panelbutton.h"

#include <kservice.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

class ServiceButton : public PanelButton
{
    Q_OBJECT

public:
    ServiceButton( const QString& desktopFile, QWidget* parent );
    ServiceButton( const KService::Ptr& service, QWidget* parent );
    ServiceButton( const KConfigGroup& config, QWidget* parent );

    ~ServiceButton();

    virtual void saveConfig(KConfigGroup& config) const;
    virtual void properties();

protected Q_SLOTS:
    void slotUpdate();
    void slotSaveAs(const KUrl&, KUrl&);
    void slotExec();
    void performExec();

protected:
    void initialize();
    void loadServiceFromId(const QString &id);
    void readDesktopFile();
    virtual void startDrag();
    virtual void dropEvent(QDropEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *);
    QString defaultIcon() const { return "exec"; }
    bool checkForBackingFile();

    KService::Ptr  _service;
    QString        _id;
};

#endif
