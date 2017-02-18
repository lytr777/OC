#!/usr/bin/env bash

#find -L $1 -mtime +7 -type l
for f in `find -L $1 -mtime +7`
do
if [[ ! -e $f ]]
then
echo $f
fi
done
exit 0
