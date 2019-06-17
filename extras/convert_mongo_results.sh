#!/bin/bash

# A shell script to convert the results document
# created by Robo3T into a  JSON document ready 
# for importing back as apreadsheet etc.

# turn into json array
sed -i '1s/^/[\n/' $1
sed -i '$ a ]' $1

# remove object number comments
sed -i 's/\/\* 1 \*\///g' $1 
sed -i 's/\/\* [0-9]* \*\//,/g' $1

#convert object ids
sed -i 's/ObjectId(/{"$oid": /g' $1 
sed -i 's/"),/"},/g' $1 
