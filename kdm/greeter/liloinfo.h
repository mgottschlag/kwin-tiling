    /*

    Front-end to Lilo's boot options
    $Id$

    Copyright (C) 1999 Stefan van den Oord <oord@cs.utwente.nl>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#ifndef LILOINFO_H
#define LILOINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <qfileinfo.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kapp.h>
#include <kprocess.h>

#include <iostream.h>

/*
 * Errors:
 *      0 = No error
 *     -1 = Lilo Error (description in getErrorDescription)
 *     -2 = No next option was defined
 *     -3 = Next option does not occur in option list
 *     -4 = Hack is not enabled
 *     -5 = Attempt to write invalid next boot option
 *     -6 = Failed to run Lilo
 *     -7 = Lilo location is a non-existing file
 *     -8 = Bootmap location is a non-existing file
 */

/**
 * This class can interact with Lilo. It can provide the boot options that
 * Lilo has written to the boot sector, i.e. the options the user can choose
 * from at boot. One of these options is the default option. Furthermore,
 * Lilo has what I call a 'next boot option'. The default option is the
 * option that is automatically selected when the user does not choose
 * another one. It is possible to overrule this default option for one reboot
 * only. This is what I call the next boot option.
 *
 * The class uses a list of error codes. They are all negative numbers.
 * So when a method that can return error codes returns a nonnegative number,
 * no error has occured. Otherwise, getErrorDescription() can be used to
 * get a description of the last error that occured. Error code '-1' means that
 * trying to run Lilo resulted in an error. The text of the exact error can be
 * retrieved using getErrorDescription(). Error '-2' means that there is no
 * next option. Error '-3' means that the next option that was found does not
 * occur in the list of boot options. This probably means that the method to
 * find out the next option does not work with the version of Lilo that is used.
 * Error '-4' occurs when the next option is requested / set while enableHack
 * was not turned on in the constructor. Attempting to write an invalid next
 * option (i.e. one that does not occur in the list of boot options) results
 * in error '-5'. Error '-6' means that the Lilo command could not be
 * run (may be permission denied or file not found; the real error string can
 * be retrieved by getErrorDescription(). Error '-7' and '-8' mean that tstrhe Lilo
 * location and the bootmap file location, respectively, is a non-existing file.
 *
 * Note that calling the constructor is sufficient initialization. So you can
 * even immediately call setNextBootOption(), only risking that you supply an
 * invalid next boot option, in which case an error is returned.
 *
 * @short Front-end to Lilo's boot options
 * @author Stefan van den Oord
 * @version 0.3
 */
class LiloInfo : QObject
{
	Q_OBJECT

	public:
		/**
		 * The constructor.
		 *
		 * @param lilolocation     This string must contain the full path
		 *                         name of the lilo executable (usually
		 *                         "/sbin/lilo").
		 * @param bootmaplocation  This string must contain the full path
		 *                         name of the boot map file (usually
		 *                         "/boot/map").
		 * @param enableHack       If this parameter is false, the next
		 *                         option is not read from Lilo, and it
		 *                         cannot be set either. This parameter
		 *                         is added because it cannot be garanteed
		 *                         that the method used to find out the
		 *                         next boot option will keep working with
		 *                         future versions of Lilo.
		 * @param enableDebug      If this parameter is true, debug output
		 *                         will be written to stderr. Otherwise,
		 *                         the class operates silently.
		 */
		LiloInfo ( QString lilolocation,
		           QString bootmaplocation,
		           bool enableHack = true,
		           bool enableDebug = true );
		/**
		 * The destructor.
		 */
		~LiloInfo();

		/** 
		 * The location of the Lilo location can be changed without
		 * recreating the object.
		 * @return               The return value is an error code.
		 * @param  lilolocation  The new location of the Lilo executable.
		 */
		int changeLiloLocation ( QString lilolocation );

