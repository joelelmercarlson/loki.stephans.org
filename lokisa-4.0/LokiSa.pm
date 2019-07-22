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
# @(#) $Id: LokiSa.pm,v 1.130 2004/02/02 08:42:21 ast Exp $
#
package LokiSa;

use Carp;

use strict;
use vars qw($VERSION $AUTOLOAD $max_tuples $max_size);

$VERSION = sprintf("%d.%02d", q$Revision: 1.130 $ =~ /(\d+)\.(\d+)/);

use IO::File;

use constant GOOD_WEIGHT	=> 0.0107;
use constant SPAM_WEIGHT	=> 1.0 - GOOD_WEIGHT();
use constant TRUE_YELLOW	=> 0.5000; # must be 0.5 since x/(1-x) = 1 iff x=0.5
use constant UNKNOWN_WEIGHT	=> TRUE_YELLOW()  - 8.0 * GOOD_WEIGHT();
use constant THRESHOLD		=> TRUE_YELLOW() + 16.0 * GOOD_WEIGHT();

use constant MIN_WORD_LEN	=> 3;
use constant MAX_WORD_LEN	=> 24;
use constant MAX_EXPR_LEN	=> 2 * MAX_WORD_LEN();

use constant MAX_TUPLES		=> 7;
use constant BDY_TUPLES		=> MAX_TUPLES();	# <= MAX_TUPLES()
use constant HDR_TUPLES		=> 3;			# <= BDY_TUPLES()
use constant UNLIMITED		=> 0;

$max_tuples			= MAX_TUPLES();		# for export to sa-filter.pl
$max_size                       = UNLIMITED();    	# for export to sa-filter.pl

# used by _natomtuples to split words in body, must be a [...]
use constant WORDSEPARATOR	=> '[^a-z0-9]';

# keep PROCMAIL_PATTERN and skip_own_hdrtxt() in sync with HEADER_FORMAT
use constant PROCMAIL_PATTERN	=> "^%s:.* %s .*\\<v%s/%s\\>\$\n";
use constant PROCMAIL_WHITE	=> "^%s:.* (ok) .*\\<v%s/%s\\>\$\n";
use constant PROCMAIL_WHITE_NOK	=> "^%s:.* (nok) .*\\<v%s/%s\\>\$\n";
use constant HEADER_FORMAT	=>
	"%s: <%s> %s %s %s %f %s %f %5.2f KB/s %ds %d %s (%f:%f%s) v%s/%s\n";

sub skip_own_hdrtxt
# this pattern is ignored in *all* content, for training as well as filtering
{
    s!\S+:\s<(\S+)?>\s		# %s: <%s>
	(\S+\s)(\S+\s)(\S+\s)	# %s %s %s
	.*			# ADD ANY NEW STUFF HERE FOR DOWNWARD COMPATIBILITY
	v\d+\.\d+/\d+\.\d+	# v1.xx/1.yy
    !!mx;
}

use constant CONFIGURABLE => {
    filter		=> 'Bayes/Fisher',
    bdy_tuples		=> BDY_TUPLES(),
    hdr_tuples		=> HDR_TUPLES(),
    u_weight		=> UNKNOWN_WEIGHT(),
    s_weight		=> SPAM_WEIGHT(),
    g_weight		=> GOOD_WEIGHT(),
    threshold		=> THRESHOLD(),
    db_dir		=> "$ENV{HOME}/.lokisa",
    header		=> 'X-Lokisa',
    ok			=> '(ok)',
    nok			=> '(nok)',
    green		=> 'Green',
    red			=> 'Red',
    yellow		=> 'Yellow',
    debug		=> 0,	# >0 may break email box!
    spam_db		=> "",
    good_db		=> "",
    count_db		=> "",
    white_db		=> "",
    skip_neutral	=> 1,	# whether to skip over expressions in near TRUE_YELLOW()
    from_pattern	=> '^From\s([^\s@]+(@[^\s@]+)?)\s+\S+\s+\S+\s+[\s:\d]+$',
    address_pattern	=> '([^\s@]+(@[^\s@]+)?)', # must be embedded above!
};

