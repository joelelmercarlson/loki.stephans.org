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
# @(#) $Id: sa-filter.pl,v 1.44 2003/04/04 15:51:41 loki Exp $
#
# Read an email via stdin and print the email along with scoring 
# headers on stdout
#
use strict;
use vars qw($VERSION $revision $me);

$VERSION	= sprintf("%d.%02d", q$Revision: 1.44 $ =~ /(\d+)\.(\d+)/);
$revision	= sprintf "v%s/%s", $VERSION, $LokiSa::VERSION;

($me = $0) =~ s!.*/!!go;

BEGIN {
    unshift @INC, $1 if $0 =~ m!^(.*)/!o;
    1;
}


use LokiSa;
my $mt = $LokiSa::max_tuples;
my $ms = $LokiSa::max_size;

sub finish_message($);

sub usage ()
{
    return
	join "\n",
	"Usage: $me [-h] [-v] [-V] [-H|-W|-N] [-M #] [-D db_dir]",
	"       $me [-d #] [-n #] [-m #] [-t threshold] [-u u_weight]",
	"       -h: print this help",
	"       -D db_dir: override default db dir",
	"       -a: use all n-tuples for Fisher calculation (default skips neutral ones)",
	"       -n #: maximum n-tuple length to consider in mailbody (maximum $mt)",
	"       -m #: maximum n-tuple length to consider in mailheader (maximum $mt)",
	"       -t threshold: cutoff for Red/Green (integer between 0% and 100%)",
	"       -u u_weight: unknown word weight (integer between 0% and 100%)",
	"       -v: verbose (repeat for more verbosity) - not for procmail!",
	"       -N: print the procmail pattern to catch whitelist (nok) and exit",
	"       -W: print the procmail pattern to catch whitelist (ok) and exit",
	"       -H: print the procmail pattern to catch spam and exit",
	"       -M <kilobytes>: maximum message size to weight (0 unlimited, default $ms)",
	"       -V: print revision ($revision) and exit",
	"";
}

my %options	= ();
my $verbose	= 0;
my $options	= 'ahvnmutNWHMVD';

my %parseoptions = (
    h => sub {
	die usage;
    },
    v => sub {
	$verbose++;
    },
    a => sub {
	$options{skip_neutral} = 0;
    },
    n => sub {
	my $n = shift @ARGV;
	my $usage = "-n takes a positive integer\n";
	die $usage unless $n =~ /^(\d*)$/o;
	die $usage unless $1 > 0;
	$options{bdy_tuples} = $1;
    },
    m => sub {
	my $n = shift @ARGV;
	my $usage = "-m takes a positive integer\n";
	die $usage unless $n =~ /^(\d+)$/o;
	die $usage unless $1 > 0;
	$options{hdr_tuples} = $1;
    },
    u => sub {
	$options{u_weight}	= shift @ARGV;
    },
    s => sub {
    },
    t => sub {
	$options{threshhold}	= shift @ARGV;
    },
    H => sub {
	print new LokiSa->procmailpattern($VERSION);
	exit(0);
    },
    W => sub {
	print new LokiSa->procmailwhite($VERSION);
	exit(0);
    },
    N => sub {
	print new LokiSa->procmailwhitenok($VERSION);
	exit(0);
    },
    M => sub {
	my $m = shift @ARGV;
	if ($m =~ /^(\d+)([km]?)$/oi) {
	    $m = $1 << 10;
	    $m <<= 10 if $2 =~ /m/oi;
	}
	my $usage = "Option -M requires a positive integer with an optional abbreviation\n" .
	"Example: -M 10m (K is kilobytes, M is megabytes; case insensitive)\n";
	die $usage unless $m >= 1;
	$ms = $m;
    },
    V => sub {
	warn "$me $revision\n";
	exit(0);
    },
    D => sub {
	my $dir = shift @ARGV;
	die "'$dir' not a directory\n", usage unless -d $dir;
	die "Already saw '-D $dir' option\n", usage if $options{db_dir};
	$options{db_dir} = $dir;
    },
);

while (@ARGV) {
    last unless $ARGV[0] =~ /^-([$options])$/o;
    shift;
    $parseoptions{$1}->();
}

my $sa = new LokiSa %options;

$sa->debug($verbose);
$sa->dump() if $verbose > 1;

$|++;

# handle one or more messages on the input stream

# read the headers and check the From address against the
# white list; if it is whitelisted, print out the rest of
# the message quickly, otherwise procrastinate printing
# until we can add the combined_probability to the headers

my $from	= $sa->from_pattern();
my $pgraphs	= undef;

# weight the message headers and bodies paragraph by paragraph
$/ = "\n\n";
while (<>) {
    my $m = 0;
    my $l = length $_;
    if ( /$from/mo ) {
	$m = $l;
	if ( $pgraphs ) {
	    # mbox mode: flush last message and setup for next one
	    finish_message($pgraphs);
	    $sa->init();
	}
	# we're in a message header; /$from/ stores address in $1
	$sa->from_address($1);
	warn sprintf "%s\nADR %s\n", "="x72,
	    $sa->from_address() if $verbose > 2;
	chop;
	$pgraphs = [$_];
	warn sprintf "HDR paragraph %3d: %8d bytes\n",
	    scalar(@$pgraphs), $l if $verbose > 2;
	$sa->weight_header();
    } else {
	$m += $l;
	push @$pgraphs, $_;
	warn sprintf "BDY paragraph %3d: %8d bytes\n",
	    scalar(@$pgraphs), $l if $verbose > 2;
	# weight until -M limit is hit
	$sa->weight_body() if !$ms || $m < $ms
    }
}

finish_message($pgraphs);

exit(0);

##############
sub finish_message($)
{
    my $pgraphs = shift;
    return 0 unless @$pgraphs > 0;
    my $lokisa = $sa->mailheader($VERSION, $ms);
    print shift @$pgraphs, $lokisa;
    print "\n", @$pgraphs;
    warn sprintf "%s%s\n", $lokisa, "="x72 if $verbose;
    1;
}
