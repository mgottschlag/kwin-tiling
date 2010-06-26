/*

Copyright 1988, 1998  The Open Group
Copyright 2001,2003 Oswald Buddenhagen <ossi@kde.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the copyright holder.

*/

/*
 * xdm - display manager daemon
 * Author: Keith Packard, MIT X Consortium
 *
 * generate authorization data for XDM-AUTHORIZATION-1 as per XDMCP spec
 */

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

/*
 * Generate authorization for XDM-AUTHORIZATION-1
 *
 * When being used with XDMCP, 8 bytes are generated for the session key
 * (sigma), as the random number (rho) is already shared between xdm and
 * the server. Otherwise, we'll prepend a random number to pass in the file
 * between xdm and the server (16 bytes total)
 */

static Xauth *
xdmGetAuthHelper(unsigned short namelen, const char *name, int nbytes)
{
    Xauth *new;

    if (!(new = generateAuthHelper(namelen, name, nbytes)))
        return 0;

    /*
     * set the first byte of the session key to zero as it
     * is a DES key and only uses 56 bits
     */
    ((char *)new->data)[new->data_length - 8] = '\0';
    debug("local server auth %02[*hhx\n", new->data_length, new->data);
    return new;
}

Xauth *
xdmGetAuth(unsigned short namelen, const char *name)
{
    return xdmGetAuthHelper(namelen, name, 16);
}

#ifdef XDMCP

void
xdmGetXdmcpAuth(struct protoDisplay *pdpy,
                unsigned short authorizationNameLen,
                const char *authorizationName)
{
    Xauth *fileauth, *xdmcpauth;

    if (pdpy->fileAuthorization && pdpy->xdmcpAuthorization)
        return;
    xdmcpauth = xdmGetAuthHelper(authorizationNameLen, authorizationName, 8);
    if (!xdmcpauth)
        return;
    fileauth = Malloc(sizeof(Xauth));
    if (!fileauth) {
        XauDisposeAuth(xdmcpauth);
        return;
    }
    /* build the file auth from the XDMCP auth */
    *fileauth = *xdmcpauth;
    fileauth->name = Malloc(xdmcpauth->name_length);
    fileauth->data = Malloc(16);
    fileauth->data_length = 16;
    if (!fileauth->name || !fileauth->data) {
        XauDisposeAuth(xdmcpauth);
        free(fileauth->name);
        free(fileauth->data);
        free(fileauth);
        return;
    }
    /*
     * for the file authorization, prepend the random number (rho)
     * which is simply the number we've been passing back and
     * forth via XDMCP
     */
    memmove(fileauth->name, xdmcpauth->name, xdmcpauth->name_length);
    memmove(fileauth->data, pdpy->authenticationData.data, 8);
    memmove(fileauth->data + 8, xdmcpauth->data, 8);
    debug("accept packet auth %02[*hhx\nauth file auth %02[*hhx\n",
          xdmcpauth->data_length, xdmcpauth->data,
          fileauth->data_length, fileauth->data);
    /* encrypt the session key for its trip back to the server */
    XdmcpWrap((unsigned char *)xdmcpauth->data, (unsigned char *)&pdpy->key,
              (unsigned char *)xdmcpauth->data, 8);
    pdpy->fileAuthorization = fileauth;
    pdpy->xdmcpAuthorization = xdmcpauth;
}

/*
 * Search the Keys file for the entry matching this display.  This
 * routine accepts either plain ascii strings for keys, or hex-encoded numbers
 */

static int
xdmGetKey(struct protoDisplay *pdpy, ARRAY8Ptr displayID)
{
    FILE *keys;
    char line[1024], id[1024], key[1024];
    int keylen;

    debug("lookup key for %.*s\n", displayID->length, displayID->data);
    keys = fopen(keyFile, "r");
    if (!keys)
        return False;
    while (fgets(line, sizeof(line), keys)) {
        if (line[0] == '#' || sscanf(line, "%s %s", id, key) != 2)
            continue;
        bzero(line, sizeof(line));
        debug("key entry for %\"s %d bytes\n", id, strlen(key));
        if (strlen(id) == displayID->length &&
            !strncmp(id, (char *)displayID->data, displayID->length))
        {
            if (!strncmp(key, "0x", 2) || !strncmp(key, "0X", 2)) {
                if (!(keylen = hexToBinary(key, key + 2)))
                    break;
            } else {
                keylen = strlen(key);
            }
            while (keylen < 7)
                key[keylen++] = '\0';
            pdpy->key.data[0] = '\0';
            memmove(pdpy->key.data + 1, key, 7);
            bzero(key, sizeof(key));
            fclose(keys);
            return True;
        }
    }
    bzero(line, sizeof(line));
    bzero(key, sizeof(key));
    fclose(keys);
    return False;
}

/*ARGSUSED*/
int
xdmcheckAuthentication(struct protoDisplay *pdpy,
                       ARRAY8Ptr displayID,
                       ARRAY8Ptr authenticationName ATTR_UNUSED,
                       ARRAY8Ptr authenticationData)
{
    XdmAuthKeyPtr incoming;

    if (!xdmGetKey(pdpy, displayID))
        return False;
    if (authenticationData->length != 8)
        return False;
    XdmcpUnwrap(authenticationData->data, (unsigned char *)&pdpy->key,
                authenticationData->data, 8);
    debug("request packet auth %02[*hhx\n",
          authenticationData->length, authenticationData->data);
    if (!XdmcpCopyARRAY8(authenticationData, &pdpy->authenticationData))
        return False;
    incoming = (XdmAuthKeyPtr)authenticationData->data;
    XdmcpIncrementKey(incoming);
    XdmcpWrap(authenticationData->data, (unsigned char *)&pdpy->key,
              authenticationData->data, 8);
    return True;
}

#endif /* XDMCP */