use constant INTERNAL => {
    _prevnatoms		=> [],
    _from_address	=> "",
    _good_hash		=> undef,
    _spam_hash		=> undef,
    _count_hash		=> undef,
    _white_hash		=> undef,
    _good		=> [],
    _spam		=> [],
    _time		=> 0,
    _bytes		=> 0,
    _databases          => [ qw(count spam good white) ],
};

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my %obj = @_;
    my $self = bless \%obj, $class;
    return $self->init();
}

sub init
{
    my $self = shift;
    map { $self->{$_} = ${INTERNAL()}{$_} } keys %{INTERNAL()};
    # empty hash created per email - important when in mbox mode!
    $self->{_degree_of_belief} = {};
    # validate header and body tuple options
    for ( qw(hdr_tuples bdy_tuples) ) {
	$self->_valid_tuples($_, $self->{$_}) if defined $self->{$_};
    }
    $self->_time(time());
    map { $self->{$_} = ${CONFIGURABLE()}{$_} unless defined $self->{$_} } keys %{CONFIGURABLE()};
    for ( $self->databases() ) {
	my $dname = $self->dname($_);
	$self->$dname($self->db_dir() . '/' . $_) unless $self->$dname();
	my $hname = $self->hname($_);
	$self->$hname($self->open_db($_, 0600));
    }
    $self->dump() if $self->debug() > 2;
    return $self;
}

sub dump
{
    my $self = shift;
    my $seen = '';
    warn "======== ", ref $self, "::dump called from ", join(' ', (caller)[1,2]), "\n";
    for (sort keys %$self) {
	next if /^_[^_]+_hash/o; # skip big hashes
	my $h = $self->$_();
	$h = '' unless defined $h;
	my $v = $h;
	if ( ref $h eq 'ARRAY' ) {
	    $v = "Array of " . scalar(@$h) . ": " . join ", ", @$h;
	} elsif ( ref $h eq 'HASH' ) {
	    my @k = sort keys %$h;
	    $v = scalar(@k) . " hash keys: " . join ", ", @k;
	}
	warn sprintf "  %-16s %s\n", $_, $v;
    }
    return $self;
}

sub databases
{
    my $self = shift;
    return @{$self->_databases()};
}

sub _valid_tuples
{
    my $self	= shift;
    my $hdr_bdy	= shift;
    return $self->{$hdr_bdy} unless @_;
    my $n = shift;
    $n =~ /^(\d+)$/o;
    die "Tuple isn't a positive integer\n" unless $1;
    die sprintf "Can't specify $hdr_bdy greater that %d (Lokisa::MAX_TUPLES)\n",
	MAX_TUPLES() if $1 > MAX_TUPLES();
    $self->{$hdr_bdy} = $1;
    return $self;
}

sub from_address
# return the cached "From " address or set 
{
    my $self = shift;
    return $self->{_from_address} unless @_;
    my $p = $self->address_pattern();
    my $a = shift;
    $a =~ s/[<>]//go;
    return $self->{_from_address} = $1 if $a =~ /$p/;
    return "";
}

sub _percent
# helper for weights and threshold which can be either ]0,100[ or ]0,1[
{
    my ($self, $n, $v) = @_;
    $v /= 100.0 if $v > 1;
    die "Illegal value for $n" if $v>1 || $v<=0;
    $self->{$n} = $v;
    return $self->{$n};
}

sub AUTOLOAD
# used for setting/getting variable values with default fallbacks
# and for calling [build_]{white,spam,good}_db and methods
{
    my $self = shift;
    croak "USAGE: AUTOLOAD variable_or_dbmethod [new_value]\n" if @_ > 1;
    my $call;
    ($call = $AUTOLOAD) =~ s/.*:://o;
    return $self if $call eq 'DESTROY';
    if ( @_ ) {
	return $self->_percent($1, shift) if $call =~ /^(threshold|[ysgu]_weight)$/o;
	return $self->{$call} = shift;
    } else {
	return $self->{$call} if exists $self->{$call};
	my $class = ref $self;
	confess "Invalid $class variable or subroutine '$call'";
    }
    return $self;
}

