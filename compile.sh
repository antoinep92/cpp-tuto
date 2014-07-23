#!/bin/sh
if test -z $1 ; then
	/bin/sh $0 $1 2>/dev/null
	exit
fi
LESSID=
while true ; do
	hash=$(shasum $1 | cut -d' ' -f1)
	clang++ -std=c++11 -g -fcolor-diagnostics -fsanitize=undefined -fsanitize=address -fsanitize=integer $1 2>/tmp/build
	if test -n $LESSID ; then 
		kill $LESSID 2>/dev/null
		wait $LESSID 2>/dev/null
	fi
	less -R /tmp/build 2>/dev/null &
	LESSID=$!
	while true ; do
		inotifywait -qq $1
		newhash=$(shasum $1 | cut -d' ' -f1)
		if [ $hash != $newhash ] ; then
			break
		fi
	done
done
