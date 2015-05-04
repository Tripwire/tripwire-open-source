#!/bin/bash

#
# 2011-11-21 - v0.2 : added more checks
# 2011-09-21 - v0.1 : first attempt to create a custom file
# FRLinux - tripwire@frlinux.net
#

echo " "
echo "Please run this as ROOT, failing to do so might prevent from"
echo "indexing all directories."
echo " "

BASECONFIG="."
POL="$BASECONFIG/../twpol.txt"
POL_HEADER="$BASECONFIG/twpol-header.txt"
POL_FOOTER="$BASECONFIG/twpol-footer.txt"

echo "# `date` " > $POL
echo "#" >> $POL
echo "# New Policy file format - FRLinux tripwire@frlinux.net" >> $POL
echo "# This is experimental, check the readme before using this." >> $POL
echo "#" >> $POL
cat $POL_HEADER >> $POL

echo "" >> $POL
echo "# File System and Disk Administration Programs." >> $POL
echo "" >> $POL

if [ `uname -i` = "x86_64" ]; then
dir64="/lib64"
else
dir64=""
fi
for dir in /bin /sbin /lib /etc $dir64
do
	echo "(" >> $POL
	echo "   rulename = \"files in $dir\"," >> $POL
	echo "   severity = \$(SIG_HI)" >> $POL
	echo ")" >> $POL
	echo "{" >> $POL
	find $dir -type f -printf "	%p 				-> \$(SEC_CRIT);\n" >> $POL
	echo "}" >> $POL
	echo "" >> $POL
done

for type in root sys
do
	cat $type.tpl >> $POL
done

cat $POL_FOOTER >> $POL

echo " "
echo "Copy this file in $TRIPWIRE_INSTALL/etc/twpol.txt"
echo "then execute:  tripwire --update-policy twpol.txt"
echo " "
echo "Done."

