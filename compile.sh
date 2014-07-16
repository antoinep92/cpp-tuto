#!/bin/sh
while true ; do
	hash=$(shasum $1 | cut -d' ' -f1)
	clang++ -std=c++11 -g -fcolor-diagnostics -fsanitize=undefined -fsanitize=address -fsanitize=integer $1 2>/tmp/build
	kill %1
	less -R /tmp/build &
	while true ; do
		inotifywait -qq $1
		newhash=$(shasum $1 | cut -d' ' -f1)
		if [ $hash != $newhash ] ; then
			break
		fi
	done
done
