#!/usr/bin/perl

use tpplib_perl; # exported TPP lib function points
my $TPPVersionInfo = tpplib_perl::getTPPVersionInfo();

print "Content-type: text/html\n\n";

%box = &tpplib_perl::read_query_string;      # Read keys and values

print "<HTML><BODY BGCOLOR=\"\#FFFFFF\" OnLoad=\"self.focus();\"><PRE>";

#print "file: $box{'OutFile'}\n";

# tar out dta file
my $file = $box{'OutFile'};
my @components = split('/', $file);
my $outfile = $components[$#components];
my $tarfile = $components[0];
for(my $k = 1; $k < $#components; $k++) {
    $tarfile .= '/' . $components[$k] if(! ($components[$k] eq '.'));
}
$tarfile .= ".tgz";
if(-e $tarfile) {
    open TAR, "tar xzOf $tarfile \"$outfile\" |";
    my @results = <TAR>;
    if(@results > 0) {
	for(my $k = 0; $k < @results; $k++) {
	    print "$results[$k]";
	}
    }

} else {
    print "\n\n<font style='border:3px solid red;color:red'>ERROR: Count not open file: $tarfile does not exist</font>\n\n";
}

print "\n\n<hr size='1' noshade>";
print "<font color='#999999'>$page_title\n$TPPVersionInfo</font>\n";
print "</PRE>\n</body>\n</html>";
exit;