sub dname { return $_[1] . '_db' }
sub hname { return '_' . $_[1] . '_hash' }
sub count { return scalar(keys %{$_[1]}) }

sub open_db
# return a hashref to the requested db, re-open if mode isn't 0400
{
    my ($self, $name, $mode) = @_;
    croak "USAGE: ", ref $self, "::open_db name [mode]" unless @_ ==2 || @_ == 3;
    my $hname = $self->hname($name);
    my $hashref = $self->$hname();
    return $hashref if $hashref && ( @_ == 2 || $mode == 0400);
    dbmclose(%$hashref) || die "dbclose $name: $!";
    $hashref = {};
    my $dname = $self->dname($name);
    my $db = $self->$dname();
    # if the db didn't exist, we created it read-only, so chmod the files
    my @g = glob "${db}.*";
    if ( @g ) {
	chmod $mode, @g or die "chmod ", join(',', @g), ": $!";
	warn sprintf "Re-opening %s db with mode 0%o: %s\n", 
		$name, $mode, join(",", @g) if $self->debug() > 2;
    }
    dbmopen(%$hashref, $db, $mode) || die "dbmopen $db: $!";
    return $self->$hname($hashref);
}

sub show_db
# show db, sort either alphabetically (key) or numerically (value)
# $sorting is u = unsorted, r = alphabetically, or n = numerical
{
    my ($self, $name, $sorting) = @_;
    my $dct = $self->open_db($name);
    my $data_fmt  = "%-" . MAX_WORD_LEN . "s %8.4f\n";

    if ($name eq 'white') {
	# also check for domain wildcards which collide with specific addresses
	my %d= ();
	for ( sort keys %$dct ) {
	    # by sorting, we make sure the @ wildcards come first to define array below
	    m!([^@]*)(@.+)?!o;
	    if ( $2 ) {
		my $x = lc $2;
		if ( defined $d{$x} ) {
		    push @{ $d{$x} }, $_;	# array of specific address
		} else {
		    $d{$x} = [] unless $1;	# the @ wildcards induce this first
		}
	    }
	}
	for ( sort keys %$dct ) {
	    m!([^@]*)(@.+)?!o;
	    print $_;
	    if ( $2 ) {
		my $x = lc $2;
		if ( defined $d{$x} && @{$d{$x}} ) {
		    if ( $1 ) {
			print " WARNING: '", $x, "' overrides";
		    } else {
			print " WARNING: overrides '", join ', ', @{$d{$x}};
		    }
		}
	    }
	    print "\n";
	}
    } else {
	select(STDOUT); $|++;	# for the impatiant ;-)
	if ( $sorting eq 'n' ) {
	    for ( sort { $b->[1] <=> $a->[1] } map { [ $_, $$dct{$_}] } keys %$dct ) {
		printf $data_fmt, $_->[0], $_->[1];
	    }
	} elsif ( $sorting eq 'r' ) {
	    for ( sort keys %$dct ) {
		printf $data_fmt, $_, $$dct{$_};
	    }
	} else {
	    while ( my ($k, $v) = each %$dct ) {
		printf $data_fmt, $k, $v;
	    }
	}
    }
    return $self;
}

