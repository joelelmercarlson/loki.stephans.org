#!/usr/bin/perl -w
#
# Copyright (c) 2002-2003; All rights reserved.
# by  Joel E. Carlson      <loki@dealund.com>
# and Dr. Adrian Steinmann <ast@marabu.ch>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of Joel E. Carlson nor of Dr. Adrian Steinmann
#    nor the names of any contributors may be used to endorse or
#    promote products derived from this software without specific
#    prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY JOEL E. CARLSON AND ADRIAN STEINMANN
# AND CONTRIBUTORS ``AS IS,'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# @(#) $Id: spamgrep.pl,v 1.3 2003/04/05 07:40:19 ast Exp $
#
# spamgrep.pl tears out entries from procmail.log for counting
#
use strict;
use vars qw($VERSION $revision $me);
$|++;

use IO;

$VERSION	= sprintf("%d.%02d", q$Revision: 1.3 $ =~ /(\d+)\.(\d+)/);
$revision	= sprintf "v%s", $VERSION;

($me = $0) =~ s!.*/!!go;


sub usage
{
    return 
	join "\n",
	"Usage: $me [-h] [-c]",
	"       $me [-t] [-p] [-l]",
	"       -c  only display counts.",
	"       -t: only display the last number of records.",
	"       -l: logfile. Defaults to \$HOME/procmail.log",
	"       -p: pattern. Defaults to \$HOME/mail/SPAMBOX",
	"       -h  This help screen.",
	"       -V: print revision ($revision) and exit",
	"";
}


my @entry;
my @entries;
my $scount = 0;
my $tcount = 0;
my $size  = 0;
my $count = 0;
my $tail  = 0;
my $pattern = 'SPAMBOX';
my $logfile = 'procmail.log';

my %parseoptions = (
   c => sub { $count++; },
   t => sub { $tail = shift @ARGV; },
   p => sub { $pattern = shift @ARGV; },
   l => sub { $logfile = shift @ARGV; },
   h => sub { die usage; },
   V => sub { warn "$me $revision\n"; exit(0); },
);


while (@ARGV) {
    last unless $ARGV[0] =~ /^-([ctplhV])$/o;
    shift;

    $parseoptions{$1}->();
}

$pattern = $ENV{HOME} . '/mail/' . $pattern;
$logfile = $ENV{HOME} . '/' . $logfile;

die "$logfile: $!" unless -r $logfile;


#
# int main(int argc, char *argv[])
#
my $fh = new IO::File ("< $logfile");
while (<$fh>) {
    push @entry, $_;
    $tcount++ if (m!^From!o);
    if (m!$pattern.*\s(\d+)!io) {
        $scount++;
	$size += $1;

	push @entries, window(@entry);
	undef @entry;
    }
}
undef $fh;

unless ($count>0) { 
    if ($tail) {
	for ( ($#entries-$tail) .. $#entries) { 
	    next unless $entries[$_];
	    print $entries[$_]; 
	    line(); 
	}
    } else {
	foreach (@entries) { 
	    print; 
	    line(); 
	}
    }
}

printf "count: %d of %d (%02.2f%s)\n", 
    $scount, $tcount, 100*($scount/$tcount), '%';
printf "size:  %02.2f KB\n",  $size>>10;

#
# end main

sub line {
    print '-' x 80, "\n";
}

sub window {
    my @w  = @_;
    my $size = $#w;
    my $offset = 3;
    my $r;

    for ( ($size-$offset) .. $size ) {
	$r .= $w[$_];
    }
    return $r;
}
