use 5.008008;
use ExtUtils::MakeMaker;
use File::Basename;

my $prereq = {
    'Amazon::SQS::Simple' => '1.04',
    'Class::InsideOut' => '1.09',
    'Digest::MD5::File' => '0.07',
    'File::HomeDir' => '0',
    'File::pushd' => '1.00',
    'File::Which' => '1.07',
    'Module::Load' => '0',
    'Net::Amazon::S3' => '0.56',
    'VM::EC2' => '1.28',
    'YAML' => '0.62',
};

my $exefiles = [
    'bin/amztpp',
    'bin/amztppd',
];

if ( $^O =~ /MSWin/i )
   {
#   $prereq->{'IO::Compress'} = '0';
#   $prereq->{'Win32::Daemon'} = '0';
#   $prereq->{'Win32::Service'} = '0';
   $prereq->{'Win32::ShellQuote'} = '0';
   }
else
   {
   $prereq{'IO::Compress::Gzip'} = '0';
   }

# Is tpp installed?
my $bin = ( $^O =~ /MSWin/i ) ? 'C:\Inetpub\tpp-bin\tpp_hostname.exe' 
                              : `which tpp_hostname 2>/dev/null`;
chomp $bin;
$bin = (-f $bin) ? [ INSTALLSITESCRIPT => dirname( $bin ) ] : [];

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'AMZTPP',
    VERSION_FROM      => 'lib/TPP/AWS.pm',	# finds $VERSION
    PREREQ_PM         => $prereq,
    EXE_FILES         => $exefiles,
    MAN1PODS => { 'bin/amztpp' => 'blib/man1/amztpp.1',
                  'bin/amztppd' => 'blib/man1/amztppd.1',
                },
    @$bin,
    dist => {
            COMPRESS => 'gzip --best -f',
            },
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (
#      ABSTRACT_FROM  => 'lib/TPP.pm', # retrieve abstract from module
       AUTHOR         => 'Joseph Slagel <jslagel@systemsbiology.org>') : ()),
);

sub ppmrules {
    my $r = "\t\@echo Attempting to install prerequisites using ppm\n";
    foreach my $d ( sort keys %$prereq ) {
        $d =~ s/::/-/g;
        $d = lc $d;
        $r .= "\tppm install $d\n";
        }
    return $r;
}

sub cpanrules {
    my $r = "\t\@echo Attempting to install prerequisites using cpan\n";
    $r .= "\tcpan -i ";
    $r .= join ' ', sort keys %$prereq;
    $r .= "\n";
    return $r;
}

sub MY::postamble
{
    eval {
        use XML::SAX;
        if ( my $p = $INC{"XML/SAX.pm"} ) {
            $p =~ s/SAX.pm/SAX\/ParserDetails.ini/;
            if ( ! -e $p ) {
                $fix =<<TXT;
\t\@echo Creating missing XML::SAX ParserDetails.ini file
\t-\$(PERL) -MXML::SAX -e "close STDERR; XML::SAX->add_parser('XML::SAX::PurePerl')->save_parsers()"
TXT
            }
        }
    };

    my $rules = <<TXT;
\t\@echo "I don't know how to install prerequisites for your platform"
\t\@exit 1
TXT
    $rules = cpanrules() if ( $^O =~ /linux/ );
    $rules = ppmrules()  if ( $^O =~ /MSWin/i );

    return "install ::\n" . $fix . "\ninstalldeps :\n" . $rules;
}