sub build_db
# constructs good, spam, and white words database; called from
# AUTOLOAD via build_db (used by sa-xml.pl and sa-dbm.pl)
{
    my ($self, $name, $path) = @_;

    my $t0	= time();
    my $verbose	= $self->debug();
    my $dct	= $self->open_db($name, 0600);
    my $cnt	= $self->open_db('count', 0600);;
    my $odict	= $self->count($dct) if $verbose;
    my $num     = 0;
    my $kb      = 0;
    my $count   = 0;

    if (ref $path eq 'HASH') {
	# process input passed in hashref
	while ( my ($k, $v) = each %$path ) {
	    local $^W = 0;
	    $$dct{$k} += $v;
	    $kb += length $k;
    	}
	$count = $self->count($path);
	$kb >>= 10;
    } else {
	# process input from file
	my $fh = new IO::File;
	die "Open $path: $!" unless $fh->open("<$path");
	if ( $name eq 'white' ) {
	    ($num, $kb, $count) = $self->addresses($fh, $dct);
	} else {
	    ($num, $kb, $count) = $self->messages($fh, $dct, $cnt);
	}
    }

    if ( $verbose ) {
	$self->dump() if $verbose > 1;
	my $ndict = $self->count($self->open_db($name, 0600));
	my $d = $ndict - $odict;
	my $dt = time() - $t0;
	warn sprintf
	    "%d updates, %d recs, %d new +%4.1f%% in %s DB\n" .
	    "  %4d E-mails [%5.2f MB] in %4ds: %4.1f KB/s\n  from %s\n",
	    $count,  $ndict, $d, 
	    $ndict ? 100.0*($d/$ndict) : 100.0, 
	    $name, $num, $kb/(1<<10), $dt,
	    $dt ? $kb/$dt : 0,
	    ref $path eq 'HASH' ? 'XML' : $path;
    }

    return $self;
}

sub email_count
# this routine returns the number of Emails in which the given
# expression has occurred
{
    my ($self, $t) = @_;
    my $cnt = $self->open_db('count');
    my $count = $$cnt{$t};
    return $count ? $count : 0;
}

sub addresses
# Helper for whitelist DB creation; returns number of records found
# gobble line-by-line in case we have just From line input
{
    my ($self, $fh, $dct) = @_;
    my $bytes	= 0;
    my $count	= 0;

    while(<$fh>) {
	$bytes += length;
	next unless /^From\s(.+)$/mo;
	my $a = lc $self->from_address($1);
	if ($a) {
	    $$dct{$a} = $a;
	    $count++;
	}
    }
    undef $fh;
    if ( $self->debug() > 3 ) {
	warn "Addresses:\n";
	for ( sort keys %$dct ) { warn "$_\n" }
    }
    return ($count, $bytes>>10);
}

sub messages
# Helper for DB creation: gobbling by paragraphs, stores HDR_TUPLES-tuples
# from headers and MAX_TUPLES from bodies. This is an inner loop
# so make sure it's fast (i.e. no conditional debugging)
#
# messages is also used in sa-xml.pl via build_db to build hash of
# TUPLES
{
    my ($self, $fh, $upd, $cnt) = @_;
    my $bytes	= 0;
    my $msgs	= 0;
    my $count	= 0;
    my $from	= $self->from_pattern();
    my $nhdr	= $self->hdr_tuples();
    my $nbdy	= $self->bdy_tuples();

    my $h = {};

    local $/ = "\n\n";
    while (<$fh>) {
	$bytes += length;
	$self->skip_own_hdrtxt();
	if ( /$from/mo ) {
	    map { local $^W = 0; $$cnt{$_}++; $$upd{$_} += $$h{$_}; } keys %$h;
	    $h = {};
	    $msgs++;
	    $self->_sanitize_header();
	    $count += $self->_natomtuples(undef, $nhdr+1, sub { $$h{$_[0]}++ } );
	} else {
	    s/(=|\r)?\n\s+/ /go;	# make paragraphs "one-liners" (including mime)
	    s/<![^>]+>//go;		# delete HTML comments A ni<!YffoRJ>ce la<!ixRe>dy
	    $count += $self->_natomtuples(WORDSEPARATOR(), $nbdy+1, sub { $$h{$_[0]}++ } );
	}
    }
    undef $fh;

    map { local $^W = 0; $$cnt{$_}++; $$upd{$_} += $$h{$_}; } keys %$h;

    return ($msgs, $bytes>>10, $count);
}

