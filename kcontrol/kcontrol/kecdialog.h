/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#ifndef __kecdialog_h__
#define __kecdialog_h__

#include <qptrlist.h>
#include <qptrdict.h>

#include <kdialogbase.h>
#include <kcmodule.h>

/**
 * A method that offers a @ref KDialogBase containing arbitrary KControl Modules
 * 
 * @short A method that offers a @ref KDialogBase containing arbitrary KControl Modules
 * @author Matthias Elter <elter@kde.org>, Daniel Molkentin <molkentin@kde.org>
 */
class KExtendedCDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Constructor
     **/
    KExtendedCDialog(QWidget *parent=0, const char *name=0, bool modal=false);
    /**
     * Destructor
     **/
   virtual ~KExtendedCDialog();

    /**
     * Add a module.
     *
     * @param module Specify the name of the module that is to be added
     *               to the list of modules the dialog will show.
     *
     * @param withfallback Try harder to load the module. Might result
     *                     in the module appearing outside the dialog.
     **/
    void addModule(const QString& module, bool withfallback=true);

protected slots:
    /**
     * This slot is called when the user presses the "Default" Button
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation!
     **/
    virtual void slotDefault();

    /**
     * This slot is called when the user presses the "Apply" Button
     * You can reimplement it if needed
     *
     * @note Make sure you call the original implementation!
     **/
    virtual void slotApply();

	/**
     * This slot is called when the user presses the "OK" Button
     * You can reimplement it if needed
     *
     * @note Make sure you call the original implementation!
     **/
    virtual void slotOk();

	/**
     * This slot is called when the user presses the "Help" Button
     * You can reimplement it if needed
     *
     * @note Make sure you call the original implementation!
     **/
    virtual void slotHelp();

    void slotAboutToShow(QWidget *);

    void clientChanged(bool state);
	
private:
    struct LoadInfo { 
      LoadInfo(const QString &_path, bool _withfallback) 
         : path(_path), withfallback(_withfallback) 
         { }
      QString path; 
      bool withfallback;
    };
    QPtrList<KCModule> modules;
    QPtrDict<LoadInfo> moduleDict;
    QString _docPath;

    // For future use
    class KExtendedCDialogPrivate;
    KExtendedCDialogPrivate *d;
};

#endif
