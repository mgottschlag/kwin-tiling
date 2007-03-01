#! /bin/sh

# This script creates a KSplashX theme from KSplashML's theme using the Default engine.
# Just run in the theme's directory with theme name as argument and it'll create a new subdirectory "theme".
# Note that the positions in the icon bar may be a bit off, as KSplashML itself has them off;
# if it doesn't work for you, fiddle with the positions below (two places, also the description.txt file)
# You should also afterwards check Theme.rc (only Engine= and descriptive fields like Name= should matter)

# the -depth 8 is there to produce smaller png's (it doesn't seem to affect anything else *shrug*)

if test $# -ne 1; then
    echo Theme name missing.
    exit 1
fi
theme="$1"

# check
grep "Engine = Default" Theme.rc >/dev/null 2>&1 || ( echo "No Theme.rc or Engine is not Default"; exit 1 )

# create the subdir
rm -r "$theme" 2>/dev/null
mkdir "$theme"
mkdir "$theme"/1600x1200

# create background image from top, bar and bottom
montage splash_top.png splash_inactive_bar.png splash_bottom.png -geometry +0+0 -tile 1x3 -depth 8 "$theme"/1600x1200/background.png

# cut the icons from the icon bar
# numbers are x position, width and icon number
for i in 0-58-1 58-58-2 116-58-3 174-58-4 232-58-5 290-50-6 340-60-7; do
    x=`echo $i | sed 's/\(.*\)-.*-.*/\1/'`
    w=`echo $i | sed 's/.*-\(.*\)-.*/\1/'`
    n=`echo $i | sed 's/.*-.*-\(.*\)/\1/'`
    convert splash_inactive_bar.png -crop ${w}x58+${x}+0 -depth 8 iconin.png
    convert splash_active_bar.png -crop ${w}x58+${x}+0 -depth 8 iconac.png
    montage iconac.png iconin.png -tile 2x1 -geometry +0+0 -depth 8 "$theme"/1600x1200/icon${n}_anim.png
    cp iconac.png "$theme"/1600x1200/icon${n}.png
    rm iconin.png iconac.png
done

# preview file
cp Preview.png "$theme"/Preview.png

# Theme.rc file
cat Theme.rc \
    | sed 's/Engine = Default/Engine = KSplashX/' \
    | sed "s/\[KSplash Theme: .*\]/\[KSplash Theme: $theme\]/" \
    > "$theme"/Theme.rc

# CMakeLists.txt

cat >"$theme"/CMakeLists.txt <<EOF
install( FILES Preview.png Theme.rc DESTINATION \${DATA_INSTALL_DIR}/ksplash/Themes/$theme )
install( FILES 1600x1200/description.txt 1600x1200/background.png
    1600x1200/icon1.png 1600x1200/icon2.png 1600x1200/icon3.png
    1600x1200/icon4.png 1600x1200/icon5.png 1600x1200/icon6.png 1600x1200/icon7.png
    1600x1200/icon1_anim.png 1600x1200/icon2_anim.png 1600x1200/icon3_anim.png
    1600x1200/icon4_anim.png 1600x1200/icon5_anim.png 1600x1200/icon6_anim.png 1600x1200/icon7_anim.png
    DESTINATION \${DATA_INSTALL_DIR}/ksplash/Themes/${theme}/1600x1200 )
EOF

# now the description file

cat >"$theme"/1600x1200/description.txt <<EOF
SCALE OFF
GEOMETRY_REL CC 0 0 CC 400 270
IMAGE 0 0 background.png
ANIM 1 0 190 2 icon1_anim.png 400
WAIT_STATE 1

STOP_ANIM 1
IMAGE 0 190 icon1.png
ANIM 2 58 190 2 icon2_anim.png 400
WAIT_STATE 2

STOP_ANIM 2
IMAGE 58 190 icon2.png
ANIM 3 116 190 2 icon3_anim.png 400
WAIT_STATE 3

STOP_ANIM 3
IMAGE 116 190 icon3.png
ANIM 4 174 190 2 icon4_anim.png 400
WAIT_STATE 4

STOP_ANIM 4
IMAGE 174 190 icon4.png
ANIM 5 232 190 2 icon5_anim.png 400
WAIT_STATE 5

STOP_ANIM 5
IMAGE 232 190 icon5.png
ANIM 6 290 190 2 icon6_anim.png 400
WAIT_STATE 6

STOP_ANIM 6
IMAGE 290 190 icon6.png
ANIM 7 340 190 2 icon7_anim.png 400
WAIT_STATE 7

STOP_ANIM 7
IMAGE 340 190 icon7.png
WAIT_STATE 8
EOF
