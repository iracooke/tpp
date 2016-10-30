#
# this is where you can add customization of the tpp_gui.pl script without it being
# destroyed by a software update.
#
sub readconfig {  # don't change this
	my($key, $default) = @_; # don't change this
# do not change anything above this line, it will break everything.

#
# SET ENVIRONMENT VARIABLES HERE
#
# you can uncomment the next lines to tell TPP to gzip pepXML and protXML for more efficient storage and file transfer
# (also see the doMzXMLGzip setting below)
#	$ENV{'PEPXML_EXT'}  = ".pep.xml.gz";
#	$ENV{'PROTXML_EXT'} = ".prot.xml.gz";

my %settings = ( # don't change this line, either!

#
# SET TPP_GUI DEFAULTS HERE
# See tpp_gui.pl readconfig() calls for a list
# of what can be set here.
#

# for example, you might want to uncomment the next line to make x!tandem your default search engine.
# 'pipeline'  => 'Tandem',   # one of [Sequest Mascot SpectraST Tandem]  

# or you might uncomment this so all your raw files convert to .mzXML.gz to save space.
# 'doMzXMLGzip' => true,

# or you might want to uncomment the next line to change your mascot server setup.
# 'mascot_server' => "http://myotherserver/mascot/",  

'dummy' => 0); # don't change this
return $settings{$key} || $default; # don't change this
} # don't change this
1; # required so that file can be correctly included in another script
   #- gives a 'true' response when loaded 
