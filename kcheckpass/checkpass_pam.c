/*
 * Copyright (C) 1998 Caldera, Inc.
 * Copyright (C) 2003 Oswald Buddenhagen <ossi@kde.org>
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
 */

#include "kcheckpass.h"

#ifdef HAVE_PAM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_PAM_PAM_APPL_H
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif

struct pam_data {
  char *(*conv) (ConvRequest, const char *);
  int abort:1;
  int classic:1;
};

#ifdef PAM_MESSAGE_NONCONST
typedef struct pam_message pam_message_type;
typedef void ** pam_gi_type;
#else
typedef const struct pam_message pam_message_type;
typedef const void ** pam_gi_type;
#endif

static int
PAM_conv (int num_msg, pam_message_type **msg,
	  struct pam_response **resp,
	  void *appdata_ptr)
{
  int count;
  struct pam_response *repl;
  struct pam_data *pd = (struct pam_data *)appdata_ptr;

  if (!(repl = calloc(num_msg, sizeof(struct pam_response))))
    return PAM_CONV_ERR;

  for (count = 0; count < num_msg; count++)
    switch (msg[count]->msg_style) {
    case PAM_TEXT_INFO:
      pd->conv(ConvPutInfo, msg[count]->msg);
      break;
    case PAM_ERROR_MSG:
      pd->conv(ConvPutError, msg[count]->msg);
      break;
    default:
      switch (msg[count]->msg_style) {
      case PAM_PROMPT_ECHO_ON:
        repl[count].resp = pd->conv(ConvGetNormal, msg[count]->msg);
        break;
      case PAM_PROMPT_ECHO_OFF:
        repl[count].resp =
            pd->conv(ConvGetHidden, pd->classic ? 0 : msg[count]->msg);
        break;
#ifdef PAM_BINARY_PROMPT
      case PAM_BINARY_PROMPT:
        repl[count].resp = pd->conv(ConvGetBinary, msg[count]->msg);
        break;
#endif
      default:
        /* Must be an error of some sort... */
        goto conv_err;
      }
      if (!repl[count].resp) {
        pd->abort = 1;
        goto conv_err;
      }
      repl[count].resp_retcode = PAM_SUCCESS;
      break;
    }
  *resp = repl;
  return PAM_SUCCESS;

 conv_err:
  for (; count >= 0; count--)
    if (repl[count].resp)
      switch (msg[count]->msg_style) {
      case PAM_PROMPT_ECHO_OFF:
	dispose(repl[count].resp);
        break;
#ifdef PAM_BINARY_PROMPT
      case PAM_BINARY_PROMPT: /* handle differently? */
#endif
      case PAM_PROMPT_ECHO_ON:
	free(repl[count].resp);
	break;
      }
  free(repl);
  return PAM_CONV_ERR;
}

static struct pam_data PAM_data;

static struct pam_conv PAM_conversation = {
  &PAM_conv,
  &PAM_data
};

#ifdef PAM_FAIL_DELAY
static void
fail_delay(int retval ATTR_UNUSED, unsigned usec_delay ATTR_UNUSED, 
	   void *appdata_ptr ATTR_UNUSED)
{}
#endif


AuthReturn Authenticate(const char *caller, const char *method,
        const char *user, char *(*conv) (ConvRequest, const char *))
{
  const char	*tty;
  pam_handle_t	*pamh;
  char		*auser;
  const char	*pam_service;
  char		pservb[64];
  int		pam_error;

  PAM_data.conv = conv;
  if (strcmp(method, "classic")) {
    sprintf(pservb, "%.31s-%.31s", caller, method);
    pam_service = pservb;
  } else {
    PAM_data.classic = 1;
    pam_service = caller;
  }
  pam_error = pam_start(pam_service, user, &PAM_conversation, &pamh);
  if (pam_error != PAM_SUCCESS)
    return AuthError;

  tty = getenv ("DISPLAY");
  if (!tty)
    tty = ttyname(0);
  pam_error = pam_set_item (pamh, PAM_TTY, tty);
  if (pam_error != PAM_SUCCESS) {
    pam_end(pamh, pam_error);
    return AuthError;
  }

# ifdef PAM_FAIL_DELAY
  pam_set_item (pamh, PAM_FAIL_DELAY, (void *)fail_delay);
# endif

  pam_error = pam_authenticate(pamh, 0);
  if (pam_error != PAM_SUCCESS) {
    pam_end(pamh, pam_error);
    switch (pam_error) {
      case PAM_USER_UNKNOWN:
      case PAM_AUTH_ERR:
      case PAM_MAXTRIES: /* should handle this better ... */
      case PAM_AUTHINFO_UNAVAIL: /* returned for unknown users ... bogus */
        return AuthBad;
      default:
        return AuthError;
    }
  }

  /* just in case some module is stupid enough to ignore a preset PAM_USER */
  pam_error = pam_get_item (pamh, PAM_USER, (pam_gi_type)&auser);
  if (pam_error != PAM_SUCCESS) {
    pam_end(pamh, pam_error);
    return AuthError;
  }
  if (strcmp(auser, user)) {
    pam_end(pamh, PAM_SUCCESS); /* maybe use PAM_AUTH_ERR? */
    return AuthBad;
  }

  /* Refresh credentials (Needed e.g. for AFS (timing out Kerberos tokens)) */
  /* REINIT_CRED and REFRESH_CRED are not clearly defined and thus likely to
   * fail, so we use ESTABLISH_CRED ... */     
  pam_error = pam_setcred(pamh, PAM_ESTABLISH_CRED);
  if (pam_error != PAM_SUCCESS) {
    pam_end(pamh, pam_error);
    return AuthError;
  }

  pam_end(pamh, PAM_SUCCESS);
  return AuthOk;
}

#endif
