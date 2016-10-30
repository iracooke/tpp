#!/usr/bin/perl
#
# show_tmp_pngfile.pl
# a little CGI script to pump a temp graphic file to stdout, then delete it
#


my $serverroot = $ENV{WEBSERVER_ROOT};
$serverroot =~ s/\\/\//g;  # get those path seps pointing right!


my @qvars = split '&', $ENV{QUERY_STRING};

my $fname = $ENV{QUERY_STRING};
my $retain = 0;

foreach my $v (@qvars) {
    my @var = split '=', $v;
    if ($#var > 0) {
	if ($var[0] eq "file") {
	    $fname = $var[1];
	}
	if ($var[0] eq "keep") {
	    $retain = $var[1];
	}
	

    }


} 




$fname =~ s/\\/\//g;  # get those path seps pointing right!
my $fullpath = $fname ;
$fullpath =~ s/\/\//\//g;  # eliminate double slashes

if (not -e $fullpath) { # prepend serverroot
	$fullpath = $serverroot . $fname ;
}

if (not ($fname =~ m/\.png$/)) { # make sure nobody's trying to delete non-png files
	print "Content-type: html/text\n\n";
	print "wrong filetype";
} elsif (-e $fullpath)  {  # if it exists, show it then delete it
	# show it
	my $qt = "'";
	$qt = '"' if  ($^O eq 'MSWin32' );
	my $quotedpath = $qt.$fullpath.$qt;
	print "Content-type: image/png\n\n";
	system("cat $quotedpath"); 
	# kill it
	if ($retain == 0) {
	    system("rm $quotedpath"); 
	}
} else { # issue warning
	print "Content-type: html/text\n\n";
	print "The temporary image file $fullpath has already been deleted.  ";
	print "Try using the Copy function in your browser instead.";
}
