#!/usr/bin/perl
#
# asm_to_wwn.pm
#
# __LICENSE__
#
# Copyright (c) 2007-2012; All rights reserved.
# by  Joel E. Carlson      <joel.elmer.carlson@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of Joel E. Carlson 
#    nor the names of any contributors may be used to endorse or
#    promote products derived from this software without specific
#    prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY JOEL E. CARLSON 
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
# __LICENSE__
#
=head1 NAME

asm_to_wwn.pm - provide human readable format for san luns

=head1 DESCRIPTION

This  module  will  map  oracleasmlib  names to the corresponding lun
in the SAN.  asm_to_wwn.pm links the output of  /dev/oracleasm/disks,
/dev/mpath, /dev/raw; and /dev device trees in a human readable format.

asm_to_wwn.pm is Linux only.

=head1 REQUIREMENTS

=over

=item o
oracleasmlib (http://www.oracle.com/technology/tech/linux/asmlib/index.html)

=item o
lvm configured for filesystems

=back

=head1 OPTIONS
 
=over

=item o
-h will display help, perldoc asm_to_wwn.pm is this document.

=item o
-i will display iostat

=item o
-U will update to latest version

=back

=head1 INTEGRATION

use asm_to_wwn;

my $u = new asm_to_wwn %options;

print $u->display();

=head1 PLATFORM

Red Hat Enterprise Linux 5 x86_64 and s390x
Red Hat Enterprise Linux 6 x86_64 and s390x

=head1 BUGS

None as of now.

=head1 AUTHOR

Joel E Carlson <joel.elmer.carlson@gmail.com> 

=cut
package asm_to_wwn;
use Carp;
use File::Find;
use File::stat;
use POSIX qw(uname);
use Shell qw(pvdisplay sfdisk systool wget md5sum);
use vars qw($AUTOLOAD $me $revision $arch *name);
use strict;

*name = *File::Find::name;

# setup environment
($me = $0)   =~ s!.*/!!go;
$revision    = '2.8';
$arch        = (uname)[4];
$ENV{'PATH'} = "/bin:/usr/bin:/sbin:/usr/sbin";

__PACKAGE__->run() unless caller();

use constant config => {
    pvdisplay => '-c',
    verbose => 0,
    iostat => 5,
    multipath => '-ll',
    oracleasm => '/dev/oracleasm/disks/',
    mpath => '/dev/mpath/',
    mapper => '/dev/mapper/',
    mmfs => '/usr/lpp/mmfs/bin',
    dev => '/dev/',
    devraw => '/dev/raw/',
    sysblock => '/sys/block/',
    raw => '-qa',
    format => "%-8s %-35s %-8s %s\n",
    banner_format => "%-35s %9s %11s %11s %11s %11s\n",
    io_format =>     "%-35s %9.2f %11.2f %11.2f %11.2f %11.2f\n",
    banner_all_format => "%-35s %6s %6s %6s %6s %6s %6s %8s %8s %6s %6s %6s\n",
    io_all_format =>     "%-35s %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %8.2f %8.2f %6.2f %6.2f %6.2f\n",
    banner => [ qw(tps Read_KB/s Write_KB/s Total_Read Total_Write) ],
    banner_all => [ qw(rrqm/s wwqm/s r/s w/s rsec/s wsec/s avgrq-sz avgqu-sz await svctm %util) ],
    do_asm => 0,
    do_lvm => 0,
    do_used => 0,
    do_iostat => 0,
    do_iostat_all => 0,
    do_raw => 0,
    do_all => 1,
    do_gpfs => 0,
    do_multipath => 0,
    do_wwn => 0,
    do_update => 0,
    unassigned => 'undef',
    systool => '-c fc_host -v',
    url => 'http://github.com/joelelmercarlson/loki.stephans.org/asm_to_wwn.pm',
    null => '2>/dev/null',
};

# databases
my %raw  = (); # /dev/raw
my %asm  = (); # /dev/oracleasm/disks
my %dm   = (); # /dev/dm
my %dev  = (); # /dev $st->rdev to name
my %rdev = (); # /dev name to $st->rdev
my %rdm  = (); # $st->rdev to /dev/dm 
my %vg   = (); # store pvdisplay
my %gpfs = ();      # store gpfs
my %size = ();      # size of disks
my %multipath = (); # multipath paths
my %used = ();      # keep state of used disks

# sanitize device files
sub mydev($) {
    my $_dev = shift;
    $_dev =~ s!.*/!!;

    return $_dev;
}

# major taken from sys/sysmacros.h
sub __major($) {
    my $_dev = shift;

    return ((($_dev >> 8) & 0xfff) | (($_dev >> 32) & ~0xfff));
}
# minor taken from sys/sysmacros.h
sub __minor {
    my $_dev = shift;

    return (($_dev & 0xff) | (($_dev >> 12) & ~0xff));
}

# mm returns major, minor
sub mm($) {
    my $_dev = shift;

    return ($_dev =~ /(\d+), (\d+)/) ? $_dev :
        sprintf "%d, %d", __major($_dev), __minor($_dev);
}

# pretty printing
sub label($) {
    my $v = shift;

    return sprintf "- %s -%s\n", $v, '-' x (65-length($v));
}

# blocks to MiB
sub MiB($) {
    my $v = shift;
    return sprintf "%2.0dM", ($v * 1024)>>10>>10;
}

# lun id
sub lun($) {
    my $v = substr shift, -8;

    $v =~ s/p1//;
    return hex "0x$v";
} 

# find underlying sd of an asm device
sub rdev_to_asm($) {
    my $v = shift;

    for (keys %rdev) { return $_ if ($rdev{$_} eq $v) and ($_ =~ /^sd/); }
}

# dump atabases
sub dump_db($$) {
    my $self = shift;
    my ($name,$db) = @_;

    warn "=== ", ref $self, "::dump_db called from ", join(' ', (caller)[1,2]), "\n";
    warn sprintf "=== %s count %d\n", $name, scalar keys %$db if $self->{verbose};
    if ($self->{verbose} > 1) {
        for (sort keys %$db) {
            warn sprintf "%-16s\t%s\n", $_, $$db{$_};
        }
    }
}

# find2perl
sub wanted {
    #($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,
    #$ctime,$blksize,$blocks) = stat($filename)

    my $st;
    ($st = lstat($_)) && -b _ && ($_ !~ /hd/) &&
        ($dev{$st->rdev} = mydev($name)) &&
        ($rdev{mydev($name)} = $st->rdev);
}

# __dev
sub __dev() {
    my $self = shift;

    find(\&wanted, $self->{dev});
}

# asm luns
sub __asm() {
    my $self = shift;
    my $dh;

    if ( ! -d $self->{oracleasm} ) {
        $self->{do_asm} = 0;
        return;
    }
    chdir $self->{oracleasm};
    opendir $dh, ".";
    while (my $f = readdir($dh)) {
        next if $f =~ /^\./;
        $asm{$rdev{$f}} = $f;

       open FH, "sfdisk -s $f |";
       while (<FH>) {
           s/\s+//g;
           $size{$self->{oracleasm} . $f} = $_;
           $size{$rdev{$f}} = $_;
       }
       close FH;
    }
    closedir $dh;
}

# raw luns
sub __raw() {
    my $self = shift;

    if ( ! -d $self->{devraw} ) {
        $self->{do_raw} = 0;
        return;
    }
    chdir $self->{devraw};
    open FH, "$self->{devraw}/raw $self->{raw} |";
    while(<FH>) {
        s/://;
        my @ar = split /\s+/, $_;
        $raw{qq($ar[4] $ar[6])} = mydev($ar[0]);
    }
    close FH;
}

# dm multipath luns stand-alone x86_64
sub __dm() {
    my $self = shift;
    my $dh;

    return unless -d $self->{mpath};
    chdir $self->{mpath};
    opendir $dh, ".";
    while (my $f = readdir($dh)) {
       next if $f =~ /^\./;
       my $st = stat($f);
       $dm{$st->rdev} = $f;
       $dm{mm($st->rdev)} = $f;
    }
    closedir $dh;

    return unless -d $self->{mapper};
    chdir $self->{mapper};
    opendir $dh, ".";
    while (my $f = readdir($dh)) {
       next if $f =~ /^\./;
       next unless -b $f;

       my $st = stat($f);
       open FH, "sfdisk -s $f |";
       while (<FH>) {
           s/\s+//g;
           $size{$self->{mapper} . $f} = $_;
           $size{$st->rdev} = $_;
       }
       close FH;
    }
    closedir $dh;
}

# dasd
sub __dasd() {
    my $self = shift;
    my $dh;

    return unless $arch eq 's390x';
    chdir $self->{dev};
    opendir $dh, ".";
    while (my $f = readdir($dh)) {
       next if $f =~ /^\./;
       next if $f !~ /dasd/;
       my $st = stat($f);
       $dm{$st->rdev} = $f;
       $dm{mm($st->rdev)} = $f;

       open FH, "sfdisk -s $f |";
       while (<FH>) {
           s/\s+//g;
           $size{$self->{dev} . $f} = $_;
           $size{$st->rdev} = $_;
       }
       close FH;
    }
    closedir $dh;

    # preload our used disks
    $used{$rdev{'dasda'}}++;
    $used{$rdev{'dasda1'}}++;
    $used{$rdev{'dasdb'}}++;
    $used{$rdev{'dasdb1'}}++;
    $used{$rdev{'dasdc'}}++;
    $used{$rdev{'dasdc1'}}++;
}

# san luns under lvm
sub __lvm() {
    my $self = shift;

    for (pvdisplay($self->{pvdisplay}, $self->{null})) {
       my @ar = split /:/, $_;
       $vg{$rdev{mydev($ar[0])}} = $ar[1];

       open FH, "sfdisk -s $ar[0] |";
       while (<FH>) {
           s/\s+//g;
           $size{$rdev{mydev($ar[0])}} = $_;
       }
       close FH;
    }
}

# luns under multipath
sub __multipath() {
    my $self = shift;
    my $v;
    my @foo;

    return unless $arch eq 'x86_64';
    open FH, "multipath $self->{multipath} $self->{null} |";
    while(<FH>) {
        ($v,@foo) = split /[\s+]?dm-\d+/, $_ if $_ =~ /dm-\d+/;
        my @ar = split /\s+/, $_;
        if ($ar[2] =~ /\d+:\d+:\d+:\d+/) {
            $ar[2] = $ar[2] . " " . pop @ar if grep(/failed/i, @ar);
            push @{ $multipath{$v} }, $ar[2];
        }
    }
    close FH;
}

# gpfs
sub __mm() {
    my $self = shift;
    my @ar;

    if ( ! -d $self->{mmfs} ) {
        $self->{do_gpfs} = 0;
        return;
    }
    open FH, "$self->{mmfs}/mmlsnsd -m |";
    while(<FH>) {
        @ar = split /\s+/, $_ if $_ =~ /mpath/;
        next unless $ar[3] =~ /mpath/;
        $gpfs{$rdev{mydev($ar[3])}} = $ar[1];
    }
    close FH;
}

# rdm
sub __rdm() {
    my $self = shift;

    for (keys %rdev) {
        next unless $_ =~ /dm/;
        $rdm{$rdev{$_}} = $_;
    }
}


# dump_multipath() helper for multipath
sub dump_multipath() {
    my $self = shift;
    my @out = ();

    return unless $arch eq 'x86_64';
    for (sort {$a->[1] <=> $b->[1]} map {[$_, lun($_)]} keys %multipath) {
        my $v = $vg{$rdev{$_->[0] . "p1"}};
        my $r = $asm{$rdev{$_->[0] . "p1"}};
        my $w = $raw{mm($rdev{$_->[0] . "p1"})};
        my $d = $v || $r || $w || $self->{unassigned};
        printf "%-35s %-20s paths=%d %s\n", $_->[0], $d,
            scalar @{ $multipath{$_->[0]} }, (join " ", sort @{ $multipath{$_->[0]} });
    }
    return @out;
}

# dump_wwn() helper for ITG requests
sub dump_wwn() {
    my $self = shift;
    my @out = ();

    return unless $arch eq 'x86_64';
    for (systool($self->{systool})) { push @out, $_ }
    return @out;
}

# duration reports minutes and seconds
sub duration($) {
    my $v = shift;
    
    if ($v < 60) { return "$v secs" }
    return sprintf "%d min and %d secs", $v / 60, $v % 60;
}

# iostat displays stats for dm devices
sub dm_iostat() {
    my $self = shift;
    my %t_read = ();
    my %t_write = ();
    my %total = ();
    my %disp = ();
    my $count = 0;

    open FH, "iostat -k $self->{iostat} $self->{null} |";
    while (<FH>) {
        my @ar = split /\s+/, $_;
        if ($ar[0] =~ /device/i) {
            if (keys %disp >= 1) {
                printf $self->{banner_format},
                    duration($count * $self->{iostat}),
                    @{$self->{banner}};
                for (reverse sort {$a->[1] <=> $b->[1]} map {[$_, $total{$_}]} keys %total) { print $disp{$_->[0]} }
                undef %disp;
            }
            $count++;
        }
        else { 
            my $wwn = $rdev{mydev($ar[0])};
            my $m = sprintf " (%s)", $rdm{$rdev{mydev($ar[0])}};
            my $v = $vg{$wwn} || $gpfs{$wwn};
            my $r = $asm{$wwn};
            my $w = $raw{mm($wwn)};
            my $d = $v || $r || $w || $self->{unassigned};
            next if $used{$wwn} < 1 || $d eq $self->{unassigned};

            $t_read{$d} = ($count>1) ? $t_read{$d} + $ar[4] : 0;
            $t_write{$d} = ($count>1) ? $t_write{$d} + $ar[5] : 0;
            $total{$d} = $t_read{$d} + $t_write{$d};
            $disp{$d} = sprintf $self->{io_format}, $d . $m, $ar[1], $ar[2], $ar[3], $t_read{$d}, $t_write{$d};
        }
    }
    close FH;
    return $count;
}

# iostat displays stats for dm devices
sub dm_iostat_all() {
    my $self = shift;
    my %t_read = ();
    my %t_write = ();
    my %total = ();
    my %disp = ();
    my $count = 0;

    open FH, "iostat -x $self->{iostat} $self->{null} |";
    while (<FH>) {
        my @ar = split /\s+/, $_;
        if ($ar[0] =~ /device/i) {
            if (keys %disp >= 1) {
                printf $self->{banner_all_format},
                    duration($count * $self->{iostat}),
                    @{$self->{banner_all}};
                for (keys %disp) { print $disp{$_}; }
                undef %disp;
            }
            $count++;
        }
        else { 
            my $wwn = $rdev{mydev($ar[0])};
            my $m = sprintf " (%s)", $rdm{$rdev{mydev($ar[0])}};
            my $v = $vg{$wwn} || $gpfs{$wwn};
            my $r = $asm{$wwn};
            my $w = $raw{mm($wwn)};
            my $d = $v || $r || $w || $self->{unassigned};
            next if $used{$wwn} < 1 || $d eq $self->{unassigned};

            $disp{$d} = sprintf $self->{io_all_format}, $d . $m, $ar[1], $ar[2], $ar[3], $ar[4], $ar[5], $ar[7], $ar[8], $ar[9], $ar[10], $ar[11];
        }
    }
    close FH;
    return $count;
}


# in_use loads %used from %asm, %raw, %vg
# requires our p1 standard to work properly
sub in_use() {
    my $self = shift;
    my $match;

    $match = 'p\d$' if $arch eq 'x86_64';
    $match = '\d$' if $arch eq 's390x';

    for (keys %asm) {
        my $v = $dm{$_};
        $used{$_}++;
        $v =~ s/$match//; $used{$rdev{$v}}++; # whole disk
    }
    for (keys %raw) {
        my $v = $dm{$_};
        $used{$_}++;
        $used{$rdev{$v}}++;
        $v =~ s/$match//; $used{$rdev{$v}}++; # whole disk
    }
    for (keys %vg) {
        my $v = $dm{$_};
        $used{$_}++;
        $v =~ s/$match//; $used{$rdev{$v}}++; # whole disk
    }
    for (keys %gpfs) {
        my $v = $dm{$_};
        $used{$_}++;
        $v =~ s/$match//; $used{$rdev{$v}}++; # whole disk
    }
}

# update to latest version
sub update() {
    my $self = shift;
    my $dir = "/tmp";
    my $new  = sprintf "%s/%s", $dir, $me;
    my ($a,$c) = split /\s+/, md5sum($0);

    chdir $dir;
    unlink $me;
    wget($self->{url});
    my ($b,$c) = split /\s+/, md5sum($new);
    printf "%s is different version, please install.\n", $new if $a cmp $b;
    exit;
}

# display based on operator decisions
sub display() {
    my $self = shift;
    my @out = ();

    if ($self->{do_asm}) {
        push @out, label("asm devices");
        for (sort keys %asm) {
            push @out, sprintf $self->{format}, mm($_), ($dm{$_} or rdev_to_asm($_)), MiB($size{$_}), $asm{$_};
        }
    }

    if ($self->{do_lvm}) { 
        push @out, label("ext4 filesystems");
        for (sort keys %vg) {
            push @out, sprintf $self->{format}, mm($_), ($dm{$_} or $dev{$_}), MiB($size{$_}), $vg{$_};
        }
    }

    if ($self->{do_gpfs}) { 
        push @out, label("gpfs filesystems");
        for (sort keys %gpfs) {
            push @out, sprintf $self->{format}, mm($_), $dm{$_}, MiB($size{$_}), $gpfs{$_};
        }
    }

    if ($self->{do_raw}) {
        push @out, label("raw devices");
        for (sort keys %raw) {
            push @out, sprintf $self->{format}, mm($_), $dm{$_}, MiB($size{$_}), $raw{$_};
        }
    }

    if ($self->{do_used}) {
        push @out, label("unassigned luns");
        for (grep  /^\d+$/, sort keys %dm) {
            next if $used{$_};
            push @out, sprintf $self->{format}, mm($_), $dev{$_}, MiB($size{$_}), $self->{unassigned};
        }
    }

    return @out;
}

## standard methods
##   new     -> create my object
##   init    -> load my object
##   AUTLOAD -> extend my object
##   run     -> execute if invoked directly
sub new {
    my $invocant = shift;
    my $class = ref $invocant || $invocant;
    my %obj = @_;
    my $self = bless \%obj, $class;

    return $self->init(@_);
}

sub init {
    my $self = shift;

    # map config into object data
    map { $self->{$_} = ${config()}{$_} unless defined $self->{$_} } keys %{config()};

    if ($self->{do_all}) {
       $self->{do_asm}++;
       $self->{do_lvm}++;
       $self->{do_used}++;
       $self->{do_gpfs}++;
    }

    $self->__dev();
    $self->__asm();
    $self->__raw();
    $self->__dm();
    $self->__dasd();
    $self->__lvm();
    $self->__multipath();
    $self->__mm();
    $self->__rdm();
    $self->in_use();
   
    if ($self->{verbose} > 0) {
       $self->dump_db('config', $self->config());
       $self->dump_db('dev', \%dev)   if $self->{verbose} > 2;
       $self->dump_db('rdev', \%rdev) if $self->{verbose} > 2;
       $self->dump_db('size', \%size) if $self->{verbose} > 2;
       $self->dump_db('asm', \%asm);
       $self->dump_db('raw', \%raw);
       $self->dump_db('dm', \%dm);
       $self->dump_db('vg', \%vg);
       $self->dump_db('multipath', \%multipath);
       $self->dump_db('gpfs', \%gpfs);
       $self->dump_db('rdm', \%rdm);
       $self->dump_db('used', \%used);
    }

    # operating modes
    print $self->dump_wwn() if $self->{do_wwn};
    print $self->dump_multipath() if $self->{do_multipath};
    print $self->update() if $self->{do_update};
    print $self->dm_iostat() if $self->{do_iostat};
    print $self->dm_iostat_all() if $self->{do_iostat_all};

    return $self;
}

sub AUTOLOAD {
    my $self = shift;
    my $call;

    croak "usage: AUTOLOAD variable [new_value]\n" if @_ > 1;
    ($call = $AUTOLOAD) =~ s/.*:://;
    return $self if $call eq 'DESTROY';
    if ( @_ ) {
        return $self->{$call} = shift;
    }
    else {
        return $self->{$call} if exists $self->{$call};
        my $class = ref $self;
        confess "invalid $class variable or subroutine $call";
    }
    return $self;
}

sub run {
    # help
    sub usage() {
        return join "\n",
            "$me: [-h] [-b] [-i] [-m] [-s] [-a|-g|-l|-r|-u] [-U] [-x] [-v]",
            "where -h is help, see perldoc $me",
            "    -i iostat -k mode.",
            "    -m multipath helper. x86_64 only.",
            "    -s display hba wwn. x86_64 only.",
            "    -a display only asm disks.",
            "    -g display only gpfs disks.",
            "    -l display only lvm disks.",
            "    -r display only raw disks.",
            "    -u display only unassigned disks.",
            "    -U update to latest version. Version $revision",
            "    -x iostat -x mode.",
            "    -v verbose, repeat for verbosity",
            "";
    }

    # options
    my %options = ();
    my %parseoptions = (
        h => sub { warn usage(); exit; },
        i => sub { $options{do_iostat}++, $options{do_all}=0 },
        s => sub { $options{do_wwn}++, $options{do_all}=0 },
        m => sub { $options{do_multipath}++, $options{do_all}=0 },
        a => sub { $options{do_asm}++, $options{do_all}=0 },
        g => sub { $options{do_gpfs}++, $options{do_all}=0 },
        l => sub { $options{do_lvm}++, $options{do_all}=0 },
        u => sub { $options{do_used}++, $options{do_all}=0 },
        r => sub { $options{do_raw}++, $options{do_all}=0 },
        U => sub { $options{do_update}++, $options{do_all}=0 },
        x => sub { $options{do_iostat_all}++, $options{do_all}=0 },
        v => sub { $options{verbose}++ },
    );
    for (@ARGV) {
        last unless $ARGV[0] =~ /^-([hismalurUxv])$/o;
        shift;
        $parseoptions{$1}->();
    }

    my $u = new asm_to_wwn %options;
    print $u->display();
}
1;
