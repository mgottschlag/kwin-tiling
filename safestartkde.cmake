#!/bin/sh
#
#  FAILSAFE KDE STARTUP SCRIPT ( @KDE_VERSION_STRING@ )
#

# This script launches KDE with some potentionally unstable parts (like
# compositing effects) disabled.

# General failsafe flag
KDE_FAILSAFE=1
export KDE_FAILSAFE

# Disable KWin's compositing
KWIN_COMPOSE=N
export KWIN_COMPOSE

exec @KDE4_BIN_INSTALL_DIR@/startkde "$@"
