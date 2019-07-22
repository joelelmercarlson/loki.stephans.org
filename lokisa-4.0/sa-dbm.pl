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
# @(#) $Id: sa-dbm.pl,v 1.57 2004/02/02 08:58:42 ast Exp $
#
# Create the database for the sa-procmail filter. This was inspired
# by http://www.paulgraham.com/spam.html
#
use strict;
use vars qw($revision $me %legal_db);

$revision = sprintf "v%s/%s", 
    sprintf("%d.%02d", q$Revision: 1.57 $ =~ /(\d+)\.(\d+)/),
    $LokiSa::VERSION;

($me = $0) =~ s!.*/!!go;

map { /^(.)/o; $legal_db{$1} = $_; } qw(good spam count white);

BEGIN {
    unshift @INC, $1 if $0 =~ m!^(.*)/!o;
    1;
}

use LokiSa;

sub db_name ($);
sub db_error ();
sub no_access ($);
sub listing_error ();

sub usage ()
{
    return join "\n",
	"Usage: $me [-D db_dir] [-v|-q] [-y] { -w <white> | -s <spam> | -g <good> }",
	"         where <...> is a filename", 
	"       $me [-D db_dir] [-v|-q] [-y] { -u | -r | -n <db> }",
	"         where <db> is one of white, spam, good, count", 
	"       $me [-D db_dir] [-v|-q] {-a|-d} email_address [, ...]",
	"       $me [-h] [-V]",
	"       -h: print this help",
	"       -v: verbose (repeat for more verbosity)",
	"       -q: quiet (verbosity level is 1 for building, otherwise 0)",
	"       -D <db_dir>: override default <db_dir>",
	"       -s <spam>: build spam db from <spam> corpus (updates count db as well)",
	"       -g <good>: build good db from <good> corpus (updates count db as well)",
	"       -u: show database white, spam, good, or count; unsorted",
	"       -r: show database white, spam, good, or count; sorted alphabetically",
	"       -n: show database white, spam, good, or count; sorted reverse numerically",
	"       -w <white>: build white db from <white> list",
	"       -a: adds the given addresse(s) into the whitelist db",
	"       -d: deletes the given addresse(s) out of the whitelist db",
	"       -y: prompt `Are you sure?' (for interactive scripts)",
	"       -V: print revision ($revision) and exit",
	"";
}
die usage unless @ARGV;

my %options	= ();
my $verbose	= 0;
my $quiet	= 0;
my $listing	= '';
my $sorting	= '';
my $dir		= '';
my @args	= ();
my %db		= ();
my $del_white	= 0;
my $add_white	= 0;
my $prompt	= 0;
my @argv	= @ARGV;

my %parseoptions = (
    h => sub { 
    	die usage;
    },
    v => sub {
    	$verbose++;
    },
    w => sub {
    	$_ = shift @ARGV;
	die no_access($_) unless ($_ eq "-" or -r $_);
	$db{w} = $_;
    },
    s => sub {
    	$_ = shift @ARGV;
	die no_access($_) unless ($_ eq "-" or -r $_);
	$db{s} = $_;
    },
    g => sub {
    	$_ = shift @ARGV;
	die no_access($_) unless ($_ eq "-" or -r $_);
	$db{g} = $_;
    },
    n => sub {
    	if ($listing) {
	    warn listing_error;
	    die usage;
	}
	$sorting = 'n';
	$listing = db_name(shift @ARGV);
    },
    r => sub {
    	if ($listing) {
	    warn listing_error;
	    die usage;
	}
	$sorting = 'r';
	$listing = db_name(shift @ARGV);
    },
    u => sub {
    	if ($listing) {
	    warn listing_error;
	    die usage;
	}
	$sorting = 'u';
	$listing = db_name(shift @ARGV);
    },
    a => sub {
	die "add or delete separately\n", usage
	    if scalar(%db) > 0 || $del_white;
	$add_white++;
    },
    d => sub {
	die "add or delete separately\n", usage
	    if scalar(%db) > 0 || $add_white;
	$del_white++;
    },
    q => sub {
    	$quiet++;
    },
    V => sub { 
    	warn "$me $revision\n"; 
	exit(0); 
    },
    D => sub {
	die "Already saw '-D $dir' option\n", usage if $dir;
	$dir = shift @ARGV;
	die "Argument '$dir' is not a directory\n", usage unless -d $dir;
	$options{db_dir} = $dir;
    },
    y => sub { 
    	$prompt++;
    },
);

while(@ARGV) {
    last unless $ARGV[0] =~ /^-([hvcwsgnruadqVDy])$/o;
    shift;
    $parseoptions{$1}->();
}

# keep args free of <>
for (@ARGV) {
    s/[<>]//go; push @args, $_;
}
die "Missing arguments" if @args == 0 && ( $del_white || $add_white );

# Handle -y for scripts
if ($prompt) {
    do {
	print "Invoking $me ", join(" ", @argv), ". Are you sure [y/n]? ";
	my $ans = <STDIN>;
	exit if $ans !~ /^y/i;
    } while (!$prompt);
}

my $sa = new LokiSa %options;

$sa->debug($verbose);
$sa->dump() if $verbose > 1;

if ( $del_white || $add_white ) {
    my $dict = $sa->open_db('white', 0600);
    for ( @args ) {
	my $lc = lc $_;
	my $da = lc $1 if m!(@.+)!o;
	if ( $add_white ) {
	    if ( defined $$dict{$_} || defined $$dict{$lc} ) {
		warn "'$lc' already exists in whitelist, skipping\n";
	    } elsif ( $da && defined $$dict{$da} ) {
		$$dict{$lc} = $_;
		warn "'$da' exists in whitelist, but added '$lc' anyway\n";
	    } else {
		$$dict{$lc} = $_;
		warn "'$lc' added to whitelist\n" if $verbose;
	    }
	} else {
	    my $w = "";
	    $w = "; wildcard '$da' still exists" if $da && $da ne $lc && defined $$dict{$da};
	    if ( defined $$dict{$_} || defined $$dict{$lc} ) {
		# we used to store case sensitive although we always
		# tested non case sensitive so we allow for deletion
		# of the case-sensitive ones here
		delete $$dict{$_};
		delete $$dict{$lc};
		warn "'$_' deleted from whitelist$w\n" if $verbose || $w;
	    } else {
		warn "'$_' doesn't exist$w\n";
	    }
	}
    }
}

if ( scalar(keys %db) > 0 ) {
    $sa->debug(1) unless $quiet || $verbose;
    for ( $sa->databases() ) {
	next unless /^(.)/o && $db{$1};
	$sa->build_db($_, $db{$1});
    }
}

if ( $listing ) {
    $sa->show_db($listing, $sorting);
}

$sa->dump() if $verbose > 2;

exit (0);

############
sub db_name ($)
{
    my $name = shift;
    die usage unless $name;
    die no_access($name) unless $name =~ /^(.)/o && exists $legal_db{lc $1};
    return $legal_db{lc $1};
}

sub db_error ()
{
    return "Can only process one DB at the same time!\n";
}

sub no_access ($)
{
   my $name = shift;
   return "Can't access file or db '$name'\n";
}

sub listing_error ()
{
   return "Can only list one way at a time!\n";
}