sub _base64
# taken from MIME::Base64
{
    my ($self, $x) = @_;
    return $x if $x =~ /([0-9A-F]){76}/o;	# ignore hex (PostScript encoded)
    $x =~ tr|A-Za-z0-9+/| -_|;			# convert to uuencoded format
    return join '', map unpack("u", chr(32 + length($_)*3/4) . $_), $x =~ /(.{1,60})/gs;
}

sub _natomtuples
# given a paragraph of text, compose ntuples with $n previous atoms
# i.e. "words" delimited by space or $wordsep which may be undefined
# then, apply a given action on them if they are legal n-tuples
# inner loop so make sure it's fast (i.e. no conditional debugging)
# returns number of n-tuples acted on
{
    my ($self, $wordsep, $n, $action) = @_;
    #speed# warn "NATOMTUPLE PAR\n$_\n";
    my @pa = ();
    my $count = 0;
    s/(([A-Za-z0-9+]){76})/$self->_base64($1)/goe;	# last base64 line stays when its len != 76
    s/=([\da-fA-F]{2})/pack("C", hex($1))/goe;		# taken from MIME::QuotedPrint::decode_qp
    #speed# warn "NATOMTUPLE DEMIMED\n$_\n";
    s/(\s+$wordsep+\s+)/ $1 /go if $wordsep;
    for ( split /\s+/o, lc ) {
	next if length() < MIN_WORD_LEN;
	#speed# warn "NATOMTUPLE ATOM\t'$_'\n";
	unless ( /[\[\]\(\)\d:+\-]+/o ) {		# skip numbers, time stamps, but not IPs
	    &$action($_);
	    $count++;
 	}
	push @pa, $_;
	shift @pa if @pa == $n;
	for my $i (0 .. $#pa-1) {
	    # don't begin n-tuple with a character from $wordsep
	    next if $wordsep && $pa[$i] =~ /^$wordsep/o;
	    my $nt = join '', @pa[$i..$#pa];
	    my $l = length $nt;
	    last if $l > MAX_EXPR_LEN;
	    #speed# warn "NATOMTUPLE\t'$nt'\n";
	    &$action($nt);
	    $count++;
	}
    }
    return $count;
}

sub _sanitize_header
# removes all redundant information from mail headers
{
    # Header-Titles:
    s!^[^:]+:\s+!!gom;
    # E-mail transfer words
    s!\b(by|for|with|via|from|id|envelope-from|e?smtp|smap|uucp|\@localhost)\b!!goim;
    # Sun Nov 17 01:16:04 2002
    # Sun, 17 Nov 2002 01:14:42 +0100 (CET)
    s!
	(([A-Za-z,]{3,4}\s+){1,2}[0-9,]{1,4}|[0-9,]{1,3}\s+[A-Za-z,]{3,4}){1,2}
	(\s+[+:\.\d\-]+){1,2}
	(\s+[+\-\d]+(\s+\(([^\)]){3,7}\))?)?
    !!gox;
    s/[\(\)\[\]]//go;
}

sub weight_header
# only up to HDR_TUPLES-tuples are weighted in header, wordseparator is just whitespace
{
    my $self = shift;
    my $l = $self->_bytes() + length;
    $self->_bytes($l);
    $self->skip_own_hdrtxt();
    $self->_sanitize_header();
    return $self->_weight_content($self->hdr_tuples());
}

sub weight_body
# weight n-tuples (max MAX_TUPLES) in body, using extended wordseparator
{
    my $self = shift;
    my $l = $self->_bytes() + length;
    $self->_bytes($l);
    $self->skip_own_hdrtxt();
    s/(=|\r)?\n\s+/ /go;	# make paragraphs "one-liners" (including mime)
    s/<![^>]+>//go;		# delete HTML comments A ni<!YffoRJ>ce la<!ixRe>dy
    return $self->_weight_content($self->bdy_tuples(), WORDSEPARATOR());
}

