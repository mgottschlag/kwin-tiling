#!/bin/sh

prefix=`kde4-config --localprefix`
source="${prefix}/share/config"
dest="${prefix}/share/config/session"

# move session config files

if [ -n "$prefix" -a -d "$source" ]; then
	while [ ! -d "$dest" ]; do
		dir="$dest"
		while [ ! -d `dirname "$dir"` ]; do
			dir=`dirname "$dir"`
		done
		mkdir "$dir" || exit 1
	done

	files=`eval ls -1 "$source/*:[0-9a-f]*" 2> /dev/null`
	if [ -n "$files" ]; then
		for i in $files; do
                        origfile=`basename "$i"`
                        newfile=`echo "$origfile" | sed -e 's^:^_^'`
                        if [ -n "$newfile" -a ! -e "$dest/$newfile" ]; then
                                mv "$source/$origfile" "$dest/$newfile"
                        fi
		done
	fi
fi

# update references in ksmserverrc

sed -e 's^share/config/\([^/:]*\):^share/config/session/\1_^'
