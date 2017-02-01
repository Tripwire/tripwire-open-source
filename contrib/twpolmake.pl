#!/usr/bin/perl
# Tripwire Policy File customize tool
# ----------------------------------------------------------------
# Copyright (C) 2003 Hiroaki Izumi
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# ----------------------------------------------------------------
# Usage:
#    perl twpolmake.pl {Pol file}
# ----------------------------------------------------------------
#
$POLFILE=$ARGV[0];

open(POL,"$POLFILE") or die "open error: $POLFILE" ;
my($myhost,$thost) ;
my($sharp,$tpath,$cond) ;
my($INRULE) = 0 ;

while (<POL>) {
    chomp;
    if (($thost) = /^HOSTNAME\s*=\s*(.*)\s*;/) {
        $myhost = `hostname` ; chomp($myhost) ;
        if ($thost ne $myhost) {
            $_="HOSTNAME=\"$myhost\";" ;
        }
    }
    elsif ( /^{/ ) {
        $INRULE=1 ;
    }
    elsif ( /^}/ ) {
        $INRULE=0 ;
    }
    elsif ($INRULE == 1 and ($sharp,$tpath,$cond) = /^(\s*\#?\s*)(\/\S+)\b(\s+->\s+.+)$/) {
        $ret = ($sharp =~ s/\#//g) ;
        if ($tpath eq '/sbin/e2fsadm' ) {
            $cond =~ s/;\s+(tune2fs.*)$/; \#$1/ ;
        }
        if (! -s $tpath) {
            $_ = "$sharp#$tpath$cond" if ($ret == 0) ;
        }
        else {
            $_ = "$sharp$tpath$cond" ;
        }
    }
    print "$_\n" ;
}
close(POL) ;
