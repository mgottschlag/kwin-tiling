/*****************************************************************
ksmserver - the KDE session management server
								  
Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H "$Id: $"

#define KSMVendorString "KDE"
#define KSMReleaseString "1.0"


int KSMAuthCount = 1;
const char *KSMAuthNames[] = {"MIT-MAGIC-COOKIE-1"};
extern "C" {
extern IcePoAuthStatus _IcePoMagicCookie1Proc (_IceConn *, void **, int, int, int, void *, int *, void **, char **);
extern IcePaAuthStatus _IcePaMagicCookie1Proc (_IceConn *, void **, int, int, void *, int *, void **, char **);
}
IcePoAuthProc KSMAuthProcs[] = {_IcePoMagicCookie1Proc};
IcePaAuthProc KSMServerAuthProcs[] = {_IcePaMagicCookie1Proc};


#endif
