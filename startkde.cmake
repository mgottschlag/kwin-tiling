#!/bin/sh
#
#  DEFAULT KDE STARTUP SCRIPT ( @KDE_VERSION_STRING@ )
#

if test "x$1" = x--failsafe; then
    KDE_FAILSAFE=1 # General failsafe flag
    KWIN_COMPOSE=N # Disable KWin's compositing
    export KWIN_COMPOSE KDE_FAILSAFE
fi

# When the X server dies we get a HUP signal from xinit. We must ignore it
# because we still need to do some cleanup.
trap 'echo GOT SIGHUP' HUP

# Check if a KDE session already is running and whether it's possible to connect to X
kcheckrunning
kcheckrunning_result=$?
if test $kcheckrunning_result -eq 0 ; then
	echo "KDE seems to be already running on this display."
	xmessage -geometry 500x100 "KDE seems to be already running on this display." > /dev/null 2>/dev/null
	exit 1
elif test $kcheckrunning_result -eq 2 ; then
	echo "\$DISPLAY is not set or cannot connect to the X server."
        exit 1
fi

# we have to unset this for Darwin since it will screw up KDE's dynamic-loading
unset DYLD_FORCE_FLAT_NAMESPACE

# in case we have been started with full pathname spec without being in PATH
bindir=`echo "$0" | sed -n 's,^\(/.*\)/[^/][^/]*$,\1,p'`
if [ -n "$bindir" ]; then
  qbindir=`$bindir/kde4-config --qt-binaries`
  if [ -n "$qbindir" ]; then
    case $PATH in
      $qbindir|$qbindir:*|*:$qbindir|*:$qbindir:*) ;;
      *) PATH=$qbindir:$PATH; export PATH;;
    esac
  fi
  case $PATH in
    $bindir|$bindir:*|*:$bindir|*:$bindir:*) ;;
    *) PATH=$bindir:$PATH; export PATH;;
  esac
fi

# Boot sequence:
#
# kdeinit is used to fork off processes which improves memory usage
# and startup time.
#
# * kdeinit starts klauncher first.
# * Then kded is started. kded is responsible for keeping the sycoca
#   database up to date. When an up to date database is present it goes
#   into the background and the startup continues.
# * Then kdeinit starts kcminit. kcminit performs initialisation of
#   certain devices according to the user's settings
#
# * Then ksmserver is started which takes control of the rest of the startup sequence

# The user's personal KDE directory is usually ~/.kde, but this setting
# may be overridden by setting KDEHOME.

kdehome=$HOME/@KDE_DEFAULT_HOME@
test -n "$KDEHOME" && kdehome=`echo "$KDEHOME"|sed "s,^~/,$HOME/,"`

# see kstartupconfig source for usage
mkdir -m 700 -p $kdehome
mkdir -m 700 -p $kdehome/share
mkdir -m 700 -p $kdehome/share/config
cat >$kdehome/share/config/startupconfigkeys <<EOF
kcminputrc Mouse cursorTheme 'Oxygen_White'
kcminputrc Mouse cursorSize ''
ksplashrc KSplash Theme Default
ksplashrc KSplash Engine KSplashX
krandrrc Display ApplyOnStartup false
krandrrc Display StartupCommands ''
krandrrc [Screen0]
krandrrc [Screen1]
krandrrc [Screen2]
krandrrc [Screen3]
kcmfonts General forceFontDPI 0
kdeglobals Locale Language '' # trigger requesting languages from KLocale
EOF
kstartupconfig4
returncode=$?
if test $returncode -ne 0; then
    xmessage -geometry 500x100 "kstartupconfig4 does not exist or fails. The error code is $returncode. Check your installation."
    exit 1
fi
[ -r $kdehome/share/config/startupconfig ] && . $kdehome/share/config/startupconfig

