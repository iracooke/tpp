#!/usr/bin/env perl
#
# Download the latest revision of Proteowizard from the TeamCity website using
# the TeamCity REST API.
#
use strict;
use warnings;

use LWP::UserAgent;

my $tc = "https://teamcity.labkey.org";

$| = 1;

chdir( $ARGV[1] ) if ( $ARGV[1] );

if ( $ARGV[0] eq '--src' ) {

   my ( $v, $f, $wo ) = getSource();
   my $a = LWP::UserAgent->new();
   my $u = "$tc$v";
   print "downloading $u...\n";
   my $r = $a->get( $u );
   die "Error: $r->content" unless $r->is_success();
   open( FH, ">VERSION" );
   print FH $r->content;
   close( FH );

   $u = "$tc$wo";
   $u =~ s/metadata/content/;
   $wo =~ s/.*\///;
   print "downloading $wo...\n";
   $r = $a->get( $u );
   die "Error: $r->content" unless $r->is_success();
   open( FH, ">$wo" );
   binmode FH;
   print FH $r->content;
   close( FH );

   exit;

} elsif ( $ARGV[0] eq '--win32' ) {

   print "==========================================================\n";
   print "  Downloading Proteowizard version with vendor licenses.\n";
   print "  We are assume that do agree with the license, right?\n";
   print "==========================================================\n";

   my ( $v, $win32) = getWin32();

   my $a = LWP::UserAgent->new();
   my $u = "$tc$win32";
   $win32 =~ s/.*\///;
   print "downloading $win32...\n";
   my $r = $a->get( $u );
   die "Error: " . $r->content unless $r->is_success();
   open( FH, ">$win32" );
   binmode FH;
   print FH $r->content;
   close( FH );

   exit;

} else {
   print "download-pwiz <directory>\n";
   print "\n";
   print "download latest revision of Proteowizard\n";
   print "\n";
   print "  --win32  32-bit Windows version\n" ;
   print "  --src    source without libraries/tests\n" ;
   print "  --help   provides the help text you're reading now\n" ;
   exit 1;
}

exit;

sub getSource {
   my $a = LWP::UserAgent->new();
   my $u = "$tc/guestAuth/app/rest/builds/buildType:bt81/artifacts/children";
   my $r = $a->get( $u, Accept => 'application/json' );
   die "Error: $r->content" unless $r->is_success();
   my $j = json_get($r->content);

   my ( $version, $full, $wo );
   foreach ( @{$j->{files}} ) {
      print "artifact $_->{name} ";
      # VERSION
      if ( $_->{name} =~ /VERSION/ ) {
#         print "$_->{content}->{href}";
         $version = $_->{content}->{href};
      }
      # pwiz-src-3_0_5622.tar.bz2
      if ( $_->{name} =~ /pwiz-src-\d.*.tar.bz2/ ) {
#         print "$_->{href}";
         $full = $_->{href};
      }
      # pwiz-src-without-lt-3_0_5622.tar.bz2
      if ( $_->{name} =~ /pwiz-src-without-lt-\d.*.tar.bz2/ ) {
#         print "$_->{href}\n";
         $wo = $_->{href};
      }
      print "\n";
   }

   return( $version, $full, $wo );
}

sub getWin32 {
   my $a = LWP::UserAgent->new();
   my $u = "$tc/guestAuth/app/rest/builds/buildType:bt36/artifacts/children";
   my $r = $a->get( $u, Accept => 'application/json' );
   die "Error: $r->content" unless $r->is_success();
   my $j = json_get($r->content);

   my ( $version, $win32 );
   foreach ( @{$j->{files}} ) {
      print "artifact $_->{name} ";
      # VERSION
      if ( $_->{name} =~ /VERSION/ ) {
#         print "$_->{content}->{href}";
         $version = $_->{content}->{href};
      }
      # pwiz-bin-windows-x86-vc100-release-3_0_5622.tar.bz
      if ( $_->{name} =~ /pwiz-bin-windows-x86.*.tar.bz2/ ) {
#         print "$_->{href}\n";
          $win32 = $_->{content}->{href};
      }
      print "\n";
   }

   return( $version, $win32 );
}



sub json_get($) {
    local $_ = shift ;

    sub _err(;$) {
        use Carp ;
        my $mis = shift || '' ;
        $mis &&= "(Expecting '$mis')"  ;
        croak "Invalid JSON $mis:\n" . substr $_, 0, 25

    }
    sub _value () {
        s/^\s*(?:\,\s*)?//s ;
        ( /^[\]\}]/     ? undef    :
          /^\{/s        ? _hash () :
          /^\[/s        ? _array() :
          /^\"/s        ? _str  () :
          s/^null\b//s  ? ''       :
          s/^false\b//s ? ''       :
          s/^true\b//s  ?  1       :
          s/^(\w+)//s   ? $1       :
                          _err     )
    }

    sub _str() {
        s/^\s*\"(.*?)\"//s ;
        my $str = $1 or return '';
        while ($str =~ /([\\]+)\z/s and

               1 & length $1 ) {
            s/^(.*?)\"//s ;
            $str .= '"' . $1 ;
        }
        $str =~ s/\\u(.{4})/chr(hex($1))/eg;
        $str

    }
    sub _key() {
        s/^\s*\,//s ;
        /^\s*[\]\}]/ and return ;
        my $key = _str ;
        s/^\s*\://s or _err ':';
        $key

    }
    sub _hash() {
        my ($key, $hash) ;
        s/^\s*\{//s or _err '{';
        $hash->{$key} = _value while defined ($key = _key  ) ;
        s/^\s*\}//s or _err '}';
        $hash

    }
    sub _array() {
        my ($arr, $val) ;
        s/^\s*\[//s or _err '[' ;
        push @$arr, $val       while defined ($val = _value) ;
        s/^\s*\]//s or _err ']' ;
        $arr

    }
    _value
}
