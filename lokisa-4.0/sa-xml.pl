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
# @(#) $Id: sa-xml.pl,v 1.28 2003/04/03 06:29:12 ast Exp $
#
# Create the database for the sa-procmail filter. This was inspired
# by http://www.paulgraham.com/spam.html
#
use strict;
use vars qw($revision $me);

$revision = sprintf "v%s/%s", 
    sprintf("%d.%02d", q$Revision: 1.28 $ =~ /(\d+)\.(\d+)/),
    $LokiSa::VERSION;

($me = $0) =~ s!.*/!!go;

BEGIN {
    unshift @INC, $1 if $0 =~ m!^(.*)/!o;
    1;
}

use LokiSa;
use XML::TreeBuilder;
use XML::Writer;
use IO::File;

#
# prototypes
sub import_db($);
sub export_db($$$);
sub encode_element($);
sub decode_element($);
sub db_error();
sub ie_error();
sub noaccess($);

sub usage ()
{
    return join "\n",
	"Usage: $me [-D db_dir] [-v|-q] [-e {file|-}] [-y] { -w | -s | -g }",
	"       $me [-D db_dir] [-v|-q] [-i {file|-}] [-y]",
	"       $me [-h] [-V]",
	"       -h: print this help",
	"       -D db_dir: override default db dir",
	"       -s export spam DB",
	"       -g export good DB",
	"       -w export whitelist DB",
	"       -i file: import from file",
	"       -e: export",
	"       -v: verbose (repeat for more verbosity)",
	"       -q: quiet (verbosity level is 1 for building, otherwise 0)",
	"       -y: prompt ``Are you sure?'' - good for scripting",
	"       -V: print revision ($revision) and exit",
	"";
}
die usage unless @ARGV;


my %options	= ();
my $verbose	= 1;
my $quiet	= 0;
my $dir		= '';
my $prompt	= 0;
my @argv	= @ARGV;
my $db		= '';
my $file	= '';
my $efile	= '';
my $ifile	= '';