# XCursor mouse theme needs to be applied here to work even for kded or ksmserver
if test -n "$kcminputrc_mouse_cursortheme" -o -n "$kcminputrc_mouse_cursorsize" ; then
    @EXPORT_XCURSOR_PATH@

    kapplymousetheme "$kcminputrc_mouse_cursortheme" "$kcminputrc_mouse_cursorsize"
    if test $? -eq 10; then
        XCURSOR_THEME=default
        export XCURSOR_THEME
    elif test -n "$kcminputrc_mouse_cursortheme"; then
        XCURSOR_THEME="$kcminputrc_mouse_cursortheme"
        export XCURSOR_THEME
    fi
    if test -n "$kcminputrc_mouse_cursorsize"; then
        XCURSOR_SIZE="$kcminputrc_mouse_cursorsize"
        export XCURSOR_SIZE
    fi
fi

. krandrstartup

if test "$kcmfonts_general_forcefontdpi" -ne 0; then
    xrdb -quiet -merge -nocpp <<EOF
Xft.dpi: $kcmfonts_general_forcefontdpi
EOF
fi


dl=$DESKTOP_LOCKED
unset DESKTOP_LOCKED # Don't want it in the environment

ksplash_pid=
if test -z "$dl"; then
  # languages as resolved by KLocale, for the splash screens use
  # klocale_languages is assembled by kdostartupconfig4 calling KLocale
  KLOCALE_LANGUAGES="$klocale_languages"
  export KLOCALE_LANGUAGES
  # the splashscreen and progress indicator
  case "$ksplashrc_ksplash_engine" in
    KSplashX)
      ksplash_pid=`ksplashx "${ksplashrc_ksplash_theme}" --pid`
      ;;
    KSplashQML)
      ksplash_pid=`ksplashqml "${ksplashrc_ksplash_theme}" --pid`
      ;;
    None)
      ;;
    Simple)
      ksplash_pid=`ksplashsimple "${ksplashrc_ksplash_theme}" --pid`
      ;;
    *)
      ;;
  esac
  # no longer needed in the environment
  unset KLOCALE_LANGUAGES
fi

# Source scripts found in <localprefix>/env/*.sh and <prefixes>/env/*.sh
# (where <localprefix> is $KDEHOME or ~/.kde, and <prefixes> is where KDE is installed)
#
# This is where you can define environment variables that will be available to
# all KDE programs, so this is where you can run agents using e.g. eval `ssh-agent`
# or eval `gpg-agent --daemon`.
# Note: if you do that, you should also put "ssh-agent -k" as a shutdown script
#
# (see end of this file).
# For anything else (that doesn't set env vars, or that needs a window manager),
# better use the Autostart folder.

libpath=`kde4-config --path lib | tr : '\n'`

for prefix in `echo "$libpath" | sed -n -e 's,/lib[^/]*/,/env/,p'`; do
  for file in "$prefix"*.sh; do
    test -r "$file" && . "$file"
  done
done

# Set the path for Qt plugins provided by KDE
QT_PLUGIN_PATH=${QT_PLUGIN_PATH+$QT_PLUGIN_PATH:}`kde4-config --path qtplugins`
export QT_PLUGIN_PATH

# Activate the kde font directories.
#
# There are 4 directories that may be used for supplying fonts for KDE.
#
# There are two system directories. These belong to the administrator.
# There are two user directories, where the user may add her own fonts.
#
# The 'override' versions are for fonts that should come first in the list,
# i.e. if you have a font in your 'override' directory, it will be used in
# preference to any other.
#
# The preference order looks like this:
# user override, system override, X, user, system
#
# Where X is the original font database that was set up before this script
# runs.

usr_odir=$HOME/.fonts/kde-override
usr_fdir=$HOME/.fonts

if test -n "$KDEDIRS"; then
  kdedirs_first=`echo "$KDEDIRS"|sed -e 's/:.*//'`
  sys_odir=$kdedirs_first/share/fonts/override
  sys_fdir=$kdedirs_first/share/fonts
else
  sys_odir=$KDEDIR/share/fonts/override
  sys_fdir=$KDEDIR/share/fonts
fi

# We run mkfontdir on the user's font dirs (if we have permission) to pick
# up any new fonts they may have installed. If mkfontdir fails, we still
# add the user's dirs to the font path, as they might simply have been made
# read-only by the administrator, for whatever reason.