sub _weight_content
# weight n-tuples, $wordsep may be undefined
{
    my ($self, $n, $wordsep) = @_;
    s/([$wordsep]+)/ $1 /goi if $wordsep;

    my $sh 	= $self->hname('spam');
    my $s	= $self->$sh();

    my $gh 	= $self->hname('good');
    my $g	= $self->$gh();

    my $sw	= $self->s_weight();
    my $gw	= $self->g_weight();
    my $uw	= $self->u_weight();
    my $skip	= $self->skip_neutral();

    $self->_natomtuples(
	$wordsep, $n+1,
	sub { $self->_weight_expr($_[0], $s, $g, $sw, $gw, $uw, $skip) }
    );

    return $self;
}

sub _weight_expr
# for a given expression, return its weight, u_weight, or TRUE_YELLOW()
# if not near TRUE_YELLOW(), save weight and expression for combined
# probablility
{
    my ($self, $t, $s, $g, $sw, $gw, $uw, $skip) = @_;

    return TRUE_YELLOW() unless $t;

    my $n = $self->email_count($t);
    return TRUE_YELLOW() unless $n;

    my $sv = $$s{$t};
    my $gv = $$g{$t};

    $sv = 0 unless $sv;
    $gv = 0 unless $gv;

    my $p;			# bayes probability
    # $sw = spam weight
    # $gw = good weight
    # $uw = unknown weight
    # $sv = number occurrences in spam 
    # $gv = number occurrences in good
    if ( $sv == $gv ) {
	# equal occurrence in good and spam
	$p = TRUE_YELLOW();
    } elsif ( $sv && $gv ) { 
	# calculate bayes when term has differnet occurrences in good and spam
	$p = $sv / ($gv + $sv);
    } elsif ( $gv ) {
	# a good term is blessed g_weight (near 0)
	$p = $gw;
    } else {
	# a spam term is damned s_weight (near 1)
	$p = $sw; 
    } 

    # degree_of_belief = [ (s * x) + (n * p) ] / (s + n)
    # p = bayes probability
    # s = strength we give our background information
    # x = assumed probability of unknown words
    # n = number of Emails that have contained given expression
    my $v = ($uw + $n * $p) / (1 + $n);
    # store weigth and expression, skipping "neutral" values by default
    unless ( $skip && ( abs(TRUE_YELLOW() - $v) < 3.0 * GOOD_WEIGHT() ) ) {
	#speed# warn sprintf "%5d %5d %f %5d %f %s\n", $sv, $gv, $p, $n, $v, $t;
	my $h = $self->{_degree_of_belief};
	$h->{v}->{$t} = $v;
	$h->{t}->{$v} = $t;
	local $^W = 0;
	$h->{n}->{$t}++;
    }

    return $v;
}

sub whitelisted
# for a given address, return true when whitelisted, otherwise false
{
    my ($self, $address) = @_;
    return 0 unless $address;
    my $hname = $self->hname('white');
    my $white = $self->$hname();
    # return if full address or the domain is in whitelist
    return defined $$white{lc $address} || ( $address =~ m!(@.+)!oi && defined $$white{lc $1} );
}

sub mailheader
# return the mailheader string
{
    my $self		= shift;
    my $revision	= shift;
    my $maxsize         = shift;
    my $threshold	= $self->threshold();
    my $address		= $self->from_address();
    my $whitelisted	= $self->whitelisted($address);
    my $green		= $self->green();
    my $yellow		= $self->yellow();
    my $red		= $self->red();

    my ($prob, $color);

    my ($n, $prob_spam, $prob_good)	= $self->fisher();

    $prob	= ( $prob_spam + $prob_good ) / 2.0;
    $color	= ( $prob > $threshold ) ? ( $whitelisted ? $yellow : $red ) : $green;

    my $dt	= time() - $self->_time();

    return sprintf HEADER_FORMAT(),
	$self->header(), $address,
	$whitelisted ? $self->ok() : $self->nok(),
	$self->filter(), $color, $prob,
	($prob > $threshold) ? ">" : ($prob == $threshold) ? "=" : "<",
	$threshold,
	$dt ? ($self->_bytes()>>10)/$dt : 0,
	$dt, $n,
	$maxsize > 1<<20 ? sprintf "%dM", $maxsize>>20 :
	    $maxsize > 1<<10 ? sprintf "%dK", $maxsize>>10 :
		$maxsize ? $maxsize : 'unlimited',
	$prob_spam, $prob_good, 
	$self->skip_neutral() ? '' : ' all',
	$revision, $VERSION;
}

