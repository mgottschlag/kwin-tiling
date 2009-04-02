/* This file is part of the KDE libraries

    Copyright (c) 2001  Martin R. Jones <mjones@kde.org>
    Copyright 2006  David Faure <faure@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSCREENSAVER_H
#define KSCREENSAVER_H

#include <QtGui/QWidget>

#include <kdemacros.h>
#include <kaboutdata.h> // needed by all users of this header, so no point in a forward declaration

class KScreenSaverPrivate;
class KBlankEffectPrivate;

/**
 * Provides a QWidget for a screensaver to draw into.
 *
 * You should derive from this widget and implement your screensaver's
 * functionality.
 *
 * @see KScreenSaverInterface
 *
 * @short Provides a QWidget for a screensaver to draw into.
 * @author Martin R. Jones <mjones@kde.org>
 */
class KDE_EXPORT KScreenSaver : public QWidget
{
    Q_OBJECT
public:
    /**
     * @param id The winId() of the widget to draw the screensaver into.
     */
    KScreenSaver( WId id=0 );
    ~KScreenSaver();

protected:
    /**
     * You cannot create a new widget with this widget as parent, since this
     * widget may not be owned by your application.  In order to create
     * widgets with a KScreenSaver as parent, create the widget with no parent,
     * call embed(), and then show() the widget.
     *
     * @param widget The widget to embed in the screensaver widget.
     */
    void embed( QWidget *widget );
    bool event( QEvent* event );

private:
    //Note: To keep binary compatibility this class must have only one member, which is a pointer.
    //      If more members are needed, use the d-pointer technique.
    //      See http://techbase.kde.org/Policies/Binary_Compatibility_Issues_With_C%2B%2B
    //KScreenSaverPrivate *d;
    QWidget *embeddedWidget;
};

/**
 *
 * To use libkscreensaver, derive from KScreenSaverInterface and reimplement
 * its virtual methods. Then write
 * <code>
 *   int main( int argc, char *argv[] )
 *   {
 *       MyKScreenSaverInterface kssi;
 *       return kScreenSaverMain( argc, argv, kssi );
 *   }
 * </code>
 *
 * In order to convert a KDE-3 screensaver (which used to export kss_* symbols and had no main function)
 * to KDE-4, you can use the following python script
 * kdebase/workspace/kscreensaver/libkscreensaver/kscreensaver-kde3to4-porting.py
 */
class KDE_EXPORT KScreenSaverInterface
{
public:
    /**
     * Destructor
     */
    virtual ~KScreenSaverInterface();
    /**
     * Reimplement this method to return the KAboutData instance describing your screensaver
     */
    virtual KAboutData *aboutData() = 0;
    /**
     * Reimplement this method to return your KScreenSaver-derived screensaver
     */
    virtual KScreenSaver* create( WId id ) = 0;
    /**
     * Reimplement this method to return your modal setup dialog
     */
    virtual QDialog* setup();
};

/**
 * The entry point for the program's main()
 * @see KScreenSaverInterface
 */
KDE_EXPORT int kScreenSaverMain( int argc, char** argv, KScreenSaverInterface& screenSaverInterface );

/**
*
* Blanks a widget using various effects.
*
* @short Blanks a widget using various effects.
* @author Martin R. Jones <mjones@kde.org>
*/
class KBlankEffect : public QObject
{
    Q_OBJECT
public:
    KBlankEffect( QObject *parent=0 );
    ~KBlankEffect();

    enum Effect { Random=-1, Blank=0, SweepRight, SweepDown, Blocks,
                  MaximumEffects };

    /**
     * Blank a widget using the specified effect.
     * Some blanking effects take some time, so you should connect to
     * doneBlank() to know when the blanking is complete.
     *
     * @param w The widget to blank.
     * @param effect The type of effect to use.
     */
    void blank( QWidget *w, Effect effect=Random );

    typedef void (KBlankEffect::*BlankEffect)();

Q_SIGNALS:
    /**
     * emitted when a blanking effect has completed.
     */
    void doneBlank();

protected Q_SLOTS:
    void timeout();

protected:
    void finished();

    void blankNormal();
    void blankSweepRight();
    void blankSweepDown();
    void blankBlocks();

protected:
    static BlankEffect effects[];
    KBlankEffectPrivate *d;
};
#endif