test -d "$sys_odir" && xset +fp "$sys_odir"
test -d "$usr_odir" && (mkfontdir "$usr_odir" ; xset +fp "$usr_odir")
test -d "$usr_fdir" && (mkfontdir "$usr_fdir" ; xset fp+ "$usr_fdir")
test -d "$sys_fdir" && xset fp+ "$sys_fdir"

# Ask X11 to rebuild its font list.
xset fp rehash

# Set a left cursor instead of the standard X11 "X" cursor, since I've heard
# from some users that they're confused and don't know what to do. This is
# especially necessary on slow machines, where starting KDE takes one or two
# minutes until anything appears on the screen.
#
# If the user has overwritten fonts, the cursor font may be different now
# so don't move this up.
#
xsetroot -cursor_name left_ptr

# Get Ghostscript to look into user's KDE fonts dir for additional Fontmap
if test -n "$GS_LIB" ; then
    GS_LIB=$usr_fdir:$GS_LIB
    export GS_LIB
else
    GS_LIB=$usr_fdir
    export GS_LIB
fi

lnusertemp=`kde4-config --path exe --locate lnusertemp`
if test -z "$lnusertemp"; then
  # Startup error
  echo 'startkde: ERROR: Could not locate lnusertemp in '`kde4-config --path exe` 1>&2
fi

# Link "tmp" "socket" and "cache" resources to directory in /tmp
# Creates:
# - a directory /tmp/kde-$USER and links $KDEHOME/tmp-$HOSTNAME to it.
# - a directory /tmp/ksocket-$USER and links $KDEHOME/socket-$HOSTNAME to it.
# - a directory /var/tmp/kdecache-$USER and links $KDEHOME/cache-$HOSTNAME to it.
# Note: temporary locations can be overriden through the KDETMP and KDEVARTMP
# environment variables
for resource in tmp cache socket; do
    if "$lnusertemp" $resource >/dev/null; then
        : # ok
    else
        echo 'startkde: Call to lnusertemp failed (temporary directories full?). Check your installation.'  1>&2
        test -n "$ksplash_pid" && kill "$ksplash_pid" 2>/dev/null
        xmessage -geometry 600x100 "Call to lnusertemp failed (temporary directories full?). Check your installation."
        exit 1
    fi
done

# In case of dcop sockets left by a previous session, cleanup
#dcopserver_shutdown

echo 'startkde: Starting up...'  1>&2

# Make sure that D-Bus is running
if test -z "$XDG_DATA_DIRS"; then
    XDG_DATA_DIRS="`kde4-config --prefix`/share:/usr/share:/usr/local/share"
    export XDG_DATA_DIRS
fi
# D-Bus autolaunch is broken
if test -z "$DBUS_SESSION_BUS_ADDRESS" ; then
    eval `dbus-launch --sh-syntax --exit-with-session`
fi
if qdbus >/dev/null 2>/dev/null; then
    : # ok
else
    echo 'startkde: Could not start D-Bus. Can you call qdbus?'  1>&2
    test -n "$ksplash_pid" && kill "$ksplash_pid" 2>/dev/null
    xmessage -geometry 500x100 "Could not start D-Bus. Can you call qdbus?"
    exit 1
fi


# Mark that full KDE session is running (e.g. Konqueror preloading works only
# with full KDE running). The KDE_FULL_SESSION property can be detected by
# any X client connected to the same X session, even if not launched
# directly from the KDE session but e.g. using "ssh -X", kdesu. $KDE_FULL_SESSION
# however guarantees that the application is launched in the same environment
# like the KDE session and that e.g. KDE utilities/libraries are available.
# KDE_FULL_SESSION property is also only available since KDE 3.5.5.
# The matching tests are:
#   For $KDE_FULL_SESSION:
#     if test -n "$KDE_FULL_SESSION"; then ... whatever
#   For KDE_FULL_SESSION property:
#     xprop -root | grep "^KDE_FULL_SESSION" >/dev/null 2>/dev/null
#     if test $? -eq 0; then ... whatever
#
# Additionally there is (since KDE 3.5.7) $KDE_SESSION_UID with the uid
# of the user running the KDE session. It should be rarely needed (e.g.
# after sudo to prevent desktop-wide functionality in the new user's kded).
#
# Since KDE4 there is also KDE_SESSION_VERSION, containing the major version number.
# Note that this didn't exist in KDE3, which can be detected by its absense and
# the presence of KDE_FULL_SESSION.
#
KDE_FULL_SESSION=true
export KDE_FULL_SESSION
xprop -root -f KDE_FULL_SESSION 8t -set KDE_FULL_SESSION true