my %parseoptions = (
    h => sub { 
    	die usage;
    },
    v => sub {
    	$verbose++;
    },
    s => sub {
    	if ($db) {
	    warn db_error;
	    die usage;
	}
	$db = 's';
    },
    g => sub {
	if ($db) {
	    warn db_error;
	    die usage;
	}
	$db = 'g';
    },
    w => sub {
	if ($db) {
	    warn db_error;
	    die usage;
	}
	$db = 'w';
    },
    i => sub {
	if ($efile) {
	    warn ie_error;
	    die usage;
	}
	$ifile = shift @ARGV;
	die noaccess($ifile) unless ($ifile eq '-' or -r $ifile);
    },
    e => sub {
	if ($ifile) {
	    warn ie_error;
	    die  usage;
	}
	$efile = shift @ARGV;
	die noaccess($efile) unless ($efile eq '-');
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
    last unless $ARGV[0] =~ /^-([hvsgwieqVDy])$/o;
    shift;
    $parseoptions{$1}->();
}
$file = shift @ARGV if @ARGV;

my $sa = new LokiSa %options;

if ( $db ) {
    for ( $sa->databases() ) {
	/^(.)/o;
	next unless $db eq $1;
	$db = $_;
	last;
    }
} elsif ( $efile eq '-' ) {
    die "Need to specify a DB or input readable input file\n", usage
	unless $file && -r $file;
} elsif ( $ifile && $file ) {
    die "Can't read file $file\n", usage
	unless $file eq '-' || -r $file;
}

die "Need a DB { -w | -s | -g } if -e" if ($efile and not $db); 

# Handle -y for scripts
if ( $prompt ) {
    do {
	print "Invoking $me ", join(" ", @argv), ". Are you sure [y/n]? ";
	my $ans = <STDIN>;
	exit if $ans !~ /^y/i;
    } while (!$prompt);
}

$sa->debug($verbose) unless $quiet;
$sa->dump() if $verbose > 1;

$ifile ? import_db($ifile)
    : $efile ? export_db($db, $efile, $file)
	: die "$me: missing -i or -e option\n", usage;

$sa->dump() if $verbose > 2;
exit(0);

############
sub import_db($)
# requires: $ifile = import file
{
    my ($ifile) = @_;
    my $db;
    my %seen;

    my $t0 = time();
    my $count = 0;
    my $kb    = 0;
    my $dt    = 0;

    my $tree = new XML::TreeBuilder;
    $tree->parse_file($ifile);

    my ($v) = $tree->look_down(_tag => "lokisa");
    $v = $v->attr("database");
    $db = $v;

    my @sa = $tree->look_down(_tag => "sa");
    for my $href (@sa) {
	for (keys %$href ) {
	    if ($_ eq "_content") {
		$seen{ decode_element($href->{$_}->[0]) } = $href->attr("v");
		$count++;
	    }
	}
    }
    undef @sa;
    $tree = $tree->delete;

    $dt = time() - $t0;

    if ($verbose) {
	$sa->dump() if $verbose > 1;
	warn sprintf "%d XML tokens\n", $count;
	if ( $ifile ne '-' ) {
	    $kb = (-s $ifile)>>10;
	    warn sprintf "   [%5.2f MB] in %4ds: %4.1f KB/s\n  from %s\n",
		$kb>>10, $dt,
		$dt ? $kb/$dt : 0,
		$ifile;
	}
    }

    $sa->build_db($db, \%seen) if %seen;
}

sub export_db($$$)
# export_db($db, $efile, $path)
# $db = database name, $efile = outputfile
# $path = filename (defined only when doing one input file)
{   
    my ($db, $efile, $path) = @_;
    my $dict = {};
 
    unless ($path) {
        $dict = $sa->open_db($db);
    } else {
	my $t0 = time();
    	my $fh = new IO::File "<$path";
	my ($num, $kb, $count) = $sa->messages($fh, $db, $dict);
	my $dt = time() - $t0;

	if ($verbose) {
	    $sa->dump() if $verbose > 1;
	    warn sprintf "%d %d:%d-atomtuples\n",
		$count, $sa->hdr_tuples(), $sa->bdy_tuples();
	}
	undef $fh;
    }

    my $t0	= time();
    my $out	= new IO::File ">$efile";
    my $writer	= new XML::Writer(OUTPUT=>$out, DATA_MODE=>1, DATA_INDENT=>2);
    my $count	= 0;
    my $dt	= 0;

    $writer->xmlDecl("UTF-8");
    $writer->doctype("lokisa", undef, "lokisa.dtd");
    $writer->startTag("lokisa", "database" => $db);

    for (keys %$dict) { 
	$writer->startTag("sa", "v" => $$dict{$_});
	$writer->characters(encode_element($_));
	$writer->endTag("sa");
	$count++;
    }

    $writer->endTag("lokisa");
    $writer->end();
    $out->close();

    $dt = time() - $t0;

    if ($verbose) {
	$sa->dump() if $verbose > 1;
	warn sprintf "%d XML tokens\n", $count;
	if ( $efile ne '-' ) {
	    my $kb = (-s $efile)>>10;
	    warn sprintf "   [%5.2f MB] in %4ds: %4.1f KB/s\n  from %s to %s\n",
		$kb>>10, $dt,
		$dt ? $kb/$dt : 0, 
		$path ? $path : $db,
		$efile;
	}
    }
}

sub encode_element($)
# encode non-printables as html entities: ^P == #020
{
   my $arg = shift;
   $arg =~ s@([^!-~])@sprintf("#%03d", ord($1))@ge;
   return $arg;
}

sub decode_element($)
# encode non-printables as html entities: ^P == #020
{
   my $arg = shift;
   $arg =~ s@#([0-9]){3}@chr($1)@ge;
   return $arg;
}

sub db_error()
{
    return "Can only process one DB at the same time!\n";
}
sub ie_error()
{
   return "Can't import and export at the same time!\n";
}

sub noaccess($)
{
   my $arg = shift;
   return "Can't access file: $arg\n";
}

# eof
