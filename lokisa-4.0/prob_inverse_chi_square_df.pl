#!/usr/bin/perl -w
# @(#) $Id: prob_inverse_chi_square_df.pl,v 1.1 2003/04/03 14:57:01 ast Exp $

use Carp;

sub prob_inverse_chi_square_df (@)
# Return prob(chi_square >= chi) with df degrees of freedom.
# This represents an overall combined "probability" via Fisher;
# This is from the March 2003 Linux Journal Issue 107 article by
# Gary Robinson "A Statistical Approach to the Spam Problem", see
# http://www.bgl.nu/bogofilter/fisher.html
# http://wecanstopspam.org/jsp/Wiki?GarySpamArticle
{
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

sub show ($$)
{
    print join ("\t", @_, '=', prob_inverse_chi_square_df(@_) ), "\n";
    1;
}

for my $df (100 .. 150) {
    show (100, 2*$df);
}

