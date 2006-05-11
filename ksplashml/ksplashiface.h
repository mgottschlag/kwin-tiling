/***************************************************************************
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#ifndef __KSPLASHIFACE_H__
#define __KSPLASHIFACE_H__

#include <dcopobject.h>

#include <QString>

/** @short DCOP interface for KSplash. */
class KSplashIface : virtual public DCOPObject
{
  K_DCOP
public:
  KSplashIface( const char *name = "ksplash" ) : DCOPObject(name) {}

k_dcop:
  virtual ASYNC upAndRunning( QString ) = 0;
  virtual ASYNC setMaxProgress(int) = 0;
  virtual ASYNC setProgress(int) = 0;
  virtual ASYNC setStartupItemCount( int count ) = 0;
  virtual ASYNC programStarted( QString programIcon, QString programName, QString description ) = 0;
  virtual ASYNC startupComplete() = 0;
  virtual ASYNC close() = 0;
};

#endif
