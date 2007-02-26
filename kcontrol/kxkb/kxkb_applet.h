/***************************************************************************
 *   Copyright (C) 2007 Andriy Rysin                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KXKBAPPLET_H
#define KXKBAPPLET_H

#include <QString>
#include <QMouseEvent>
#include <QPixmap>

#include <kpanelapplet.h>
#include <kconfig.h>


class KxkbWidget;

class KxkbApplet : public KPanelApplet
{
  Q_OBJECT
public:
    KxkbApplet(const QString& configFile, Plasma::Type t = Plasma::Normal,
                 int actions = 0, QWidget *parent = 0, Qt::WFlags f = 0);
    ~KxkbApplet();
    
    virtual int widthForHeight(int height) const;
    virtual int heightForWidth(int width) const;
    /**
     * Is called when the user selects "About" from the applets RMB menu.
     * Reimplement this function to launch a about dialog.
     *
     * Note that this is called only when your applet supports the About action.
     * See @ref Action and @ref KPanelApplet().
     **/

//  public slots:
//    virtual void about();
    /**
     * Is called when the user selects "Help" from the applets RMB menu.
     * Reimplement this function to launch a manual or help page.
     *
     * Note that this is called only when your applet supports the Help action.
     * See @ref Action and @ref KPanelApplet().
     **/
//    virtual void help();
    /**
     * Is called when the user selects "Preferences" from the applets RMB menu.
     * Reimplement this function to launch a preferences dialog or kcontrol module.
     *
     * Note that this is called only when your applet supports the preferences action.
     * See @ref Action and @ref KPanelApplet().
     **/
//    virtual void preferences();
    
//protected:
//    void resizeEvent(QResizeEvent *);
//    void mousePressEvent(QMouseEvent *e);


private:
	KxkbWidget* kxkbWidget;
//private:
//    KConfig *ksConfig;
//    QWidget *mainView;
//    KPopupMenu *mContextMenu;
//    KPopupFrame *mBrightnessChooserFrame;
//    BrightnessChooserImpl *chooser;

//private: // Private methods
};

#endif
