/*
    Copyright (c) 2003 Chris Howells <howells@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; version         *
    * 2 of the License.                                                     *
    *                                                                       *
    *************************************************************************
*/

#ifndef RANDOM_H
#define RANDOM_H

class QWidget;
class QCheckBox;

class KRandomSetup : public KDialog
{
	Q_OBJECT
	public:
		KRandomSetup( QWidget *parent = NULL, const char *name = NULL );

	private:

		QWidget *preview;
		QCheckBox *openGL;
		QCheckBox *manipulateScreen;

  private Q_SLOTS:
		void slotOk();

};

#endif
