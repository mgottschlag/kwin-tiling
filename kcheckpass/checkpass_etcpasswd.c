/*
 * Copyright (c) 1998 Christian Esken <esken@kde.org> 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      Copyright (C) 1998, Christian Esken <esken@kde.org>
 */

#include "kcheckpass.h"

#ifdef HAVE_ETCPASSWD

/*******************************************************************
 * This is the authentication code for /etc/passwd passwords
 *******************************************************************/

#include <string.h>
#include <stdlib.h>

AuthReturn Authenticate(const char *method, char *(*conv) (ConvRequest, const char *))
{
  struct passwd *pw;
  char *login, *passwd;
  char *crpt_passwd;
  int result;

  if (strcmp(method, "classic"))
    return AuthError;

  if (!(login = conv(ConvGetNormal, 0)))
    return AuthAbort;

  /* Get the password entry for the user we want */
  pw = getpwnam(login);

  free(login);

  /* getpwnam should return a NULL pointer on error */
  if (pw == 0)
    return AuthBad;

  if (!(passwd = conv(ConvGetHidden, 0)))
    return AuthAbort;

  /* Encrypt the password the user entered */
  crpt_passwd = crypt(passwd, pw->pw_passwd);

  dispose(passwd);

  /* Are they the same? */
  result = strcmp(pw->pw_passwd, crpt_passwd);

  if (result == 0)
    return AuthOk; /* Success */
  else
    return AuthBad; /* Password wrong or account locked */
}

#endif