KDE_SESSION_VERSION=4
export KDE_SESSION_VERSION
xprop -root -f KDE_SESSION_VERSION 32c -set KDE_SESSION_VERSION 4

KDE_SESSION_UID=`id -ru`
export KDE_SESSION_UID

# We set LD_BIND_NOW to increase the efficiency of kdeinit.
# kdeinit unsets this variable before loading applications.
LD_BIND_NOW=true @KDE4_LIBEXEC_INSTALL_DIR@/start_kdeinit_wrapper +kcminit_startup
if test $? -ne 0; then
  # Startup error
  echo 'startkde: Could not start kdeinit4. Check your installation.'  1>&2
  test -n "$ksplash_pid" && kill "$ksplash_pid" 2>/dev/null
  xmessage -geometry 500x100 "Could not start kdeinit4. Check your installation."
  exit 1
fi

# If the session should be locked from the start (locked autologin),
# lock now and do the rest of the KDE startup underneath the locker.
if test -n "$dl"; then
  if ! kwrapper4 kscreenlocker --locked; then
    echo 'startkde: Initial session lock failed. Terminating for security reasons.' 1>&2
    exit 1
  fi
fi

# finally, give the session control to the session manager
# see kdebase/ksmserver for the description of the rest of the startup sequence
# if the KDEWM environment variable has been set, then it will be used as KDE's
# window manager instead of kwin.
# if KDEWM is not set, ksmserver will ensure kwin is started.
# kwrapper4 is used to reduce startup time and memory usage
# kwrapper4 does not return useful error codes such as the exit code of ksmserver.
# We only check for 255 which means that the ksmserver process could not be
# started, any problems thereafter, e.g. ksmserver failing to initialize,
# will remain undetected.
test -n "$KDEWM" && KDEWM="--windowmanager $KDEWM"
kwrapper4 ksmserver $KDEWM
if test $? -eq 255; then
  # Startup error
  echo 'startkde: Could not start ksmserver. Check your installation.'  1>&2
  test -n "$ksplash_pid" && kill "$ksplash_pid" 2>/dev/null
  xmessage -geometry 500x100 "Could not start ksmserver. Check your installation."
fi

# wait if there's any crashhandler shown
while qdbus | grep "^[^w]*org.kde.drkonqi" > /dev/null ; do
    sleep 5
done

echo 'startkde: Shutting down...'  1>&2
# just in case
test -n "$ksplash_pid" && kill "$ksplash_pid" 2>/dev/null

# Clean up
kdeinit4_shutdown
# KDE3 support
kde3 kdeinit_shutdown 2>/dev/null
kde3 dcopserver_shutdown --wait 2>/dev/null

echo 'startkde: Running shutdown scripts...'  1>&2

# Run scripts found in $KDEDIRS/shutdown
for prefix in `echo "$libpath" | sed -n -e 's,/lib[^/]*/,/shutdown/,p'`; do
  for file in `ls "$prefix" 2> /dev/null | egrep -v '(~|\.bak)$'`; do
    test -x "$prefix$file" && "$prefix$file"
  done
done

unset KDE_FULL_SESSION
xprop -root -remove KDE_FULL_SESSION
unset KDE_SESSION_VERSION
xprop -root -remove KDE_SESSION_VERSION
unset KDE_SESSION_UID

echo 'startkde: Done.'  1>&2