sub procmailpattern
# return the procmail pattern string
{
    my ($self, $revision) = @_;
    return sprintf PROCMAIL_PATTERN(), $self->header(), $self->red(), $revision, $VERSION;
}

sub procmailwhite
# return the procmail pattern string for whitelist
{
    my ($self, $revision) = @_;
    return sprintf PROCMAIL_WHITE(), $self->header(), $revision, $VERSION;
}

sub procmailwhitenok
# return the procmail pattern string for whitelist
{
    my ($self, $revision) = @_;
    return sprintf PROCMAIL_WHITE_NOK(), $self->header(), $revision, $VERSION;
}

sub fisher
# return n, prob_spam, prob_good
# where n is the number of known expressions found and prob_{spam,good}
# are the probabilities that the hypothesis the email is {spam, good}
{
    my $self		= shift;
    my $verbose		= $self->debug();

    # calculate -2 * sum(ln(f(w)))

    my $n = 0;	# term count (includes multiplicity)
    my $q = 0;	# sum of ln of beliefs that this email is spam
    my $r = 0;	# sum of ln of beliefs that this email is good
    my $h = $self->{_degree_of_belief};
    my ($thash, $nhash, $vhash) = ($h->{t}, $h->{n}, $h->{v});

    for my $v ( keys %$thash ) {
	my $t = $$thash{$v};
	my $c = $$nhash{$t};
	$n += $c;
	$q += $c*log($v);
	$r += $c*log(1-$v);
    }

    if ( $verbose > 1 ) {
	warn $n ? "Degree of beliefs of $n expressions:\n" : "No known expressions found\n";
	for my $v ( sort { $a <=> $b } keys %$thash ) {
	    my $t = $$thash{$v};
	    my $c = $$nhash{$t};
	    warn sprintf "%4d: %f %-60s\n", $c, $v, $t;
	}
    }

    return 0, TRUE_YELLOW(), TRUE_YELLOW() unless $n;

    return $n,
	$self->prob_inverse_chi_square_df(-2*$q, 2*$n),
	1 - $self->prob_inverse_chi_square_df(-2*$r, 2*$n);
}

sub prob_inverse_chi_square_df
# Return prob(chi_square >= chi) with df degrees of freedom.
# Represents an overall combined "probability" via Fisher calculations
# This is from the March 2003 Linux Journal Issue 107 article by
# Gary Robinson "A Statistical Approach to the Spam Problem", see
# http://www.bgl.nu/bogofilter/fisher.html
# http://www.bgl.nu/bogofilter/smindev.html
# http://wecanstopspam.org/jsp/Wiki?GarySpamArticle
{
    my $self = shift;
    my ($chi, $df) = @_;
    croak "USAGE: prob_inverse_chi_square_df chi df (chi>0, df even)\n"
	unless $chi && $chi>0 && $df && !($df&1);
    my $m = $chi / 2.0;
    my $t = exp(-$m);		# watch for exp(-m) underflow to 0 if chi is large
    my $s = $t;
    for my $i ( 1 .. $df>>1 ) {
	$t *= $m / $i;
	$s += $t;
	return 1.0 if $s >= 1;	# for small chi and large df, roundoffs may make prob > 1
    }
    return $s;
}

1;

