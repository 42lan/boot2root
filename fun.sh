#!/bin/bash

# Get all files containing a string 'file' in it
files=$(grep -E "file[0-9]{1,3}$" *)

# Pattern of grep output wrapped in capture groups
#(ZQTK1.pcap://file687)
regex="(.*)://(.*)"

# Itterate over the files and rename them according to file number in the comment
for file in $files
do
  if [[ $file =~ $regex ]]
  then
    filename=${BASH_REMATCH[1]}
    fileno=${BASH_REMATCH[2]}
    #echo $filename $fileno
    mv $filename $fileno
  fi
done

# Concatenate all files sorted numerically separated by a newline
for file in $(ls -v file*)
do
  (cat $file; echo) >> main.c
done

# Compile and execute
gcc main.c && ./a.out