		/**
		 * The location of the Lilo boot map file can be changed without
		 * recreating the object.
		 * @return               The return value is an error code.
		 * @param  bootmaplocation  The new location of the boot map file.
		 */
		int changeBootmapLocation ( QString bootmaplocation );
		/**
		 * Whether the Lilo hack may be used can be changed without recreating
		 * the object.
		 * @param enableHack       If this parameter is false, the next
		 *                         option is not read from Lilo, and it
		 *                         cannot be set either. This parameter
		 *                         is added because it cannot be garanteed
		 *                         that the method used to find out the
		 *                         next boot option will keep working with
		 *                         future versions of Lilo.
		 */
		void changeHackState ( bool enableHack ) { useHack = enableHack; }

		/**
		 * This method can be used to retrieve a list of available boot
		 * options. Note that the parameter bootOptions must be allocated
		 * before calling this method!
		 * @return               The return value is an error code.
		 * @param   bootOptions  The object that will hold the boot options.
		 *                       The QStringList must be allocated before calling
		 *                       this method!
		 */
		int getBootOptions ( QStringList *bootOptions );

		/**
		 * This method returns a string representation of the default boot
		 * option.
		 * @return               The return value is an error code.
		 * @param  defaultOption The object that will hold the default boot
		 *                       option.
		 */
		int getDefaultBootOption ( QString &defaultOption );
		/**
		 * This method returns the index of the default boot option in the
		 * list provided by getBootOptions.
		 * @see LiloInfo#getBootOptions
		 * @return               If the return value is negative, it is an
		 *                       error code. Otherwise, it is the index of
		 *                       the default boot option in the list provided
		 *                       by getBootOptions.
		 */
		int getDefaultBootOptionIndex();

		/**
		 * This method provides the next boot option.
		 * @return               The return value is an error code.
		 * @param   nextOption   The object that will hold a string 
		 *                       representation of the next boot option.
		 */
		int getNextBootOption ( QString &nextOption );
		/**
		 * This method returns the index of the next boot option in the
		 * list provided by getBootOptions.
		 * @see LiloInfo#getBootOptions
		 * @return               If the return value is negative, it is an
		 *                       error code. Otherwise, it is the index of
		 *                       the next boot option in the list provided
		 *                       by getBootOptions.
		 */
		int getNextBootOptionIndex();

		/**
		 * This method sets the next boot option. The provided string must
		 * be in the list of boot options as provided by getBootOptions.
		 * @see LiloInfo#getBootOptions
		 * @see LiloInfo#clearNextBootOption
		 * @return                  The return value is an error code.
		 * @param   nextBootOption  The string representation of the desired
		 *                          next boot option. If the string is empty,
		 *                          this is equivalent to calling
		 *                          clearNextBootOption().
		 */
		int setNextBootOption ( QString nextBootOption );
		/**
		 * This method sets the next boot option.
		 * @see LiloInfo#getBootOptions
		 * @return                       The return value is an error code.
		 * @param   nextBootOptionIndex  The index of the next boot option in
		 *                               the list of boot options as provided
		 *                               by getBootOptions.
		 */
		int setNextBootOption ( int nextBootOptionIndex );

		/**
		 * This method clears the next boot option.
		 * @return                       The return value is an error code.
		 */
		int clearNextBootOption();

		/**
		 * This method returns a string representation of the last error that
		 * occured. If the last error was a lilo error, the string contains
		 * the exact contents of the error. Since the error state is reset,
		 * this method can only be called once per error.
		 * @return   The description of the last error that occured.
		 */
		QString getErrorDescription();

	private:
		QString liloloc, bootmaploc;
		bool optionsAreRead;               // true as soon as lilo -q is called
		bool nextOptionIsRead;          // true as soon as lilo -q -v is called
		QStringList *options;
		int indexDefault;             // index in options of the default option
		int indexNext;                   // index in options of the next option
		bool debug, useHack;
		int error;
		QString liloErrorString;      // is set when lilo resulted in an error;
		                             // getErrorDescription() then returns this
		                                               // string and clears it.

		void initialize ( QString lilolocation, QString bootmaplocation );

		bool getOptionsFromLilo();
		bool getNextOptionFromLilo();

		int setNextBootOption ( int nextBootOptionIndex, bool clearNextOption );

	private slots:
		void getOptionsFromStdout ( KProcess *, char *buffer, int len );
		void getNextOptionFromStdout ( KProcess *, char *buffer, int len );
		void processStderr ( KProcess *, char *buffer, int len );
};

#endif // LILOINFO_H
