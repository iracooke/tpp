#!/usr/bin/perl -w
# 
# Front-end to SPC Analysis Tools (TPP)
#
# $Id: tpp_gui.pl 6751 2014-11-14 20:03:04Z slagelwa $
#
# 2005-2014 lmendoza@isb
#
use strict;
use CGI qw/-nosticky :standard/;
use CGI::Carp qw(carpout);
use CGI::Pretty;
use Cwd 'realpath';
use File::Basename;  # defines dirname()
use POSIX qw(setsid);
use IO::Compress::Zip qw( :all );
use XML::Parser;

# grab our tpplib exports from the same directory as this script
use lib realpath(dirname($0)); # puts this script's directory in perl's include path, @INC 
use tpplib_perl; # our exported TPP lib function points


########################################################################
# Configuration variables - 
# to durably change these defaults, set them in the tpp_gui_config.pl file 
# found in this directory (otherwise they'll get stomped in the next update)
#
# developers: syntax is readconfig(keyword,defaultvalue)
require 'tpp_gui_config.pl';
#
my $base_dir    = readconfig('base_dir',"/cygdrive/c/Inetpub/tpp-bin/");   # full path to this file
my $tmp_dir     = readconfig('tmp_dir',"/tmp/");                           # keep log files here
my $tpp_bin_url = readconfig('tpp_bin_url',"/tpp-bin");
my $java_cmd    = readconfig('java_cmd',"java");                           # you may need to set the full java path in config file
my $tpp_url     = readconfig('tpp_url',"$tpp_bin_url/tpp_gui.pl");
my $www_root    = readconfig('www_root',"/cygdrive/c/Inetpub/wwwroot/");   # full path to web server root
my $data_dir    = readconfig('data_dir',"${www_root}ISB/data/");           # full path to data directory top-level
my $log_file    = readconfig('log_file',"${tmp_dir}tpp_web.log");          # full path to log file
my $users_dir   = readconfig('users_dir',"${base_dir}users/");             # full path to users directory (passwds etc)
my $crypt_key   = readconfig('crypt_key','isbTPPspc');
my $ck_version  = readconfig('ck_version',1);
my $pipeline    = readconfig('pipeline','Comet');                          # set to default [Sequest Mascot SpectraST Tandem]  more to come...
my $rawfile     = readconfig('rawfile','raw');                             # set to default [raw wiff ddir rawdir]
my $debug       = readconfig('debug',0);                                   # set to 0 (zero) to suppress debug output to log file
my $doMzXMLGzip = readconfig('doMzXMLGzip',0);                             # set to 1 to tell converters to gzip their mzxml output
my $mascot_server = readconfig('mascot_server',"http://myserver/mascot/"); # Mascot server top-level url (if applicable)
my $htpasswd    = readconfig('htpasswd',"");                               # optionally authenticate using htpasswd file 
my $tppbin      = readconfig('tppbin','/usr/bin/');                        # Location of TPP bin
my $useBasicAuth = readconfig('useBasicAuth',0);                           # use basic authentication
my $s3cmd       = readconfig('s3cmd', undef );				   # full path to s3cmd program (enables S3 sync)
my $useSGE      = readconfig('useSGE', undef );				   # use qrsh to run commands on SGE
my $useSudo     = readconfig('useSudo', undef );			   # use sudo to run commands as authenticated user
#
# end config section
########################################################################

$ENV{LD_LIBRARY_PATH} = "/tools/lib:/proteomics/sw/qt/lib:/proteomics/sw/OpenMS/lib:/proteomics/sw/lib/perl5:/proteomics/sw/lib";

#
### vars above this line might need to get changed more often than those below ###
#
my %command = (
	       'chdir'       => 'cd',     # in win32 we use our run_in exe to handle compound cmds
	       'cp'          => '/usr/bin/cp',         # full path to cp
	       'mv'          => '/bin/mv',             # full path to mv
	       'md5'         => '/usr/bin/md5sum',     # full path to md5sum
	       'echo'        => '/bin/echo',           # full path to echo
	       'head'        => '/usr/bin/head',       # full path to head
	       'tail'        => '/usr/bin/tail',       # full path to tail
	       'touch'       => '/usr/bin/touch',      # full path to touch
	       'wc'          => '/usr/bin/wc',         # full path to wc
	       'wget'        => '/usr/bin/wget',       # full path to wget
	       'rm'          => '/usr/bin/rm',         # full path to rm
# TPP executables
	       'readw'       => "${tppbin}ReAdW",      # some Windows installations only
	       'mzwiff'      => "${tppbin}mzWiff",     # some Windows installations only
	       'masswolf'    => "${tppbin}massWolf",   # some Windows installations only
	       'trapper'     => "${tppbin}trapper",    # some Windows installations only
	       'dna2aa'      => "${tppbin}translateDNA2AA-FASTA",
	       'decoyfasta'  => "${tppbin}decoyFASTA",
	       'decoyfastagen'=> "${tppbin}decoyFastaGenerator.pl",
	       'decoyvalpeps' => "${tppbin}ProphetModels.pl",
	       'decoyvalprots'=> "${tppbin}ProtProphModels.pl",
	       'mayu'        => "${tppbin}Mayu.pl",
	       'idconvert'   => "${tppbin}idconvert",
	       'msconvert'   => "${tppbin}msconvert",
	       'mzxml2search'=> "${tppbin}MzXML2Search",
	       'dta2mzxml'   => "${tppbin}dta2mzxml",
	       'indexmzxml'  => "${tppbin}indexmzXML",
	       'out2xml'     => "${tppbin}Out2XML",
	       'mascot2xml'  => "${tppbin}Mascot2XML",
	       'comet2xml'   => "${tppbin}Comet2XML",
	       'tandem2xml'  => "${tppbin}Tandem2XML",
	       'protxml2html'=> "${tppbin}protxml2html.pl",
	       'runsearch'   => "${tppbin}runsearch",  # windows installations only 
	       'tandem'      => "${tppbin}tandem",
	       'comet'       => "${tppbin}comet",
	       'spectrast'   => "${tppbin}spectrast",
	       'runMaRiMba'  => "${tppbin}run_marimba.pl",
	       'lib2html'    => "${tppbin}Lib2HTML",
	       'downloader'  => "${tppbin}fileDownloader.pl",
	       'renamedat'   => "${tppbin}renamedat.pl",
	       'respect'     => "${tppbin}RespectParser",
	       'qualscore'   => "${java_cmd} -jar ${tppbin}qualscore.jar",
	       'get_prots'   => "${tppbin}get_prots.pl",
	       'compareprots'=> "${tppbin}compareProts.pl",
	       'heatmapprots'=> "${tppbin}compareProts_ClusterHM.pl",
	       'xinteract'   => "${tppbin}xinteract",
	       'interprophet'=> "${tppbin}InterProphetParser",
	       'ptmprophet'  => "${tppbin}PTMProphetParser",
	       'xpresspep'   => "${tppbin}XPressPeptideParser",
	       'asappep'     => "${tppbin}ASAPRatioPeptideParser",
	       'librapep'    => "${tppbin}LibraPeptideParser",
	       'runprophet'  => "${tppbin}ProteinProphet",
	       'updatepaths' => "${tppbin}updatepaths.pl",
	       'chargefile'  => "${tppbin}createChargeFile.pl",
	       'mergecharges'=> "${tppbin}mergeCharges.pl",
	       'amztpp'      => "${tppbin}amztpp",
	       'tpp_hostname'=> "${tppbin}tpp_hostname",
	       );

my $in_windows = ($^O eq 'MSWin32');
my $command_sep = ";";
#$|=1;
my $ppid;
if ($in_windows) {
    $command_sep =  "&";
    $ppid = -1; # no getpid in activestate perl
    $www_root = lcfirst($www_root);
    $data_dir = lcfirst($data_dir);

} else {
    $ppid = getppid();
}


# Globals
my $action = '';
my $auth_user;
my $user_session;
my $session_file;
my $sfnum = 0;  # index for "show files" widgets on screen
my $errors;
my @directories;
my @messages;
my @header_params;
my @session_data;
my @all_commands;
my @residues = qw(A C D E F G H I K L M N P Q R S T V W Y);
my %tmp_hash = ();
my $session_lastdir;
my $TPPVersionInfo = tpplib_perl::getTPPVersionInfo();
my $title = 'ISB/SPC Trans Proteomic Pipeline - ';
my $amz_s3url;

# list of action values coming from the web page (on the right)
my %web_actions = (
		   'login'         , 'Login',
		   'logout'        , 'Log Out',
		   'showpage'      , 'display',
		   'genAAdbase'    , 'Generate Translated AA Database',
		   'genDecoyDB'    , 'Generate Decoy Database',
		   'DecoyValPeps'  , 'Decoy Peptide Validation',
		   'DecoyValProts' , 'Decoy Protein Validation',
		   'runMayu'       , 'Run Mayu',
		   'genLibraCond'  , 'Generate Condition File',
		   'calcRetTime'   , 'Run RTCalc',
		   'trainRetTime'  , 'Create RTCalc Model',
		   'switchRawFile' , 'switchRawFile',
		   'mzxmlgen'      , 'Convert to mzXML',
		   'mzmlgen'       , 'Convert to mzML',
		   'toMzIdentML'   , 'Convert to mzIdentML',
		   'mzxml2other'   , 'Convert Files',
		   'dta2mzxml'     , 'Create mzXML file',
		   'runIndexmzxml' , 'Re-Index MzXML files',
		   'chargefile'    , 'Create charge file',
		   'mergecharges'  , 'Merge charges into mzXML',
		   'runqualscore'  , 'Run QualScore',
		   'runrespect'    , 'Run reSpect',
		   'compareprots'  , 'Compare Protein Lists',
		   'runspectrast'  , 'Run SpectraST',
		   'runtandem'     , 'Run Tandem Search',
		   'runcomet'      , 'Run Comet Search',
		   'fetchlibs'     , 'Download Selected Libraries',
		   'createidx'     , 'Import Library Files',
		   'runlib2html'   , 'Convert Library Files',
		   'runMaRiMba'    , 'Run MaRiMba',
		   'runsearch'     , 'Run Search',
		   'toPepXML'      , 'Convert to PepXML',
		   'xinteract'     , 'Run XInteract',
		   'runinterprophet','Run InterProphet',
		   'runptmprophet' , 'Run PTMProphet',
		   'runprophet'    , 'Run ProteinProphet',
		   'updatePaths'   , 'Update File Paths',
		   'selectFiles'   , 'Select',
		   'addFiles'      , 'Add Files',
		   'saveFile'      , 'Save Changes',
		   'saveParams'    , 'Save Search Parameters File',
		   'removeFiles'   , 'Remove',
		   'cancelSelect'  , 'Cancel',
		   'deleteFiles'   , 'Delete',
		   'cancelDelete'  , 'Do NOT Delete',
		   'confirmDelete' , 'Delete Files',
		   'copyFiles'     , 'Copy',
		   'pasteFiles'    , 'Paste',
		   'downloadFiles' , 'Download',
		   'uploadFile'    , 'Upload',
		   'newDir'        , 'Create new directory:',
		   'setWorkDir'    , 'Set as Working Directory',
		   'newpassword'   , 'Set New Password',
		   'addAmazonKeys' , 'Verify and Use Amazon EC2 Keys',
		   'delAmazonKeys' , 'De-register this Amazon EC2 Account',
		   'cleanAmazon'   , 'Shut down instances and delete all data',
		   'cancelClean'   , 'Do NOT shut down and delete',
		   'confirmClean'  , 'Shut down and Delete',
		   'fetchdat'      , 'Retrieve File',
		   'filterdat'     , 'Filter File',
		   'newMascotURL'  , 'Switch',
		   'switchPipeline', 'switchPipeline',
		   'deleteSession' , 'Delete this Session',
		   'deleteCommand' , 'deleteCommand',
		   'killCommand'   , 'killCommand',
		   'userTimeoutRecovery' , 'userTimeoutRecovery',
		   's3cmd'         , 'S3 Sync',
		   's3SyncUp'      , 'Sync to S3    ',
		   's3SyncDown'    , 'Sync from S3',
		   'none'          , 'None'
		  );

# map page alias to function that renders it
my %pages = (
	     'login'      ,  \&pageLogin,
	     'home'       ,  \&pageHome,
	     'account'    ,  \&pageAccount,
	     'sessions'   ,  \&pageSessions,
	     'clusters'   ,  \&pageClusters,
	     'decoyvalpeps', \&pageDecoyValPeps,
	     'decoyvalprots',\&pageDecoyValProts,
	     'dna2aa'     ,  \&pageDNA2AA,
	     'decoyfasta' ,  \&pageDecoyFasta,
	     'mayu'       ,  \&pageMayu,
	     'conditionxml', \&pageLibraCondition,
	     'rtcalc',	     \&pageRTCalc,
	     'rttrain',	     \&pageRTTrain,
	     'mzxml2search', \&pageMzXml2Other,
	     'dta2mzxml'  ,  \&pageDta2MzXml,
	     'indexmzxml' ,  \&pageIndexMzXml,
	     'chargefile' ,  \&pageCreateChargeFile,
	     'mergecharges', \&pageMergeCharges,
	     'getspeclibs',  \&pageGetSpecLibs,
	     'runspectrast', \&pageRunSpectraST,
	     'spectrastlib', \&pageCreateSpecLib,
	     'lib2html'   ,  \&pageLib2HTML,
	     'qualscore'  ,  \&pageQualscore,
	     'respect'    ,  \&pageReSpect,
	     'compareprots', \&pageCompareProts,
	     'runmascot'  ,  \&pageRunMascot,
	     'runtandem'  ,  \&pageRunTandem,
	     'runcomet'   ,  \&pageRunComet,
	     'updatepaths',  \&pageUpdatePaths,
	     'filebrowser',  \&pageBrowseFiles,
	     'filechooser',  \&fileChooser,
	     'showfile'   ,  \&showFile,
	     'editfile'   ,  \&editFile,
	     'editparams' ,  \&editParams,
	     'mzxml'      ,  \&pageMzXml,
	     'idconvert'  ,  \&pageIdConvert,
	     'msconvert'  ,  \&pageMsConvert,
	     'runsearch'  ,  \&pageRunSearch,
	     'topepxml'   ,  \&pageConverters,
	     'xinteract'  ,  \&pageXInteract,
	     'iprophet'   ,  \&pageInterProphet,
	     'ptmprophet' ,  \&pagePTMProphet,
	     'runprophet' ,  \&pageRunProphet,
	     'marimba'    ,  \&pageMaRiMba,
	     'jobs'       ,  \&pageJobs,
	     's3cmd'      ,  \&pageS3cmd,
	     );

# map session file prefixes/labels
my %proc_types = (
		  'mzxml'      ,  'ToMzXML',
		  'mzml'       ,  'ToMzML',
		  'mz2other'   ,  'FromMzXML',
		  'tocharge'   ,  'ToCharge',
		  'chargefile' ,  'ChargeFile',
		  'converters' ,  'ToPepXML',
                  'searchdb'   ,  'SearchDatabase',
		  'sequest'    ,  'SequestParams',
		  'runsearch'  ,  'Search',
		  'speclib'    ,  'SpectralLibrary',
		  'rawspeclib' ,  'RawSpectralLibrary',
		  'nistspeclib',  'NISTSpectralLibrary',
		  'xinteract'  ,  'XInteract',
		  'runprophet' ,  'ProtProphet',
		  'textfile'   ,  'TextFile',  # will change if too generic
		  'updatefiles',  'UpdatePaths',
		  'lastdir'    ,  'LastDirectory',
		  'copyfile'   ,  'CLIPBOARD',
		  'rawfile'    ,  '@RAWFILE_TYPE',
		  'mascoturl'  ,  '@MASCOT_SERVER',
		  'pipeline'   ,  '@PIPELINE_TYPE',
		  'logindate'  ,  '@LOGIN_DATETIME',
		  'protxml'    ,  'ProtXMLFile',
		  'resultspage',  'ResultsPage',
	          'extrapaths' ,  'UpdateAdditionalPaths'
		  );

if (param('Action') && param('Action') =~ /^AJAX/) {
    handleAjaxRequest();
} else {
    main();
}
exit(0);


########################################################################
# Main
#
# Initialize, fork based on Action, display page
#
########################################################################
sub main {
    &printToLog("\n==== NEW Request ==================== <".scalar(localtime).">\n") if ($debug);

    &init();

    my $page = 'login';

    if ( !($action) ) {
	&fatalError('NO_ACTION');
    } elsif ( $action eq $web_actions{"none"} ) {
	# force a login
	&killSession();
	$page = 'login';
    } elsif ( $action eq $web_actions{"logout"} ) {
	&killSession();
	splice @messages, 0;   # forget all messages
	push @messages, "You have been logged out.";
	if ( !$useBasicAuth ) {
	   push @messages, "Please log in to use the ISB/SPC Tools.";
        } else {
           push @messages, "Please reload this page or click on the following" .
                           " link to start a new session: " .
                           a( { href => $tpp_url }, $tpp_url );
        }
	$page = 'login';
    } elsif ( $action eq $web_actions{"login"} ) {
	&killSession();
	# authenticate login info
	$page = (&authenticateUser() eq 'OK') ? 'home' : 'login';

    # All actions below require a valid login session
    } elsif ( !($user_session) ) {
	# force a login
	&killSession();
	if ( !$useBasicAuth ) {
           push @messages, "Please log in to use the ISB/SPC Tools.";
        }
	$page = 'login';
    } elsif ( $action eq $web_actions{'userTimeoutRecovery'} ) {
	if (param('show_job')) {
	    &userTimeoutRecovery(param('show_job'));
	} else {
	    push @messages, "Can't modify job log file: no job id passed.";
	}

	$page = 'jobs';
    } elsif ( $action eq $web_actions{'showpage'} ) {
	$page = param('page');
    } elsif ( $action eq $web_actions{'cancelSelect'} ) {
	push @messages, "File selection cancelled. No files have been added.";
	$page = param('ref_page');

    } elsif ( $action eq $web_actions{'saveFile'} ) {
	my $file_path = param('file_path');
	my $file_contents = param('file_contents');

	&saveFile(file_path     => $file_path,
		  file_contents => $file_contents);

	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'addFiles'} ) {
	$page = 'filechooser';
    } elsif ( $action eq $web_actions{'removeFiles'} ) {
	my @listoffiles = param(param('remfiles'));
	my $ref_page = param('ref_page');

	if (@listoffiles) {
	    &removeFiles(@listoffiles);
	} else {
	    push @messages, "Please select the file(s) that you wish to remove from processing.";
	}

	$page = $ref_page;

    } elsif ( $action eq $web_actions{'selectFiles'} ) {
	my $d_src = param('quickdir') || param('workdir') || $data_dir;
	my @listoffiles = param('src_files');
	my $ref_page = param('ref_page') || 'topepxml';

	my $proc_type = param('proc_type') || param('ref_page') || 'converters';

	## /
	if (@listoffiles) {
	    my $update_dir = !param('quickdir');
	    &addFiles(ref_page => $proc_type,
		      dir      => $d_src,
		      updatedir=> $update_dir,
		      files    => \@listoffiles);

	    push @messages, "Your files have been added:";
	    push @messages, @listoffiles;
	} else {
	    # small bug here when no files are selected - user not shown file select   FIXME
	    push @messages, "Please select the file(s) that you wish to add for processing.";
	}
	## \
	$page = $ref_page;

    } elsif ( $action eq $web_actions{'deleteFiles'} ) {
	my $d_src = param('workdir') || $data_dir;
	my @listoffiles = param('src_files');

	if (@listoffiles) {
	    push @messages, "Please confirm that you want to permanently delete the following file(s) from the file system:";
	    push @messages, @listoffiles;

	    # this creates a confirm delete dialog
	    push @messages, "DIALOG:[confirmDelete|workdir,src_files][cancelDelete|workdir]";

	} else {
	    push @messages, "Please select the file(s) that you wish to delete.";
	}

	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'cancelDelete'} ) {
	push @messages, "File deletion cancelled. No files have been deleted.";
	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'confirmDelete'} ) {
	my $d_src = param('workdir');
	my @listoffiles = param('src_files');

	if (@listoffiles && $d_src) {
	    &deleteFiles(dir   => $d_src,
			 files => \@listoffiles);

	} else {
	    push @messages, "Can't delete files: no base directory or file list!!";
	}

	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'downloadFiles'} ) {
    	
	my $d_src = param('workdir');
	my @listoffiles = param('src_files');

        if ( $#listoffiles == 0 && -f "$d_src$listoffiles[0]" ) {
	    &downloadFile( $d_src, $listoffiles[0] );
	} elsif ( @listoffiles ) {
	    &downloadFilesArchive(dir => $d_src, files => \@listoffiles);
	} else {
	    push @messages, "Can't download files: no base directory or file list!!";
	}

	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'copyFiles'} ) {
	my $d_src = param('workdir');
	my @listoffiles = param('src_files');

	if (@listoffiles) {
	    &addFiles(ref_page => 'copyfile',
		      dir      => $d_src,
		      files    => \@listoffiles);

	    push @messages, "Your files have been added to the clipboard:";
	    push @messages, @listoffiles;
	} else {
	    # small bug here when no files are selected - user not shown file select   FIXME
	    push @messages, "Please select the file(s) that you wish to copy.";
	}

	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'pasteFiles'} ) {

	my $d_src = param('workdir');

	if ($d_src) {
	    &pasteFiles($d_src);
	} else {
	    push @messages, "Can't delete files: no base directory passed!!";
	}

	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'s3cmd'} ) {

        my $s3cfg_file = "${users_dir}$auth_user/.s3cfg";
	my $d_src = param('workdir');
        if ( !$d_src ) {
	    push @messages, "Can't sync to S3: no base directory passed!!";
	    $page = 'filebrowser';
        } elsif ( ! -f $s3cfg_file ) {
	    push @messages, "Can't sync to S3: missing user .s3cfg file";
	    $page = 'filebrowser';
        } elsif ( !($amz_s3url = read_s3cfg( $s3cfg_file )) ) {
	    push @messages, "Can't sync to S3: no tpp_s3url found in s3cfg file $s3cfg_file";
	    $page = 'filebrowser';
        } else {
	    $page = 's3cmd';
	}

    } elsif ( $action eq $web_actions{'s3SyncUp'} ) {

	my $d_src = param('workdir');
        my $paths = param('local_paths');

        my $s3cfg_file = "${users_dir}$auth_user/.s3cfg";
        if ( !($amz_s3url = read_s3cfg( $s3cfg_file )) ) {
	    push @messages, "Can't sync to S3: no tpp_s3url found in s3cfg file $s3cfg_file";
	    $page = 'filebrowser';
        } else {
            my $d = $d_src; $d =~ s/$data_dir//; 
            $page = runS3Sync( $d_src, $paths, "$amz_s3url/$d" );
        }

    } elsif ( $action eq $web_actions{'s3SyncDown'} ) {

	my $d_src = param('workdir');
        my $paths = param('remote_paths');

        $page = runS3Sync( $d_src, $paths, '.' );

    } elsif ( $action eq $web_actions{'uploadFile'} ) {

	my $d_src = param('workdir');
	if ( !$d_src) {
	    push @messages, "Can't upload file: no base directory passed!!";
	} else {
	    &uploadFile( $d_src );
	}
 
	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'setWorkDir'} ) {
	&updateSession(action => 'workDir',
		       dir    => param('workdir'));

	push @messages, param('workdir')." is now the current working directory.";
	$page = 'filebrowser';

    } elsif ( $action eq $web_actions{'newDir'} ) {
        &createFolder(param('new_folder'));
        $page = param('ref_page');

    } elsif ( $action eq $web_actions{'fetchdat'} ) {
	$page = &fetchMascotResultFile();
    } elsif ( $action eq $web_actions{'filterdat'} ) {
	$page = &filterMascotFile();
    } elsif ( $action eq $web_actions{'genLibraCond'} ) {
	$page = &generateLibraConditionFile();
    } elsif ( $action eq $web_actions{'calcRetTime'} ) {
	$page = &calculateRetentionTime();
    } elsif ( $action eq $web_actions{'trainRetTime'} ) {
	$page = &trainRetentionTime();
    } elsif ( $action eq $web_actions{'updatePaths'} ) {
	$page = &updatePaths();
    } elsif ( $action eq $web_actions{'chargefile'} ) {
	$page = &runCreateChargeFile();
    } elsif ( $action eq $web_actions{'mergecharges'} ) {
	$page = &runMergeCharges();
    } elsif ( $action eq $web_actions{'mzxml2other'} ) {
	$page = &mzToSearch() ;
    } elsif ( $action eq $web_actions{'mzxmlgen'} ) {
	$page = &toMzXML();
    } elsif ( $action eq $web_actions{'mzmlgen'} ) {
	$page = &toMzML();
    } elsif ( $action eq $web_actions{'toMzIdentML'} ) {
	$page = &toMzIdentML();
    } elsif ( $action eq $web_actions{'dta2mzxml'} ) {
	$page = &dtaToMzXML();
    } elsif ( $action eq $web_actions{'runIndexmzxml'} ) {
	$page = &runIndexMzXML();
    } elsif ( $action eq $web_actions{'saveParams'} ) {
	$page = &saveParams();
    } elsif ( $action eq $web_actions{'runsearch'} ) {
	$page = &runSearch();
    } elsif ( $action eq $web_actions{'runtandem'} ) {
	$page = &runTandem();
    } elsif ( $action eq $web_actions{'runcomet'} ) {
	$page = &runComet();
    } elsif ( $action eq $web_actions{'runspectrast'} ) {
	$page = &runSpectraST();
    } elsif ( $action eq $web_actions{'runrespect'} ) {
	$page = &runReSpect();
    } elsif ( $action eq $web_actions{'runqualscore'} ) {
	$page = &runQualScore();
    } elsif ( $action eq $web_actions{'compareprots'} ) {
	$page = &runCompareProts();	
    } elsif ( $action eq $web_actions{'fetchlibs'} ) {
	$page = &fetchSpectralLibs();
    } elsif ( $action eq $web_actions{'createidx'} ) {
	$page = &createSpectraSTIndexFiles();
    } elsif ( $action eq $web_actions{'runlib2html'} ) {
	$page = &runLib2HTML();
    } elsif ( $action eq $web_actions{'runMaRiMba'} ) {
	$page = &runMaRiMba();
    } elsif ( $action eq $web_actions{'genAAdbase'} ) {
	$page = &runDNA2AA();
    } elsif ( $action eq $web_actions{'genDecoyDB'} ) {
	$page = &runDecoyFasta();
    } elsif ( $action eq $web_actions{'DecoyValPeps'} ) {
	$page = &runDecoyValPeps();
    } elsif ( $action eq $web_actions{'DecoyValProts'} ) {
	$page = &runDecoyValProts();
    } elsif ( $action eq $web_actions{'runMayu'} ) {
	$page = &runMayu();
    } elsif ( $action eq $web_actions{'toPepXML'} ) {
	$page = &toPepXML();
    } elsif ( $action eq $web_actions{'xinteract'} ) {
	$page = &runXInteract();
    } elsif ( $action eq $web_actions{'runinterprophet'} ) {
	$page = &runInterProphet();
    } elsif ( $action eq $web_actions{'runptmprophet'} ) {
	$page = &runPTMProphet();
    } elsif ( $action eq $web_actions{'runprophet'} ) {
	$page = &runProteinProphet();
    } elsif ( $action eq $web_actions{'switchRawFile'} ) {
	# re-use addFiles, with a small hack (I know, I know...)
	$rawfile = param('rawfile_type') if param('rawfile_type');

	my @orig_messages = @messages;  #remember any previous messages
	for (sort @session_data) {
	    if (/^$proc_types{'rawfile'}/) {
		&removeFiles($_);
	    }
	}
	@messages = @orig_messages;
	push @messages, "The default instrument file type for your session has been changed to: $rawfile";
	&addFiles(ref_page => 'rawfile',
		  files    => [$rawfile]);

	$page = param('refpage') ? param('refpage') : 'mzxml';

    } elsif ( $action eq $web_actions{'switchPipeline'} ) {
	# re-use addFiles, with a small hack
	$pipeline = param('pipeline_type') if param('pipeline_type');

	my @orig_messages = @messages;  #remember any previous messages
	for (sort @session_data) {
	    if (/^$proc_types{'pipeline'}/) {
		&removeFiles($_);
	    }
	}
	@messages = @orig_messages;
	push @messages, "The pipeline type for your session has been changed to: <b>$pipeline</b>";
	&addFiles(ref_page => 'pipeline',
		  files    => [$pipeline]);

	$page = 'home';

    } elsif ( $action eq $web_actions{'newMascotURL'} ) {
	&setNewMascotURL();
	$page = 'runmascot';

    } elsif ( $action eq $web_actions{'deleteSession'} ) {
	if (param('session_select')) {
	    &deleteSessionFiles(param('session_select'));
	} else {
	    push @messages, "Can't delete session files: no session id passed!!";
	}
	$page = 'sessions';

    } elsif ( $action eq $web_actions{'deleteCommand'} ) {
	if (param('job_id')) {
	    &deleteCommandFile(param('job_id'));
	} else {
	    push @messages, "Can't delete command log file: no command id passed!";
	}
	$page = 'jobs';

    } elsif ( $action eq $web_actions{'killCommand'} ) {
	if (param('job_id') &&	param('jpid')) {
	    &killCommand(param('job_id'), param('jpid'));
	} else {
	    push @messages, "Can't kill command: no job id passed!";
	}
	param('show_job',param('job_id'));
	$page = 'jobs';

    } elsif ( $action eq $web_actions{'addAmazonKeys'} ) {
	$page = &verifyEC2Keys();

    } elsif ( $action eq $web_actions{'delAmazonKeys'} ) {
	$page = &removeEC2Keys();

    } elsif ( $action eq $web_actions{'cleanAmazon'} ) {
	push @messages, "Please confirm that you want to shut down all running EC2 nodes (if any) and delete all data from Amazon S3";
	# this creates a confirm delete dialog
	push @messages, "DIALOG:[confirmClean][cancelClean]";
	$page = 'clusters';

    } elsif ( $action eq $web_actions{'cancelClean'} ) {
	push @messages, "Amazon action cancelled. No nodes have been shut down or files deleted.";
	$page = 'clusters';

    } elsif ( $action eq $web_actions{'confirmClean'} ) {
	$page = &realcleanEC2();

    } elsif ( $action eq $web_actions{'newpassword'} ) {
	&setNewPassword();
	$page = 'account';
    } else {
	&fatalError("UNK_ACTION($action)");
    }

    &printToLog("Page is ($page)\n") if ($debug);

    if ($page eq 'none') {
	&printToLog("[[[NONE]]] Doing nothing...\n") if ($debug);

    } else {

	my $tab_page = ($page eq 'filechooser') ? param('ref_page') : $page;
	&openHTMLPage("$title $page",$tab_page);

	my $displayed;
	&printToLog("Looking for page($page)...") if ($debug);
	while (my($page_name, $function) = each %pages) {
	    if ($page_name eq $page) {
		$function->();
		$displayed = $page_name;
		&printToLog("found it!\n") if ($debug);
		last;
	    }
	}
	unless ($displayed) {
	    print qq(<h1 id="error">Error Found!</h1>Page Mapping undefined!);
	    &printToLog("NOT found!!\n") if ($debug);
	    &printToLog("ERROR: page \"$page\" not mapped!\n") if ($debug);
	}

	&closeHTMLPage();
    }

    &printToLog("==== End of Request ================= page[$page] pid[$$] ppid[$ppid]\n") if ($debug);

    my $parpid = $$; # save this to kill jobs under windows

    if (@all_commands) {

	# daemonize this bad boy
	defined(my $pid = fork)   or die "Can't fork: $!";
	if ($pid) {
	    unless ($in_windows) { # refresh page immediately (100ms)
		my $cmd_file  = shift(@all_commands);
		$cmd_file =~ /cmd_/;
		my $link = "$tpp_url?Action=$web_actions{'showpage'}&page=jobs&show_job=$'";
		print "<script type=\"text/javascript\">setTimeout('window.location = \"$link\"',100);</script>";
	    }
	    exit;  # done with parent
	}
	chdir '/'                 or die "Can't chdir to /: $!";
	umask 0;
	unless ($in_windows) {
	    unless ($debug) {
		open STDIN, '/dev/null'   or die "Can't read /dev/null: $!";
		open STDOUT, '>/dev/null' or die "Can't write to /dev/null: $!";
	    }
	    setsid                    or die "Can't start a new session: $!";
	    unless ($debug) {
		open STDERR, '>&STDOUT'  or die "Can't dup stdout: $!";
	    }
	}

	if ($in_windows) { &runCommands($parpid); }  else { &runCommands($$); }
    }

#    &printToLog(cgi_error(),"\n");

}


########################################################################
# init
#
# Initialize, check browser type, session, input variables
#
########################################################################
sub init {

    # Check for log file
    if (! -f $log_file) { `$command{touch} $log_file`; }

    if ($debug) {
	&printToLog("------- Incoming parameters -------\n");
	for my $key (sort(param())) {
	    # do not log passwords
	    my $val = ($key eq 'password') ? '******' : param($key);
	    &printToLog("\t$key\t:\t$val\n");
	}
	&printToLog("-----------------------------------\n");
    }

    # Session detection
    $user_session = $useBasicAuth ? &basicAuthCheck() : &getSession();
    &getSessionParams($user_session);

    # Get Action
    $action = param('Action') || $web_actions{'none'};

    # Clean up directory (and file?) names
    if (param('workdir')) {
	unless (-d param('workdir')) {
	    push @messages, "Attempted to access non-existing directory.";
	    $errors++;
	    param('workdir', $data_dir);
	}

	param('workdir', realpath(param('workdir'))."/");

	if ($in_windows) {
	    param('workdir', lcfirst(param('workdir')));  # accept C:\  etc
	}

	if (param('workdir') !~ /$data_dir/) {
	    push @messages, "Attempted to access forbidden directory.";
	    $errors++;
	    param('workdir', $data_dir);
	}
    } else {
	param('workdir',$session_lastdir); #as read from the session file
    }

    1;
}


########################################################################
# getAllCommandsStatus
#
# Read from .jobs file; check for status updates for non-"viewed" jobs; update file if necessary
#  
# Returns: Return type = all     :: entire array (file)
#                      = updated :: sid, new status of new only
#                      = summary :: count per status type
#
########################################################################
sub getAllCommandsStatus {
    my $req_type = shift || 'summary';
    my $req_session = shift || '';

    my (%summary, %updated);

    my $jobs_file = "${users_dir}$auth_user/.jobs";

    use Fcntl qw(:DEFAULT :flock);

    open(JOBS, "$jobs_file") || &fatalError("CANNOT_OPEN_JOBS_FILE:$jobs_file:$!");
    flock(JOBS, LOCK_SH) || &fatalError("CANNOT_LOCK_SH_JOBS_FILE:$jobs_file:$!");
    my @jobs_data = <JOBS>;
    close(JOBS);

    return if (!@jobs_data);

    my $update = 0;
    foreach (@jobs_data) {
	next if (($req_session) && !(/cmd_$req_session/));
	chomp;
	my ($jcmd, $jname, $jloc, $jstatus, $jpid) = split /\t/, $_;

	unless ($jstatus eq 'viewed') {
	    $jcmd =~ s/^cmd_//;
	    my $curr_jstatus = &getCommandStatus($jcmd) || "--UNKNOWN!--";

	    if ($curr_jstatus ne $jstatus) {
		$_ =~ s/$jstatus/$curr_jstatus/;
		$updated{$jcmd} = $curr_jstatus;
		$jstatus = $curr_jstatus;
		$update++;
	    }
	}
	$summary{$jstatus}++;
    }

    if ($update) {
	sysopen(JOBS, $jobs_file, O_WRONLY) || &fatalError("CANNOT_UPDATE_JOBS_FILE:$jobs_file:$!");
	flock(JOBS, LOCK_EX) || &fatalError("CANNOT_LOCK_EX_JOBS_FILE:$jobs_file:$!");
	truncate(JOBS, 0) || &fatalError("CANNOT_TRUNCATE_JOBS_FILE:$jobs_file:$!");
	foreach (@jobs_data) {
	    print JOBS "$_\n";
	}
	close(JOBS);
    }

    return @jobs_data if ($req_type eq 'all');
    return %summary if ($req_type eq 'summary');
    return %updated if ($req_type eq 'updated');

}


########################################################################
# getCommandStatus
#
# Input:   command id (combo of session id and command timestamp)
# Returns: one of [none, running, finished, viewed, queued, killed]
#
########################################################################
sub getCommandStatus {
    my $sid = shift || 'NONE';
    my $retstatus = '';

    my $cmd_file = "${users_dir}$auth_user/cmd_$sid";

    if (-e $cmd_file) {
	&printToLog("Found command session file: $cmd_file ...") if $debug;
	chomp (my $status = `$command{tail} -1 $cmd_file`);
	if ($status =~ /END COMMAND BLOCK/) {
	    &printToLog("last line is $status [no longer running]\n") if $debug;
	    $retstatus = 'finished';
	} elsif ($status =~ /COMMAND TERMINATED/) {
	    $retstatus = 'killed';
	} elsif ($status =~ /OUTPUT SEEN OK/) {
	    $retstatus = 'viewed';
	} elsif ($status =~ /QUEUED TO AMAZON EC2/) {
	    $retstatus = 'queued';
	} else {
	    &printToLog("last line is $status [still running]\n") if $debug;
	    $retstatus = 'running';
	}
    } else {
	&printToLog("Command session file not found: $cmd_file\n") if $debug;
	$retstatus = 'none';
    }

    return $retstatus;
}


########################################################################
# printToLog
#
# Print. To. Log.
#
########################################################################
sub printToLog {
    my ($msg) = shift || "No message!\n";
    open(LOUT, ">>$log_file") || &fatalError('NOLOGFILE',"Cannot open $log_file for writing: $!\n");
    select((select(LOUT), $| = 1)[0]);	# autoflush
    print LOUT $msg;
    close(LOUT);
    return 1;
}


########################################################################
# getSessionParams  
#
# parse session file, load values
#                   
#
########################################################################
sub getSessionParams {
    my $user_sesssion = shift;

    if ($user_session) {
	$session_file = "${users_dir}$auth_user/session_$user_session";
	if (!open(SESSION, "$session_file")) {
	    &killSession();
	    &fatalError("INIT_CANNOT_OPEN_SESSION_FILE:$session_file:$!");
	}
	@session_data = <SESSION>;
	close(SESSION);
	if ($debug) {
	    &printToLog("------- Session Info --------------\n");
	    for (@session_data) { chomp; &printToLog("$_\n"); }
	    &printToLog("-----------------------------------\n");
	}

	for (@session_data) {
	    chomp;
	    $pipeline        = $' if (/$proc_types{'pipeline'}:/);  #'
	    $mascot_server   = $' if (/$proc_types{'mascoturl'}:/);  #'
	    $rawfile         = $' if (/$proc_types{'rawfile'}:/);  #'
	    $session_lastdir = $' if (/$proc_types{'lastdir'}:/);  #'
	}

    }
    else {
	&printToLog("Command session file not searched for; new session\n") if $debug;
    }

}

########################################################################
# getSession
#
# Get cookie, parse values, verify checksum
# Return session id from cookie
#
########################################################################
sub getSession {
    my $tpp_cookie;

    if ($tpp_cookie = cookie("TPPSession")) {
	# &getSessionParams();   # implement this! FIXME

	&printToLog("TPPSession cookie found: $tpp_cookie\n") if ($debug);

	my ($session_id,$c_user,$c_md5) = split /:/, $tpp_cookie;
	my @md5res = split /\s+/, `$command{echo} $session_id:$c_user:$crypt_key | $command{md5}`;
	&fatalError("MD5_GEN:$?") if ($?);
	my $md5sum = $md5res[0];

	if ($md5sum eq $c_md5) {
	    $auth_user = $c_user;
	    &printToLog("Valid session found for user $auth_user\n") if ($debug);
	    return $session_id;
	} else {
	    &printToLog("Invalid session found: $tpp_cookie\n") if ($debug);
	    return 0;
	}
    } else {
	&printToLog("User session not found.\n") if ($debug);
        return 0;
    }
}


########################################################################
# setSession
#
# Set user session to cookie
#
########################################################################
sub setSession {
    my $user = shift;
    &printToLog("in setSession\n") if ($debug);

    srand;
    my $new_session = "";
    my @charList = ('A'..'Z',0..9);
    for (my $i = 0; $i<9; $i++) {
        $new_session .= $charList[int(rand 36)];
    }

    &printToLog($command{echo}.' '.$new_session.':'.$user.':'.$crypt_key.' | '.$command{md5})  if ($debug);

    my @md5res = split /\s+/, `$command{echo} $new_session:$user:$crypt_key | $command{md5}`;
    &fatalError("MD5_GEN:$?") if ($?);

    my $md5sum = $md5res[0];
    &printToLog("MD5sum: $md5sum\n") if ($debug);

    push @header_params, "Set-Cookie: TPPSession=$new_session:$user:$md5sum;";
    $user_session = $new_session;

    $session_file = "${users_dir}$user/session_$user_session";
    open(SESSION, ">$session_file") || &fatalError("CANNOT_CREATE_SESSION_FILE:$session_file:$!");
    print SESSION "$proc_types{'logindate'}:".scalar(localtime)."\n";
    print SESSION "$proc_types{'pipeline'}:${pipeline}"."\n";
    print SESSION "$proc_types{'mascoturl'}:${mascot_server}"."\n";
    print SESSION "$proc_types{'lastdir'}:${data_dir}"."\n";
    print SESSION "$proc_types{'resultspage'}:results"."\n";
    close(SESSION);

    &printToLog("New session started: $user_session\n") if ($debug);
    &printToLog("Created session file: $session_file\n") if ($debug);

    push @messages, "Welcome, $user.";
    $ck_version++;
}


########################################################################
# updateSession
#
# Update entries in session file
#    action -> remove, delete, workDir, resultsPage
#
########################################################################
sub updateSession {
    my %args = @_;
    my $update_action = $args{action};
    my $dir           = $args{dir};
    my @files         = @{$args{files}} if $args{files};

    my @s_file_list;
    my $update_work_dir = ($update_action eq 'workDir') ? 'true' : '';

    open(SESSION, ">$session_file") || &fatalError("UPDATE_CANNOT_WRITE_SESSION_FILE:$session_file:$!");

    for my $session_file (sort @session_data) {
	chomp $session_file;

	my $remove = '';

	# DELETE
	if ($update_action eq 'delete') {
	    $update_work_dir = 'true';

	    for my $remove_file (@files) {
		if ($session_file =~ /$dir$remove_file$/) {
		    &printToLog("REMOVING SESSION FILE: $session_file\n") if ($debug);
		    $remove = 'true';
		}
	    }
	}

	# REMOVE
	elsif ($update_action eq 'remove') {
	    for my $remove_file (@files) {
		if ($session_file eq $remove_file) {
		    &printToLog("REMOVING SESSION FILE: $session_file\n") if ($debug);

		    $remove = 'true';
		    $remove_file =~ /:/;
		    push @messages, $';  #'
		}
	    }
	}

	# set our extra results "page"... pass the page name hash as $dir
	elsif ($update_action eq 'resultsPage') {
	    if ($session_file =~ /^$proc_types{'resultspage'}/) {
		$session_file = "$proc_types{'resultspage'}:$dir";
	    }
	}

	# others
	elsif ($session_file =~ /^$proc_types{$update_action}/  &&
	       $proc_types{$update_action} ) {
	    $session_file = "$proc_types{$update_action}:$dir";
	}


	# update lastdir
	if ($update_work_dir) {  # this should be an 'if' only - since the 'delete' action also triggers this
	    if ($session_file =~ /^$proc_types{'lastdir'}/) {
		$session_file = "$proc_types{'lastdir'}:$dir";
	    }
	}

	unless ($remove) {
	    print SESSION "$session_file\n";
	    push @s_file_list, $session_file;
	}

    }

    close(SESSION);
    @session_data = @s_file_list;

}


########################################################################
# killSession
#
# Remove session cookie
#
########################################################################
sub killSession {
    if (cookie("TPPSession")) {
	&printToLog("Stopping user(session): $auth_user($user_session)\n") if ($debug);

	# unset globals for user
	$user_session = '';
	$auth_user = '';
	push @header_params, "Set-Cookie: TPPSession=; expires=Wednesday, 09-Nov-99 23:12:40 GMT;";
    }

}


########################################################################
# authenticateUser
#
# Authenticate user/pwd for login
#
# Returns "OK" if successful
#
########################################################################
sub authenticateUser {
    my $retstr = 'NOT_OK';

    &printToLog("in authenticateUser...\n") if ($debug);

    # pass parameters from POST
    if (&verifyPassword( param('username'), param('password') ) eq 'OK') {
	$auth_user = param('username');

	#set session cookie, open session file
	&setSession($auth_user);

	# Check for jobs file
        my $jobs_file = "${users_dir}$auth_user/.jobs";
	if (! -f $jobs_file) { `$command{touch} $jobs_file`; }

	$retstr = 'OK';
    }

    &printToLog("$retstr\n") if ($debug);
    return $retstr;
}

########################################################################
# basicAuthCheck
#
########################################################################
sub basicAuthCheck {
    if (cookie("TPPSession")) {
        if ( !param('Action') ) {
            param('Action', $web_actions{'showpage'} );
            param('page', 'home' );
        }
        return &getSession();
    }
    if ( !($auth_user = remote_user()) ) {
	&fatalError( 'No basic authentication user found' );
    }
    &printToLog("Basic authentication in use, REMOTE_USER is $auth_user\n") if ( $debug );
    if ( ! -d "${users_dir}$auth_user" ) {
	push @messages, "No user directory found for $auth_user.  Please " .
	    'contact your system adminstrator for access.';
        $auth_user = undef;
	return 0;
    }

    &setSession($auth_user);

    # Check for jobs file
    my $jobs_file = "${users_dir}$auth_user/.jobs";
    if (! -f $jobs_file) { `$command{touch} $jobs_file`; }

    param('Action', $web_actions{'showpage'} );
    param('page', 'home' );
    return $user_session;
}


########################################################################
# readAllowedDirs
#
########################################################################
sub readAllowedDirs {
    # get parameters
    my $user = shift || '';
    my $dfile = "${users_dir}$user/.directories";
    if (-f $dfile) {
	open(DIRS, $dfile) || 
	    &fatalError('DIR_FILE',"Error reading allowed directories file!");
	while(<DIRS>) {
	    chomp (my $dir = $_);
	    if (-d $dir) {
		push @directories, $dir;
	    }
	    else {
		&printToLog("$dir is not a valid directory\n") if ($debug);
	    }
	}
    }
}


########################################################################
# verifyPassword
#
# Compare password to encrypted one in file
#
# Returns "OK" if successful
#
########################################################################
sub verifyPassword {
    my $retstr = 'NOT_OK';

    &printToLog("verifying password...\n") if ($debug);

    # get parameters
    my $user = shift || '';
    my $pswd = shift || '';
    my $upwd = '';
    my $ckey = '';
    
    # get encrypted password
    if ( $htpasswd ) {			# use htpasswd to authenticate
        &printToLog("looking in $htpasswd for password\n") if ($debug);
	open(PWD, $htpasswd) || &fatalError('PWD_FILE',"Error verifying password!");
	while ( <PWD> ) {
	    chop();
	    my ( $u, $p ) = split /:/;
	    $upwd = $p if ( $u eq $user );	
	}
	close(PWD);
	$ckey = substr( $upwd, 0, 2 );
    } else {				# use local password file in user dir
        my $pfile = "${users_dir}$user/.password";
        &printToLog("looking in $pfile for password\n") if ($debug);
        if ( -e $pfile ) {
	    open(PWD, $pfile) || &fatalError('PWD_FILE',"Error verifying password!");
	    chomp($upwd = <PWD>);
	    close(PWD);
        }   
        $ckey = $crypt_key;
    }
    &printToLog("stored password is: --$upwd--\n") if ($debug);

    if ( !($user && $pswd)) {
	push @messages, "Not enough information for login. Please fill out all fields.";
    } elsif (!$upwd) {
	push @messages, "User $user not found. Please check your user name, or log in as guest.";
    } else {
	if (crypt($pswd, $ckey) eq $upwd) {
	    $retstr = 'OK';
	} else {
	    push @messages, "Incorrect password. Please re-enter.";
	}
    }

    &printToLog("$retstr\n") if ($debug);
    return $retstr;
}


########################################################################
# fatalError
#
# Display error page
#
########################################################################
sub fatalError {
    my $error_code = $_[0];
    my $error_msg  = $_[1] || 'A fatal error has been encountered.';

    &openHTMLPage("$title Error");
    print qq(<h1 id="error">Error Found!</h1>$error_code: $error_msg);
    &closeHTMLPage();

    # avoid an infinite loop!
    unless ($error_code eq 'NOLOGFILE') {
	&printToLog("ERROR: $error_code\n");
	&printToLog("==== Processing of Request Terminated ======\n") if ($debug);
    }

    exit;
}


########################################################################
# listFiles
#
# Find files of a certain type in a given directory
#
########################################################################
sub listFiles {
    my $dir  = shift;
    my $pattern   = shift;
    my $is_suffix = shift || 'true';

    my @file_list;

    opendir BASEDIR, $dir || &fatalError("BAD_DIR:$!");


    if ($pattern eq '*') {
	my @dir_ls = grep !/^\./, readdir BASEDIR;
	for (sort @dir_ls) {
	    push @file_list, $_ if (-f "${dir}$_");
	}

    } elsif ($is_suffix eq 'true') {
	if ($pattern eq 'mzxml') {  # handle mzData and mzML as well
	    @file_list = grep /\.mzxml|mzdata|mzml(\.gz)?$/i, readdir BASEDIR;
	} else {
	    @file_list = grep /\.$pattern(\.gz)?$/i, readdir BASEDIR;
	}
    } else {
	@file_list = grep /$pattern/i, readdir BASEDIR;

    }

    closedir BASEDIR;
    return @file_list;

}

########################################################################
# listDirs
#
# Return a list of directories in a given directory
#
########################################################################
sub listDirs {
    my $dir = shift;
    my @dir_list;

    opendir BASEDIR, $dir || &fatalError("BAD_DIR:$!");
    my @dir_ls = grep !/^\./, readdir BASEDIR;

    for (sort @dir_ls) {
	push @dir_list, $_ if (-d "${dir}/$_");
    }
    closedir BASEDIR;
    return @dir_list;

}


########################################################################
# createFolder
#
# Makes a new directory
#
# Contributed by Bill Nelson
########################################################################
sub createFolder {
    my $folderName = shift;
    my $dir = param('workdir');
    if( $folderName =~ /[\/?:\\*<>|]/ ) {
        push @messages, "ERROR: The folder name contains an illegal character.\"$folderName\"";
    } elsif ($folderName eq "" ) {
        push @messages, "ERROR: No folder name was entered.";
    } else {
	if (mkdir( ${dir}.$folderName )) {
	    push @messages, "Folder \"${dir}$folderName\" successfully created.";
	    Delete('new_folder');
	} else {
	    push @messages, "ERROR: creating the folder \"${dir}$folderName\":$!";
	}
    }

}


########################################################################
# pasteFiles
#
# Paste (copy) files from session clipboard into given directory
#
########################################################################
sub pasteFiles {
    my $dir = shift;

    my @src_files;
    my @copied_files;
    my @notcopied_files;

    # Get clipboard files from session
    for my $clipboard_file (sort @session_data) {
	push @src_files, $' if ($clipboard_file =~ /^$proc_types{'copyfile'}:/);  #'
    }

    if (!@src_files) {
	push @messages, "Clipboard is empty. Cannot paste files.";
	return;
    }

    foreach (@src_files) {
	my $fname = basename($_);

	# is there already a file with the same name in this directory?
	if (-e "$dir$fname") {
	    push @notcopied_files, "$_ (file with <b>same name</b> already present)";
	} else {
	    # attempt to copy
	    `$command{cp} $_ "$dir$fname"`;
	    if ($?) {
		push @notcopied_files, "$_ (copy operation failed; error code:<b>$?</b>)";
		&printToLog("Copy failed: $?\n");
	    } else {
		push @copied_files, "$_";
	    }
	}
    }

    if (@copied_files) {
	push @messages, "These files have been <b>copied</b> into the current directory ($dir):";
	push @messages, @copied_files;

	&updateSession(action => 'workDir',
		       dir    => $dir);

    }

    if (@notcopied_files) {
	push @messages, "These files could <b>NOT</b> be copied into the current directory ($dir):";
	push @messages, @notcopied_files;
    }

}

########################################################################
# downloadFile
#
# Download a single file
#
########################################################################
sub downloadFile {
    my ( $dir, $file ) = @_;
    my $path = "$dir$file";

    # Open file
    if ( !open( FILE, "<$path" ) ) {
        push @messages, "Could not open file $file for download: $!";
        return;
    }

    # Output a HTTP Header that should raise a file download dialog box
    print header( -type => "application/octet-stream; name=\"$file\"",
                  -Content_length => (stat($path))[7],
                  -Content_disposition => "attachment; filename=\"$file\"" );

    # Dump contents
    select STDOUT; $| = 1;	# don't buffer output
    my $block;
    my $bsize = (stat($path))[12] || 512;
    while ( read( FILE, $block, $bsize ) ) { print $block }
    close FILE;

    exit 0;		# don't return from here
}

########################################################################
# downloadFiles
#
# Download files from session 
#
########################################################################
sub downloadFilesArchive {
    my %args = @_;
    my $dir = $args{dir};
    my $file_list = $args{files};

    chdir( $dir );
    
    # Output a HTTP Header that should raise a file download dialog box
    print header( -type => "application/zip; name=\"tpp_download.zip\"",
                  -Content_disposition => "attachment; filename=\"tpp_download.zip\"",
	          -Transfer_Encoding => 'chunked' );

    my $chunk = sub {
        my $length = length($_);
        $_ = sprintf("%x", $length) . "\r\n" . $_ . "\r\n";
        $_ .= "\r\n" unless $length;
        1;
    };

    select STDOUT; $| = 1;	# Don't buffer output

    # Author changed name of option and didn't make it backwards compatible
    my $filter = ( $IO::Compress::Zip::VERSION >= 2.040 ) ? 'FilterContainer' : 'FilterEnvelope';
    unless ( zip $file_list => '-', $filter => $chunk) {
        push @messages, "Could not create archive for download: $!";
        return;
    };
    exit 0;		# don't return from here
}

########################################################################
# uploadFile
#
# Upload a file into the working directory
#
########################################################################
sub uploadFile {
    my $dir = shift;

    my $max_file_size       = readconfig( 'max_file_size', 2147483647 );
    my $max_filename_length = readconfig( 'max_filename_length', 160 );
    my $safe_filename_chars = "a-zA-Z0-9_.-";         # safe filename characters 

    $CGI::POST_MAX = $max_file_size;     # set max file size CGI will read

    # File to upload?
    my $file = param('file');
    if ( !$file )  {
        my $err = cgi_error() || '';
        ( $err && $err =~ /POST too large/ )
            ? push @messages, 'File exceeded maximum allowed size.',
            : push @messages, "There was a problem uploading a file: $err";
	return;
    }

    # Valid file name?
    my $filename = param('file');
    my ( $name, $path, $extension ) = fileparse ( $filename, '\..*' );  
    $filename = $name . $extension;  
    $filename =~ tr/ /_/;  
    $filename =~ s/[^$safe_filename_chars]//g;    # ...remove unsafe chars
    if ( length($filename) == 0 || length($filename) > $max_filename_length ) {
        push @messages, "Invalid file name, can't upload file.";
        return;
    }

    # Don't over-write an existing file
    if ( -e "$dir/$filename" ) {
        push @messages, 'A file with this name already exists';
        return;
    }

    # Copy contents to file
    if ( !open ( FILE, ">$dir/$filename" ) ) {
        push @messages, "File open error: $!";
        return;
    }
    binmode FILE;  
    while ( <$file> ) {  
        print FILE;
    }
    close FILE;  
    push @messages, "File $filename uploaded successfully.";
}

########################################################################
# deleteFiles
#
# Delete selected files from file system. Also remove them from session.
#
########################################################################
sub deleteFiles {
    my %args = @_;
    my $dir   = $args{dir};
    my @files = @{$args{files}};

    my @deleted_files;
    my @notdeleted_files;

    foreach my $file (@files) {
	if (unlink "$dir$file") {
	    push  @deleted_files, $file;
	} else {
	    push  @notdeleted_files, $file;
	}
    }

    if (@deleted_files) {
	push @messages, "These files have been <b>deleted</b> from the file system:";
	push @messages, @deleted_files;

	&updateSession(action => 'delete',
		       dir    => $dir,
		       files  => \@deleted_files);

    }

    if (@notdeleted_files) {
	push @messages, "These files could <b>NOT</b> be deleted from the file system:";
	push @messages, @notdeleted_files;
    }
    return;

}


########################################################################
# removeFiles
#
# Remove selected files from processing
#
########################################################################
sub removeFiles {
    my @file_list = @_;

    # minor bug if user refreshes page: msg is shown, but no files are removed.   FIXME maybe...
    push @messages, "These files have been removed from processing:";

    &updateSession(action => 'remove',
		   files  => \@file_list);

}


########################################################################
# saveFile
#
# Save updated contents of file to disk
#
########################################################################
sub saveFile {
    my %args = @_;
    my $path     = $args{file_path};
    my $contents = $args{file_contents};

    open(FILE, ">$path") || &fatalError("SAVEFILE_CANNOT_OPEN_FILE:$path:$!");
    print FILE $contents;
    close FILE;

    push @messages, "File <b>$path</b> has been updated on disk";

}



########################################################################
# addFiles
#
# Add selected files for processing
#
########################################################################
sub addFiles {
    my %args = @_;
    my $ref_page   = $args{ref_page};
    my $files_dir  = $args{dir};
    my $update_dir = $args{updatedir};
    my @file_list  = @{$args{files}};

    my @s_file_list;
    my %val_seen = ();

    # Prepend directory to file name
    @file_list = map "$proc_types{$ref_page}:${files_dir}$_", @file_list;


    push @session_data, @file_list;

    open(SESSION, ">$session_file") || &fatalError("ADD_CANNOT_WRITE_SESSION_FILE:$session_file:$!");
    for (sort @session_data) {
	chomp;
	if (/^$proc_types{'lastdir'}/ && $files_dir && $update_dir) {
	    $_ = "$proc_types{'lastdir'}:${files_dir}";
	}
	if ($_ && !$val_seen{$_}++ ) {
	    print SESSION "$_\n";
	    push @s_file_list, $_;
	}
    }
    close(SESSION);

    @session_data = @s_file_list;

}


########################################################################
# prepareCommands
#
# Prepare commands to run; execute some checks
# Returns page to go to
#
########################################################################
sub prepareCommands {
    my $call_page = shift;
    my @commandsandfiles = @_;

    if ($debug) {
	&printToLog("in prepareCommands ");
	&printToLog(@commandsandfiles);
	&printToLog("\n");
    }

    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    my $cmd_date = sprintf("%4d%02d%02d-%02d%02d%02d",$year+1900,$mon+1,$mday,$hour,$min,$sec);

    my $cmd_file = "${users_dir}$auth_user/cmd_${user_session}_$cmd_date";
    push @all_commands, $cmd_file;  # store command file location in zeroth element
    push @all_commands, $call_page; # store call page in first element

    open(COUT, ">$cmd_file") || &fatalError('NOCMDFILE',"Cannot open $cmd_file for writing: $!\n");

    print COUT "# Commands for session $user_session on ".scalar(localtime)."\n";
    while (my ($com, $outf) = splice(@commandsandfiles, 0, 2)) {
	push @all_commands, $com;
	print COUT "# COMMAND:$com\n";
	foreach my $output_file (split /:::/, $outf) {
	    print COUT "# OUTFILE:$output_file\n";
	}
    }
    close (COUT);

    return 'jobs';
}


########################################################################
# runCommands
#
# Run Commands contained in global array @all_commands  (hmmm...)
#
########################################################################
sub runCommands {
    my $parent_pid = shift;

    my $cmd_file  = shift(@all_commands);
    my $call_page = shift(@all_commands);

    my $host = 'localhost';

    param('runon_cluster') || param('runon_cluster','');
    if (param('runon_cluster') eq 'on_Amazon_cloud') {
	my $aws_file = "${users_dir}$auth_user/.awssecret";
	$ENV{AWS_CREDENTIAL_FILE} = $aws_file;
	$host = 'Amazon cloud';
    } elsif ( $useSGE ) {
	$host = 'SGE';
    }

    # update .jobs file
    use Fcntl qw(:DEFAULT :flock);
    my $jobs_file = "${users_dir}$auth_user/.jobs";

    sysopen(JOBS, $jobs_file, O_WRONLY | O_APPEND) || &fatalError("CANNOT_APPEND_JOBS_FILE:$jobs_file:$!");
    flock(JOBS, LOCK_EX) || &fatalError("CANNOT_LOCK_EX_JOBS_FILE:$jobs_file:$!");
    print JOBS basename($cmd_file) . "\t$call_page\t$host\trunning\t$parent_pid\n";
    close(JOBS);


    open(COUT, ">>$cmd_file") || &fatalError('NOCMDFILE',"Cannot open $cmd_file for appending: $!\n");
    print COUT "# PARENT PID::$parent_pid\n";
    print COUT "# BEGIN COMMAND BLOCK\n";

    &printToLog("[DAEMON] begin command block\n") if ($debug);

    if ($in_windows) { # refresh page immediately (100ms)
	$cmd_file =~ /cmd_/;
	my $link = "$tpp_url?Action=$web_actions{'showpage'}&page=jobs&show_job=$'";
	print "<script type=\"text/javascript\">setTimeout('window.location = \"$link\"',100);</script>";
    }

    foreach my $cmd (@all_commands) {
        if ( $useSGE ) {
            $cmd = shell_quote( $cmd );
            $cmd = "qrsh -now n -N 'TPP-$call_page' $cmd";
        }
        if ( $useSudo ) {
            $cmd = "sudo -E -u $auth_user $cmd";
        }

	print COUT "###### BEGIN Command Execution ######\n";
	print COUT "[".scalar(localtime)."] EXECUTING: $cmd \n";

	&printToLog("[DAEMON] begin command...\n") if ($debug);

	print COUT "OUTPUT:\n";
	my $status;
	if ($in_windows) {
	    close (COUT); # flush seems to help
	}
	chomp($status = `$cmd 1>>$cmd_file 2>&1`);
	if ($in_windows) {
	    open(COUT, ">>$cmd_file") || &fatalError('NOCMDFILE',"Cannot open $cmd_file for appending: $!\n");	
	}
	&printToLog("[DAEMON] system call returned...\n") if ($debug);

	print COUT "END OUTPUT\n";
	print COUT "RETURN CODE:$?\n";

	&printToLog("[DAEMON] end command with status:\n----------------------------------------\n$status\n----------------------------------------\n") if ($debug);

	print COUT "###### End Command Execution ######\n";
    }
    print COUT "# All finished at ".scalar(localtime)."\n";

    if ( (param('runon_cluster') eq 'on_Amazon_cloud') && ($? == 0) ){
	print COUT "# QUEUED TO AMAZON EC2\n";
    } else {
	print COUT "# END COMMAND BLOCK\n";
    }

    close (COUT);

    &printToLog("[DAEMON] Exiting...\n") if ($debug);

}


########################################################################
# userTimeoutRecovery
#
# allow user to rescue session from timed out commands
#
########################################################################
sub userTimeoutRecovery {
    my $jid = shift || 0;

    my $cmd_file = "${users_dir}$auth_user/cmd_$jid";

    open(COUT, ">>$cmd_file") || &fatalError('NOCMDFILE',"Cannot open $cmd_file for appending: $!\n");

    &printToLog("[DAEMON] user initiated command end due to timeout\n----------------------------------------\n") if ($debug);

    print COUT "###### User claims command timeout ######\n";
    print COUT "# All finished at ".scalar(localtime)."\n";
    print COUT "# END COMMAND BLOCK\n";
    close (COUT);

    push @messages, "The command in question is now deemed as 'Finished'.";
    push @messages, "WARNING: If you attempted to run multiple commands, some may not have run. Please check if the expected output file(s) exist, or re-run.";
}


########################################################################
# killCommand
#
# Send system call to stop a running process
#
########################################################################
sub killCommand {
    my $jid = shift || 0;
    my $jpid = shift || 0;

    if (&getCommandStatus($jid) ne 'running') {
	push @messages, "Command $jid is not running.  Did NOT kill command.";
	return;
    }


    my $kstat = ($in_windows) ? `taskkill /PID $jpid /T /F` : `pkill -9 -s $jpid`;
    if ($?) {
	push @messages, "Unable to kill command $jpid! ($kstat)";

    } else {
	push @messages, "The command in question has been terminated.";
	push @messages, "Attempting to adjust status....";
	sleep (1);

	my $cmd_file = "${users_dir}$auth_user/cmd_$jid";

	open(COUT, ">>$cmd_file") || return;

	&printToLog("[DAEMON] user manually terminated command\n----------------------------------------\n") if ($debug);

	print COUT "###### User terminated command  ######\n";
	print COUT "# All finished at ".scalar(localtime)."\n";
	print COUT "# COMMAND TERMINATED\n";
	close (COUT);

	push @messages, "...success!";
    }

    return;
}


########################################################################
# deleteCommandFile
#
# Delete old (non-running) command log file
#
########################################################################
sub deleteCommandFile {
    my $jid = shift || 0;

    if (&getCommandStatus($jid) eq 'running') {
	push @messages, "Command $jid is still running.  Did NOT delete command log file.";
	return;
    }

    if (unlink "${users_dir}$auth_user/cmd_$jid") {
	push @messages, "Deleted command log file";
    } else {
	push @messages, "Could NOT delete command log file! ($jid)";
	return;
    }

    my $jobs_file = "${users_dir}$auth_user/.jobs";

    use Fcntl qw(:DEFAULT :flock);
    open(JOBS, "$jobs_file") || &fatalError("CANNOT_OPEN_JOBS_FILE:$jobs_file:$!");
    flock(JOBS, LOCK_SH) || &fatalError("CANNOT_LOCK_SH_JOBS_FILE:$jobs_file:$!");
    my @jobs_data = <JOBS>;
    close(JOBS);

    sysopen(JOBS, $jobs_file, O_WRONLY) || &fatalError("CANNOT_UPDATE_JOBS_FILE:$jobs_file:$!");
    flock(JOBS, LOCK_EX) || &fatalError("CANNOT_LOCK_EX_JOBS_FILE:$jobs_file:$!");
    truncate(JOBS, 0) || &fatalError("CANNOT_TRUNCATE_JOBS_FILE:$jobs_file:$!");
    foreach (@jobs_data) {
	print JOBS "$_" unless (/^cmd_$jid/);
    }
    close(JOBS);

}


########################################################################
# deleteSessionFiles
#
# Delete old (not current!) session and command files.
#
########################################################################
sub deleteSessionFiles{
    my $sid_file = shift;
    $sid_file =~ /session_(.+)/;

    my $del_sid = $1;

    # is this the current session?
    if ($del_sid eq $user_session) {
	push @messages, "Cannot delete current session!";
	return;
    }

    my $cmdstatus;
    my @cmd_files = &listFiles("${users_dir}$auth_user/","cmd_$del_sid",'false');

    if (@cmd_files) {
	foreach (@cmd_files) {
	    s/cmd_//;
	    $cmdstatus .= &getCommandStatus($_).',';
	}
    } else {
	$cmdstatus = 'none';
    }

    if ($cmdstatus =~ 'running') {
	push @messages, "Session is still running a command.  Did NOT delete.";
	return;
    } elsif ($cmdstatus ne 'none') {
	# delete command files
	foreach (@cmd_files) {
	    if (unlink "${users_dir}$auth_user/cmd_$_") {
		push @messages, "Deleted session command file";
	    } else {
		push @messages, "Could NOT delete session command file! ($_)";
	    }
	}

	my $jobs_file = "${users_dir}$auth_user/.jobs";

	use Fcntl qw(:DEFAULT :flock);
	open(JOBS, "$jobs_file") || &fatalError("CANNOT_OPEN_JOBS_FILE:$jobs_file:$!");
	flock(JOBS, LOCK_SH) || &fatalError("CANNOT_LOCK_SH_JOBS_FILE:$jobs_file:$!");
	my @jobs_data = <JOBS>;
	close(JOBS);

	sysopen(JOBS, $jobs_file, O_WRONLY) || &fatalError("CANNOT_UPDATE_JOBS_FILE:$jobs_file:$!");
	flock(JOBS, LOCK_EX) || &fatalError("CANNOT_LOCK_EX_JOBS_FILE:$jobs_file:$!");
	truncate(JOBS, 0) || &fatalError("CANNOT_TRUNCATE_JOBS_FILE:$jobs_file:$!");
	foreach (@jobs_data) {
	    print JOBS "$_" unless (/^cmd_$del_sid/);
	}
	close(JOBS);

    }

    # now delete the session file
    if (unlink "${users_dir}$auth_user/$sid_file") {
	push @messages, "Deleted session file";
    } else {
	push @messages, "Could NOT delete session file!";
    }

}


########################################################################
# setNewPassword
#
# Re-set Petunia user password
#
########################################################################
sub setNewPassword{

    if (param('new_password') !~ /.....+/ ){
	push @messages, "New password is too short. Please pick a password that is 5 characters or longer.";
	Delete('new_password');
	Delete('ver_password');
	return;
    }

    if (param('new_password') ne param('ver_password')){
	push @messages, "New passwords do not match. Please try again.";
	Delete('new_password');
	Delete('ver_password');
	return;
    }

    if (param('new_password') eq param('cur_password')){
	push @messages, "New password matches current. Password not changed.";
	Delete('cur_password');
	Delete('new_password');
	Delete('ver_password');
	return;
    }

    return if (&verifyPassword($auth_user,param('cur_password')) ne 'OK');


    # now update the password file
    &printToLog("updating user password...\n") if ($debug);

    my $pfile = "${users_dir}$auth_user/.password";
    open(PWD, ">$pfile") || &fatalError('PWD_FILE',"Error opening password file!");
    print PWD crypt(param('new_password'), $crypt_key);
    close(PWD);

    push @messages, "Your password has been updated.";
    Delete('cur_password');
    Delete('new_password');
    Delete('ver_password');

    return;
}


########################################################################
# getEC2Info
#
# Returns Amazon Web Services account id (salted) and machine id if we
# happen to be running on a EC2 instance
#
########################################################################
sub getEC2Info {
   return() unless ( $TPPVersionInfo =~ /\(AMI\) rev/ );

   my $url = 'http://169.254.169.254/latest/dynamic/instance-identity/document';
   my $res = `$command{wget} -qO - $url 2>/dev/null`;
   return() if ( $? );

   my $accountId = undef;
   my $imageId   = undef;
   if ( $res =~ /"accountId"\s+:\s+"(\d+)"/ ) {
      $accountId = crypt($1, $crypt_key);
   }
   if ( $res =~ /"imageId"\s+:\s+"(ami-\w+)"/ ) {
      $imageId = $1;
   }

   return( $accountId, $imageId );
}


########################################################################
# verifyEC2Keys
#
# Test that amztpp works with user-supplied keys, and save them in a file, if so
#
########################################################################
sub verifyEC2Keys {
    my $retpage = 'clusters';

    &validateInput('not_blank',param('amz_access'),'Amazon Access Key');
    &validateInput('not_blank',param('amz_secret'),'Amazon Secret Key');
    return $retpage if ($errors);

    my $amz_access = param('amz_access');
    my $amz_secret = param('amz_secret');
    my $amz_region = '';                       # use a blank EC2 region for now
    my $amz_s3url  = param('amz_s3url');
    my $amz_bucket = '';
    if ( $amz_s3url ) {
        if ( $amz_s3url !~ /^([a-z0-9][a-z0-9-.]+[a-z0-9]).*$/ ) {
	    push @messages, "Please enter a valid value for Amazon bucket name";
	    $errors++;
            return $retpage;
        } else {
            $amz_bucket = $1;
	}
    }

    # attempt to get status. cmd line gets parsed by /bin/sh or cmd.exe!
    $ENV{EC2_ACCESS_KEY} = $amz_access;
    $ENV{EC2_SECRET_KEY} = $amz_secret;
    my @response = `$command{amztpp} -l $base_dir/amztpp.log status 2>&1`;
    delete $ENV{EC2_ACCESS_KEY}; delete $ENV{EC2_SECRET_KEY};

    shift @response if $response[0] =~ /ParserDetails.ini/;  # sigh...

    if (!@response) {
	push @messages, "The amztpp executable does not appear to be installed or in working order.  Please verify and re-install if necessary.";
	&printToLog("AMZTPP_CK:$?") if ($?);
        return $retpage;

    } elsif ($response[0] =~ m|invalidaccesskeyid|i ||
# JS: commenting out the line below. signatures should always match
#	     $response[0] =~ m|signaturedoesnotmatch|i ||
	     $response[0] =~ m|authfailure|i ) {
	push @messages, "<span title='". escapeHTML(@response) ."'>Your credentials were not valid; please re-enter them.</span>";
	$errors++;
	&printToLog("AMZTPP_CK:$?") if ($?);
        return $retpage;
## check for other non-status responses!!  ToDo
    } elsif ( $? ) {
	&printToLog("AMZTPP_CK: Error code: $? @response");
        &fatalError("AMZTPP_CK: Error checking credentials<br>@response");
	return $retpage;
    }

    # Check s3cmd 
    my $s3cfg_file = "${users_dir}$auth_user/.s3cfg";
    if ( $amz_s3url && $s3cmd ) {
	# ...s3cmd requires you to always have a config file :(

	write_s3cfg( $s3cfg_file, $amz_access, $amz_secret, "s3://$amz_s3url" );
	@response = `$s3cmd -c $s3cfg_file info s3://$amz_bucket 2>&1`;


        if (!@response) {
            unlink $s3cfg_file;
	    $errors++;
	    push @messages, "The s3cmd executable does not appear to be installed or in working order.  Please verify and re-install if necessary.";
	    &printToLog("S3CMD_CK:$?") if ($?);
            return $retpage;
        } elsif ($response[0] =~ m|ERROR|i) {
            unlink $s3cfg_file;
	    &printToLog("S3CMD_CK:@response");
	    $errors++;
	    push @messages, "Your bucket was not valid; please re-enter them.";
            return $retpage;
        } elsif( $? ) {
            unlink $s3cfg_file;
	    &printToLog("S3CMD_CK: Error code: $? @response");
            &fatalError("S3CMD_CK: Error checking bucket<br>@response");
        }
    } elsif ( -f $s3cfg_file ) {
	unlink $s3cfg_file;
    }

    my $aws_file = "${users_dir}$auth_user/.awssecret";
    open(AWS, ">$aws_file",) || &fatalError("CANNOT_CREATE_AWS_FILE:$aws_file:$!");
    chmod 0600, $aws_file;
    print AWS "$amz_access\n";
    print AWS "$amz_secret\n";
    print AWS "$amz_region\n";
    print AWS "$amz_bucket\n" if ( $amz_bucket );
    close(AWS);
    return $retpage;
}

########################################################################
# removeEC2Keys
#
# Remove file with user-supplied keys
#
########################################################################
sub removeEC2Keys {
    my $retpage = 'clusters';

    unlink "${users_dir}$auth_user/.s3cfg";
    if (unlink "${users_dir}$auth_user/.awssecret") {
	push @messages, "De-registered Amazon EC2 account";
    } else {
	push @messages, "Could NOT de-register Amazon EC2 account!";
    }

    return $retpage;
}

########################################################################
# realcleanEC2
#
# Shut down instances, delete files and queues
#
########################################################################
sub realcleanEC2 {
    my $retpage = 'clusters';

    my $aws_file = "${users_dir}$auth_user/.awssecret";

    $ENV{AWS_CREDENTIAL_FILE} = $aws_file;
    my @response = `$command{amztpp} realclean 2>&1`;

    if (!@response) {
	push @messages, "The amztpp executable does not appear to be installed or in working order.  Please verify and re-install if necessary.";
	&printToLog("AMZTPP_CK:$?") if ($?);
    }

    push @messages, 'Executing amztpp realclean:';
    for (@response) {
	next if /ParserDetails.ini/;
	s/amztpp: /.../;
	push @messages, $_;
    }
    push @messages, '<b>Please wait one minute before submitting any requests to amztpp!</b>';

    return $retpage;
}

########################################################################
# Reads the S3 url out of the s3cfg file
########################################################################
sub read_s3cfg {
    my ( $file ) = @_;

    my ( $url, $key );
    open(S3CFG, "<$file") || &fatalError("CANNOT_READ_S3CFG_FILE:$file:$!");
    while ( <S3CFG> ) {
	$url = $1 if ( /\s*tpp_s3url\s*=\s*(s3:\/\/.+)/ );
        $key = $1 if ( /\s*access_key\s*=\s*(\w+)/ );
    }
    close(S3CFG);

    if ( !$url && $key ) {
        $url = lc("s3://tpp-$key/tppdata");	# default
    }
    $url =~ s/\/$//;
    return $url;
}

########################################################################
# Write out a s3cmd config file
########################################################################
sub write_s3cfg {
    my ( $file, $access, $secret, $s3url ) = @_;

    open(S3CFG, ">$file") || &fatalError("CANNOT_CREATE_S3CFG_FILE:$file:$!");
    # chmod 0600, $file;
    print S3CFG "# Petunia autogenerated\n";
    print S3CFG "[default]\n";
    print S3CFG "access_key = $access\n";
    print S3CFG "bucket_location = US\n";
    print S3CFG "secret_key = $secret\n";
    print S3CFG "use_https = True\n";
    print S3CFG "tpp_s3url = $s3url\n";
    close(S3CFG);
    return;
}

########################################################################
# Runs a job that will do a S3 sync
########################################################################
sub runS3Sync {
    my ( $dir, $src, $dst ) = @_;

    my $s3cfg_file = "${users_dir}$auth_user/.s3cfg";
    if ( ! -f $s3cfg_file ) {
        push @messages, "Can't sync to S3: missing s3cfg credentials!!";
	return 'filebrowser';
    }
    if ( !$src ) {
        push @messages, "Can't sync to S3: missing source paths";
	return 'filebrowser';
    } elsif ( !$dst ) {
        push @messages, "Can't sync to S3: missing destination paths";
	return 'filebrowser';
    }

    my $options = "-c $s3cfg_file --acl-private -r";
    $options .= ' --exclude .s3cfg';
    $options .= ' --exclude .awssecret';
    $options .= ' --dry-run' if param('dry_run');
    $options .= param('delete_removed') ? ' --delete-removed' 
                                        : ' --no-delete-removed';

    my $command = "$command{chdir} $dir ". $command_sep ." $s3cmd $options sync ";
    $command .= "$src $dst";

    # Execute command!
    return &prepareCommands('s3cmd',$command,$dir);
}


########################################################################
# setNewMascotURL
#
# Set Mascot URL for current session
#
########################################################################
sub setNewMascotURL{

    my $new_mascot_server = param('new_mascot_server');
    $new_mascot_server .= "/" unless ($new_mascot_server =~ m|/$|);

    my $mascot_server_home = $new_mascot_server.'home.html';

    if (param('new_mascot_server') !~ m|^http://| ){
	push @messages, "URL must start with http:// . Please re-enter";
	return;
    }

    # verify that the URL has a MASCOT home page with expected links
    my @response = `$command{wget} -q -O - $mascot_server_home`;
    # check for error! FIXME

    my $score;
    foreach (@response) {
	# the following may not always be the case for all versions...
	$score++ if (m|x-cgi/ms-status.exe| && /Database Status/);
	$score++ if (m|x-cgi/ms-review.exe| && /Search Log/);
	$score++ if (m|x-cgi/ms-config.exe| && /Configuration Editor/);  # contributed by C.Dantec
    }
    if ($score < 3) {
	push @messages, "The URL <pre>$new_mascot_server</pre> does not appear to point to a Mascot server. Please re-enter";
	return;
    }

    # re-use addFiles, with a small hack  -- need to extend updateSession sub  FIXME
    $mascot_server = $new_mascot_server;

    my @orig_messages = @messages;  #remember any previous messages
    for (sort @session_data) {
	if (/^$proc_types{'mascoturl'}/) {
	    &removeFiles($_);
	}
    }
    @messages = @orig_messages;

    &addFiles(ref_page => 'mascoturl',
	      files    => [$mascot_server]);

    # reset input form
    param('new_mascot_server','http://');

}


########################################################################
# runProteinProphet
#
# Assemble user options; run command
#
########################################################################
sub runProteinProphet{

    # Retrieve File List
    my @file_list;
    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runprophet'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' xml file(s) to process.";
	return 'runprophet';
    }

    # Perform Basic Validation
    &validateInput('no_spaces',param('prt_outdir'),'Output directory');
    &validateInput('no_spaces',param('prt_outfile'),'Output filename');
    &validateInput('extraopts',param('prt_extraopts'),'Extra Command-line Options') if param('prt_extraopts');
    return 'runprophet' if ($errors);

    # Build command
    #
    # ProteinProphet <interact_pepxml_file1> [<interact_pepxml_file2>[....]] <output_protxml_file> (ICAT) (GLYC) (XPRESS) (ASAP_PROPHET) (ACCURACY) (ASAP) (PROTLEN) (NORMPROTLEN) (GROUPWTS) (INSTANCES) (REFRESH) (DELUDE) (NOOCCAM) (NOPLOT) (PROTMW) (IPROPHET)
    # (perhaps others, but usage statement might not be complete)

    my $command;
    my $options = '';

    $options .= ' IPROPHET' if param('prt_ipro');
    $options .= ' ICAT' if param('prt_icat');
    $options .= ' GLYC' if param('prt_nglyc');
    $options .= ' XPRESS' if param('prt_xpress');
    $options .= ' ASAP_PROPHET' if param('prt_asapget');
    $options .= ' LIBRA' if param('prt_libra');
    $options .= ' EXCLUDE_ZEROS' if param('prt_nozero');
    $options .= ' PROTLEN' if param('prt_protlen');
    $options .= ' PROTMW' if param('prt_molwt');
    # advanced
    $options .= ' DELUDE' if param('prt_delude');
    $options .= ' NOOCCAM' if param('prt_noccam');
    $options .= ' NOGROUPS' if param('prt_nogroups');
    $options .= ' NORMPROTLEN' if param('prt_nspnorm');
    $options .= ' INSTANCES' if param('prt_instances');
    $options .= ' GROUPWTS' if param('prt_grpwts');

    $options .= ' '.param('prt_extraopts') if (param('prt_extraopts'));

    my $outdir = param('prt_outdir');

    my $out_file = "$outdir/".param('prt_outfile');
    $command = "$command{runprophet} @file_list $out_file $options";

    # this is the link displayed to the user
    $out_file =~ s/\.xml(\.gz)?/\.xml/i;

    my @commands;
    push @commands, ($command, $out_file);

    # call protxml2html?
    if (param('prt_dohtml')) {
	my $in_file = $out_file;
	$out_file =~ s/xml$/html/i;

	$command = "$command{protxml2html} -file $in_file HTML";
	push @commands, ($command, $out_file);
    }

    # Execute command!
    return &prepareCommands('runprophet',@commands);

}


########################################################################
# runPTMProphet
#
# Assemble options; run command
#
########################################################################
sub runPTMProphet {

    # Retrieve File List
    my @file_list;
    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runprophet'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' xml file(s) to process.";
	return 'ptmprophet';
    }

    # Perform Basic Validation
    &validateInput('no_spaces',param('ptmproph_outdir'),'File path (directory)');
    &validateInput('no_spaces',param('ptmproph_outfile'),'Output filename');
    my $allres = '';
    for (1..5) {
	$allres .= param("ptmproph_res$_");
	# validate residues string?
	&validateInput('float',param("ptmproph_res${_}md"),"Mass shift for residue group $_") if param("ptmproph_res$_");
    }
    &validateInput('not_blank',$allres,'Modified Residues or Termini');
    &validateInput('float',param('ptmproph_mztol'),'m/z tolerance');
    &validateInput('extraopts',param('ptmproph_extraopts'),'Extra Command-line Options') if param('ptmproph_extraopts');
    return 'ptmprophet' if ($errors);

    # Build command
    #
    # PTMProphetParser [NOUPDATE] [NOEM] [MZTOL=<number>] <amino acids, n, or c>,<mass_shift> <interact_pepxml_file>
    my $command;
    my $options;

    for (1..5) {
	$options .= param("ptmproph_res$_").','.param("ptmproph_res${_}md").',' if param("ptmproph_res$_");
    }
    chop($options); #remove last comma

    $options .= ' MZTOL='.param('ptmproph_mztol') if param('ptmproph_mztol');
    $options .= ' NOUPDATE' if param('ptmproph_noupdate');

    $options .= ' '.param('ptmproph_extraopts') if (param('ptmproph_extraopts'));

    my $outdir = param('ptmproph_outdir');

    my $out_file = param('ptmproph_outfile');
    $command = "$command{chdir} $outdir". $command_sep ." $command{ptmprophet} $options $file_list[0] $out_file";

    # this is the link displayed to the user
    $out_file = "$outdir/$out_file";

    my @commands;
    push @commands, ($command, $out_file);

    # Execute command!
    return &prepareCommands('ptmprophet',@commands);
}


########################################################################
# runInterProphet
#
# Assemble options; run command
#
########################################################################
sub runInterProphet{

    # Retrieve File List
    my @file_list;
    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runprophet'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' xml file(s) to process.";
	return 'iprophet';
    }

    &validateInput('extraopts',param('iproph_extraopts'),'Extra Command-line Options') if param('iproph_extraopts');
    &validateInput('no_spaces',param('iproph_outdir'),'File path (directory)');
    &validateInput('no_spaces',param('iproph_outfile'),'Output filename');

    # Build command
    #
    # InterProphetParser [NONSS] [NONSE] [NONRS] [NONSM] [NONSI] [NONSP] [NOFPKM] <interact_pepxml_file1> [<interact_pepxml_file2>[....]] <output_pepxml_file>
    my $command;
    my $options = '';

    $options .= ' NONSS' if param('iproph_nonss');
    $options .= ' NONSE' if param('iproph_nonse');
    $options .= ' NONSM' if param('iproph_nonsm');
    $options .= ' NONSI' if param('iproph_nonsi');
    $options .= ' NONRS' if param('iproph_nonrs');
    $options .= ' NONSP' if param('iproph_nonsp');
    $options .= ' NOFPKM'if param('iproph_nofpkm');

    $options .= ' '.param('iproph_extraopts') if param('iproph_extraopts');

    my $outdir = param('iproph_outdir');

    my $out_file = param('iproph_outfile');
    $command = "$command{chdir} $outdir". $command_sep ." $command{interprophet} $options @file_list $out_file";

    param('xp_res1', '') if (param('xp_res1') eq '--');
    param('xp_res2', '') if (param('xp_res2') eq '--');
    param('xp_res3', '') if (param('xp_res3') eq '--');
    param('as_res1', '') if (param('as_res1') eq '--');
    param('as_res2', '') if (param('as_res2') eq '--');
    param('as_res3', '') if (param('as_res3') eq '--');
    param('as_res4', '') if (param('as_res4') eq '--');
    param('as_res5', '') if (param('as_res5') eq '--');
    param('as_labres1', '') if (param('as_labres1') eq '--');
    param('as_labres2', '') if (param('as_labres2') eq '--');
    param('as_labres3', '') if (param('as_labres3') eq '--');
    param('as_labres4', '') if (param('as_labres4') eq '--');
    param('as_labres5', '') if (param('as_labres5') eq '--');

    $options = '';
    if (param('xp_run')) {
	&validateInput('float',param('xp_mass'),'XPRESS mass tolerance');
	&validateInput('float',param('xp_res1md'),'Labeled Residue 1 Mass Difference in XPRESS') if param('xp_res1');
	&validateInput('float',param('xp_res2md'),'Labeled Residue 2 Mass Difference in XPRESS') if param('xp_res2');
	&validateInput('float',param('xp_res3md'),'Labeled Residue 3 Mass Difference in XPRESS') if param('xp_res3');
	&validateInput('integer',param('xp_fixscanrange'),'Scan Range around Peak Apex') if (param('xp_fixscan'));
	&validateInput('integer',param('xp_minquantscans'),'Number of Chromatogram Points Needed for Quantitation') if (param('xp_minquantscans'));
	&validateInput('integer',param('xp_numisopeaks'),'Number of Isotopic Peaks to Sum') if (param('xp_numisopeaks'));
    }
    if (param('as_run')) {
	&validateInput('not_blank',param('as_labres1'),'(first) Labeled Residue in ASAPRatio');
	&validateInput('float',param('as_res1mass'),'Residue 1 Mass in ASAPRatio') if param('as_res1');
	&validateInput('float',param('as_res2mass'),'Residue 2 Mass in ASAPRatio') if param('as_res2');
	&validateInput('float',param('as_res3mass'),'Residue 3 Mass in ASAPRatio') if param('as_res3');
	&validateInput('float',param('as_res4mass'),'Residue 4 Mass in ASAPRatio') if param('as_res4');
	&validateInput('float',param('as_res5mass'),'Residue 5 Mass in ASAPRatio') if param('as_res5');
	&validateInput('float',param('as_area'),'Area Flag for ASAPRatio display') if param('as_area');
	&validateInput('float',param('as_mzpeak'),'m/z range to include in summation of peak') if param('as_mzpeak');
	&validateInput('float',param('as_miniprob'),'Min. iProphet probability to attempt quantitation') if param('as_miniprob');
    }
    if (param('lb_run')) {
	&validateInput('not_blank',param('lb_condition'),'Libra Condition File');
	&validateInput('no_spaces',param('lb_condition'),'Libra Condition File');
	&validateInput('file_exists',$outdir."/".param('lb_condition'),'Libra Condition File');
    }
    return 'iprophet' if ($errors);

    # Build command: XPRESS
    if (param('xp_run')) {
	$options .= $command_sep.' '.$command{xpresspep}.' '.$out_file;

	$options .= ' -m'.param('xp_mass');
	$options .= ' -a' if param('xp_massunit') eq 'PPM';
	$options .= ' -b' if param('xp_heavy');

	# only 3 residue mass modifications allowed. Need more?
	for (1..3) {
	    $options .= ' -n'.param('xp_res'.$_).','.param('xp_res'.$_.'md') if param('xp_res'.$_);
	}

	$options .= ' -L' if (param('xp_fix') eq 'light');
	$options .= ' -H' if (param('xp_fix') eq 'heavy');
	$options .= ' -F'.param('xp_fixscanrange') if param('xp_fixscan');
	$options .= ' -c'.param('xp_minquantscans') if param('xp_minquantscans');
	$options .= ' -p'.param('xp_numisopeaks') if param('xp_numisopeaks');

	if (param('xp_metalabel')) {
	    if (param('xp_metatype') eq 'N') {
		if (param('xp_metaquant') eq 'heavy') { $options .= ' -N'; }
		else                                  { $options .= ' -M'; }
	    } else {
		if (param('xp_metaquant') eq 'heavy') { $options .= ' -P'; }
		else                                  { $options .= ' -O'; }
	    }
	}
    }

    # Build command: ASAPRatio
    if (param('as_run')) {
	$options .= $command_sep.' '.$command{asappep}.' '.$out_file;

	$options .= ' -l'.param('as_labres1');
	$options .= param('as_labres2') if param('as_labres2');
	$options .= param('as_labres3') if param('as_labres3');
	$options .= param('as_labres4') if param('as_labres4');
	$options .= param('as_labres5') if param('as_labres5');

	$options .= ' -b' if param('as_heavy');
	$options .= ' -F' if param('as_fixedscan');
	$options .= ' -C' if param('as_cidonly');
	$options .= ' -f'.param('as_area') if param('as_area');
	$options .= ' -Z' if param('as_zerobg');
	$options .= ' -B' if param('as_highbgok');
	$options .= ' -r'.param('as_mzpeak') if param('as_mzpeak');
	$options .= ' -i'.param('as_miniprob') if param('as_miniprob');

	$options .= ' -S' if param('as_static');
	# only 5 residue mass modifications allowed. Need more?
	if (param('as_res1mass')) {
	    $options .= ' -m'.param('as_res1').param('as_res1mass');
	    $options .= param('as_res2').param('as_res2mass') if param('as_res2');
	    $options .= param('as_res3').param('as_res3mass') if param('as_res3');
	    $options .= param('as_res4').param('as_res4mass') if param('as_res4');
	    $options .= param('as_res5').param('as_res5mass') if param('as_res5');
	}
    }

    # Build command: Libra
    if (param('lb_run')) {
	$options .= $command_sep.' '.$command{librapep}.' '.$out_file.' ';
	$options .= param('lb_condition');
    }
	
    $command .= $options;  # tsk tsk...

    # this is the link displayed to the user
    $out_file = "$outdir/$out_file";

    my @commands;
    push @commands, ($command, $out_file);

    # Build command: ProteinProphet
    if (param('pr_run')) {
	$options  = 'IPROPHET';
	$options .= ' XPRESS' if param('xp_run');
	$options .= ' ASAP_PROPHET' if param('as_run');
	$options .= ' LIBRA' if param('lb_run');

	my $in_file = $out_file;
	$out_file =~ s/pep.xml$/prot.xml/i;

	$command = "$command{runprophet} $in_file $out_file $options";
	push @commands, ($command, $out_file);
    }

    # Execute command!
    return &prepareCommands('iprophet',@commands);

}

########################################################################
# runXInteract
#
# Assemble user options; run command
#
########################################################################
sub runXInteract{

    # Retrieve File List
    my @file_list;
    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'xinteract'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to process!";
	push @messages, "Please 'Select' xml file(s) to process.";
	return 'xinteract';
    }

    my $outd = param('xinter_outd');

    # Re-set some parameters
    param('ipro_prot', '') unless (param('ipro_run'));
    param('xp_res1', '') if (param('xp_res1') eq '--');
    param('xp_res2', '') if (param('xp_res2') eq '--');
    param('xp_res3', '') if (param('xp_res3') eq '--');
    param('as_res1', '') if (param('as_res1') eq '--');
    param('as_res2', '') if (param('as_res2') eq '--');
    param('as_res3', '') if (param('as_res3') eq '--');
    param('as_res4', '') if (param('as_res4') eq '--');
    param('as_res5', '') if (param('as_res5') eq '--');
    param('as_labres1', '') if (param('as_labres1') eq '--');
    param('as_labres2', '') if (param('as_labres2') eq '--');
    param('as_labres3', '') if (param('as_labres3') eq '--');
    param('as_labres4', '') if (param('as_labres4') eq '--');
    param('as_labres5', '') if (param('as_labres5') eq '--');

    # Perform Basic Validation
    &validateInput('no_spaces',param('xinter_outd'),'File path (directory)');
    &validateInput('no_spaces',param('xinter_outf'),'Output filename');
    &validateInput('float',param('xinter_pppfilter'),'PeptideProphet Probability Filter Cut-off');
    &validateInput('integer',param('xinter_pppeplen'),'Minimum Peptide Length');
    &validateInput('extraopts',param('pep_extraopts'),'Extra Command-line Options') if param('pep_extraopts');

    if (param('pep_usedecoy')) {
	&validateInput('not_blank',param('pep_decoystr'),'Decoy protein string identifier');
	&validateInput('no_spaces',param('pep_decoystr'),'Decoy protein string identifier');
    }
    if (param('xp_run')) {
	&validateInput('float',param('xp_mass'),'XPRESS mass tolerance');
	&validateInput('float',param('xp_res1md'),'Labeled Residue 1 Mass Difference in XPRESS') if param('xp_res1');
	&validateInput('float',param('xp_res2md'),'Labeled Residue 2 Mass Difference in XPRESS') if param('xp_res2');
	&validateInput('float',param('xp_res3md'),'Labeled Residue 3 Mass Difference in XPRESS') if param('xp_res3');
	&validateInput('integer',param('xp_fixscanrange'),'Scan Range around Peak Apex') if (param('xp_fixscan'));
	&validateInput('integer',param('xp_minquantscans'),'Number of Chromatogram Points Needed for Quantitation') if (param('xp_minquantscans'));
	&validateInput('integer',param('xp_numisopeaks'),'Number of Isotopic Peaks to Sum') if (param('xp_numisopeaks'));
    }
    if (param('as_run')) {
	&validateInput('not_blank',param('as_labres1'),'(first) Labeled Residue in ASAPRatio');
	&validateInput('float',param('as_res1mass'),'Residue 1 Mass in ASAPRatio') if param('as_res1');
	&validateInput('float',param('as_res2mass'),'Residue 2 Mass in ASAPRatio') if param('as_res2');
	&validateInput('float',param('as_res3mass'),'Residue 3 Mass in ASAPRatio') if param('as_res3');
	&validateInput('float',param('as_res4mass'),'Residue 4 Mass in ASAPRatio') if param('as_res4');
	&validateInput('float',param('as_res5mass'),'Residue 5 Mass in ASAPRatio') if param('as_res5');
	&validateInput('float',param('as_area'),'Area Flag for ASAPRatio display') if param('as_area');
	&validateInput('float',param('as_mzpeak'),'m/z range to include in summation of peak') if param('as_mzpeak');
    }
    if (param('ptmproph_run')) {
	&validateInput('not_blank',param('ptmproph_res1'),'(first) Modified Residue in PTMProphet');
	&validateInput('float',param('ptmproph_res1md'),'Mass shift of Residue 1 in PTMProphet');
	&validateInput('float',param('ptmproph_res2md'),'Mass shift of Residue 2 in PTMProphet') if param('ptmproph_res2');
	&validateInput('float',param('ptmproph_res3md'),'Mass shift of Residue 3 in PTMProphet') if param('ptmproph_res3');
	&validateInput('float',param('ptmproph_res4md'),'Mass shift of Residue 4 in PTMProphet') if param('ptmproph_res4');
	&validateInput('float',param('ptmproph_res5md'),'Mass shift of Residue 5 in PTMProphet') if param('ptmproph_res5');
 	&validateInput('float',param('ptmproph_mztol'),'PTMProphet m/z tolerance');
    }
    if (param('lb_run')) {
	&validateInput('not_blank',param('lb_condition'),'Libra Condition File');
	&validateInput('no_spaces',param('lb_condition'),'Libra Condition File');
	&validateInput('file_exists',$outd."/".param('lb_condition'),'Libra Condition File');
    }
    return 'xinteract' if ($errors);


    # Build command
    #
    # xinteract (generaloptions) (-Oprophetoptions) (-Mptmprophetoptions) (-Xxpressoptions) \
    #		(-Aasapoptions) (-L<conditionfile>libraoptions) xmlfile1 xmlfile2 ...
    my $command;
    my $options = '';


    # deal with general options
    $options .= ' -p'.param('xinter_pppfilter');
    $options .= ' -l'.param('xinter_pppeplen');
    $options .= ' '.param('pep_extraopts') if (param('pep_extraopts'));
    $options .= ' -PPM' if param('pep_accmassunit') eq 'PPM';

    if (param('pep_run')) {
	$options .= ' -O';

	$options .= 'A' if param('pep_accmass');
	$options .= 'i' if param('pep_icat');
	$options .= 'f' if param('pep_noicat');
	$options .= 'g' if param('pep_nglyc');
	$options .= 'm' if param('pep_maldi');
	$options .= 'I' if param('pep_pI');
	$options .= 'R' if param('pep_hydro');
	$options .= 'H' if param('pep_phospho');
	$options .= 'x' if param('pep_xclaster');
	$options .= 'l' if param('pep_nclaster');
	$options .= 'N' if param('pep_nontt');
	$options .= 'M' if param('pep_nonmc');
	$options .= 'F' if param('pep_force');
	$options .= 'P' if param('pep_nonparam');
	$options .= 'G' if param('pep_neggamma');
	$options .= 'E' if param('pep_expect');
	$options .= 'd' if param('pep_decoyprobs');
	$options .= 'p' if param('pep_prot');

	# leave these options at the end
	$options .= ' -d'.param('pep_decoystr') if param('pep_usedecoy');

	# cheap way for doing this...
	$options .= ' -I1' if param('pep_ign1');
	$options .= ' -I2' if param('pep_ign2');
	$options .= ' -I3' if param('pep_ign3');
	$options .= ' -I4' if param('pep_ign4');
	$options .= ' -I5' if param('pep_ign5');

    } else {
	$options .= ' -nP';
    }
 
    if (param('ipro_run')) {
	$options .= ' -i';

	$options .= 'p' if param('ipro_prot');

	$options .= 'P' if param('ipro_nonsp');
	$options .= 'R' if param('ipro_nonrs');
	$options .= 'I' if param('ipro_nonsi');
	$options .= 'M' if param('ipro_nonsm');
	$options .= 'E' if param('ipro_nonse');
	$options .= 'S' if param('ipro_nonss');
    }

    if (param('ptmproph_run')) {
	$options .= ' -M';

	$options .= '-'.param('ptmproph_res1').','.param('ptmproph_res1md');
	$options .= ','.param('ptmproph_res2').','.param('ptmproph_res2md') if param('ptmproph_res2');
	$options .= ','.param('ptmproph_res3').','.param('ptmproph_res3md') if param('ptmproph_res3');
	$options .= ','.param('ptmproph_res4').','.param('ptmproph_res4md') if param('ptmproph_res4');
	$options .= ','.param('ptmproph_res5').','.param('ptmproph_res5md') if param('ptmproph_res5');

	$options .= '-MZTOL='.param('ptmproph_mztol');
	$options .= '-NOUPDATE' if param('ptmproph_noupdate');
    }

    if (param('xp_run')) {
	$options .= ' -X';

	$options .= '-m'.param('xp_mass');
	$options .= '-a' if param('xp_massunit') eq 'PPM';
	$options .= '-b' if param('xp_heavy');

	# only 3 residue mass modifications allowed. Need more?
	for (1..3) {
	    $options .= '-n'.param('xp_res'.$_).','.param('xp_res'.$_.'md') if param('xp_res'.$_);
	}

	$options .= '-L' if (param('xp_fix') eq 'light');
	$options .= '-H' if (param('xp_fix') eq 'heavy');
	$options .= '-F'.param('xp_fixscanrange') if param('xp_fixscan');
	$options .= '-c'.param('xp_minquantscans') if param('xp_minquantscans');
	$options .= '-p'.param('xp_numisopeaks') if param('xp_numisopeaks');

	if (param('xp_metalabel')) {
	    if (param('xp_metatype') eq 'N') {
		if (param('xp_metaquant') eq 'heavy') { $options .= '-N'; }
		else                                  { $options .= '-M'; }
	    } else {
		if (param('xp_metaquant') eq 'heavy') { $options .= '-P'; }
		else                                  { $options .= '-O'; }
	    }
	}
    }

    if (param('as_run')) {
	$options .= ' -A';

	$options .= '-l'.param('as_labres1');
	$options .= param('as_labres2') if param('as_labres2');
	$options .= param('as_labres3') if param('as_labres3');
	$options .= param('as_labres4') if param('as_labres4');
	$options .= param('as_labres5') if param('as_labres5');

	$options .= '-b' if param('as_heavy');
	$options .= '-F' if param('as_fixedscan');
	$options .= '-C' if param('as_cidonly');
	$options .= '-f'.param('as_area') if param('as_area');
	$options .= '-Z' if param('as_zerobg');
	$options .= '-B' if param('as_highbgok');
	$options .= '-r'.param('as_mzpeak') if param('as_mzpeak');

	$options .= '-S' if param('as_static');
	# only 5 residue mass modifications allowed. Need more?
	if (param('as_res1mass')) {
	    $options .= '-m'.param('as_res1').param('as_res1mass');

	    $options .= param('as_res2').param('as_res2mass') if param('as_res2');
	    $options .= param('as_res3').param('as_res3mass') if param('as_res3');
	    $options .= param('as_res4').param('as_res4mass') if param('as_res4');
	    $options .= param('as_res5').param('as_res5mass') if param('as_res5');
	}

    }

    if (param('lb_run')) {
	$options .= ' -L';
	$options .= param('lb_condition');
    }


    my @commands;

    if (param('xinter_multfiles')) {
	for (@file_list) {
	    my $filename = basename($_);
	    my $filedir  = dirname($_);
	
		my @filearr = split /\./, $filename;

	    my $opts = " -Ninteract-".$filearr[0].".pep.xml ". $options;

	    $command = "$command{chdir} $filedir". $command_sep ." $command{xinteract} $opts $filename";

	    my $out_file = "$filedir/interact-".$filearr[0].".pep.xml";
		$out_file =~ s/\.xml(\.gz)?$/\.xml/i;

	    # deal with extra output files
	    $out_file .= &extraInteractFiles($out_file);

	    push @commands, ($command, $out_file);
	}
    } else {
	my @file_list_stripped = &stripPaths(files => \@file_list,
					     bpath =>$outd);

	$options = ' -N'.param('xinter_outf')." $options";

	$command = "$command{chdir} $outd". $command_sep ." $command{xinteract} $options @file_list_stripped";

	my $out_file = "$outd/".param('xinter_outf');
	$out_file =~ s/\.xml(\.gz)?$/\.xml/i;

	# deal with extra output files
	$out_file .= &extraInteractFiles($out_file);

	push @commands, ($command, $out_file);
    }


    # Execute command!
    return &prepareCommands('xinteract',@commands);
}

########################################################################
# extraInteractFiles
#
# Return names of extra output files
#
########################################################################
sub extraInteractFiles {
    my $out_file = shift;

    my $extra_files = '';

    my $pepxml_ext = `$command{tpp_hostname} GET_PEPXML_EXT!`; # get TPP's idea of canonical pepxml file ext
    my $protxml_ext = `$command{tpp_hostname} GET_PROTXML_EXT!`; # get TPP's idea of canonical protxml file ext


    if (param('pep_prot')) {
	my $file = $out_file;
	if ($file =~ /($pepxml_ext)$/) {
	    $file =~ s/($pepxml_ext).*?$/$protxml_ext/;
	} else {
	    $file =~ s/\.xml$/$protxml_ext/;
	}
	$extra_files .= ":::$file";
    }

    if (param('ipro_run')) {
	my $file = $out_file;
	if ($file =~ /($pepxml_ext)$/) {
	    $file =~ s/($pepxml_ext)$/.ipro$pepxml_ext/;
	} else {
	    $file =~ s/\.xml$/.ipro.xml/;
	}
	$extra_files .= ":::$file";
    }

    if (param('ptmproph_run')) {
	my $file = $out_file;
	if ($file =~ /($pepxml_ext)$/) {
	    $file =~ s/($pepxml_ext)$/.ptm.ipro$pepxml_ext/;
	} else {
	    $file =~ s/\.xml$/.ptm.ipro.xml/;
	}
	$extra_files .= ":::$file";
    }

    if (param('ipro_prot')) {
	my $file = $out_file;
	if ($file =~ /($pepxml_ext)$/) {
	    $file =~ s/($pepxml_ext)$/.ipro$protxml_ext/;
	} else {
	    $file =~ s/\.xml$/.ipro$protxml_ext/;
	}
	$extra_files .= ":::$file";
    }

    return $extra_files;
}


########################################################################
# toPepXML
#
# Convert pre-selected files to pepXML format for TPP processing
#
########################################################################
sub toPepXML {

    # Retrieve File List
    my @file_list;
    my ($sequest_params, $search_database);

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'converters'}:/); #'
	$sequest_params = $' if (/$proc_types{'sequest'}:/); #'
        $search_database = $' if (/$proc_types{'searchdb'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to convert!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'topepxml';
    }
    my $pepxml_ext = `$command{tpp_hostname} GET_PEPXML_EXT!`; # get TPP's idea of canonical pepxml file ext

    # Perform Basic Validation
    if (param('c_semi')) {
	param('c_enzyme', "semi".param('c_enzyme'));
    }

    # Build commands
    #
    # Out2XML /path/ <# of top hits to report [1, 10]> (-P/full/path/mysequest.params) (-pI) (-Eenzyme) (-all) (-M) (-m|a)
    # Mascot2XML summary.dat -D/full/path/mydatabase.fasta (-pI) (-Eenzyme) (-noztgz) (-desc) (-shortid)
    # Comet2XML summary.cmt.tar.gz (-Eenzyme)
    my @commands;
    for my $file (@file_list) {
	my $command = '';
	my $options = '';
	my $out_file = '';

	my $fdir  = dirname($file);

	if ($file =~ /dat$/) {
            $options .= ' -D'.$search_database if $search_database;
	    $options .= ' -pI' if param('c_pI');
	    $options .= ' -E'.param('c_enzyme');
	    $options .= ' -notgz' if param('c_notgz');
	    $options .= ' -desc' if param('c_desc');
	    $options .= ' -shortid' if param('c_shortid');

	    $out_file = $file;
	    $out_file =~ s/\.dat$/$pepxml_ext/i; # default was .xml before Jan 2008

	    $command = "$command{mascot2xml} $file $options";

	} elsif ($file =~ /cmt.tar.gz$/) {
	    $options .= ' -E'.param('c_enzyme');

	    $out_file = $file;
	    $out_file =~ s/\.cmt.tar.gz$/$pepxml_ext/i; # default was .xml before Jan 2008

	    $command = "$command{comet2xml} $file $options";

	} elsif ($file =~ /tandem(\.gz)?$/) {

	    $out_file = "$file$pepxml_ext"; # default was .xml before Jan 2008
	    $command = "$command{tandem2xml} $file $out_file";

	} else {
	    # assume it is a directory for Out2XML
	    $options .= param('c_hits');
	    $options .= ' -P'.$sequest_params if $sequest_params;
	    $options .= ' -M'  if param('c_maldi');

	    $options .= ' -m'  if (param('c_masses') eq 'mono');
	    $options .= ' -a'  if (param('c_masses') eq 'average');

	    $options .= ' -all' if param('c_allpeps');
	    $options .= ' -pI'  if param('c_pI');
	    $options .= ' -E'.param('c_enzyme');

	    $out_file = "$file$pepxml_ext"; # default was .xml before Jan 2008

	    $command = "$command{out2xml} $file $options";

	}
	$command = "$command{chdir} $fdir". $command_sep ." " . $command;

	push @commands, ($command, $out_file);

    }

    return &prepareCommands('topepxml',@commands);

}


########################################################################
# runReSpect
#
# Assemble user options; run command
#
########################################################################
sub runReSpect {
    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runprophet'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' pepxml file(s) with probabilities to analyze.";
	return 'respect';
    }

    # Perform Basic Validation
    &validateInput('float',param('respect_minprob'),'Minimum Probability');
    &validateInput('float',param('respect_mztol'),'m/z Ion Tolerance');
    &validateInput('extraopts',param('respect_extraopts'),'Extra Command-line Options') if param('respect_extraopts');
    return 'respect' if ($errors);

    # Build commands
    #
    # respectparser OPTIONS <file.pep.xml>
    my @commands;
    for my $file (@file_list) {
	my $options = '';
	$options .= ' MINPROB='.param('respect_minprob');
	$options .= ' MZTOL='.param('respect_mztol');
	$options .= ' '.param('respect_extraopts') if (param('respect_extraopts'));

	my $command  = "$command{respect} $options $file";
	my $out_file = dirname($file);

	push @commands, ($command, $out_file);
    }

    # Execute commands!
    return &prepareCommands('respect',@commands);
}


########################################################################
# runQualScore
#
# Assemble user options; run command
#
########################################################################
sub runQualScore{
    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runprophet'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' searched pepxml file(s) to analyze.";
	return 'qualscore';
    }

    # Perform Basic Validation
    &validateInput('float',param('qual_cutoff'),'Qualscore Cutoff');
    &validateInput('float',param('qual_pepprob'),'PeptideProphet Probability Max. Treshold for Unassigned');
    return 'qualscore' if ($errors);

    # Build commands
    #
    # java -jar qualscore.jar <options> <interact-data file>
    my @commands;
    for my $file (@file_list) {
	my $command;
	my $options = '';
	my $out_file = '';

	my $fname = basename($file);
	my $fdir  = dirname($file);

	$options .= ' -c '.param('qual_cutoff') if param('qual_cutoff');
	$options .= ' -p '.param('qual_pepprob') if param('qual_pepprob');

	$options .= ' -l' if param('qual_listfile');
	$options .= 'p'   if param('qual_listproph');
	$options .= ' -a' if param('qual_listall');
#	$options .= ' -g' if param('qual_plot');

	$command = "$command{chdir} $fdir". $command_sep ." $command{qualscore} $options $fname";
	push @commands, ($command, $fdir);
    }

    # Execute command!
    return &prepareCommands('qualscore',@commands);
}


########################################################################
# runCompareProts
#
# Assemble user options; run command
#
########################################################################
sub runCompareProts {
    my @protx_file_list;
    my @protxls_file_list;

    push @protx_file_list, param('comprot_reflist');

    # Retrieve File List
    for (@session_data) {
	next if $_ =~ param('comprot_reflist');
	chomp;
	push @protx_file_list, $' if (/$proc_types{'protxml'}:/); #'
    }

    if (!@protx_file_list) {
	push @messages, "Please select at least two protxml files to compare!";
	return 'compareprots';
    }

    # Perform Basic Validation
    &validateInput('not_blank',param('comprot_outdir'),'Output directory');
    &validateInput('no_spaces',param('comprot_outdir'),'Output directory');
    &validateInput('no_spaces',param('prot_inc_tag'),'Include protein name tag') if param('prot_inc_tag');
    &validateInput('no_spaces',param('prot_exc_tag'),'Exclude protein name tag') if param('prot_exc_tag');
    if (param('comprot_type') eq 'heatmap') {
	&validateInput('float',param('comprot_minprob'),'Minimum ProteinProphet Probability Cut-off') if param('comprot_minprob');
	&validateInput('integer',param('comprot_window'),'Protein Window Size') if param('comprot_window');
    }
    return 'compareprots' if ($errors);

    # Build commands
    #
    my $outdir = param('comprot_outdir') || dirname($protx_file_list[0]);
    my $out_file = $outdir."/COMPARE";

    my @commands;

    for my $protx_file (@protx_file_list) {
	my $command = '';

	my $fdir  = dirname($protx_file);
	my $protxls_file = substr($protx_file, 0, length($protx_file)-4) . ".xls";

	$command = "$command{chdir} $fdir". $command_sep ." $command{protxml2html} -file $protx_file EXCEL". $command_sep ." $command{get_prots}";
	$command .= " -I ".param('prot_inc_tag') if param('prot_inc_tag');
	$command .= " -X ".param('prot_exc_tag') if param('prot_exc_tag');

	$command .= " $protxls_file; ";
	$protxls_file .= "_filtered.xls";

	$out_file .= "_".basename($protxls_file);

	push @protxls_file_list, $protxls_file;
	push @commands, ($command, $protxls_file);
    }

    my $command;

    if (param('comprot_type') eq 'heatmap') {
	$command  = "$command{chdir} $outdir". $command_sep ." $command{heatmapprots} -D3";
	$command .= ' -p'.param('comprot_minprob') if param('comprot_minprob');
	$command .= ' -w'.param('comprot_window') if param('comprot_window');
	$command .= ' -nP' if param('comprot_noprot');
	$command .= ' -nF' if param('comprot_nofile');
	$command .= " @protxls_file_list";

	$out_file = "$outdir/compare.xls";

    } else {

	my $esc_qt = $in_windows ? '\\"' : '';
	$command = "$command{chdir} $outdir". $command_sep ." $command{compareprots}  -D3 -N$out_file "
	. ' ' . $esc_qt . "-h'max group pct share of spectrum ids'" . $esc_qt . ' ' 
	. ' ' . $esc_qt . "-h'max group pct share of spectrum ids rank'" . $esc_qt . ' ' 
	. ' ' . $esc_qt . "-h'uniprot link'" . $esc_qt . ' '  
	. ' ' . $esc_qt . "-h'description'" . $esc_qt . ' ' 
	. ' ' . $esc_qt . "-h'entry no.'" . $esc_qt . ' ' 
	. ' ' . $esc_qt . "-h'protein probability'" . $esc_qt . ' ' 
	. ' ' . $esc_qt . "-h'percent share of spectrum ids'" . $esc_qt . ' ' 
	. ' ' . $esc_qt . "-h'percent share of spectrum ids rank'" . $esc_qt . ' ' 
	. " @protxls_file_list";
    }

    push @commands, ($command, $out_file.'.d3.html');

    # Execute command!
    return &prepareCommands('compareprots',@commands);
}

########################################################################
# runMayu
#
# Assemble user options; run command
#
########################################################################
sub runMayu {
    # Retrieve File List
    my @file_list;
    my $db_file;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'xinteract'}:/); #'
	 $db_file = $' if (/$proc_types{'searchdb'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' pepXML file(s) to process.";
	return 'mayu';
    }

    # Perform Basic Validation
    &validateInput('not_blank',param('mayu_tag'),'Tag for decoy proteins');
    &validateInput('no_spaces',param('mayu_tag'),'Tag for decoy proteins');
    &validateInput('integer'  ,param('mayu_nmc'),'Number of missed enzymatic cleavages');
    &validateInput('extraopts',param('mayu_extraopts'),'Extra Command-line Options') if param('mayu_extraopts');
    return 'mayu' if ($errors);

    # Build commands
    my @commands;
    for my $input_file (@file_list) {
	my $options = "-verbose -PmFDR";
	$options .= " -A $input_file";
	$options .= " -C $db_file";
	$options .= ' -E '.param('mayu_tag');
	$options .= ' -I '.param('mayu_nmc');
	$options .= ' -M '.dirname($input_file).'/Mayu';

	$options .= ' '.param('mayu_extraopts') if (param('mayu_extraopts'));

	my $out_files = dirname($input_file).'/Mayu_main_1.07.csv' . ':::';
	$out_files   .= dirname($input_file).'/Mayu_main_1.07.txt';

	my $command = "$command{mayu} $options";
	push @commands, ($command, $out_files);
    }

    # Execute commands!
    return &prepareCommands('mayu',@commands);
}


########################################################################
# runDNA2AA
#
# Assemble user options; run command
#
########################################################################
sub runDNA2AA {

    # Retrieve File
    my $input_file;

    for (@session_data) {
	chomp;
	 $input_file = $' if (/$proc_types{'searchdb'}:/); #'
    }

    if (!$input_file) {
	push @messages, "There are no files to process!";
	push @messages, "Please 'Select' a DNA / transcript database (fasta) file to process.";
	return 'dna2aa';
    }

    # Perform Basic Validation
    &validateInput('not_blank',param('dbase_outfile'),'Output filename');
    &validateInput('no_spaces',param('dbase_outfile'),'Output filename');
    return 'dna2aa' if ($errors);

    # Build commands
    #
    # translateDNA2AA-FASTA <input_file>
    #
    my $command;

    my @commands;

    my $file = "$input_file.new";
    $command = "$command{dna2aa} $input_file";
    push @commands, ($command, $file);

    my $out_file = param('dbase_outfile');
    $command = "$command{mv} -v $file $out_file";
    push @commands, ($command, $out_file);

    # Execute command!
    return &prepareCommands('decoyfasta',@commands);
}


########################################################################
# runDecoyFasta
#
# Assemble user options; run command
#
########################################################################
sub runDecoyFasta{

    # Retrieve File
    my $input_file;

    for (@session_data) {
	chomp;
	 $input_file = $' if (/$proc_types{'searchdb'}:/); #'
    }

    if (!$input_file) {
	push @messages, "There are no files to process!";
	push @messages, "Please 'Select' a protein database (fasta) file to process.";
	return 'decoyfasta';
    }

    # Perform Basic Validation
    &validateInput('not_blank',param('decoy_outfile'),'Output filename');
    &validateInput('no_spaces',param('decoy_outfile'),'Output filename');
    &validateInput('not_blank',param('decoy_tag'),'Tag for decoy proteins');
    &validateInput('no_spaces',param('decoy_tag'),'Tag for decoy proteins');
    return 'decoyfasta' if ($errors);


    # Build commands
    #
    # decoyFASTA [options] <input_file> <output_file> [<filter_file>]
    #   -- not yet implemented: multiple -t options, filter_file
    #
    # decoyFastaGenerator.pl <input_file> <prefix> <output_file>
    #
    my $command;
    my $options;
    my $out_file = param('decoy_outfile');

    if (param('decoy_exe') eq 'decoyfasta') {
	$options  = '-t '.param('decoy_tag');
	$options .= ' -no_orig' if param('decoy_orig');
	$options .= ' -no_reverse' if param('decoy_norev');
	$command = "$command{decoyfasta} $options $input_file $out_file";

    } else {
	$options = param('decoy_tag');
	$command = "$command{decoyfastagen} $input_file $options $out_file";
    }

    my @commands;
    push @commands, ($command, $out_file);

    # Execute command!
    return &prepareCommands('decoyfasta',@commands);

}

########################################################################
# runDecoyValPeps
#
# Assemble user options; run command
#
########################################################################
sub runDecoyValPeps{

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'xinteract'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' pepXML file(s) to process.";
	return 'decoyvalpeps';
    }

    # Perform Basic Validation
    &validateInput('not_blank',param('decoy_tag'),'Tag for decoy proteins');
    &validateInput('no_spaces',param('decoy_tag'),'Tag for decoy proteins');
    &validateInput('no_spaces',param('excl_tag'),'Tag for excluded proteins');
    &validateInput('float',param('ratio'),'Decoy / Incorrect Protein Ratio') if param('ratio');
    return 'decoyvalpeps' if ($errors);

    # Build commands
    my @commands;
    for my $input_file (@file_list) {
	my $options = '-d '.param('decoy_tag');
	$options .= ' -x '.param('excl_tag') if param('excl_tag');
	$options .= ' -r '.param('ratio') if param('ratio');
	$options .= ' -u ' if param('uniq_ipepseq');
	$options .= ' -q -n ' if param('uniq_ppepseq');
	$options .= ' -i '.$input_file ;

	my $command = "$command{decoyvalpeps} $options";
	push @commands, ($command, dirname($input_file));
    }

    # Execute commands!
    return &prepareCommands('decoyvalpeps',@commands);
}

########################################################################
# runDecoyValProts
#
# Assemble user options; run command
#
########################################################################
sub runDecoyValProts{

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runprophet'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' protXML file(s) to process.";
	return 'decoyvalprots';
    }

    # Perform Basic Validation
    &validateInput('not_blank',param('decoy_tag'),'Tag for decoy proteins');
    &validateInput('no_spaces',param('decoy_tag'),'Tag for decoy proteins');
    &validateInput('no_spaces',param('excl_tag'),'Tag for excluded proteins');
    &validateInput('float',param('ratio'),'Decoy / Incorrect Protein Ratio') if param('ratio');
    return 'decoyvalprots' if ($errors);

    # Build commands
    my @commands;
    for my $input_file (@file_list) {
	my $options = '-d '.param('decoy_tag');
	$options .= ' -x '.param('excl_tag') if param('excl_tag');
	$options .= ' -r '.param('ratio') if param('ratio');
	$options .= ' -i '.$input_file ;

	my $command = "$command{decoyvalprots} $options";
	push @commands,  ($command, dirname($input_file));
    }

    # Execute command!
    return &prepareCommands('decoyvalprots',@commands);
}


########################################################################
# runMaRiMba
#
# Assemble user options; run command
#
########################################################################
sub runMaRiMba {
    my ($speclib_input, $fasta_ref, $filter_file);

    for (@session_data) {
	chomp;
	$speclib_input = $' if (/$proc_types{'speclib'}:/); #'
	$fasta_ref     = $' if (/$proc_types{'searchdb'}:/); #'
	$filter_file   = $' if (/$proc_types{'textfile'}:/); #'
    }

    if (!$speclib_input || !$fasta_ref ) {
	push @messages, "You must specify a library file and a sequence database to generate MRM list!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'marimba';
    }

    # Re-set some parameters
    param('mrm_nores1', '') if (param('mrm_nores1') eq '--');
    param('mrm_nores2', '') if (param('mrm_nores2') eq '--');
    param('mrm_nores3', '') if (param('mrm_nores3') eq '--');
    param('mrm_nores4', '') if (param('mrm_nores4') eq '--');
    param('mrm_nores5', '') if (param('mrm_nores5') eq '--');
    param('mrm_nores6', '') if (param('mrm_nores6') eq '--');
    param('mrm_label1', '') if (param('mrm_label1') eq '--');
    param('mrm_label2', '') if (param('mrm_label2') eq '--');
    param('mrm_label3', '') if (param('mrm_label3') eq '--');
    param('mrm_label4', '') if (param('mrm_label4') eq '--');
    param('mrm_label5', '') if (param('mrm_label5') eq '--');
    param('mrm_label6', '') if (param('mrm_label6') eq '--');

    # Perform Basic Validation
    &validateInput('not_blank',param('mrm_outfile'),'Output File');
    &validateInput('integer',param('min_mrm_numtrans'),'Minimum Number of Transitions per Peptide');
    &validateInput('integer',param('mrm_numtrans'),'Maximum Number of Transitions per Peptide');
    &validateInput('float',param('mrm_mzmin'),'Minimum m/z') if (param('mrm_mzmin'));
    &validateInput('float',param('mrm_mzmax'),'Maximum m/z') if (param('mrm_mzmax'));
    &validateInput('float',param('mrm_pimin'),'Minimum pI') if (param('mrm_pimin'));
    &validateInput('float',param('mrm_pimax'),'Maximum pI') if (param('mrm_pimax'));
    if (param('mrm_usemods')) {
	&validateInput('not_blank',param('mrm_label1'),'Labeled Residue(s)');
	&validateInput('float',param('mrm_deltamz1'),'Labeled Residue 1 Mass Difference');
	&validateInput('float',param('mrm_deltamz2'),'Labeled Residue 2 Mass Difference') if param('mrm_label2');
	&validateInput('float',param('mrm_deltamz3'),'Labeled Residue 3 Mass Difference') if param('mrm_label3');
	&validateInput('float',param('mrm_deltamz4'),'Labeled Residue 4 Mass Difference') if param('mrm_label4');
	&validateInput('float',param('mrm_deltamz5'),'Labeled Residue 5 Mass Difference') if param('mrm_label5');
	&validateInput('float',param('mrm_deltamz6'),'Labeled Residue 6 Mass Difference') if param('mrm_label6');
    }
    return 'marimba' if ($errors);

    # Build command
    #
    # run_marimba.pl [options] <splib file> <protein database> <output file>
    my $out_file = param('mrm_outfile');

    my $options = '-t'.param('mrm_numtrans');
    $options .= ' -s'.param('min_mrm_numtrans') if (param('min_mrm_numtrans') > 1);

    $options .= ' -x' if param('mrm_xclmods');
    $options .= ' -k' if (param('mrm_tryptic') eq 'try-cont');
    $options .= ' -u' if (param('mrm_nounique') eq 'non-proteo');
    $options .= ' -I' if param('mrm_nonmono');
    $options .= ' -N' if param('mrm_nloss');
    $options .= ' -S' if param('mrm_secloss');
    $options .= ' -Q' if param('mrm_massshift');
    $options .= ' -m'.param('mrm_mzmin') if param('mrm_mzmin');
    $options .= ' -M'.param('mrm_mzmax') if param('mrm_mzmax');
    $options .= ' -p'.param('mrm_pimin') if param('mrm_pimin');
    $options .= ' -P'.param('mrm_pimax') if param('mrm_pimax');

    if ($filter_file) {
	$options .= param('mrm_restrict') eq 'peps' ? ' -r ' : ' -R ';
	$options .= $filter_file;
    }

    if (param('mrm_nores1')) {
	$options .= ' -X '.param('mrm_nores1');
	$options .= ','.param('mrm_nores2') if param('mrm_nores2');
	$options .= ','.param('mrm_nores3') if param('mrm_nores3');
	$options .= ','.param('mrm_nores4') if param('mrm_nores4');
	$options .= ','.param('mrm_nores5') if param('mrm_nores5');
	$options .= ','.param('mrm_nores6') if param('mrm_nores6');
    }

    if (param('mrm_xclnE') || param('mrm_xclnQ')) {
	$options .= ' -T ';
	$options .= 'E' if param('mrm_xclnE');
	$options .= 'Q' if param('mrm_xclnQ');
    }

    if (param('mrm_ionb') || param('mrm_iony')) {
	$options .= ' -i ';
	$options .= 'b' if param('mrm_ionb');
	$options .= 'y' if param('mrm_iony');
    }

    my $pre = ' -z ';
    for (1..5) {
	if ( param("mrm_prec$_") ) {
	    $options .= "$pre$_";
	    $pre = ',';
	}
    }

    $pre = ' -Z ';
    for (1..5) {
	if ( param("mrm_ion$_") ) {
	    $options .= "$pre$_";
	    $pre = ',';
	}
    }

    $pre = ' -L ';
    for (1..10) {
	if ( param("mrm_length$_") ) {
	    $options .= "$pre$_";
	    $pre = ',';
	}
    }


    if (param('mrm_usemods')) {
	$options .= ' -l '.param('mrm_label1').param('mrm_deltamz1');
	$options .= ','.param('mrm_label2').param('mrm_deltamz2') if param('mrm_label2');
	$options .= ','.param('mrm_label3').param('mrm_deltamz3') if param('mrm_label3');
	$options .= ','.param('mrm_label4').param('mrm_deltamz4') if param('mrm_label4');
	$options .= ','.param('mrm_label5').param('mrm_deltamz5') if param('mrm_label5');
	$options .= ','.param('mrm_label6').param('mrm_deltamz6') if param('mrm_label6');
    }


    my $command =  "$command{runMaRiMba} $options $speclib_input $fasta_ref $out_file";

    my @commands;
    push @commands, ($command, $out_file);

    # Execute command!
    return &prepareCommands('marimba',@commands);

}


########################################################################
# createSpectraSTIndexFiles ==> importSpectraSTLibs
#
# Assemble user options; run command (in 'create' mode)
#
########################################################################
sub createSpectraSTIndexFiles{

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'nistspeclib'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no raw library files for index generation!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'spectrastlib';
    }

    # input file format validation
    my $iff_ext = (param('sil_fileformat') eq 'pepxml') ? 'xml' : param('sil_fileformat');
    foreach (@file_list) {
	if (! /$iff_ext$/) {
	    push @messages, "There are one or more input files of the incorrect type <b>(".param('sil_fileformat').")</b>.";
	    push @messages, "Please Edit your input file selection and resubmit.";
	    return 'spectrastlib';
	}
    }
    if (param('sil_fileformat') eq 'splib'){
	if (param('sil_build') eq 'Quality_Filter') {
	    if ($#file_list > 0) {
		push @messages, "You can only run Quality Filtering on one file at a time.";
		push @messages, "Please Edit your input file selection and resubmit.";
		return 'spectrastlib';
	    }
	}
	elsif (param('sil_join') =~ /^Subtract/) {
	    if ($#file_list < 1) {
		push @messages, "You need to provide more than one input file to join via subtraction.";
		push @messages, "Please Edit your input file selection and resubmit.";
		return 'spectrastlib';
	    }
	    # re-order input files
	    my @tmp_arr;
	    push @tmp_arr, param('sil_primaryfile');
	    foreach (@file_list) {
		push @tmp_arr, $_ unless ($_ eq param('sil_primaryfile'));;
	    }
	    @file_list = @tmp_arr;
	}
    }


    # Re-set some parameters
    param('sil_filterfield', '') if (param('sil_filterfield') eq '--');
    param('sil_qfominprob', '1.01') if (!param('sil_qfominprob'));

    # Perform Basic Validation
    if (param('sil_filterfield')) {
	&validateInput('not_blank',param('sil_filterval'),'Filtering Criterion');
    }
    &validateInput('float',param('sil_minprob'),'Minimum Probability') if (param('sil_minprob'));
    &validateInput('integer',param('sil_minreplicates'),'Number of Replicates') if (param('sil_minreplicates'));
    &validateInput('integer',param('sil_decoynum'),'Number of artificial decoys') if (param('sil_decoynum'));
    &validateInput('extraopts',param('sil_extraopts'),'Extra Command-line Options') if param('sil_extraopts');
    return 'spectrastlib' if ($errors);


    # Build commands
    #
    # Shared options by all modes:
    # spectrast [-cN<name>] [-L<logfile>] -ca[!] -cz[!] [-cf<filter>] file1.msp [file2.msp ...]
    my $options = '';

    my $fdir  = dirname($file_list[0]);
    
    $options .= ' -cN'.param('sil_outf') if param('sil_outf');
    $options .=  ' -L'.param('sil_logf') if param('sil_logf');

    $options .= ' '.param('sil_extraopts') if (param('sil_extraopts'));

    if (param('sil_filterfield')){
	$options .=  ' -cf"'.param('sil_filterfield');
	$options .=  ' '.param('sil_filterop');
	$options .=  ' '.param('sil_filterval').'"';
    }

    # specific options depending on input file format
    if (param('sil_fileformat') eq 'pepxml'){
	$options .= ' -cn'.param('sil_dataident') if param('sil_dataident');
	$options .= ' -cP'.param('sil_minprob')   if param('sil_minprob');

    } elsif (param('sil_fileformat') eq 'splib'){
	$options .= ' -cr'.param('sil_minreplicates') if param('sil_minreplicates');
	$options .= ' -cy'.param('sil_decoynum') if param('sil_decoynum');
	
	if (param('sil_build') eq 'Quality_Filter') {
	    $options .= ' -cAQ';

	    $options .=  ' -cL'.param('sil_qforemove');
	    $options .=  ' -cl'.param('sil_qfoflag');

	} else {
	    $options .= ' -cAC' if (param('sil_build') eq 'Consensus');
	    $options .= ' -cAB' if (param('sil_build') eq 'Best_Replicate');
	    $options .= ' -cAD -cc' if (param('sil_build') eq 'Decoy_Generation');

	    # 'join' action parameter
	    $options .= ' -cJ';
	    $options .= 'U' if (param('sil_join') eq 'Union');
	    $options .= 'I' if (param('sil_join') eq 'Intersection');
	    $options .= 'S' if (param('sil_join') eq 'Subtract');
	    $options .= 'H' if (param('sil_join') eq 'Subtract_homologs');
	    $options .= 'A' if (param('sil_join') eq 'Append');
	}
    }

    my $out_file = $fdir; # it creates several files; so just link to directory

    my $command = "$command{chdir} $fdir". $command_sep ." $command{spectrast} $options @file_list";

    my @commands;
    push @commands, ($command, $out_file);

    # Execute commands!
    return &prepareCommands('spectrastlib',@commands);
}


########################################################################
# fetchSpectralLibs
#
#  Download spectral libraries via custom script (checks md5 and unzips)
#
########################################################################
sub fetchSpectralLibs {
    my @listoflibs = param('splib_getfile');
    my $outdir     = param('splib_dir');
    my @commands;

    unless (@listoflibs) {
	push @messages, "Please 'Select' file(s) to download.";
	return 'getspeclibs';
    }

    for my $hashfile (@listoflibs) {
	my $command = '';
	my ($file,$md5) = split /::/, $hashfile;

	my $outfile = "$outdir/$file";
	$outfile =~ s/\.zip$//i;

	$command = "$command{chdir} $outdir". $command_sep ." $command{downloader} $file $md5";
	push @commands, ($command, $outfile);
    }

    # Execute commands
    return &prepareCommands('getspeclibs',@commands);

}


########################################################################
# runLib2HTML
#
# Assemble user options; run command
#
########################################################################
sub runLib2HTML{

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'nistspeclib'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no library files for html generation!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'lib2html';
    }

    # Build commands
    #
    # Lib2HTML [-P<path to plotspectrast.cgi>] something.splib 
    my @commands;
    my ($command, $options, $out_file, $fname, $fdir);

    for my $file (@file_list) {
	$command = '';
	$options = "-P$tpp_bin_url/plotspectrast.cgi";
	$out_file = '';

	$fname = basename($file);
	$fdir  = dirname($file);

	$out_file = $file;
	$out_file =~ s/\.splib$/\.html/i;

	$command = "$command{lib2html} $options $file";

	push @commands, ($command, $out_file);
    }

    # Execute commands!
    return &prepareCommands('lib2html',@commands);
}


########################################################################
# runSpectraST
#
# Assemble user options; run command
#
########################################################################
sub runSpectraST{

    # Retrieve File List
    my @file_list;
    my $lib_file;
    my $db_file;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runsearch'}:/); #'
	$lib_file = $' if (/$proc_types{'speclib'}:/); #'
	$db_file = $' if (/$proc_types{'searchdb'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to analyze!";
	push @messages, "Please 'Select' mz[X]ML file(s) to search.";
	return 'runspectrast';
    }

    # Re-set some parameters
    param('sst_cysmod', '') if (param('sst_cysmod') eq 'none');

    # Perform Basic Validation
    &validateInput('integer',param('sst_gethomohits'),'Rank of hits used in homology detection') if (param('sst_gethomohits'));
    &validateInput('float',param('sst_tophitminf'),'Minimum F value threshold for top hit to be displayed') if (param('sst_tophitminf'));
    &validateInput('float',param('sst_lowhitminf'),'Minimum F value threshold for lower hit to be displayed') if (param('sst_lowhitminf'));
    &validateInput('integer',param('sst_minpeaks'),'Minimum number of peaks') if (param('sst_minpeaks'));
    &validateInput('float',param('sst_minintensity'),'Minimum peak intensity for peaks to be counted') if (param('sst_minintensity'));
    &validateInput('float',param('sst_noiselevel'),'Noise Peak Threshold') if (param('sst_noiselevel'));
    &validateInput('float',param('sst_maxmz'),'m/z cutoff to remove spectra') if (param('sst_maxmz'));
    &validateInput('float',param('sst_thminint'),'Intensity treshold for 515.3 Th peak removal') if (param('sst_thminint'));
    &validateInput('float',param('sst_mzscalepow'),'m/z scaling power') if (param('sst_mzscalepow'));
    &validateInput('float',param('sst_intscalepow'),'Intensity scaling power') if (param('sst_intscalepow'));
    &validateInput('float',param('sst_noidscale'),'Scaling factor for unassigned peaks in the library spectra') if (param('sst_noidscale'));
    &validateInput('float',param('sst_binsperth'),'Number of bins per Th') if (param('sst_binsperth'));
    &validateInput('float',param('sst_peakfrac'),'Fraction of the scaled intensity of a peak assigned to neighboring bins') if (param('sst_peakfrac'));
    &validateInput('dir_exists',param('sst_outdir'),'Output directory');
    &validateInput('extraopts',param('sst_extraopts'),'Extra Command-line Options') if param('sst_extraopts');
    return 'runspectrast' if ($errors);

    # Build command
    #
    # spectrast [options] .mzXML_file1 [.mzXML_file2  .mzXML_file3  ... ]
    my $command;
    my $options = '';

    my $fdir  = param('sst_outdir');

    # deal with basic options
    $options .= ' -sL'.$lib_file if ($lib_file);
    $options .= ' -sD'.$db_file if ($db_file);
    $options .= ' -sT'.param('sst_dbtype') if (param('sst_dbtype'));
    $options .= ' -sF'.param('sst_optsfile') if (param('sst_useoptsfile'));
    $options .= ' -sC'.param('sst_cysmod') if (param('sst_cysmod'));
    $options .= ' -sM'.param('sst_mztolerance') if (param('sst_mztolerance'));

    if (param('sst_useavgmass'))  { $options .= ' -sA'; }
    else                          { $options .= ' -sA!'; }

    # deal with advanced/general options
    $options .= ' -s_NO1' if (param('sst_ignoreplusone'));
    $options .= ' -s_NOS' if (param('sst_ignorebadid'));
    $options .= ' -s_SAV' if (param('sst_savedtas'));
    $options .= ' -s_TGZ' if (param('sst_maketgz'));
    $options .= ' -s_HOM'.param('sst_gethomohits') if (param('sst_gethomohits'));

    $options .= ' '.param('sst_extraopts') if (param('sst_extraopts'));

    if (param('sst_usecached'))   { $options .= ' -sR'; } 
    else                          { $options .= ' -sR!';} ### necessary?? 

    # deal with advanced/output options
    $options .= ' -s_SH1' if (param('sst_onlytop'));
    $options .= ' -s_SHM' if (param('sst_excludebelowf'));
    $options .= ' -sE'.param('sst_outformat') if (param('sst_outformat'));
    $options .= ' -s_FV1'.param('sst_tophitminf') if (param('sst_tophitminf'));
    $options .= ' -s_FV2'.param('sst_lowhitminf') if (param('sst_lowhitminf'));
    $options .= ' -sO'.param('sst_outdir');

    # deal with advanced/filtering options  [ deprecated ]
    $options .= ' -s_CNT'.param('sst_minintensity') if (param('sst_minintensity'));
    $options .= ' -s_RNT'.param('sst_noiselevel') if (param('sst_noiselevel'));
    $options .= ' -s_XNP'.param('sst_minpeaks') if (param('sst_minpeaks'));
    $options .= ' -s_R51'.param('sst_thminint') if (param('sst_thminint'));
    $options .= ' -s_XMZ'.param('sst_maxmz') if (param('sst_maxmz'));

    # deal with advanced/processing options  [ deprecated ]
    $options .= ' -s_INS'.param('sst_intscalepow') if (param('sst_intscalepow'));
    $options .= ' -s_MZS'.param('sst_mzscalepow') if (param('sst_mzscalepow'));
    $options .= ' -s_UAS'.param('sst_noidscale') if (param('sst_noidscale'));
    $options .= ' -s_BIN'.param('sst_binsperth') if (param('sst_binsperth'));
    $options .= ' -s_NEI'.param('sst_peakfrac') if (param('sst_peakfrac'));

    $command = "$command{chdir} $fdir". $command_sep ." $command{spectrast} $options @file_list";

    my @commands;
    push @commands, ($command, $fdir);

    # Execute commands!
    return &prepareCommands('runspectrast',@commands);
}


########################################################################
# runSearch
#
# Assemble user options; run command
#
########################################################################
sub runSearch{

    # Retrieve File List
    my @file_list;
    my $sequest_params;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runsearch'}:/); #'
	$sequest_params = $' if (/$proc_types{'sequest'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to search!";
	push @messages, "Please 'Select' mz[X]ML file(s) to search.";
	return 'runsearch';
    }

    my $pepxml_ext = `$command{tpp_hostname} GET_PEPXML_EXT!`; # get TPP's idea of canonical pepxml file ext


    # Perform Basic Validation
    # ToDo: compare Start and End scan numbers, if present.  FIXME
    &validateInput('integer',param('run_scanstart'),'Start Scan Number') if (param('run_scanstart'));
    &validateInput('integer',param('run_scanend'),'End Scan Number') if (param('run_scanend'));
    &validateInput('integer',param('run_numpeaks'),'Number of Peaks') if param('run_numpeaks');
    return 'runsearch' if ($errors);

    # Build commands
    #
    # runsearch [options] [.mzXML files]
    my @commands;
    for my $file (@file_list) {
	my $command;
	my $options = '';
	my $out_file = '';

	my $fname = basename($file);
	my $fdir  = dirname($file);

	if ($sequest_params) {
	    $options .= '-p'.$sequest_params;
	}
	$options .= ' -F'.param('run_scanstart') if param('run_scanstart');
	$options .= ' -L'.param('run_scanend') if param('run_scanend');

	$options .= ' -R' if param('run_research');

	$options .= ' -P'.param('run_numpeaks') if param('run_numpeaks');

	$options .= ' -1' if (param('run_peptides') eq 'plusone');
	$options .= ' -2' if (param('run_peptides') eq 'plustwo');
	$options .= ' -3' if (param('run_peptides') eq 'plusthree');


	$out_file = $file;
	$out_file =~ s/\.mz[X]?ML$/$pepxml_ext/i; # default was .xml before Jan 2008

	$command = "$command{chdir} $fdir". $command_sep ." $command{runsearch} $options $fname";

	push @commands, ($command, $out_file);
    }

    # Execute command!
    return &prepareCommands('runsearch',@commands);
}


########################################################################
# runComet
#
# Assemble user options; run command
#
########################################################################
sub runComet{
    # Retrieve File List
    my @file_list;
    my $comet_params;
    my $dbase_path;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runsearch'}:/); #'
	$comet_params  = $' if (/$proc_types{'sequest'}:/); #'
	$dbase_path    = $' if (/$proc_types{'searchdb'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to search!";
	push @messages, "Please 'Select' mz[X]ML file(s) to search.";
	return 'runcomet';
    }

    my $pepxml_ext = `$command{tpp_hostname} GET_PEPXML_EXT!`; # get TPP's idea of canonical pepxml file ext

    # Perform Basic Validation -- None at the moment...
    return 'runcomet' if ($errors);

    # Build commands
    #
    # comet [options] [.mzXML files]
    my @commands;
    for my $file (@file_list) {
	my $command;
	my $options = '';
	my $out_file = '';

	my $fname = basename($file);
	my $fdir  = dirname($file);

	if ($comet_params) {
	    $options .= ' -P'.$comet_params;

	    my $ofile  = dirname($comet_params).'/'.$fname;
	    $out_file = $ofile;
	    $ofile =~ s/\.mz[X]?ML$//i;

	    $options .= ' -N'.$ofile;
	}

	if ($dbase_path) {
	    $options .= ' -D'.$dbase_path;
	}

	$out_file ||= $file;
	$out_file =~ s/\.mz[X]?ML$/$pepxml_ext/i;

	if (param('runon_cluster') eq 'on_Amazon_cloud') {
            if ( $dbase_path ) {
		my $new_params = "$fdir/comet.amztpp.params";
		open(SRC, "<$comet_params") || &fatalError("CANNOT_READ_COMET_PARAMS_FILE:$comet_params:$!");
		open(DST, ">$new_params") || &fatalError("CANNOT_WRITE_COMET_PARAMS_FILE:$new_params:$!");
		while ( <SRC> ) {
                    s/^(\s*database_name\s*=\s*).*/$1$dbase_path/i;
                    print DST $_;
		}
		close(SRC);
		close(DST);
		$comet_params = $new_params;
            }
	    $command = "$command{amztpp} comet -v -p $comet_params $file";

	    my $amz_output_files;
            my $root = $file;
	    $root =~ s/\.mz[X]?ML(\.gz)?$//i;
            $amz_output_files .= "$root$pepxml_ext" . ':::';
            $amz_output_files .= "$root.comet.log" . ':::';
	    unlink("$root.comet.log") if (-e "$root.comet.log");

	    $amz_output_files .= "NONE";
	    push @commands, ($command, $amz_output_files);

	    my $amz_log = "$tpp_bin_url/amztpp.log";
	    push @commands, ("$command{amztpp} -v --log $base_dir/amztpp.log --ec2-type m1.xlarge start", $amz_log);
	    last;

	} else {
	    $command = "$command{chdir} $fdir". $command_sep ." $command{comet} $options $fname";
	}

	push @commands, ($command, $out_file);
    }

    # Execute command!
    return &prepareCommands('runcomet',@commands);
}


########################################################################
# runTandem
#
# Create taxonomy and param files; run command(s)
#
########################################################################
sub runTandem{
    # Retrieve File List
    my @file_list;
    my $tandem_params;
    my $dbase_path;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'runsearch'}:/); #'
	$tandem_params = $' if (/$proc_types{'sequest'}:/); #'
	$dbase_path    = $' if (/$proc_types{'searchdb'}:/); #'
    }
    if (!@file_list) {
	push @messages, "There are no files to search!";
	push @messages, "Please 'Select' mz[X]ML file(s) to search.";
	return 'runtandem';
    }

    # Re-set some parameters
    param('runon_cluster', '') unless (param('runon_cluster') );

    # Write out (temp) taxonomy file
    my $taxon_path = dirname($tandem_params) . "/taxonomy.xml";
    open(FILE, ">$taxon_path") || &fatalError("CANNOT_CREATE_TAXONOMY_FILE:$taxon_path:$!");
    print FILE <<EOTAX;
<?xml version="1.0"?>
<bioml label="x! taxon-to-file matching list">
  <taxon label="mydatabase">
    <file format="peptide" URL="$dbase_path" />
  </taxon>
</bioml>
EOTAX
    close FILE;

    my $pepxml_ext = `$command{tpp_hostname} GET_PEPXML_EXT!`; # get TPP's idea of canonical pepxml file ext

    # Generate params files and build commands
    #
    # tandem tandem.params
    my @commands;
    for my $file (@file_list) {
	my $command;

	my $out_file = dirname($tandem_params) . '/' . basename($file);
	$out_file =~ s/\.mz[X]?ML(\.gz)?$/\.tandem/i;

	my $fname = basename($file);
	my $fdir  = dirname($file);

	my $params   = "$out_file.params";
	if (param('runon_cluster') eq 'on_Amazon_cloud') {
	    $params   = "$fdir/tandem.amztpp.params";
	}

	open(PARAMS, "$tandem_params") || &fatalError("CANNOT_READ_PARAMS_FILE:$tandem_params:$!");
	open(FILE, ">$params") || &fatalError("CANNOT_CREATE_PARAMS_FILE:$params:$!");

	my $inserted_params;
	while(<PARAMS>) {
	    if (/(<bioml>)/i) {
		print FILE $`.$1;
		print FILE <<EOPARAMS;

   <note type="input" label="list path, taxonomy information">$taxon_path</note>
   <note type="input" label="protein, taxon">mydatabase</note>
   <note type="input" label="spectrum, path">$file</note>
   <note type="input" label="output, path">$out_file</note>
 
EOPARAMS
		print FILE $'; #'# print stuff after <bioml> match, just in case
		$inserted_params = 1;

	    } else {
		print FILE $_
		    unless (  # skip these!
			      m|<note.*label=\"list.*path.*taxonomy.*information\">.*</note>|i ||
			      m|<note.*label=\"protein.*taxon\">.*</note>|i ||
			      m|<note.*label=\"spectrum.*path\">.*</note>|i ||
			      m|<note.*label=\"output.*path\">.*</note>|i
			      );
	    }
	}
	close FILE;
	close PARAMS;

	if (!$inserted_params){
	    push @messages, "Invalid input parameters xml: $tandem_params . No &lt;bioml&gt; tag found!";
	    return 'runtandem';
	}

	if (param('runon_cluster') eq 'on_Amazon_cloud') {
	    $command = "$command{amztpp} xtandem -v -p $params @file_list";

	    my $amz_output_files;
	    for (@file_list) {
		s/\.mz[X]?ML(\.gz)?$/\.tandem/i;
		$amz_output_files .= $_ . ':::';
		$amz_output_files .= "$_$pepxml_ext" . ':::';
		$amz_output_files .= "$_.log" . ':::';
		unlink("$_.log") if (-e "$_.log");
	    }
	    $amz_output_files .= "NONE";
	    push @commands, ($command, $amz_output_files);

	    my $amz_log = "$tpp_bin_url/amztpp.log";
	    push @commands, ("$command{amztpp} -v --log $base_dir/amztpp.log --ec2-type m1.xlarge start", $amz_log);
	    last;

	} else {
	    $command = "$command{chdir} $fdir". $command_sep ." $command{tandem} $params";
	}

	push @commands, ($command, $out_file);

	if (param('tan_2pep')) {
	    my $out_file_pep = "$out_file$pepxml_ext"; # default was .xml before Jan 2008
	    $command = "$command{chdir} $fdir". $command_sep ." $command{tandem2xml} $out_file $out_file_pep";
	    push @commands, ($command, $out_file_pep);
	}

    }

    # DEBUG
#    push @messages, @commands;
#    return 'jobs';

    # Execute command!
    return &prepareCommands('runtandem',@commands);
}

########################################################################
# saveParams
#
# Save search engine parameters file (Tandem, for starters...)
#
########################################################################
sub saveParams {
    unless (param('file_path')) {
	push @messages, "Please specify a filename!";
	return 'editparams';
    }

    my $file_path = param('file_path');
    my @remparams = param('remparam') if param('remparam');

    my $file_contents = '<?xml version="1.0" encoding="UTF-8"?>' . "\n<bioml>\n";

    for my $key (sort(param())) {
	if ($key =~ /^SPARAM_.*_/) {
	    my $param = $';  #'
	    my $remove = 0;

	    for my $rem (@remparams) {
		$remove++ if $key =~ /^$rem/;
	    }
	    $file_contents .= "<note type=\"input\" label=\"$param\">" . param($key) . "</note>\n" unless $remove;

	}
	elsif ($key =~ /^XTRA_SPARAM_NAME_/) {
	    my $val = $key;
	    $val =~ s/NAME/VALUE/;
	    $file_contents .= "<note type=\"input\" label=\"" . param($key) . "\">" . param($val) . "</note>\n" if param($key);
	}

    }
    $file_contents .= "</bioml>\n";

    &saveFile(file_path     => $file_path,
	      file_contents => $file_contents);

    return 'filebrowser';
}


########################################################################
# toMzXML
#
# Convert pre-selected files to mzXML format for TPP processing
#
########################################################################
sub toMzXML {

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'mzxml'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to convert!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'mzxml';
    }

    # Perform Basic Validation
    &validateInput('extraopts',param('tomz_extraopts'),'Extra Command-line Options') if param('tomz_extraopts');
    return 'mzxml' if ($errors);

    # Build commands
    #
    # readw    [-c -z -v -g] <raw file path> [<output file>]
    # mzWiff   [-c -c1 -z -v -g] <raw file path> [<output file>] (+ many others...)
    # massWolf [-c -z -v -g] <raw dir path> [<output file>]
    # trapper  [-c -z -v -g] <raw dir path> [<output file>]
    my @commands;
    for my $file (@file_list) {
	my $command = '';
	my $options = '-v --mzXML'; # always verbose and mzXML

	$options .= ' -z'  if (param('tomz_zlib'));
	$options .= ' -c'  if (param('tomz_centroid'));
	$options .= ' -c'  if (param('tomz_centroid2'));
	$options .= ' -c1' if (param('tomz_centroid1'));
	$options .= ' -g'  if (param('tomz_gzip'));

	my $out_file = $file; # modified below
	my $input_file = $file;

	if ($rawfile eq "raw") {
	    $command = "$command{readw} $options $input_file";
	    $out_file =~ s/\.raw$/\.mzXML/i;

	} elsif ($rawfile eq "wiff") {

	    if (param('tomz_usemzinfo')) {
		$options .= ' -G';
	    } else {
		$options .= ' -I"'.param('tomz_ionisation').'"';
		$options .= ' -T"'.param('tomz_mstype').'"';
		$options .= ' -D"'.param('tomz_detector').'"';
	    }

	    $options .= ' --Analyst'   if (param('tomz_analyst') eq 'analyst');
	    $options .= ' --AnalystQS' if (param('tomz_analyst') eq 'analystqs');

	    $options .= ' '.param('tomz_extraopts') if (param('tomz_extraopts'));

	    $command = "$command{mzwiff} $options $input_file";
	    $out_file =~ s/\.wiff$/\.mzXML/i;

	} elsif ($rawfile eq "rawdir") {
	    $command = "$command{masswolf} $options $input_file";
	    $out_file =~ s/\.raw$/\.mzXML/i;
	    $out_file =~ s/\.RAW$/\.mzXML/i;

	} elsif ($rawfile eq "ddir") {
	    $command = "$command{trapper} $options $input_file";
	    $out_file =~ s/\.d$/\.mzXML/i;

	} else {
	    $command = "$command{echo} Cannot process $file : Unknown type.";
	    $out_file = "NO_FILE";

	}
	# did user ask to gzip entire mzxml file?
	$out_file = $out_file.".gz" if (param('tomz_gzip')); 
	push @commands, ($command, $out_file);

    }

    # Execute commands!
    return &prepareCommands('mzxml',@commands);

}


########################################################################
# toMzML
#
# Convert pre-selected files to the mzML format  (will merge with above eventually)
#
########################################################################
sub toMzML {

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'mzml'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to convert!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'msconvert';
    }

    # Perform Basic Validation
    &validateInput('extraopts',param('msconvert_extraopts'),'Extra Command-line Options') if param('msconvert_extraopts');
    return 'msconvert' if ($errors);

    # Build commands
    #
    # msconvert [-c arg -z -v] --mzML <raw file path>
    my @commands;
    for my $file (@file_list) {
	my $command = '';
	my $out_file = '';
	my $ext = '';
	my $fdir  = dirname($file);

	my $options = '-v '; # always verbose

	if (param('msconvert_mzxml')) {
	    $options .= '--mzXML';
	    $ext = '.mzXML';
	} else {
	    $options .= '--mzML';
	    $ext = '.mzML';
	}

	$options .= " -o $fdir";

	$options .= ' -z' if (param('msconvert_zlib'));
	$options .= ' -g' if (param('msconvert_gzip'));
	$options .= ' --filter "peakPicking true [1,2]"' if (param('msconvert_centroid'));
	$options .= ' '.param('msconvert_extraopts') if (param('msconvert_extraopts'));

	$command = "$command{msconvert} $file $options";

	$out_file = $file; # modified below
	if ($file =~ /raw$/i) {
	    $out_file =~ s/\.raw$/$ext/i;
	} elsif ($file =~ /wiff$/i) {
	    $out_file =~ s/\.wiff$/$ext/i;
	} elsif ($file =~ /baf$/i) {
	    $out_file =~ s/\.baf$/$ext/i;
	} elsif ($file =~ /yep$/i) {
	    $out_file =~ s/\.yep$/$ext/i;
	} elsif ($file =~ /\.d$/i) {
	    $out_file =~ s/\.d$/$ext/i;
	} else {
	    $command = "$command{echo} Cannot process $file : Unknown type.";
	    $out_file = "NO_FILE";
	}
	# did user ask to gzip entire mzml file?
	$out_file = $out_file.".gz" if (param('msconvert_gzip')); 
	push @commands, ($command, $out_file);

    }

    # Execute commands!
    return &prepareCommands('msconvert',@commands);

}


########################################################################
# toMzIdentML
#
# Convert pre-selected files to mzIdentML format
#
########################################################################
sub toMzIdentML {
    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'protxml'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to convert!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'idconvert';
    }

    # Build command
    #
    # idconvert <protxml_file>
     my @commands;
    for my $file (@file_list) {
	my $command = '';
	my $options = '-v';
	my $out_file = $file;

	my $fdir  = dirname($file);

	$command = "$command{idconvert} $options $file";
	$command = "$command{chdir} $fdir". $command_sep ." " . $command;

	$out_file =~ s/\.xml$/.mzid/;

	push @commands, ($command, $out_file);
    }

    return &prepareCommands('idconvert',@commands);
}


########################################################################
# dtaToMzXML
#
# Run dta2mzxml
#
########################################################################
sub dtaToMzXML{

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'mzxml'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to convert!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'dta2mzxml';
    }

    # Build command
    #
    # dta2mzxml (-recount -charge -plustwo -byname) <dta files to convert>
    my @commands;
    for my $file (@file_list) {
	my $command;
	my $options = '';

	my $fdir  = dirname($file);

	# options
	$options .= ' -recount' if (param('dta2mz_recount'));
	$options .= ' -charge'  if (param('dta2mz_charge'));
	$options .= ' -plustwo' if (param('dta2mz_plus2'));
	$options .= ' -byname'  if (param('dta2mz_byname'));
	$options .= ' -g'  if (param('dta2mz_gzip'));

	# glob all dta files
	$command = "$command{chdir} $file". $command_sep ." $command{dta2mzxml} $options *.dta";

	push @commands, ($command, $fdir);
    }

    # Execute commands!
    return &prepareCommands('dta2mzxml',@commands);
}


########################################################################
# runIndexMzXML
#
# Assemble user options; run command
#
########################################################################
sub runIndexMzXML {

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'mz2other'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no mzXML files for re-indexing!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'lib2html';
    }

    # Build commands
    #
    # indexmzXML *mzXML
    my @commands;
    my ($command, $options, $old_file, $fname, $fdir);

    for my $file (@file_list) {
	$command = '';
	$old_file = '';

	$fname = basename($file);
	$fdir  = dirname($file);

	$old_file = $file;
	$old_file =~ s/(\.mz[X]?ML)$/.old_index$1/i; # rename input and output files

	$command = "$command{indexmzxml} $file";
	push @commands, ($command, $file);

	$command = "$command{mv} -v $file $old_file";
	push @commands, ($command, $old_file);

	$command = "$command{mv} -v $file.new $file";
	push @commands, ($command, 'NONE');
    }

    # Execute commands!
    return &prepareCommands('indexmzxml',@commands);
}


########################################################################
# mzToSearch
#
# Convert pre-selected mzXML files to other formats using MzXML2Search
#
########################################################################
sub mzToSearch {

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'mz2other'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to convert!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'mzxml2search';
    }

    # Re-set some parameters
    param('m2s_activation', '') if (param('m2s_activation') eq 'all');

    # Perform Basic Validation
    # ToDo: compare Start and End scan and mass numbers, if present.  FIXME
    &validateInput('integer',param('m2s_scanstart'),'Start Scan Number') if (param('m2s_scanstart'));
    &validateInput('integer',param('m2s_scanend'),'End Scan Number') if (param('m2s_scanend'));
    &validateInput('integer',param('m2s_precmass'),'Mass Precision') if param('m2s_precmass');
    &validateInput('integer',param('m2s_precint'),'Intensity Precision') if param('m2s_precint');
    &validateInput('integer',param('m2s_maxpeaks'),'Maximum Top Peaks Count') if param('m2s_maxpeaks');
    &validateInput('integer',param('m2s_minpeaks'),'Minimum Peak Count') if param('m2s_minpeaks');
    &validateInput('float',param('m2s_minintensity'),'Minimum Peak Intensity') if param('m2s_minintensity');
    &validateInput('float',param('m2s_massmin'),'Minimum MH+ Mass') if param('m2s_massmin');
    &validateInput('float',param('m2s_massmax'),'Maximum MH+ Mass') if param('m2s_massmax');
    return 'mzxml2search' if ($errors);

    # Build commands
    #
    # MzXML2Search [options] filename.mzxml
    my @commands;
    my ($command, $options, $out_file,$fname,$fdir,$out_ext);

    for my $file (@file_list) {
	$command = '';
	$options = '';
	$out_file = '';

	$fname = basename($file);
	$fdir  = dirname($file);

	$options  = '-'.param('m2s_format');
	$options .= ' -A'.param('m2s_activation') if (param('m2s_activation'));
	$options .= ' -M'.param('m2s_mslevel');
	$options .= '-'.param('m2s_mslevel2') if (param('m2s_mslevel2'));

	$options .= ' -P'.param('m2s_minpeaks');
	$options .= ' -N'.param('m2s_maxpeaks')  if (param('m2s_maxpeaks'));
	$options .= ' -I'.param('m2s_minintensity') if (param('m2s_minintensity'));
	$options .= ' -F'.param('m2s_scanstart') if (param('m2s_scanstart'));
	$options .= ' -L'.param('m2s_scanend')   if (param('m2s_scanend'));
	$options .= ' -B'.param('m2s_massmin')   if (param('m2s_massmin'));
	$options .= ' -T'.param('m2s_massmax')   if (param('m2s_massmax'));
	$options .= ' -Z'.param('m2s_maxcharge') if (param('m2s_maxcharge'));
	$options .= ' -pm'.param('m2s_precmass') if (param('m2s_precmass'));
	$options .= ' -pi'.param('m2s_precint')  if (param('m2s_precint'));

	$options .= ' -X' if (param('m2s_remprec'));
	$options .= ' -Q' if (param('m2s_remitraq'));
	$options .= ' -G' if (param('m2s_remtmt'));

	if (param('m2s_charge1')) {
	    if (param('m2s_chargedefault')) {
		$options .= ' -c';
	    } else {
		$options .= ' -C';
	    }
	    $options .= param('m2s_charge1');
	    $options .= '-'.param('m2s_charge2') if (param('m2s_charge2'));
	}


	$out_file = $file;
	$out_ext = param('m2s_format');

	if (param('m2s_format') eq 'dta') {
	    # a directory full of files gets created
	    $out_file =~ s/\.mz[X]?ML$//i;
	} else {
	    $out_file =~ s/\.mz[X]?ML$/\.$out_ext/i;
	}
	$command = "$command{chdir} $fdir". $command_sep ." $command{mzxml2search} $options $fname";

	push @commands, ($command, $out_file);
    }

    # Execute commands!
    return &prepareCommands('mzxml2search',@commands);
}


########################################################################
# runCreateChargeFile
#
#  Create .charge files
#
########################################################################
sub runCreateChargeFile {

    # Retrieve File List
    my @file_list;

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'tocharge'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files from which to extract charge information!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'chargefile';
    }

    # Build commands
    #
    # createChargeFile.pl [OPTIONS + <FILES>]
    my @commands;
    for my $file (@file_list) {
	my $command = '';
	my $options = '';
	my $out_file = $file;

	my $fdir  = dirname($file);

	if ($file =~ /ms2$/) {
	    $options .= '-f ms2 -i';
	    $out_file =~ s/ms2$/charge/i;
	} else {
	    $options .= '-f dta -d';
	    $out_file .= ".charge";
	}

	$command = "$command{chargefile} $options $file";

	$command = "$command{chdir} $fdir". $command_sep ." " . $command;

	push @commands, ($command, $out_file);

    }

    return &prepareCommands('chargefile',@commands);

}


########################################################################
# runMergeCharges
#
#  Merge charge information into mzXML files
#
########################################################################
sub runMergeCharges {

    # Retrieve File List
    my ($mzfile, $charges);

    for (@session_data) {
	chomp;
	$mzfile = $' if (/$proc_types{'runsearch'}:/); #'
	$charges= $' if (/$proc_types{'chargefile'}:/); #'
    }

    if (!($mzfile && $charges)) {
	push @messages, "There are not enough files specified!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'mergecharges';
    }

    # Build commands
    #
    # mergeCharges.pl -i inputmzXMLFile -c inputChargeFile
    my $command = '';
    my $options = '';
    my $out_file = '';

    my $fdir  = dirname($mzfile);

    $command = "$command{mergecharges} -i $mzfile -c $charges";
    
    $command = "$command{chdir} $fdir". $command_sep ." " . $command;

    my @commands;
    push @commands, ($command, $mzfile);


    return &prepareCommands('mergecharges',@commands);

}


########################################################################
# updatePaths
#
#  Update file paths inside pepXML, protXML, etc files
#
########################################################################
sub updatePaths {

    # Retrieve File List
    my @file_list;
    my @dir_list;
    my $db;
    my $ext = param('backup');

    for (@session_data) {
	chomp;
	push @file_list, $' if (/$proc_types{'updatefiles'}:/); #'
	push @dir_list, $'  if (/$proc_types{'extrapaths'}:/); #'
        $db  = $' if (/$proc_types{'searchdb'}:/); #'
    }

    if (!@file_list) {
	push @messages, "There are no files to update!";
	push @messages, "Please 'Select' file(s) to process.";
	return 'updatepaths';
    }

    # Build commands
    #
    # updatepaths.pl [OPTIONS] <FILES>
    my @commands;
    my $options = '';
    $options .= " -d $db"  if ( $db );
    $options .= param('verbose') ? ' -vv' : ' -v';
    $options .= " -n"      if ( param('dry_run') );
    $options .= " -b $ext" if ( $ext );
    for my $path (@dir_list) {
        $options .= " -p $path";
    }
    for my $file (@file_list) {
	my $out_file = $file;

	my $fdir    = dirname($file);
	my $command = "$command{updatepaths} $options $file";
	$command = "$command{chdir} $fdir". $command_sep ." " . $command;

	push @commands, ($command, $out_file);

    }

    return &prepareCommands('updatepaths',@commands);

}


########################################################################
# generateLibraConditionFile
#
#  Write a condition.xml file using user-supplied parameters
#
########################################################################
sub generateLibraConditionFile {
    if (!param('cond_outfile')) {
	push @messages, "Please specify an output file!";
	Delete('cond_outfile');
	return 'conditionxml';
    }

    # Perform Basic Validation
    &validateInput('not_blank',param('cond_outfile'),'Output filename');
    &validateInput('no_spaces',param('cond_outfile'),'Output filename');
    my @use_channels;
    for my $channel (1..8) {
	if (param("cond_usemz_$channel")) {
	    &validateInput('float',param("cond_mz_$channel"),"m/z for reagent $channel");
	    &validateInput('float',param("cond_affect_${channel}_m2"),"% isotopic contribution to -2 for reagent $channel");
	    &validateInput('float',param("cond_affect_${channel}_m1"),"% isotopic contribution to -1 for reagent $channel");
	    &validateInput('float',param("cond_affect_${channel}_p1"),"% isotopic contribution to +1 for reagent $channel");
	    &validateInput('float',param("cond_affect_${channel}_p2"),"% isotopic contribution to +2 for reagent $channel");
	    push @use_channels, $channel;
	}
    }
    if (@use_channels < 1) {
	push @messages, "You must select at least one reagent to use for quantitation analysis!";
	return 'conditionxml';
    }

    &validateInput('float',param('cond_masstol'),'Mass Tolerance');
    &validateInput('integer',param('cond_thresh'),'Minimum Threshhold Intensity');
    return 'conditionxml' if ($errors);

    # make a hash of selected (integer) reagent masses; useful for assigning isotopic contributions
    my %mz_xml_map;
    for (my $index = 0; $index <= $#use_channels; $index++) {
	my $int_mz = int param("cond_mz_$use_channels[$index]");
	$mz_xml_map{$int_mz} = $index+1;
    }

    my $xmlfile = param('cond_outfile');
    open(XMLFILE, ">$xmlfile") || &fatalError("CANNOT_CREATE_CONDITIONXML_FILE:$xmlfile:$!");
    print XMLFILE <<EOTOP;
<?xml version="1.0" encoding="UTF-8"?>
<SUMmOnCondition>
  <fragmentMasses>
EOTOP

    for my $channel (@use_channels) {
	print XMLFILE '    <reagent mz="'.param("cond_mz_$channel").'" />'."\n";
    }
    print XMLFILE '  </fragmentMasses>'."\n";

    print XMLFILE '  <isotopicContributions>'."\n";
    for (my $index = 0; $index <= $#use_channels; $index++) {
	print XMLFILE '    <contributingMz value="'.($index+1).'">'."\n";

	my $int_mz = int param("cond_mz_$use_channels[$index]");
	print XMLFILE '      <affected mz="'.$mz_xml_map{(${int_mz}-2)}.'" correction="'.(param("cond_affect_${use_channels[$index]}_m2")/100).'" />'."\n" if ($mz_xml_map{(${int_mz}-2)});
	print XMLFILE '      <affected mz="'.$mz_xml_map{(${int_mz}-1)}.'" correction="'.(param("cond_affect_${use_channels[$index]}_m1")/100).'" />'."\n" if ($mz_xml_map{(${int_mz}-1)});
	print XMLFILE '      <affected mz="'.$mz_xml_map{(${int_mz}+1)}.'" correction="'.(param("cond_affect_${use_channels[$index]}_p1")/100).'" />'."\n" if ($mz_xml_map{(${int_mz}+1)});
	print XMLFILE '      <affected mz="'.$mz_xml_map{(${int_mz}+2)}.'" correction="'.(param("cond_affect_${use_channels[$index]}_p2")/100).'" />'."\n" if ($mz_xml_map{(${int_mz}+2)});

	print XMLFILE '    </contributingMz>'."\n";
    }
    print XMLFILE '  </isotopicContributions>'."\n";

    print XMLFILE '  <massTolerance value="'.param("cond_masstol").'" />'."\n";
    print XMLFILE '  <centroiding type="'.param("cond_centroid").'" iterations="1" />'."\n";
    print XMLFILE '  <normalization type="'.param("cond_norm").'" />'."\n";
    print XMLFILE '  <reporterFromMS3 value="1" />'."\n" if param("cond_ms3");
    print XMLFILE '  <output type="1" />'."\n"; # should this ever be an option?
    print XMLFILE '  <quantitationFile name="quantitation.tsv" />'."\n";# this one should! (once Libra supports it...)
    print XMLFILE '  <minimumThreshhold value="'.param("cond_thresh").'" />'."\n";
    print XMLFILE '</SUMmOnCondition>'."\n";
    close XMLFILE;

    my $url = $xmlfile;
    $url =~ s/$www_root/\//;

    push @messages, "The file <b>$xmlfile</b> has been successfully generated. [ <a target=\"_blank\" href=\"$url\">View</a> ]";
    return 'conditionxml';
}

########################################################################
# pageRTCalc - made by Richard Stauffer (RBS), August 2010
#
# creates GUI for Retention Time Predictions page
#
########################################################################
sub pageRTCalc
{
    my $infile;
    my $outfile;
    my $modelfile;

    for (@session_data)
    {
	chomp;
	if (/$proc_types{'lastdir'}:/)
	{
	    $infile = $';  #'
	    $outfile = $'; #'
	    $modelfile = $'; #'
	    last;
	}
    }
    $outfile .= 'rtcalc.txt';

    my @sorttypes = ('None',
		     'Alphabetical', 'Reverse Alphabetical',
		     'Lowest to Highest', 'Highest to Lowest',
		     'Shortest to Longest', 'Longest to Shortest');

    #start form
    print start_form('POST', $tpp_url);

    #first tab - Choose Output Location and File Name
    print
	&printTitle(title => '1. Choose Output Location and File Name'),
	"<div class = formentry>",
	#single line textbox takes output file path
	"Output File: ",
	textfield(-name => 'rtcalc_outfile',
		  -value => $outfile,
		  -size => 80,
		  -maxlength => 140),
	"</div>\n";

    #second tab - Model File
    print
	br,
	&printTitle(title => '2. Choose Coefficients (.coeff) or Artificial Neural Networks (.ann) File (optional)'),
	"<div class = formentry>",
	textfield(-name => 'rtcalc_modelfile',
		  -value => $modelfile,
		  -size => 80,
		  -maxlength => 140),
	checkbox(-name => "rtcalc_usemodelfile",
		 -checked => 0,
		 -label => 'Use this model file instead of default coefficients file'),
	"</div>\n";

    #third tab - Peptide List
    print
	br,
	&printTitle(title => '3. Load or paste peptide list'),
	"<div class = formentry>",
	
	"Either choose an input file: ",
	textfield(-name => 'rtcalc_infile',
		  -value => $infile,
		  -size => 80,
		  -maxlength => 140),
	checkbox(-name => "rtcalc_usefile",
		 -checked => 0,
		 -label => 'Use file (instead of pasted list)'),
	br, br,

	#multiline textbox takes list of peptides
	"Or paste a list of peptides: (separate peptides with whitespace)",
	br,
	textarea(-name=>'rtcalc_peplist',
		 -rows => 20,
		 -columns => 104),
	br,
	"</div>\n",
	br;

    #fourth tab - Calculate Retention Times
    print
	&printTitle(title => '4. Calculate Retention Times'),
	"<div class = formentry>",

	#submit button
	submit(-name => 'Action',
	       -value => $web_actions{'calcRetTime'}),
	"</div>\n";

    #end form
    print end_form;
}

########################################################################
# calculateRetentionTime - made by Richard Stauffer (RBS), August 2010
#
# generates text file of input peptides with calculated retention time
#
########################################################################
sub calculateRetentionTime
{
    my $outfile = param('rtcalc_outfile');
    my $modelfile = param('rtcalc_modelfile');
    my $pepfile;
	
    #number that corresponds with model file type
    my $modelfiletype; #1 = coeff, 2 = ann, 0 = other - attempt to use as coeff

    #set up $outfile
	
    #check if output file text box is empty
    if (!$outfile) 
    {
	push @messages, "Please specify an output file!";
	Delete('rtcalc_outfile');
	return 'rtcalc';
    }
	
    my $slash = chop($outfile);
    $outfile .= $slash;
    if ($slash eq "/" || $slash eq "\\")
    {
	$outfile .= "rtcalc.txt";
	push @messages, "No file name specified, saved as rtcalc.txt.";
    }
	
    #set up $modelfile
	
    #make sure coefficients file can be used
    if (param('rtcalc_usemodelfile'))
    {
	if (!$modelfile) #check if it's empty
	{	
	    push @messages, "Please specify a model file!";
	    Delete('rtcalc_modelfile');
	    return 'rtcalc';
	}
	else
	{
	    #check if it exists
	    if (!(-e "$modelfile") || (-d "$modelfile")) #if given path doesn't exist or it is a directory
	    {
		push @messages, "The model file <b>$modelfile</b> does not exist.";
		return 'rtcalc';
	    }
	    #otherwise coefficients file should load okay
	}
		
	#check if given model file is coeff file or ann file
	my $doti = rindex($modelfile, '.');

	if ($doti != -1)
	{
	    my $tmptype = substr($modelfile, $doti);

	    if ($tmptype eq ".coeff")
	    {
		$modelfiletype = 1;
	    }
	    elsif ($tmptype eq ".ann")
	    {
		$modelfiletype = 2;
	    }
	    else
	    {
		$modelfiletype = 0;
	    }
	}
    }

    #set up $pepfile
	
    if (param('rtcalc_usefile')) #load peptide list from file
    {
	$pepfile = param('rtcalc_infile');
	if (!$pepfile)
	{
	    push @messages, "Please specify an input file!";
	    return 'rtcalc';
	}
	if(!(-e "$pepfile") || (-d "$pepfile"))
	{
	    push @messages, "The input file <b>$pepfile</b> does not exist!";
	    return 'rtcalc';
	}
    }
    else #use peptides pasted in text box
    {
	$pepfile = param('rtcalc_peplist');
	if (!$pepfile)
	{
	    push @messages, "There are no peptides listed.";
	    return 'rtcalc';
	}
		
	#print pepfile to text file then reassign pepfile to file path

	open(TXTFILE, ">$outfile.tmp") || &fatalError("Cannot create temporary file: $outfile.tmp: $!");
	print TXTFILE uc($pepfile);
	close(TXTFILE);
	$pepfile = $outfile . ".tmp";
    }
    #$pepfile should now be the file path of the peptide list

    my $command = "RTCalc.exe PEPS=$pepfile ";

    if (param('rtcalc_usemodelfile'))
    {
	if ($modelfiletype <= 1) #use model file as coefficients file
	{
	    $command .= "COEFF=$modelfile OUTFILE=$outfile";
	}
	else #use model file as artificial neural networks file
	{
	    $command .= "ANN=$modelfile OUTFILE=$outfile";
	}
    }
    else #uses default coefficients file
    {
	$command .= "OUTFILE=$outfile";
    }
	
    my @commands;
    push @commands, ($command, $outfile);

    if (!param('rtcalc_usefile')) #pepfile was pasted in
    {
	push @commands, ("$command{rm} $pepfile", $pepfile);
    }
	
    return &prepareCommands('rtcalc', @commands);
}


########################################################################
# pageRTTrain - made by Richard Stauffer (RBS), August 2010
#
# creates GUI for Retention Time Training page
#
########################################################################
sub pageRTTrain
{
    my $infile;
    my $outfile;
	
    #radio button labels
    my %rttrain_radlab =
	(
	 'coeff' => 'Coefficients ',
	 'ann' => 'Artificial Neural Network'
	 );

    for (@session_data)
    {
	chomp;
	if (/$proc_types{'lastdir'}:/)
	{
	    $infile = $';  #'
	    $outfile = $';  #'
	    last;
	}
    }
    $outfile .= 'rtcalc.coeff';
	
    # start form
    print start_form('POST', $tpp_url);

    # first tab - Choose Output Location and File Name
    print
	&printTitle(title => '1. Choose Model Output Location and File Name'),
	"<div class = formentry>",
	"Output file type: ",
	radio_group(-name => 'rttrain_outtype',
		    -values => ['coeff', 'ann'],
		    -default => 'coeff',
		    -labels => \%rttrain_radlab,
		    -onclick => 'switchmodeltype(this.value, "rttrain_outfile_id")'),
	br, br,
	# single line textbox takes output file path
	"Output file: ",
	textfield(-name => 'rttrain_outfile',
		  -id => 'rttrain_outfile_id',
		  -value => $outfile,
		  -size => 80,
		  -maxlength => 140),
	"</div>\n";
	
    # second tab - Peptide List
    print
	br,
	&printTitle(title => '2. Load or paste training file'),
	"<div class = formentry>",
	
	"Either choose input training file: ",
	textfield(-name => 'rttrain_infile',
		  -value => $infile,
		  -size => 80,
		  -maxlength => 140),
	checkbox(-name => "rttrain_usefile",
		 -checked => 0,
		 -label => 'Use File (instead of pasted text)'),
	br, br,
	
	# multiline textbox takes list of peptides
	"Or paste training file:",
	br,
	textarea(-name=>'rttrain_peplist',
		 -rows => 20,
		 -columns => 104),
	br,
	"</div>\n",
	br;
    
    # third tab - Calculate Retention Times
    print
	&printTitle(title => '3. Create Model File'),
	"<div class = formentry>",
	
	# submit button
	submit(-name => 'Action',
	       -value => $web_actions{'trainRetTime'}),
	"</div>\n";

    # end form
    print end_form;
}

########################################################################
# trainRetentionTime - made by Richard Stauffer (RBS), August 2010
#
# generates RTCalc coefficients or artificial neural networks file of input training file
#
########################################################################
sub trainRetentionTime
{
    my $outfile = param('rttrain_outfile');
    my $trainfile;

    my $createcoeff = param('rttrain_outtype') eq 'coeff'; #generate coefficients file as opposed to ann file
	
    #set up $outfile
	
    #check if output file text box is empty
    if (!$outfile) 
    {
	push @messages, "Please specify an output file!";
	Delete('rttrain_outfile');
	return 'rttrain';
    }

    my $slash = chop($outfile);
    $outfile .= $slash;
    if ($slash eq "/" || $slash eq "\\")
    {
	if ($createcoeff)
	{
	    $outfile .= "rtcalc.coeff";
	    push @messages, "No file name specified, saved as rtcalc.coeff.";
	}
	else
	{
	    $outfile .= "rtcalc.ann";
	    push @messages, "No file name specified, saved as rtcalc.ann.";
	}
    }
	
    #set up $trainfile
    
    if (param('rttrain_usefile')) #load peptide list from file
    {
	$trainfile = param('rttrain_infile');
	if (!$trainfile)
	{
	    push @messages, "Please specify a training file!";
	    return 'rttrain';
	}
	if(!(-e "$trainfile") || (-d "$trainfile"))
	{
	    push @messages, "The training file <b>$trainfile</b> does not exist!";
	    return 'rttrain';
	}
    }
    else #use training file pasted into text box
    {
	$trainfile = param('rttrain_peplist');
	if (!$trainfile)
	{
	    push @messages, "The text box is empty.";
	    return 'rttrain';
	}
	
	#print trainfile to text file then reassign trainfile to file path
	
	open(TXTFILE, ">$outfile.tmp") || &fatalError("Cannot create temporary file: $outfile.tmp: $!");
	print TXTFILE uc($trainfile);
	close(TXTFILE);
	
	$trainfile = $outfile . ".tmp";
    }
    #$trainfile should now be the file path of the peptide list
    
    my $command ="RTCalc.exe TRAIN=$trainfile ";
    
    if ($createcoeff)
    {
	$command .= "COEFF=$outfile";
    }
    else
    {
	$command .= "ANN=$outfile";
    }
    
    my @commands;
    push @commands, ($command, $outfile);
    
    if (!param('rttrain_usefile'))
    {
	push @commands, ("$command{rm} $trainfile", "$trainfile");
    }
    
    return &prepareCommands('rttrain', @commands);
}


########################################################################
# filterMascotFile
#
# filter file in tpp from mascot according to Mascot JobId, Mascot user Name,
# Mascot database, Mascot title
# easy to add one or more filters from Mascot search logs page
#
# Contributed by Chris Dantec
#
########################################################################
sub filterMascotFile {
    if (param('mascot_job')|| param('mascot_db')|| param('mascot_ti') || param('mascot_name')){
	push @messages, "You have filtered data on :";
	push @messages, " Mascot Job Id = ".param('mascot_job') if (param('mascot_job'));
	push @messages, " Mascot Database = ".param('mascot_db') if (param('mascot_db'));
	push @messages, " Mascot Request Title = ".param('mascot_ti') if (param('mascot_ti'));
	push @messages, " Mascot User Name = ".param('mascot_name') if (param('mascot_name'));
    }
    return 'runmascot';
}


########################################################################
# fetchMascotResultFile
#
# use wget to retrieve Mascot result file from Mascot server
#
########################################################################
sub fetchMascotResultFile {

    if (!param('mascotsrcdat')) {
	push @messages, "Please select a .dat file to transfer.";
	return 'runmascot';
    } elsif (param('mascotsrcdat') !~ /^\?/) {
	&fatalError('BADQUERY',"Query string is malformed:".param('mascotsrcdat')."\n");
    }

    # Build command
    #
    # wget -q -O DIR/FILE URL
    my $fdir = param('workdir');
    my $url  = "${mascot_server}x-cgi/ms-status.exe".param('mascotsrcdat');

    param('mascotsrcdat') =~ /ResJob=/;
    my $out_file = $fdir.$'; #'

    my $command = "$command{wget} -q -O $out_file \"$url\"";
#    my $command = "$command{wget} --http-user=mascotLogin --http-password=mascotPassword -q -O $out_file \"$url\""; # LM: use this if you need to use a password to your Mascot server; fill in as required

    my @commands;
    push @commands, ($command, $out_file);

    if (param('mascotrename')) {
	$command = "$command{chdir} $fdir". $command_sep ." $command{renamedat} $out_file";
	push @commands, ($command, $fdir);
    }

    # Execute commands!
    return &prepareCommands('runmascot',@commands);

}


########################################################################
# isTPPXmlFile
#
# determine xml type from extension or by examining first 5 lines of file
#
# return:  mz / pep / prot / params / unk(nown) / err (could not open)
#
########################################################################
sub isTPPXmlFile {
    my $file_path = shift;
    my $rc = 'unk';
    my $lc = 0;

    return 'mz' if $file_path =~ /\.mzxml|mzdata|mzml(\.gz)?$/i;

    # handle possibly gzipped pepXML
    my $tmpxmlfile = $file_path =~ /\.gz$/ ? tpplib_perl::uncompress_to_tmpfile($file_path,100000) : $file_path; # decompress .gz if needed, limit to first 100KB
    open(FILE, "$tmpxmlfile") || return 'err';

    while (<FILE>){
	if (/msms_pipeline_analysis/) {
	    # only consider versions 1.XXX for now
	    $rc = 'pep' if ($_ =~ /pepXML_v1.*\.xsd/);
            # also consider dtd (omssa)
	    $rc = 'pep' if ($_ =~ /DOCTYPE.*pepXML\.dtd/);
	    last;
	}
	elsif (/protein_summary/) {
	    $rc = 'prot' if ($_ =~ /protXML.*\.xsd/);
	}
	elsif (/<bioml>/) {
	    $rc = 'params';
	}
	last if ($lc++ > 5);
    }
    close(FILE);

    if ( ( $in_windows && lc($tmpxmlfile) ne lc($file_path) ) ||
	 ( !$in_windows && $tmpxmlfile ne $file_path) ) {
	unlink($tmpxmlfile);
    }	 # did we decompress xml.gz?

    return $rc;
}


########################################################################
# validateInput
#
# Perform general user input validation
#
# Validation types:  integer, float, file_exists, dir_exists, not_blank
#                  ToDo: [range:min,max]
########################################################################
sub validateInput {
    my $val_type = shift;
    my $val_value = shift;
    my $val_string = shift || "that field.";


    if ($val_type eq 'not_blank') {
	if (!$val_value) {
	    push @messages, "Please enter a value for $val_string";
	    $errors++;
	}
    }

    if ($val_type eq 'no_spaces') {
	if ($val_value =~ /\s+/) {
	    push @messages, "Please make sure that there are no spaces in the value for $val_string";
	    $errors++;
	}
    }

   if ($val_type eq 'integer') {
	if ($val_value !~ /^\d+$/) {
	    push @messages, "Please enter a numeric (integer) value for $val_string";
	    $errors++;
	}
    }

    if ($val_type eq 'float') {
	if ($val_value !~ /^-?\d+\.?\d*$/) {
	    push @messages, "Please enter a numeric (float) value for $val_string";
	    $errors++;
	}
    }


    if ($val_type eq 'extraopts') {  # do not allow command-splitting; add more validation rules?
	if ($val_value =~ /;/) {
	    push @messages, "You cannot use semicolons within $val_string";
	    $errors++;
	}
    }


    if ($val_type eq 'file_exists') {
	if (!-e $val_value) {
	    push @messages, "Please enter a valid file name for $val_string";
	    $errors++;
	}
    }

    if ($val_type eq 'dir_exists') {
	if (!-d $val_value) {
	    push @messages, "Please enter a valid directory/folder name for $val_string";
	    $errors++;
	}
    }

    return;

}


########################################################################
# openHTMLPage
#
# Open page
#
########################################################################
sub openHTMLPage {
    my $title = shift;
    my $page = shift || 'unknown';

    if (@header_params) {
	for my $hp (@header_params) {
	    print "$hp\n";
	}
    }

    print header();

    my $check_version = ($ck_version > 1); # 1 for the user setting, plus 1 when logging-in is detected (via setSession)

    my $onLoad = ($check_version) ? 'checkVersion(); ' : '';
    $onLoad .= ($page eq 'login') ? '' : 'getStatus();';

    print start_html(-title=>"$title",
		     -author=>'Institute for Systems Biology',
		     -encoding=>'UTF-8',
		     -dtd=>'HTML 4.0 Transitional',
		     -style=>{'src'=>'./tpp_gui.css'},
		     -head=>['<link rel="icon" href="./images/petunia.ico" type="image/x-ion">'],
		     -bgcolor=>"#c0c0c0",
		     -onLoad=>"$onLoad"
		     );

#1d3887",
#c0c0c0",


    print <<"EOSCRIPT";
    <SCRIPT LANGUAGE=JavaScript>
    var c_element;
    var c_element_sh;

    function showmenu(elmnt){   
        document.getElementById(elmnt).style.visibility="visible";
    }

    function hidemenu(elmnt){
        document.getElementById(elmnt).style.visibility="hidden";
    }

    function showhide(elementId){
	c_element = document.getElementById(elementId);
	c_element_sh = document.getElementById(elementId + "_sh");

	if(c_element.className != "hideit"){
	    c_element.className = "hideit";
	    c_element_sh.innerHTML = "&nbsp;&nbsp;&nbsp;Show&nbsp;[&nbsp;+&nbsp;]";
	} else {
	    if (document.getElementById(elementId + "_head")) {
		elementClass = document.getElementById(elementId + "_head").className;
		elementClass = elementClass.replace(/head/,"");
	    } else {
		elementClass = 'formentry';
	    }
	    c_element.className = elementClass;
	    c_element_sh.innerHTML = "&nbsp;&nbsp;&nbsp;Hide&nbsp;[&nbsp;-&nbsp;]";
	}
    }

    function hilight(elementId){
	c_element = document.getElementById(elementId);
	c_element_sh = document.getElementById(elementId + "_cb");

	if(c_element_sh.checked){
	    c_element.className = "remfileSelected";
	} else {
	    c_element.className = "remfile";
	}
    }

    function toggleAll(checkAllName, elementName, rowBaseId){
	var aCheckBoxes = document.getElementsByName(elementName);
	var bChecked = document.getElementById(checkAllName).checked;

	for(var i=0; i<aCheckBoxes.length; i++) {
	    var grandparent = aCheckBoxes[i].parentNode.parentNode;
	    var greatgrandparent = grandparent.parentNode; // support for older perl CGI modules, where <label> is the parent of <input>
	    if ( grandparent.style.display != 'none' && greatgrandparent.style.display != 'none' ) {  // only toggle visible checkboxes

		aCheckBoxes[i].checked = bChecked;

		if (rowBaseId) { hilight(rowBaseId+i); }

	    }
	}
    }

    // All-purpose wrapper for XMLHttpRequest call
    var http_req = new Array();  // 0:jobs count updater; 1:jobs page updater; 2:version check; 3:session info retrieval

    function executeXHR(index, callback, url) {
	// branch for native XMLHttpRequest object
	if (window.XMLHttpRequest) {
	    http_req[index] = new XMLHttpRequest();
	    http_req[index].onreadystatechange = callback;
	    http_req[index].open("GET", url, true);
	    http_req[index].send(null);
	} // branch for IE/Windows ActiveX version
	else if (window.ActiveXObject) {
	    http_req[index] = new ActiveXObject("Microsoft.XMLHTTP");
	    if (http_req[index]) {
		http_req[index].onreadystatechange = callback;
		http_req[index].open("GET", url, true);
		http_req[index].send();
	    }
	}

	// warn if we cannot create this object    FIXME
	if (!http_req[index]) {

	}
    }

    </SCRIPT>

EOSCRIPT

    # include AJAX code for monitoring command execution
    if ($page eq 'jobs') {
	print <<"EOSCRIPT";
	<script type="text/javascript">	
	    var firsttime = 1;

	    // Retrieve status of commands executing on server
	    function getStatus() {
		var url = "$tpp_url?Action=AJAXCheckStatusUpdates&rand="+Math.random();
		var callback = processStatusRequest;
		executeXHR(1, callback, url);
	    }
	    // Process server status response
	    function processStatusRequest() {
		var tt = 0; // for timeout
		var t;  // idem
		var n = 0;
		if (http_req[1].readyState == 4) {
		    if (http_req[1].status == 200) {
			xmlresponse  = http_req[1].responseXML.documentElement;
			var c = xmlresponse.getElementsByTagName('command');

			for (i=0;i<c.length;i++) {
			    var rcmid = c[i].getElementsByTagName('cid')[0].firstChild.data;
			    var rstat = c[i].getElementsByTagName('status')[0].firstChild.data;

			    if (document.getElementById(rcmid + '_status')) {

				if (document.getElementById(rcmid + '_status').className != rstat) {
				    document.getElementById(rcmid + '_status').className = rstat;
				    document.getElementById(rcmid + '_status').innerHTML = "<b>* " + rstat + "</b>";
				}
				if (rstat != 'viewed') { if (rstat != 'killed') { tt++;} }

			    } else {
				n++;
			    }
			}

			var rdate = xmlresponse.getElementsByTagName('date')[0].firstChild.data;
			var rmsg  = xmlresponse.getElementsByTagName('message')[0].firstChild.data;

			var htmlstat = "Status as of: " + rdate + "<br/>";
			if (rmsg.length > 2) { htmlstat += "<li>" + rmsg + "</li>"; }
			if (n > 0)           { htmlstat += "<li>" + n + " new commands have been launched! Please refresh this page to view them.</li>"; }
			if (tt == 0)         { htmlstat += "<li>Output from all commands has been viewed; auto-refresh is now <b>OFF</b>.</li>"; }

			// cheap way to blink
			var btime = 0;
			var bint  = 250;
			for (b=0;b<3*(1-firsttime);b++) {
			    setTimeout(function(){document.getElementById('cmdstatus_info').innerHTML='<b style="color:#ff8820"> Updating... </b>';},btime);
			    btime += bint;
			    setTimeout(function(){document.getElementById('cmdstatus_info').innerHTML='&nbsp;';},btime);
			    btime += bint;
			}
			setTimeout(function(){document.getElementById('cmdstatus_info').innerHTML=htmlstat;},btime);
			firsttime = 0;

		    } else {
			document.getElementById('cmdstatus_info').innerHTML = "<b>There was a problem retrieving the XML data: " + http_req[1].statusText + "</b>";
		    }

		    if (tt == 0) {
			clearTimeout(t);
		    } else {
			// loop loop loop ...
			t = setTimeout("getStatus()", 10000);
		    }
		}
	    }

	</script>

EOSCRIPT

    } elsif ($page ne 'login') {

	print <<"EOSCRIPT";
	<script type="text/javascript">
	
	    // Retrieve status of commands executing on server
	    function getStatus() {
		var url = "$tpp_url?Action=AJAXCheckStatus&rand="+Math.random();
		var callback = processStatusRequest;
		executeXHR(0, callback, url);
	    }
	    // Process server status response
	    var wasrunning = 0;
	    function processStatusRequest() {
		var tt = 0; // for timeout
		var t;  // idem
		var htmlstat = "Jobs ";
		var htmltooltip = '';
		var pagetitle = "$title";
		var numrunning = 0;

		if (http_req[0].readyState == 4) {
		    if (http_req[0].status == 200) {
			xmlresponse  = http_req[0].responseXML.documentElement;
			var c = xmlresponse.getElementsByTagName('status_summary');

			for (i=0;i<c.length;i++) {
			    var rstat = c[i].getElementsByTagName('status')[0].firstChild.data;
			    var rscnt = c[i].getElementsByTagName('count')[0].firstChild.data;

			    if (rstat == 'running') {
				numrunning = rscnt;
				wasrunning = 1;

                            } else {
				htmlstat += "<span class='" + rstat + "'>&nbsp;"+ rscnt + "&nbsp;</span>";
			    }

			    htmltooltip += "(" + rscnt + " jobs " + rstat + ") ";

			    if (rstat != 'viewed') { tt++;}
			}

			if (wasrunning == 1) {
			    htmlstat += "<span class='running'>&nbsp;" + numrunning + "&nbsp;</span>";
			    document.title = "(" + numrunning + ") " + pagetitle;
			}

			var rdate = xmlresponse.getElementsByTagName('date')[0].firstChild.data;
			var rmsg  = xmlresponse.getElementsByTagName('message')[0].firstChild.data;

			if (htmltooltip.length < 2) { htmltooltip = "(no jobs) "; }

			htmltooltip += " -- as of: " + rdate;

			if (rmsg.length > 2) { htmlstat += " ***"; htmltooltip += rmsg; }

			document.getElementById('title_Jobs').style.textDecoration = 'none';
			document.getElementById('title_Jobs').innerHTML = htmlstat;
			document.getElementById('title_Jobs').title = htmltooltip;

		    } else {
			document.getElementById('title_Jobs').innerHTML = "(!)";
			document.getElementById('title_Jobs').title = "There was a problem retrieving the XML data: " + http_req[0].statusText;
		    }

		    if (tt == 0) {
			clearTimeout(t);
		    } else {
			// loop loop loop ...
			t = setTimeout("getStatus()", 5000);
		    }
		}
	    }

	</script>
EOSCRIPT

    }

    if ($check_version) {
	print <<"EOSCRIPT";
	<script type="text/javascript">
	
	    // Retrieve current version of TPP
	    function checkVersion() {
		var url = "$tpp_url?Action=AJAXCheckTPPVersion&rand="+Math.random();
		var callback = processVersionRequest;
		executeXHR(2, callback, url);
	    }
	    // Process server status response
	    function processVersionRequest() {
		if (http_req[2].readyState == 4) {
		    if (http_req[2].status == 200) { // ignore bad/invalid responses
			document.getElementById('messages').innerHTML += http_req[2].responseText;
		    }
		}
	    }

	</script>
EOSCRIPT

    }

    print h1($title);

    if ($auth_user) {

	# navigation tabs and menus
	my $page_list = "";
	my @sections = (
			{ name => 'Home',              pages=> 'filebrowser,account,chargefile,mzxml2search,msconvert,decoyvalpeps,respect,spectrastlib,jobs'},
			{ name => 'Files',             pages=> 'filebrowser,updatepaths,idconvert'},
			{ name => 'Account',           pages=> 'account,sessions,clusters'},
			{ name => 'Pre-Process',       pages=> 'chargefile,mergecharges'},
			{ name => 'mzXML Utils',       pages=> 'mzxml2search,mzxml,dta2mzxml,indexmzxml'},
			{ name => 'Analysis Pipeline', pages=> 'msconvert,runsearch,topepxml,xinteract,iprophet,ptmprophet,runprophet'},
			{ name => 'Decoy',             pages=> 'decoyvalpeps,decoyvalprots,decoyfasta,mayu'},
			{ name => 'Utilities',         pages=> 'respect,qualscore,compareprots,conditionxml,dna2aa,rtcalc,rttrain'},
			{ name => 'SpectraST Tools',   pages=> 'spectrastlib,getspeclibs,lib2html,marimba'},
			{ name => 'Jobs',              pages=> 'jobs'}
			);
	if ($pipeline eq 'Mascot') {
	    $sections[5] = { name => 'Analysis Pipeline', pages=> 'msconvert,runmascot,topepxml,xinteract,iprophet,ptmprophet,runprophet'};
	} elsif ($pipeline eq 'SpectraST') {
	    $sections[5] = { name => 'Analysis Pipeline', pages=> 'msconvert,runspectrast,xinteract,iprophet,ptmprophet,runprophet'};
	} elsif ($pipeline eq 'Tandem') {
	    $sections[5] = { name => 'Analysis Pipeline', pages=> 'msconvert,runtandem,topepxml,xinteract,iprophet,ptmprophet,runprophet'};
	} elsif ($pipeline eq 'Comet') {
	    $sections[5] = { name => 'Analysis Pipeline', pages=> 'msconvert,runcomet,xinteract,iprophet,ptmprophet,runprophet'};
	}

	my %tabs = (
		    home          => ['Home',''],
		    account       => ['Manage My Account','Change your Petunia password'],
		    sessions      => ['Manage Session(s)','Delete and view old session files'],
		    clusters      => ['Amazon Cloud','Manage Amazon Cloud Credentials and Check Status'],
		    jobs          => ['Jobs','View and Monitor Jobs Status and Output'],
		    chargefile    => ['Create Charge Files',''],
		    mergecharges  => ['Merge Charges',''],
		    mzxml         => ['mzXML (legacy)','Convert instrument files to mzXML using a legacy converter'],
		    idconvert     => ['mzIdent','Use idconvert to convert TPP protXML files to mzIdentML'],
		    msconvert     => ['mzML/mzXML','Use msconvert to convert instrument files to open formats, including mzML'],
		    runsearch     => ['Database Search','Run Sequest'],
		    runmascot     => ['Database Search','Transfer a Mascot-searched results file to this machine for analysis by the TPP'],
		    runtandem     => ['Database Search','Run X!Tandem'],
		    runcomet      => ['Database Search','Run Comet'],
		    runspectrast  => ['SpectraST Search','Run SpectraST'],
		    topepxml      => ['pepXML','Convert search results to pepXML for processing by the TPP'],
		    xinteract     => ['Analyze Peptides','Interface to xinteract'],
		    iprophet      => ['Combine Analyses','Run iProphet'],
		    ptmprophet    => ['Analyze PTMs','Validate Post-Translational Modification sites'],
		    runprophet    => ['Analyze Proteins','Run ProteinProphet'],
		    filebrowser   => ['Browse Files','Find and view your files!'],
		    mzxml2search  => ['Convert mz[X]ML Files','Convert mzXML/mzML files to other open formats'],
		    dta2mzxml     => ['dta to mzXML','Create an mzXML file from a directory of .dta files'],
		    indexmzxml    => ['Index mzXML Files','(Re-)index mzXML files'],
		    updatepaths   => ['Update Paths','Update file paths from a TPP experiment run on a different location'],
		    decoyfasta    => ['Decoy Databases','Generate databases for use in target-decoy search strategies'],
		    decoyvalpeps  => ['Decoy Peptide Validation',''],
		    decoyvalprots => ['Decoy Protein Validation',''],
		    mayu          => ['Mayu','Determine peptide and/or protein FDRs using Mayu'],
		    dna2aa        => ['DNA to AA','Generate an aminoacid database from a genomic database via 6-frame translation'],
		    conditionxml  => ['Libra Conditions','Generate a Libra condition file'],
		    rtcalc        => ['RT Prediction','Retention Time Prediction tool'],
		    rttrain       => ['RT Training','Retention Time Training tool'],
		    spectrastlib  => ['SpectraST Library Import','Generate and manipulate spectral libraries'],
		    getspeclibs   => ['Download Spectral Libraries','...from the PeptideAtlas Spectrum Library Central'],
		    lib2html      => ['Lib2HTML','Convert SpectraST libs to user-browsable HTML'],
		    marimba       => ['MaRiMba','Create transition lists for S/MRM experiments using spectral libraries'],
		    qualscore     => ['QualScore','Extract high-quality, unassigned spectra from a TPP experiment'],
		    respect       => ['reSpect','Extract potentially chimeric spectra from a TPP experiment'],
		    compareprots  => ['Compare Proteins','Compare ProteinProphet Results'],
		    );

	print "<table width='100%'><tr><td>\n";
	print "<table><tr>\n";
	my $sect_ct = 0;
	foreach my $section (@sections) {
	    my $html_class = ( $section->{pages} =~ /\b$page\b/ ) ? 'black' : 'gray';
	    my $link_to = (split /,/, $section->{pages})[0];
	    my $sect_name = $section->{name};

	    $sect_name .= " ($pipeline)" if ($sect_name eq "Analysis Pipeline");
	    $sect_ct++;

	    if ($sect_name eq 'Home') {  #special case...
		$html_class = ($page eq 'home') ? 'black' : 'gray';
		$link_to = 'home';
	    } else {
		print "<td>|</td>\n";  # separator
	    }

	    if ($html_class eq 'black') {
		$page_list = $section->{pages};
		print "<td class=\"$html_class\">$sect_name</td>\n";
	    } else {
		if ( ($sect_name eq 'Home') || ($sect_name eq 'Jobs') ) {  #special cases...
		    print
			"<td>",
			"<b><a href=\"$tpp_url?Action=$web_actions{'showpage'}&page=$link_to\" id=\"title_$sect_name\" class=\"$html_class\">",
			"$sect_name</a></b>";

		} else {
		    print
			"<td onmouseover=\"showmenu('menu$sect_ct')\" onmouseout=\"hidemenu('menu$sect_ct')\">",
			"<b><a href=\"$tpp_url?Action=$web_actions{'showpage'}&page=$link_to\" class=\"$html_class\">",
			"$sect_name</a></b><br />\n",
			"<table class=\"menu\" id=\"menu$sect_ct\" cellspacing=\"0\" width=\"230\">\n";

		    foreach my $menu (split /,/, $section->{pages}) {
			print
			    "<tr><td class=\"menu\">",
			    "<a class=\"menuitem\" title='$tabs{$menu}[1]' href=\"$tpp_url?Action=$web_actions{'showpage'}&page=$menu\">",
			    "&nbsp;&nbsp;$tabs{$menu}[0]</a></td></tr>\n";
		    }
		    print "</table>\n";
		}
		print "</td>\n";
	    }
	}
	print "</tr></table></td>\n";

	print
	    '<td align="right">',
	    start_form('POST',$tpp_url),
	    "You are logged in as <b>$auth_user</b>.",
	    submit(-name=>'Action',
		   -value=>$web_actions{'logout'}),
	    end_form,
	    "</td>\n";

	print "</tr></table>\n";


	if ($page eq 'home') {  #special case...
	    $tabs{filebrowser} = ['FILES'];
	    $tabs{account}     = ['ACCOUNT'];
	    $tabs{chargefile}  = ['PRE-PROCESS'];
	    $tabs{mzxml2search}= ['mzXML UTILS'];
	    $tabs{msconvert}   = ['ANALYSIS PIPELINE'];
	    $tabs{decoyvalpeps}= ['DECOY'];
	    $tabs{respect}     = ['UTILITIES'];
	    $tabs{spectrastlib}= ['SPECTRAST TOOLS'];
	    $tabs{jobs}        = ['JOBS'];
	}
	print << "EONAV";
	   <table cellpadding="3" cellspacing="0">
	   <tr>
EONAV

	foreach my $tab (split /,/, "home,$page_list") {
	    my $html_class = ($page eq $tab) ? "navselected" : "nav";

	    print
		"<td class=$html_class>",
		a({-href => "$tpp_url?Action=$web_actions{'showpage'}&page=$tab",
		   -title=> $tabs{$tab}[1]},
		  "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;$tabs{$tab}[0]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
		  ),
		  "</td>";
	}
	print "</tr></table>\n";

    }

    #open div for user input area
    print "\n\n<div class=level1>\n";

    # div for commands status info display
    if ($page eq 'jobs') {
	print
	    &printTitle(title  => ' Commands Status ',
			div_id => 'cmdstatus_info'),
	    "<div id=cmdstatus_info class=formentry>\n",
	    " -- checking ... --",
	    "</div>",
	    br;
    }

    # print messages to user
    if (@messages) {
	print
	    &printTitle(title  => ' Messages ',
			class  => 'messageshead',
			div_id => 'messages'),
	    "<div id=messages class=messages>\n<ul>";
	for my $msg (@messages) {
	    if ($msg =~ /DIALOG:\[(.+)\]\[(.+)\]/) {
		&printToLog("I found a dialog!\n  ok    : $1\n  cancel: $2\n") if ($debug);

		my ($ok_action,$ok_fields) = split /\|/, $1;
		my ($cancel_action,$cancel_fields) = split /\|/, $2;

		print
		    "</ul>",
		    start_form('POST',$tpp_url);

		foreach my $hidden (split /,/, $ok_fields) {
		    print
			hidden(-name=>$hidden);
		}
		print
		    submit(-name=>'Action',
			   -value=>$web_actions{$ok_action}),
		    end_form,
		    start_form('POST',$tpp_url);

		foreach my $hidden (split /,/, $cancel_fields) {
		    print
			hidden(-name=>$hidden);
		}
		print
		    submit(-name=>'Action',
			   -value=>$web_actions{$cancel_action}),
		    end_form,
		    "<ul>";

	    } else {
		print "<li>$msg</li>" if ($msg);
	    }

	}
	print "</ul></div><br>\n\n";
    }

}


########################################################################
# printTitle
#
# Returns a string with the formatted title for a sub-section
#
########################################################################
sub printTitle {
    my %args = @_;
    my $string = $args{title};
    my $class  = $args{class} || 'formentryhead';
    my $div_id = $args{div_id} || '';

    my $ret_string = "<table cellspacing=0>\n<tr>\n";

    if ($div_id) {
	$ret_string .=
	    "<td id=${div_id}_head class=$class>&nbsp;$string&nbsp;&nbsp;&nbsp;</td>\n".
	    "<td><a id=${div_id}_sh onclick=\"showhide('$div_id')\">&nbsp;&nbsp;&nbsp;[&nbsp;Show / Hide&nbsp;]</a></td>";
    } else {
	$ret_string .= "<td class=$class>&nbsp;$string&nbsp;&nbsp;&nbsp;</td>\n";
    }

    $ret_string .= "</tr>\n</table>\n\n";

    return $ret_string;
}


########################################################################
# closeHTMLPage
#
# Close page
#
########################################################################
sub closeHTMLPage {

    print <<"EOF_PAGE";
<br>
</div> <!-- tool div -->
<br><br>

<table width="100%" border="0">
<tr>
 <td align="left">
 <a href="http://www.systemsbiology.org/" target="_blank" title="ISB website">
 <img border="0" src="images/isb_logo.gif"/>
 </a>

 <a href="http://www.proteomecenter.org/" target="_blank" title="Seattle Proteome Center at ISB">
 <img border="0" src="images/spc_logo.png"/>
 </a>
 </td>

 <td align="right">
 <a href="http://www.nigms.nih.gov/" target="_blank" title="National Institute of General Medical Sciences website">
 <img border="0" src="images/nigms.jpg"/></a>
 </td>
</tr>
</table>
<br>

<div class="gray"><b>$TPPVersionInfo</b></div>

</body>
</html>

EOF_PAGE

    return 1;
}


########################################################################
# pageLogin
#
# The login page
#
########################################################################
sub pageLogin {

    return if ( $useBasicAuth );

    # reset password
    param('password',"");

    print start_form('POST',$tpp_url);

    print "<table>\n<tr><td>User Name:</td><td>";
    print textfield(-name=>'username',
		    -title=>'default username=guest',
		    -default =>'guest',
		    -size=>20,
		    -maxlength=>20);
    print "</td></tr>\n<tr><td>Password:</td><td>\n";
    print password_field(-name=>'password',
			 -title=>'default password=guest',
			 -size=>20,
			 -maxlength=>20);
    print "</td></tr>\n<tr><td>&nbsp;</td><td>";
    print submit(-name=>'Action',
		 -value=>$web_actions{'login'});
    print "</td></tr>\n</table>\n\n";
    print end_form;
}


########################################################################
# pageHome
#
# The home page
#
########################################################################
sub pageHome {

    Delete('Action');

    print
	h2('Welcome'),
	p(
	  'Welcome to the Trans-Proteomic Pipeline (TPP) web interface.',
	  'These tools and interfaces were developed and are being maintained at the',
	  a({href=>'http://www.systemsbiology.org'},'Institute for Systems Biology'),
	  '(ISB) under a grant from',
	  a({href=>'http://www.nigms.nih.gov'},'NIGMS.'),
	  'Please visit',
	  a({href=>'http://www.proteomecenter.org'},'www.proteomecenter.org'),
	  'and',
	  a({href=>'http://tools.proteomecenter.org'},'tools.proteomecenter.org'),
	  'for more information.'),
	p(
	  start_form(-method => 'POST',
		     -action => $tpp_url,
		     -name => 'switchPipeline'),
	  b('Please select analysis pipeline you want to use: '),
	  hidden(-name=>'Action',
		 -value=>$web_actions{'switchPipeline'}),
	  popup_menu(-name=>'pipeline_type',
		     -values => [qw'Comet Mascot Sequest SpectraST Tandem'],
		     -default => $pipeline,
		     -onChange => 'document.forms.switchPipeline.submit();'),
	  end_form),
	br;

    print
	&printTitle(title=>'Analysis Pipeline'),
	"<div class=formentry>",
	p('Follow these steps to convert, search, and analyze your data:');

    if ($pipeline eq 'Mascot') {
	print
	    dl(
	       dt('1. RAW to mzML Conversion'),
	       dd('Convert original .RAW files to the standard mzML input format used by the tools'),
	       dt('2. mzML to mgf Conversion'),
	       dd('Convert mzML file to format that Mascot can search'),
	       dt('3. Peptide Database Search and Identification'),
	       dd('Upload file to Mascot server for searching.'),
	       dd('Transfer the .dat output file once the search is finished.'),
	       dt('4. Conversion to pepXML'),
	       dd('Convert original search results to the pepXML input format used by xinteract'),
	       dt('5. Data Curation and (optional) Peptide validation and Quantification'),
	       dd('Use Xinteract to filter, sort, group, and highlight data based on various criteria. You can also validate peptide identifications using PeptideProphet and/or use ASAPRatio, XPRESS, or Libra to calculate the relative abundances of proteins and the corresponding confidence intervals from ICAT-type ESI-LC/MS or iTRAQ data.'),
	       dt('6. Protein Assignment and Validation'),
	       dd('ProteinProphet provides a statistical model for validation of peptide identifications at the protein level '),
	       );

    } elsif ($pipeline eq 'Tandem') {
	print
	    dl(
	       dt('1. RAW to mzML Conversion'),
	       dd('Convert original .RAW files to the standard mzML input format used by the tools'),
	       dt('2. Edit Tandem Default Parameters, if necessary'),
	       dd('It is recommended that you copy the default input file to your local directory, and edit it as necessary'),
	       dt('3. Peptide Database Search and Identification'),
	       dd('Perform X-Tandem search.'),
	       dt('4. Conversion to pepXML'),
	       dd('Convert original search results to the pepXML input format used by xinteract'),
	       dt('5. Data Curation and (optional) Peptide validation and Quantification'),
	       dd('Use Xinteract to filter, sort, group, and highlight data based on various criteria. You can also validate peptide identifications using PeptideProphet and/or use ASAPRatio, XPRESS, or Libra to calculate the relative abundances of proteins and the corresponding confidence intervals from ICAT-type ESI-LC/MS or iTRAQ data.'),
	       dt('6. Protein Assignment and Validation'),
	       dd('ProteinProphet provides a statistical model for validation of peptide identifications at the protein level '),
	       );

    } elsif ($pipeline eq 'SpectraST') {
	print
	    dl(
	       dt('1. RAW to mzML Conversion'),
	       dd('Convert original .RAW files to the standard mzML input format used by the tools'),
	       dt('2. Peptide Library Search and Identification'),
	       dd('Query spectra are searched against a spectral library for peptide identifications using SpectraST.'),
	       dt('3. Data Curation and (optional) Peptide validation and Quantification'),
	       dd('Use Xinteract to filter, sort, group, and highlight data based on various criteria. You can also validate peptide identifications using PeptideProphet and/or use ASAPRatio or XPRESS to calculate the relative abundances of proteins and the corresponding confidence intervals from ICAT-type ESI-LC/MS data.'),
	       dt('4. Protein Assignment and Validation'),
	       dd('ProteinProphet provides a statistical model for validation of peptide identifications at the protein level '),
	       );

    } else {
	print
	    dl(
	       dt('1. RAW to mzML Conversion'),
	       dd('Convert original .RAW files to the standard mzML input format used by the tools'),
	       dt('2. Peptide Database Search and Identification'),
	       dd('This is a front-end to Sequest (runsearch)'),
	       dt('3. Conversion to pepXML'),
	       dd('Convert original search results to the pepXML input format used by xinteract'),
	       dt('4. Data Curation and (optional) Peptide validation and Quantification'),
	       dd('Use Xinteract to filter, sort, group, and highlight data based on various criteria. You can also validate peptide identifications using PeptideProphet and/or use ASAPRatio or XPRESS to calculate the relative abundances of proteins and the corresponding confidence intervals from ICAT-type ESI-LC/MS data.'),
	       dt('5. Protein Assignment and Validation'),
	       dd('ProteinProphet provides a statistical model for validation of peptide identifications at the protein level '),
	       );
    }

    print
	p('Please use the links on the top navigation bar to access these programs. Some of these interfaces contain inputs that only experienced users should modify.'),
	p(
	  'We hope this tools suite and interface are useful.',
	  'Please send feedback or post questions at our Google Groups spctools-discuss mailing list. Click',
	  a({href=>'http://tools.proteomecenter.org/help.php'},'here'),
	  'to find out how to join this list.',
	  '</div>'
	  );

    print
	h2('Resources and Links'),
	ul(
	   li(a({target=>'SPCext', href=>'http://www.proteomecenter.org/'},'SPC')),
	   li(a({target=>'SPCext', href=>'http://tools.proteomecenter.org/'},'SPC Tools')),
	   li(a({target=>'SPCext', href=>'http://tools.proteomecenter.org/wiki/index.php?title=Main_Page'},'SPC Tools WIKI')),
	   li(a({target=>'SPCext', href=>'http://sourceforge.net/projects/sashimi/'},'Sashimi')),
	   li(a({target=>'SPCext', href=>'http://groups.google.com/group/spctools-discuss'},'SPCTools-Discuss at Google Groups')),
	   );

}


########################################################################
# pageAccount
#
# The user account settings page
#
########################################################################
sub pageAccount {

    my $logindate = "";
    for (@session_data) {
	chomp;
	$logindate = "Logged in since $'" if (/$proc_types{'logindate'}:/);  #'}
    }

    print
	h2("Account Management Page: <span class='nav'>&nbsp;&nbsp;&nbsp;$auth_user&nbsp;&nbsp;&nbsp;</span>");

    print
	&printTitle(title=>'Change password'),
	"<div class=formentry>",
	start_form('POST',$tpp_url),
	'<table cellspacing="2" border="0">',
	'<tr>',
	'<td>Current password:</td>',
	'<td>',
	password_field(-name=>'cur_password',
		       -value=>'',
		       -size=>20,
		       -maxlength=>20),
	'</td></tr>',
	'<tr>',
	'<td>New password:</td>',
	'<td>',
	password_field(-name=>'new_password',
		       -value=>'',
		       -size=>20,
		       -maxlength=>20),
	'</td></tr>',
	'<tr>',
	'<td>Re-type new password:</td>',
	'<td>',
	password_field(-name=>'ver_password',
		       -value=>'',
		       -size=>20,
		       -maxlength=>20),
	'</td></tr>',
	'<tr>',
	'<td colspan="2" align="right">',
	submit(-name=>'Action',
	       -value=>$web_actions{'newpassword'}),
	end_form,
	'</td></tr></table>',
	"</div>\n";

#    print
#	br,br,
#	&printTitle(title=>'Email Address'),
#	"<div class=formentry>",
#	"Enter email address to be notified when jobs are finished running.",
#	br,br,br,
#	"Coming soon!",
#	"</div>\n";

    print br.br.&printTitle(title=>$logindate) if ($logindate);

}

########################################################################
# pageSessions
#
# The user sessions management page
#
########################################################################
sub pageSessions {
    my $user_session_dir = "${users_dir}$auth_user/";
    my @session_files = &listFiles($user_session_dir,'session_','false');

    if (@session_files) {

	my %tmp;
	my %labels;
	my @sorted_files;

	foreach (@session_files) {
	    my @fstat = stat $user_session_dir.$_;
	    my $fileAge  = ($fstat[9]);
	    $tmp{$fileAge} = $_;
	}
	foreach my $sDate (reverse sort (keys %tmp)) {
	    # sort files by create date
	    my $fileAge  = scalar localtime($sDate);
	    push @sorted_files, $tmp{$sDate};
	    # is this the current session?
	    if ($session_file =~ /$tmp{$sDate}$/ ) {
		$labels{$tmp{$sDate}} = "$tmp{$sDate} -- [ *** CURRENT SESSION *** ]";
	    } else {
		$labels{$tmp{$sDate}} = "$tmp{$sDate} -- [ $fileAge ]";
	    }
	}

	print
	    &printTitle(title=>'Manage Old Sessions'),
	    "<div class=formentry>",
	    "View and delete previous user sessions.",
	    br,br,
	    start_form('POST',$tpp_url),
	    scalar(@session_files) . " session files found:",
	    br,
	    popup_menu(-name=>'session_select',
		       -id=>'session_select',
		       -values=>\@sorted_files,
		       -default=>"session_$user_session",
		       -labels=>\%labels,
		       -onChange => "getSessionInfo(document.getElementById('session_select').value)"),
	    "&nbsp;&nbsp;&nbsp;",
	    submit(-name=>'Action',
		   -value=>$web_actions{'deleteSession'}),
	    end_form,
	    "</div>\n",
	    br,br;

	# some needed JavaScript
	print <<"EOSCRIPT";
	<script type="text/javascript">

	    // Retrieve selected session info
	    function getSessionInfo(usid) {
		var url = "$tpp_url?Action=AJAXGetSessionInfo&p1="+usid+"&p2=$auth_user&rand="+Math.random();
		var callback = processSessionInfoRequest;
		executeXHR(3, callback, url);
	    }

            function processSessionInfoRequest() {
                // only if req shows "loaded"
		    if (http_req[3].readyState == 4) {
                        // only if "OK"
			if (http_req[3].status == 200) {
			    document.getElementById('sel_session').innerHTML = http_req[3].responseText;
			} else {
			    document.getElementById('sel_session').innerHTML = "<b>There was a problem retrieving the HTML data: " + http_req[3].statusText + "</b>";
                        }
		    }
	    }
	</script>
EOSCRIPT

	print
	    &printTitle(title=>'View Session Info'),
	    "<div id=sel_session class=formentry>",
	    " --- Please select a session from the list above ---",
	    "</div>\n",
	    br,br;

    } else {

	print
	    "This account has no old sessions.",
	    br,br;
    }
}


########################################################################
# pageClusters
#
# The Cluster Accounts management page (start with Amazon Cloud)
#
########################################################################
sub pageClusters {

    my $aws_file = "${users_dir}$auth_user/.awssecret";

    my $class = ($action eq $web_actions{'addAmazonKeys'}) ? 'formentry' : (-e $aws_file) ? 'hideit' : 'formentry';

    print
	&printTitle(title=>'Register Amazon EC2 Account',
		    div_id => "amzreg"),
	"<div id=amzreg class=$class>",
	"Enter your Amazon Access and Public keys to run searches on the Amazon Cloud",
	br,br,
	start_form('POST',$tpp_url),
	'<table cellspacing="2" border="0">',
	'<tr>',
	'<td>Amazon Access Key:</td>',
	'<td>',
	textfield(-name=>'amz_access',
		  -value=>'',
		  -size=>25,
		  -maxlength=>20),
	'</td>',
        '</tr><tr>',
	'<td>Amazon Secret Key:</td>',
	'<td>',
	password_field(-name=>'amz_secret',
		       -value=>'',
		       -size=>45,
		       -maxlength=>40),
	'</td></tr>',
	'<tr><td>Amazon S3 Bucket/Folder Path:</td>',
	'<td>',
	textfield(-name=>'amz_s3url',
		  -value=>'',
		  -size=>25,
		  -maxlength=>63), ' (optional)',
	'</td></tr>',
	'</table>';

    if (-e $command{amztpp}) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'addAmazonKeys'}),
	    end_form,
	    "</div>\n",
	    br,br;

    } else {
	print
	    br,
	    &printTitle(title  => ' WARNING ',
			class  => 'messageshead'),
	    "<div class=messages>\n",
	    "<li>It appears that you do not have <b>amztpp</b> installed in your system.</li>\n",
	    "<li>Please install this package if you wish to use this feature.  <a target='amztpp' href='http://tools.proteomecenter.org/wiki/index.php?title=TPP_AMZTPP'>More info</a></li>",
	    "</ol></div><br>\n\n",
	    end_form,
	    "</div>\n";
	return;
    }

    return unless (-e $aws_file);

    # attempt to get status
    my @errs = ();
    $ENV{AWS_CREDENTIAL_FILE} = $aws_file;
    my @status = `$command{amztpp} --xml status 2>&1`;

    if (!@status) {
	push @errs, "The amztpp executable does not appear to be installed or in working order.  Please verify and re-install if necessary.";
    } elsif ( $? ) {
        push @errs, @status;
    }

    print
	&printTitle(title=>'Registered Amazon Account Status',
		    div_id => "amzstatus"),
	"<div id=amzstatus class=formentry>";

    if ( !@errs ) {
        my $xml_response;
        for (@status) {
	    $xml_response .= "$_" unless /ParserDetails.ini/;  # sigh...
	    # print "$_"; #debug
        }

        #### Set up the XML parser and parse the returned XML
        my $parser = XML::Parser->new(
				      Handlers => {
					  Start => \&start_amztpp_element,
					  End  => \&end_amztpp_element,
					  #Char => \&amztpp_chars,
				      },
				      ErrorContext => 2 );
        eval { $parser->parse( $xml_response ); };
        push @errs, "Error parsing amztpp XML: $@" if ( $@ );
    }

    if ( @errs ) {
       print qq(<h1 id="error">Error Found!</h1>);
       print join "<br>\n", @errs;
    }

    print
	"</div>\n",
	br,
	start_form('POST',$tpp_url),
	submit(-name=>'Action',
	       -value=>$web_actions{'delAmazonKeys'}),
	"&nbsp;"x15,
	submit(-name=>'Action',
	       -value=>$web_actions{'cleanAmazon'}),
	end_form,
	br,
	"</div>\n";

}

########################################################################
# start_amztpp_element
########################################################################
sub start_amztpp_element {
    my ($handler, $element, %atts) = @_;

    if ($element eq 'client') {
        my $client = "Status as of <b>".scalar(localtime)."</b><br/><br/>";

        $client .= "Client background process is: <b>";
        $client .= $atts{'pid'} ? "running</b> (pid=$atts{'pid'})" : "stopped</b>";
        if ( -f "$base_dir/amztpp.log" ) {
            $client .= " &nbsp; [ <a target=\"_blank\" href=\"$tpp_bin_url/amztpp.log\">amztpp.log</a> ]";
        }

        print "$client\n\n<br/><br/>";
    }

    elsif ($element eq 'ec2') {
	my $ec2 = "<table border='1' cellpadding='5' style='border-collapse:collapse;solid black'>";
	$ec2 .= "<tr class='formentryhead'>";
        for my $head (qw(EC2 Instance Status AMI_id Type Started Public_DNS TPP_log Server_log)) {
	    $ec2 .= "<th>$head</th>";
        }
	$ec2 .= "</tr>\n";

	$tmp_hash{num_ec2} = 0;
	print $ec2;
    }

    elsif ($element eq 'instance') {
	$tmp_hash{num_ec2}++;
	my $srvlink = "http://$atts{'dns'}/amztppd-service.log";
	my $amzlink = "http://$atts{'dns'}/amztppd.log";
	my $ec2 = "<tr class='setting'><td class='formentryhead'></td><td>$atts{'id'}</td><td>$atts{'status'}</td><td>$atts{'image_id'}</td><td>$atts{'type'}</td><td>$atts{'started'}</td><td>$atts{'dns'}</td><td><a target='tppsrv' href='$srvlink'>TPP log</a></td><td><a target='amztpp' href='$amzlink'>server log</a></td></tr>\n";
	print $ec2;
    }

    elsif ($element eq 'spot') {
	$tmp_hash{num_ec2}++;
	# note that state is reported in the status column, and created in started
	my $ec2 = "<tr class='setting'><td class='formentryhead'></td><td>$atts{'id'}</td><td>$atts{'state'}</td><td>$atts{'image_id'}</td><td>$atts{'type'}</td><td>$atts{'created'}</td><td>-- no DNS: spot instance id = $atts{'instance_id'} --</td><td>n/a</td><td>n/a</td></tr>\n";
	print $ec2;
    }

    elsif ($element eq 'sqs') {
	my $sqs = "<table border='1' cellpadding='5' style='border-collapse:collapse;solid black'>";
	$sqs .= "<tr class='formentryhead'>";
        for my $head (qw(SQS Queue_Name Messages Timeout)) {
	    $sqs .= "<th>$head</th>";
        }
	$sqs .= "</tr>\n";

	$tmp_hash{num_sqs} = 0;
	print $sqs;
    }

    elsif ($element eq 'queue') {
	$tmp_hash{num_sqs}++;
	my $sqs = "<tr class='setting'><td class='formentryhead'></td><td>$atts{'name'}</td><td align='right'>$atts{'ApproximateNumberOfMessages'} / $atts{'ApproximateNumberOfMessagesNotVisible'}</td><td>$atts{'VisibilityTimeout'}</td></tr>\n";
	print $sqs;
    }

    elsif ($element eq 's3') {
	my $s3 = "<table border='1' cellpadding='5' style='border-collapse:collapse;solid black'>";
	$s3 .= "<tr class='formentryhead'>";
        for my $head (qw(S3 File_Name Size Last_Modified)) {
	    $s3 .= "<th>$head</th>";
        }
	$s3 .= "</tr>\n";

	$tmp_hash{num_s3} = 0;
	print $s3;
    }

    elsif ($element eq 'file') {
	$tmp_hash{num_s3}++;
	my $fsize = &human_size($atts{'size'});
	my $s3 = "<tr class='setting'><td class='formentryhead'></td><td>$atts{'key'}</td><td align='right'>$fsize</td><td>$atts{'last_modified'}</td></tr>\n";
	print $s3;
    }

}

########################################################################
# end_amztpp_element
########################################################################
sub end_amztpp_element {
    my ($handler, $element) = @_;

    if ($element =~ 'ec2') {
	print "<tr class='file'><td colspan='9'>Found $tmp_hash{num_ec2} instances</td></tr>\n";
	print "</table>\n<br/>\n";
    }

    elsif ($element =~ 'sqs') {
	print "<tr class='file'><td colspan='4'>Found $tmp_hash{num_sqs} queues</td></tr>\n";
	print "</table>\n<br/>\n";
    }

    elsif ($element =~ 's3') {
	my $ntxt = ($tmp_hash{num_s3} == 1000) ? 'over' : '';
	print "<tr class='file'><td colspan='4'>Found $ntxt $tmp_hash{num_s3} files</td></tr>\n";
	print "</table>\n<br/>\n";
    }

}


########################################################################
# pageJobs
#
# The commands execution status and output page
#
########################################################################
sub pageJobs {
    my $show_output_jid = param('show_job') || 0;
    Delete('show_job');

    my @jobs_data = &getAllCommandsStatus('all');

    if (!@jobs_data) {
	print h1("User $auth_user has no commands on record.");
	return;
    }

    my $session_class = 'formentry'; # = (!$show_output_jid || ($show_output_jid =~ /$csid/)) ? 'formentry' : 'hideit';

    print
	&printTitle(title  => " All Jobs ",
		    class  => 'formentryhead',
		    div_id => "alljobs"),
	"<div id=alljobs class=$session_class>",
	"<table border='1' cellpadding='5' style='border-collapse:collapse;solid black'>",
	"<tr class=formentryhead><th>Session ID</th><th>Job</th><th>Location</th><th>Start date / time</th><th>Actions</th><th>Status</th><th>Output</th></tr>",
	br;

    my $cmdout_status;
    my $prevcsid = '';
    my $jnum = 0;
    my %most_recent_byjob;

    foreach (@jobs_data) {
	chomp;
	my @L = split /\t/, $_;
	$L[0] =~ /_(.+)_(.*)$/;
	$L[0] = $1;
	$L[1] = $2;
	$L[1] =~ s/(\d\d\d\d)(\d\d)(\d\d)-(\d\d)(\d\d)(\d\d)/$1$2$3$4$5$6/;
	if (!exists($most_recent_byjob{$L[0]}) || $most_recent_byjob{$L[0]} < $L[1]) {
	    $most_recent_byjob{$L[0]} = $L[1];
	}
    }
    my @sorted = sort {
	chomp $a; chomp $b;
	my @A = split /\t/, $a;
	my @B = split /\t/, $b;
	$A[0] =~ /_(.+)_(.*)$/;
	$A[0] = $1;
	$A[1] = $2;
	$B[0] =~ /_(.+)_(.*)$/;
	$B[0] = $1;
	$B[1] = $2;
	$A[1] =~ s/(\d\d\d\d)(\d\d)(\d\d)-(\d\d)(\d\d)(\d\d)/$1$2$3$4$5$6/;
	$B[1] =~ s/(\d\d\d\d)(\d\d)(\d\d)-(\d\d)(\d\d)(\d\d)/$1$2$3$4$5$6/;
	if ($A[0] eq $B[0]) {
	    $A[1]  <=> $B[1];
	}
	else {
	    $most_recent_byjob{$A[0]}  <=> $most_recent_byjob{$B[0]};
	}
    } @jobs_data;

    foreach (reverse @sorted) {
	chomp;

	my ($jcmd, $jname, $jloc, $jstatus, $jpid) = split /\t/, $_;
	$jcmd =~ /_(.+)_(.*)$/;
	my $csid = $1;
	my $cdate = $2;

	if ($prevcsid) {
	    $jnum = 0 if ($prevcsid ne $csid);
	}
	$jnum++;
	$jname ||= "Job $jnum";

	my $output_link = "$tpp_url?Action=$web_actions{'showpage'}&page=jobs&show_job=${csid}_$cdate";
	my $tr_class = (!$prevcsid) ? '' : ($prevcsid ne $csid) ? 'style="border-top: 3px solid black"' : '';
	my $link_text = 'View';
	my $action_link = ($jstatus =~ /running|queued/) ? "<b><a href=\"$tpp_url?Action=$web_actions{'killCommand'}&job_id=${csid}_$cdate&jpid=$jpid\">Kill job</a></b>" : "<a href=\"$tpp_url?Action=$web_actions{'deleteCommand'}&job_id=${csid}_$cdate\">Delete Log</a>";

	if ($show_output_jid eq "${csid}_$cdate"){
	    $tr_class .= ' class=file';
	    $link_text = 'Refresh';
	    $cmdout_status = $jstatus;

	} elsif ($csid eq $user_session) {
	    $tr_class .= ' class=setting';
	} else {
	    $tr_class .= ' class=level1';
	}

	$cdate =~ s/(\d\d\d\d)(\d\d)(\d\d)-(\d\d)(\d\d)(\d\d)/$1-$2-$3 $4:$5:$6/;

	print << "EOROW";
	<tr $tr_class>
	    <td>$csid</td>
	    <td>$jname</td>
	    <td>$jloc</td>
	    <td>$cdate</td>
	    <td>$action_link</td>
	    <td id="${jcmd}_status" class="$jstatus">$jstatus</td>
	    <td><a href="$output_link">$link_text</a></td>
	    </tr>
EOROW

        $prevcsid = $csid;

    }

    print
	"</table>",
	"</div>\n",
	br;

    return unless ($show_output_jid);

    my $cmd_file = "${users_dir}$auth_user/cmd_$show_output_jid";
    my $cmdout_class = ($cmdout_status eq 'running') ? 'cmdrunhead' : ($cmdout_status eq 'finished') ? 'messageshead' : ($cmdout_status eq 'queued') ? 'cmdqueuehead' : ($cmdout_status eq 'killed') ? 'cmdkilledhead' : 'cmdreadyhead';
    my $jobnum = 0;
    my $open_div = 0;

    print
	&printTitle(title  => " Output for job id $show_output_jid ",
		    class  => $cmdout_class);

    &checkAmzFiles($cmd_file) if $cmdout_status eq 'queued';
    &getOutputFiles($cmd_file) unless ($cmdout_status eq 'running' || $cmdout_status eq 'killed');

    print br;

    open(COMF, "$cmd_file") || &fatalError('NOCMDFILE',"Cannot open $cmd_file for reading: $!\n");
    while (<COMF>){
	# TODO? Deal with backspaces (x08) and carriage returns (x0D)?
	chomp;
	my $pre = '';
	my $post = '';
	$_ = escapeHTML($_);

	if (/End Command Execution/) {
	    print "</div>\n".br;
	    $open_div--;

	} elsif (/User terminated command/) {
	    if ($open_div) {
		print "</div>\n".br;
		$open_div--;
	    }

	    print
		&printTitle(title=>"This command was manually terminated by the user.",
			    class  => 'cmdkilledhead'),
		br;

	} elsif (/User claims command timeout/) {
	    if ($open_div) {
		print "</div>\n".br;
		$open_div--;
	    }

	    print
		&printTitle(title=>"This command was deemed to have timed-out on the server as per user input.",
			    class  => 'messageshead'),
		br;

	} elsif (/^OUTPUT:/) {
	    $_ = '';  # do not display...
	    $post  = "<pre style='max-height:300px;overflow:auto' class='setting'>";

	} elsif (/^END OUTPUT/) {
	    $_ = '';  # do not display...
	    $pre  = "</pre>";

	} elsif (/EXECUTING:/) {
	    s|(\[.*\]) (EXECUTING:)|<font color=gray>$2</font>|;

	    $jobnum++;
	    print
		&printTitle(title  => " Command $jobnum $1 ",
			    class  => 'formentryhead',
			    div_id => "job_output_$jobnum"),
		"<div id='job_output_$jobnum' class='formentry' style='overflow:auto'>";

	    $open_div++;

	    $pre  = "<b>";
	    $post = "</b><br>";

	} elsif (/RETURN CODE:/) {
	    my $c_stat = ($') ? "<font color=red>FAILED</font>" : "Successful"; #'
	    print "<b>Command $c_stat</b><br>\n";

	} elsif (/All finished at/) {
	    my $endstat = ($cmdout_status eq 'queued') ? 'queued' : 'finished';
	    print
		&printTitle(title=>"All commands $endstat at <b>$'</b>",
			    class  => $cmdout_class);
	}

	next if (/^#/);
	next if (/# OUTPUT SEEN OK/);

	s|(warning)|<span style="color:#ff8820">$1</span>|i;
	s|(error)|<span style="color:red">$1</span>|i;
	print "$pre$_\n$post";
    }
    close(COMF);

    if ( ($cmdout_status eq 'finished') && open(CMD, ">>$cmd_file") ) {
	print CMD '# OUTPUT SEEN OK';
	close CMD;
    }

    if ($cmdout_status eq 'queued') {
	print
	    br,
	    'If your commands have actually completed, or the queue did not work, ',
	    a({-href => "$tpp_url?Action=$web_actions{'userTimeoutRecovery'}&show_job=$show_output_jid"},"click here"),
	    '.';
    }

    if ($open_div) {
	print
	    "</pre>\n",
	    &printTitle(title=>"This command is still running...",
			class  => $cmdout_class),
	    br,
	    'If your commands have actually completed but the server timed out, ',
	    a({-href => "$tpp_url?Action=$web_actions{'userTimeoutRecovery'}&show_job=$show_output_jid"},"click here"),
	    '.',
	    "</div>\n";
    }

    print "</div>\n";

}


########################################################################
# pageReSpect
#
# Front-end to run reSpect
#
########################################################################
sub pageReSpect {
    # files to process (from session)
    my $any_files_there = &showFiles('runprophet','respect','xml','1. Specify pepXML file with probabilities');

    print
	br,
	start_form('POST',$tpp_url),
	&printTitle(title  => '2. Options',
		    class  => 'formentryhead',
		    div_id => 'main'),
	"<div id=main class=formentry>",
	"Minimum probability: ",
	textfield(-name=>'respect_minprob',
		  -value=>'0.5',
		  -size=>10,
		  -maxlength=>10),
	br,
	"m/z tolerance (ions): ",
	textfield(-name=>'respect_mztol',
		  -value=>'0.1',
		  -size=>5,
		  -maxlength=>5),
	" Daltons",
	br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'respect_extraopts',
		  -value=>'',
		  -size=>25,
		  -maxlength=>100),
	"</div>\n";

    print
	br,
	&printTitle(title=>'3. Look for Chimeric Spectra!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runrespect'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageQualscore
#
# Front-end to run Qualscore
#
########################################################################
sub pageQualscore{
    # files to process (from session)
    my $any_files_there = &showFiles('runprophet','qualscore','xml','1. Specify search results file');

    print
	br,
	start_form('POST',$tpp_url),
	&printTitle(title  => '2. Options',
		    class  => 'formentryhead',
		    div_id => 'main'),
	"<div id=main class=formentry>",
	"Set Qualscore 'cutoff': ",
	textfield(-name=>'qual_cutoff',
		  -value=>'1.0',
		  -size=>10,
		  -maxlength=>10),
	br,
	"PeptideProphet max. treshold for unassigned: ",
	textfield(-name=>'qual_pepprob',
		  -value=>'0.1',
		  -size=>10,
		  -maxlength=>10),
	br,
	# implement -path- option!  ToDo
	checkbox(-name=>'qual_listfile',
		 -value=>'true',
		 -label=>'List spectra and quality scores to a file, rather than writing to directory'),
	br,
	checkbox(-name=>'qual_listproph',
		 -value=>'true',
		 -label=>' |-- Also add PeptideProphet probability to list'),
	br,
	checkbox(-name=>'qual_listall',
		 -value=>'true',
		 -label=>'List all quality scores'),
	br,
#	checkbox(-name=>'qual_plot',
#		 -value=>'true',
#		 -label=>'Plot score distributions'),
#	br,
	"</div>\n";

    print
	br,
	&printTitle(title=>'3. Look for Unassigned High Quality Spectra!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runqualscore'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageCompareProts
#
#  Compare Protein Lists
########################################################################
sub pageCompareProts {

    my %labels = ( 'comptwo'   => '...from two ProteinProphet results',
		   'heatmap'   => '...from multiple results, and generate heatmap' );

    # auto-select appropriate section (div) to be visible
    my $select_default = 'comptwo';
    my %div_style = ( 'comptwo' => 'hideit',
		      'heatmap' => 'hideit' );

    if (param('comprot_type')){
	$select_default = param('comprot_type');
    }
    $div_style{$select_default} = '';

    # need some javascript
    print <<"EOSCRIPT";

    <script language='JavaScript'>
    function showForm(event) {
	var divs = new Array();
	divs[0] = 'comptwo';
	divs[1] = 'heatmap';

	var divId = event.value;
	var x;
	for (x in divs) {
	    if (document.getElementById(divs[x]))
		document.getElementById(divs[x]).style.display='none';
	}
	if (document.getElementById(divId)) {
	    document.getElementById(divId).style.display='block';
	}
	return true;
    }
    </script>
EOSCRIPT

    Delete('comprot_type');


    # protxml files (from session)
    my $file_there = &showFiles('protxml','compareprots','prot.xml','1. Specify ProteinProphet ProtXML Files');

    Delete('file_ext');
    Delete('proc_type'); 

    # output dir and input files
    my $fdir;
    my @input_files;

    # get path from last xml file in list
    for (@session_data) {
	chomp;
	if (/$proc_types{'protxml'}:/) {
	    $fdir  = dirname($');    #'}
	    push @input_files, $';  #'
	}
    }

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => '2. Choose Output Directory',
		    class  => 'formentryhead'),
	"<div class=formentry>",
	"Output Directory: ",
	textfield(-name=>'comprot_outdir',
		  -value=>"$fdir",
		  -size=>100,
		  -maxlength=>200),
	"</div>\n";

    print
	br,
	&printTitle(title  => '3. Choose Protein Name Tags (optional)',
		    class  => 'formentryhead'),
	"<div class=formentry>",
	b("Include")." Protein Name Tag: ",
	textfield(-name=>'prot_inc_tag',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	br,
	b("Exclude")." Protein Name Tag: ",
	textfield(-name=>'prot_exc_tag',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	"</div>\n";

    print
	br,
	&printTitle(title=>'4. Specify Comparison Type'),
	"<div class=formentry>",
	"Compare protein lists ",
	popup_menu(-name=>'comprot_type',
		   -values => [qw'comptwo heatmap'],
		   -labels=>\%labels,
		   -default => $select_default,
		   -onChange => 'showForm(this);'),
	br,
	"Choose reference list: ",
	popup_menu(-name=>'comprot_reflist',
		   -values => \@input_files ),
	"</div>\n";


    ### COMPARE TWO submit button ###
    print
        "<div id='comptwo' class=$div_style{'comptwo'}>\n";

    print
	"<br>\n",
	&printTitle(title=>'5. Run'),
	"<div class=formentry>";

    if ($file_there == 2) {
	print
	    hidden(-name=>'comprot_type',
		   -value=>'comptwo'),
	    submit(-name=>'Action',
		   -value=>$web_actions{'compareprots'});
    } else {
	print
	    h2('Please choose two files.');
    }

    print
	br,
	"</div>\n",
	"</div>\n";



    ### HEATMAP options and submit button ###
    print
	"<div id='heatmap' class=$div_style{'heatmap'}>\n";

    print
	br,
	&printTitle(title  => '5. Comparison options',
		    class  => 'formentryhead'),
	"<div class=formentry>",
	"Minimum Probability: ",
	textfield(-name=>'comprot_minprob',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	br,
	"Protein Window Size: ",
	textfield(-name=>'comprot_window',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	br,
 	checkbox(-name=>'comprot_noprot',
		 -value=>'true',
		 -label=>'Disable Protein Clustering'),
	br,
 	checkbox(-name=>'comprot_nofile',
		 -value=>'true',
		 -label=>'Disable File Clustering'),
	"</div>\n";


    print
	"<br>\n",
	&printTitle(title=>'6. Run'),
	"<div class=formentry>";

    if ($file_there > 2) {
	print
	    hidden(-name=>'comprot_type',
		   -value=>'heatmap'),
	    submit(-name=>'Action',
		   -value=>$web_actions{'compareprots'});
    } else {
	print
	    h2('Please choose three or more files.');
    }

    print
	br,
	"</div>\n",
	end_form,
	"</div>\n";

}


########################################################################
# pageMayu
#
#  Front end to Mayu; start with simple options only
#
########################################################################
sub pageMayu {
    Delete('file_ext');
    Delete('proc_type');   # delete these...

    print
	"<p>Need more ",
	a({target=>'mayu', href=>"http://proteomics.ethz.ch/muellelu/web/LukasReiter/Mayu/"},'information on Mayu'),
	'?</p>';

    # pepXML file(s) (from session)
    my $pepXfile_there = &showFiles('xinteract','mayu','xml','1. Specify pepXML file(s) to analyze');

    # sequence database file (from session)
    my $dbfile_there = &showFiles('searchdb','mayu','*','2. Specify Sequence Database used (must have decoys)','dbase/');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => '3. Options',
		    class  => 'formentryhead',
		    div_id => 'opts'),
	"<div id=opts class=formentry>",
	"Tag (prefix) for decoy proteins: ",
	textfield(-name=>'mayu_tag',
		  -value=>'rev_',
		  -size=>20,
		  -maxlength=>40),
	br,
	"Number of missed enzymatic cleavages (NMC) allowed in search: ",
	textfield(-name=>'mayu_nmc',
		  -value=>'2',
		  -size=>2,
		  -maxlength=>1),
	br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'mayu_extraopts',
		  -value=>'',
		  -size=>25,
		  -maxlength=>100),
	"</div>\n";


    # submit button
    print
	"<br>\n",
	&printTitle(title=>'4. Run'),
	"<div class=formentry>";

    if ($pepXfile_there && $dbfile_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runMayu'});
    } else {
	print
	    h2('Not enough input files selected yet.');
    }

    print
	br,
	"</div>\n",
	end_form;

}


########################################################################
# pageDecoyFasta
#
#  Generate reverse sequence databases for use in decoy searches
#
########################################################################
sub pageDecoyFasta {
    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # sequence database file (from session)
    my $dbfile_there = &showFiles('searchdb','decoyfasta','*','1. Specify a Source Sequence Database','dbase/');

    # choose generator
    my %labels = (
		  'decoyfastagen' => ' Randomize sequences and interleave entries (best suited for trypsin as sample enzyme)',
		  'decoyfasta'    => ' Reverse protein sequences'
		  );

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => '2. Choose Decoy Algorithm',
		    class  => 'formentryhead'),
	"<div class=formentry>",
	radio_group(-name      =>'decoy_exe',
		    -linebreak =>'true',
		    -onChange  => 'showDecoyOpts(this);',
		    -values    =>['decoyfastagen','decoyfasta'],
		    -default   =>'decoyfastagen',
		    -labels    =>\%labels),
	"</div>\n";

    # need some javascript
    print <<"EOSCRIPT";
    <script language='JavaScript'>
    function showDecoyOpts(event) {
	if (event.value == 'decoyfasta') {
	    document.getElementById('decoyfasta_opts').style.display='inline';
	}
	else {
	    document.getElementById('decoyfasta_opts').style.display='none';
	}
    }
    </script>
EOSCRIPT

    my $outfile;
    if ($dbfile_there) {
	# get name from first file in list
	for (@session_data) {
	    chomp;
	    if (/$proc_types{'searchdb'}:/) {
		$outfile  = $';    #'
		last;
	    }
	}
    }
    # make a default name
    my $decoy_name = "_DECOY";
    if ($outfile =~ /\.fasta/i) {
	$outfile =~ s/(\.fasta)/$decoy_name$1/i; #'
    } elsif ($outfile =~ /\./) {
	$outfile =~ s/(.*)\./$1$decoy_name./;
    } elsif ($outfile) {
	$outfile .= $decoy_name;
    }

    print
	br,
	&printTitle(title  => '3. Choose Output File Name',
		    class  => 'formentryhead'),
	"<div class=formentry>",
	"Output Filename: ",
	textfield(-name=>'decoy_outfile',
		  -value=>$outfile,
		  -size=>80,
		  -maxlength=>140),
	"</div>\n";

    print
 	br,
 	&printTitle(title  => '4. Options',
 		    class  => 'formentryhead',
 		    div_id => 'opts'),
 	"<div id=opts class=formentry>",
 	# only 1 tag for now
 	"Tag for decoy proteins: ",
 	textfield(-name=>'decoy_tag',
 		  -value=>'DECOY',
 		  -size=>20,
 		  -maxlength=>40),
 	br,
	"<span id='decoyfasta_opts' class='hideit'>\n",
 	checkbox(-name=>'decoy_orig',
		 -value=>'true',
		 -label=>'do not copy the original proteins from the input file to the output file'),
 	br,
 	checkbox(-name=>'decoy_norev',
 		 -value=>'true',
 		 -label=>'do not reverse the sequences of the new decoy proteins being written to the output file'),
	"</span>\n",
 	"</div>\n";

    # submit button
    print
	"<br>\n",
	&printTitle(title=>'5. Run'),
	"<div class=formentry>";

    if ($dbfile_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'genDecoyDB'});
    } else {
	print
	    h2('No file selected yet.');
    }

    print
	br,
	"</div>\n",
	end_form;

}

########################################################################
# pageDecoyValPeps
#
#  Generate plots comparing  PeptideProphet Probabilities to Decoy Validation
#
########################################################################
sub pageDecoyValPeps {
    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # pepXML file(s) (from session)
    my $pepXfile_there = &showFiles('xinteract','decoyvalpeps','xml','1. Specify a pepXML file to process');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => '2. Options',
		    class  => 'formentryhead',
		    div_id => 'opts'),
	"<div id=opts class=formentry>",
	"Tag for decoy proteins: ",
	textfield(-name=>'decoy_tag',
		  -value=>'DECOY',
		  -size=>20,
		  -maxlength=>40),
	br,
	"Tag for excluded proteins: ",
	textfield(-name=>'excl_tag',
		  -value=>'',
		  -size=>20,
		  -maxlength=>40),
	br,
	"Decoy / Incorrect Protein Ratio (leave blank to estimate from low scoring IDs): ",
	textfield(-name=>'ratio',
		  -value=>'',
		  -size=>20,
		  -maxlength=>40),
	br,
	checkbox(-name=>"uniq_ipepseq",
		 -id=>"uniq_ipepseq",
		 -value=>'false',
		 -label=>'Consider only best iProphet probability for each unique peptide sequence (default: consider each PSM)'),
	br,
	checkbox(-name=>"uniq_ppepseq",
		 -id=>"uniq_ppepseq",
		 -value=>'false',
		 -label=>'Consider only best PeptideProphet probability for each unique peptide sequence (default: consider each PSM)'),
	"</div>\n";

    # submit button
    print
	"<br>\n",
	&printTitle(title=>'3. Run'),
	"<div class=formentry>";

    if ($pepXfile_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'DecoyValPeps'});
    } else {
	print
	    h2('No file selected yet.');
    }

    print
	br,
	"</div>\n",
	end_form;

}

########################################################################
# pageDecoyValProts
#
#  Generate plots comparing  ProteinProphet Probabilities to Decoy Validation
#
########################################################################
sub pageDecoyValProts {
    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # protXML file(s) (from session)
    # LM: should this be updated to take 'protxml' files?
    my $protXfile_there = &showFiles('runprophet','decoyvalprots','xml','1. Specify a protXML file to process');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => '2. Options',
		    class  => 'formentryhead',
		    div_id => 'opts'),
	"<div id=opts class=formentry>",
	"Tag for decoy proteins: ",
	textfield(-name=>'decoy_tag',
		  -value=>'DECOY',
		  -size=>20,
		  -maxlength=>40),
	br,
	"Tag for excluded proteins: ",
	textfield(-name=>'excl_tag',
		  -value=>'',
		  -size=>20,
		  -maxlength=>40),
	br,
	"Decoy / Incorrect Protein Ratio (leave blank to estimate from low scoring IDs): ",
	textfield(-name=>'ratio',
		  -value=>'',
		  -size=>20,
		  -maxlength=>40),
	"</div>\n";

    # submit button
    print
	"<br>\n",
	&printTitle(title=>'3. Run'),
	"<div class=formentry>";

    if ($protXfile_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'DecoyValProts'});
    } else {
	print
	    h2('No file selected yet.');
    }

    print
	br,
	"</div>\n",
	end_form;

}


########################################################################
# pageLibraCondition
#
#  Generate Libra Condition XML files
#
########################################################################
sub pageLibraCondition{
    Delete('cond_prefill');

    my @masses = qw(dummy 113.1 114.1 115.1 116.1 117.1 118.1 119.1 121.1);

    my %centr_labels = ( '1' => 'Average',
			 '2' => 'Intensity Weighted Mean' );

    my %norm_labels = ( '-2' => 'Normalize against TIC (not recommended)',
			'-1' => 'Normalize against sum of reagent profiles',
			'0'  => 'Normalize against most intense peak (not recommended)',
			'1'  => 'Normalize against channel 1',
			'2'  => 'Normalize against channel 2',
			'3'  => 'Normalize against channel 3',
			'4'  => 'Normalize against channel 4',
			'5'  => 'Normalize against channel 5',
			'6'  => 'Normalize against channel 6',
			'7'  => 'Normalize against channel 7',
			'8'  => 'Normalize against channel 8' );

    my %tag_labels = ( '--'     => '-- Paste commonly-used values --',
		       'itraq4' => 'iTRAQ - 4 channel',
		       'itraq8' => 'iTRAQ - 8 channel',
		       'tmt6'   => 'TMT - 6 channel',
		       'zero'   => 'Clear isotopic contributions' );

    my $outfile;
    for (@session_data) {
	chomp;
	if (/$proc_types{'lastdir'}:/) {
	    $outfile  = $';    #'
	    last;
	}
    }
    $outfile .= 'condition.xml';

    # first, some JScript
    print <<"EOSCRIPT";
    <SCRIPT LANGUAGE=JavaScript>
	var nrows = 0;
        var values = new Array();
        var masses = new Array();

	function prepopulate(type) {

	    if (type == 'itraq4') {
		nrows = 4;
		masses = new Array("114.1","115.1","116.1","117.1");
		values = new Array("0.0","1.0","5.9","0.2","0.0","2.0","5.6","0.1","0.0","3.0","4.5","0.1","0.1","4.0","3.5","0.1");

	    } else if (type == 'itraq8') {
		nrows = 8;
		masses = new Array("113.1","114.1","115.1","116.1","117.1","118.1","119.1","121.1");
		values = new Array("0.0","0.0","6.89","0.22","0.0","0.94","5.9","0.16","0.0","1.88","4.9","0.1","0.0","2.82","3.9","0.07","0.06","3.77","2.88","0.0","0.09","4.71","1.88","0.0","0.14","5.66","0.87","0.0","0.27","7.44","0.18","0.0");

	    } else if (type == 'tmt6') {
		nrows = 6;
		masses = new Array("126.1","127.1","128.1","129.1","130.1","131.1");
		values = new Array("0.0","0.0","9.3","0.4","0.1","0.7","8.7","0.4","0.1","1.1","7.3","0.3","0.1","1.9","6.0","0.2","0.1","1.8","5.1","0.1","0.1","3.5","4.5","0.1");

	    } else if (type == 'zero') {
		nrows = 0;
		values = new Array("0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0","0.0");

	    } else {
		nrows = 0;
	    }

	    for (var row=1;row<=8;row++) {
		if (row<=nrows) {
		    document.getElementById("cond_usemz_"+row).checked = true;
		    document.getElementById("mass_"+row).value = masses[row-1];
		} else {
		    document.getElementById("cond_usemz_"+row).checked = false;
		}
	    }

	    for (i in values) {
		document.getElementById("param"+i).value = values[i];
	    }

	}

    </SCRIPT>
EOSCRIPT

    print
	start_form(-method => 'POST',
		   -action => $tpp_url,
		   -name   => 'libracondition'),
	br,
	&printTitle(title  => 'Choose Output Location and File Name',
		    class  => 'formentryhead'),
	"<div class=formentry>",
	"Output Filename: ",
	textfield(-name=>'cond_outfile',
		  -value=>$outfile,
		  -size=>80,
		  -maxlength=>140),
	"</div>\n";

    print
	br,
	&printTitle(title=>'Condition file parameters'),
	"<div class=formentry>\n",
	"<table>\n",
	"<tr><th>Use</th><th>Reagent m/z</th><th>% of -2</th><th>% of -1</th><th>% of +1</th><th>% of +2</th>\n",
	"<td rowspan=\"30\">",
	br,br,
	popup_menu(-name=>'cond_prefill',
		   -values => [qw'-- itraq4 itraq8 tmt6 zero'],
		   -default => '--',
		   -labels=>\%tag_labels,
		   -onChange=> 'prepopulate(this.value);'),
	br,br,br,br,
	'<input value="Reset values" type="reset" />',
	"</td></tr>\n";

    my $param_id = 0;
    for my $channel (1..8) {
	print
	    "<tr id=\"trow$channel\"><td>",
	    checkbox(-name=>"cond_usemz_$channel",
		     -id=>"cond_usemz_$channel",
		     -value=>'true',
		     -label=>' '),
	    "</td><td>",
	    textfield(-name=>"cond_mz_$channel",
		      -id=>"mass_".$channel,
		      -value=>$masses[$channel],
		      -size=>8,
		      -maxlength=>8),
	    "</td><td>",
	    textfield(-name=>"cond_affect_${channel}_m2",
		      -id=>"param".$param_id++,
		      -value=>"0.0",
		      -size=>8,
		      -maxlength=>8),
	    "</td><td>",
	    textfield(-name=>"cond_affect_${channel}_m1",
		      -id=>"param".$param_id++,
		      -value=>"0.0",
		      -size=>8,
		      -maxlength=>8),
	    "</td><td>",
	    textfield(-name=>"cond_affect_${channel}_p1",
		      -id=>"param".$param_id++,
		      -value=>"0.0",
		      -size=>8,
		      -maxlength=>8),
	    "</td><td>",
	    textfield(-name=>"cond_affect_${channel}_p2",
		      -id=>"param".$param_id++,
		      -value=>"0.0",
		      -size=>8,
		      -maxlength=>8),
	    "</td></tr>\n\n";
    }

    print
	"<tr><td colspan=6><hr noshade size=1></td></tr>\n",
	"<tr><td colspan=2 align=right>",
	"Mass Tolerance: ",
	"</td><td colspan=4>",
	textfield(-name=>'cond_masstol',
		  -value=>"0.2",
		  -size=>8,
		  -maxlength=>8),
	"</td></tr>\n",
	"<tr><td colspan=2 align=right>",
	"Centroiding: ",
	"</td><td colspan=4>",
	popup_menu(-name=>'cond_centroid',
		   -values => [qw'1 2'],
		   -labels=>\%centr_labels,
		   -default => '2'),
	"</td></tr>\n",
	"<tr><td colspan=2 align=right>",
	"Normalization: ",
	"</td><td colspan=4>",
	popup_menu(-name=>'cond_norm',
		   -values => [qw'-2 -1 0 1 2 3 4 5 6 7 8'],
		   -labels=>\%norm_labels,
		   -default => '-1'),
	"</td></tr>\n",
	"<tr><td colspan=2 align=right>",
	"Minimum Threshhold Intensity:",
	"</td><td colspan=4>",
	textfield(-name=>'cond_thresh',
		  -value=>"20",
		  -size=>8,
		  -maxlength=>8),
	"</td></tr>\n",
	"<tr><td colspan=2 align=right>",
	"SRS data (Thermo MS3):",
	"</td><td colspan=4>",
	checkbox(-name=>"cond_ms3",
		 -value=>'true',
		 -label=>' '),
	"</td></tr>\n",
	"</table>",
	"</div>\n";

    print
	br,
	&printTitle(title=>'Generate Libra Condition (XML) File'),
	"<div class=formentry>",
	submit(-name=>'Action',
	       -value=>$web_actions{'genLibraCond'}),
	"</div>\n",
	end_form;

}


########################################################################
# pageLib2HTML
#
#  Generate user-viewable HTML versions of .splib (binary) files
#
########################################################################
sub pageLib2HTML{

    # files to process (from session)
    my $any_files_there = &showFiles('nistspeclib','lib2html','splib','1. Specify files to convert','dbase/speclibs/');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title=>'2. Generate HTML Files'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runlib2html'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageDNA2AA
#
#  Perform 6-frame translation on DNA sequence file to produce AA fasta file
#
########################################################################
sub pageDNA2AA {
    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # DNA sequence file (from session)
    my $dnafile_there = &showFiles('searchdb','dna2aa','*','1. Specify a Source DNA Sequence / Transcript Database','dbase/');

    my $outfile;
    if ($dnafile_there) {
	# get name from first file in list
	for (@session_data) {
	    chomp;
	    if (/$proc_types{'searchdb'}:/) {
		$outfile  = $';    #'
		last;
	    }
	}
    }
    # make a default name
    my $aa_suffix = "_AA";
    if ($outfile =~ /\.fasta/i) {
	$outfile =~ s/(\.fasta)/$aa_suffix$1/i; #'
    } elsif ($outfile =~ /\./) {
	$outfile =~ s/(.*)\./$1$aa_suffix./;
    } elsif ($outfile) {
	$outfile .= $aa_suffix;
    }

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => '2. Choose Output File Name',
		    class  => 'formentryhead'),
	"<div class=formentry>",
	"Output Filename: ",
	textfield(-name=>'dbase_outfile',
		  -value=>$outfile,
		  -size=>80,
		  -maxlength=>140),
	"</div>\n",
	br,
	&printTitle(title=>'3. Run'),
	"<div class=formentry>";

    if ($dnafile_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'genAAdbase'});
    } else {
	print
	    h2('No file selected yet.');
    }

    print
	br,
	"</div>\n",
	end_form;

}


########################################################################
# pageGetSpecLibs
#
#  Display spectral libraries available for download at PeptideAtlas
#  and provide links to download them
#
########################################################################
sub pageGetSpecLibs {

    my $libs_dir = "${data_dir}dbase/speclibs/";
    my $libs_dir_disp = $libs_dir;
    $libs_dir_disp =~ s|/| / |g;     # Add spaces for clarity

    my $libsdir_exists = (-d $libs_dir);
    my @dafiles;

    @dafiles = &listFiles($libs_dir,'splib') if $libsdir_exists;

    my $dirclass = $libsdir_exists ? '' : 'running';

    print
	&printTitle(title=>"View and Download Available Spectral Libraries"),
	"<div class=formentry>",
	"<table cellspacing=5 width=100%>\n",
	"<tr>\n",
	"<td colspan=2 class=file>Libraries Directory:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b class='$dirclass'>$libs_dir_disp</b></td>\n",
	"</tr>\n",
	"<tr>\n<td width=50% valign=top class=file>",
	"<h4>Locally Installed Libraries:</h4>";

    if (!$libsdir_exists) {
	print
	    br,
	    &printTitle(title  => ' WARNING ',
			class  => 'messageshead'),
	    "<div class=messages>\n",
	    "<li>The target directory for spectral libraries ( <b>$libs_dir_disp</b> ) does NOT exist.</li>\n",
	    "<li>Please create this directory before attempting to download spectral libraries.</li>\n",
	    "</div>";

    }
    elsif (@dafiles) {

	my @fstat;
	my ($fileSize, $fileAge);

	print
	    "<table border=0 cellspacing=5 width=100%>\n",
	    "<tr><th class=fileentry>File Name</th>",
	    "<th class=fileentry align=middle>Size</th>",
	    "<th class=fileentry align=middle>Date Modified</th>",
	    "</tr>\n";

	my $fc = 0;
	my $fclass;

	foreach my $sFile (@dafiles) {
	    my $filepath = $libs_dir.$sFile;
	    $fc++;
	    $fclass = ($fc % 4) ? "": "class=fileentry";

	    # get stats for each file
	    @fstat = stat $filepath;
	    $fileSize = &human_size($fstat[7]);
	    $fileAge  = scalar localtime($fstat[9]);

	    print
		"<tr><td $fclass>$sFile</td>",
		"<td $fclass align='right'>$fileSize</td><td $fclass align='right'>$fileAge</td></tr>\n";
	}

	print
	    "</table>\n",
	    "<p><b>$fc</b> files found.</p>";

    } else {
	print
	    br,
	    "There are no spectral library files (.splib) in this directory.",
	    br,br;
    }

    print
	"</td>\n";

    # Download pane
    #
    my $xml_response = `$command{wget} -q -O - http://www.peptideatlas.org/speclib/available_libs.php`;

    print
	"<td width=50% class=file><h4>Libraries Available From PeptideAtlas:</h4>";

    unless ($xml_response) {
	print
	    br,
	    "There are no spectral library files available at www.peptideatlas.org!",
	    br,br;

    } else {

	print<<EOJSCRIPT;
<SCRIPT LANGUAGE=JavaScript>
function toggleTR(trid) {
    var display;
    if (document.getElementById(trid).style.display == 'none') {
	display = '';  // should be 'table-row' -- stupid MSIE...
    } else {
	display = 'none';
    }

    document.getElementById(trid).style.display = display;
}
</SCRIPT>
EOJSCRIPT

	print
	    start_form('POST',$tpp_url),
	    "<table border=0 cellspacing=5 width=100%>\n",
	    "<tr><td>&nbsp;</td>",
	    "<th class=fileentry>File Name&nbsp;&nbsp;&nbsp;<span class=gray>(click on name to toggle more info)</span></th>",
	    "<th class=fileentry align=middle>Size</th>",
	    "<th class=fileentry align=middle>Date</th>",
	    "</tr>\n";

	#### Set up the XML parser and parse the returned XML
	my $parser = XML::Parser->new(
				      Handlers => {
					  #Start => \&start_xml_element,
					  End  => \&end_xml_element,
					  Char => \&xml_chars,
				      },
				      ErrorContext => 2 );
	eval { $parser->parse( $xml_response ); };
	&fatalError("ERROR_PARSING_XML:$@") if($@);

	print
	    "</table>\n",
	    br,
	    hidden(-name=>'splib_dir',
		   -value=>$libs_dir),
	    submit(-name=>'Action',
		   ($libsdir_exists ? () : (-disabled => undef) ),
		   -value=>$web_actions{'fetchlibs'}),
	    end_form;

    }

    print
	"</td>\n</tr>\n</table>",
	"</div>\n";

}

###############################################################################
# xml_chars
#
#  Internal SAX callback function to handle character data between tags
###############################################################################
sub xml_chars {
  my $handler = shift;
  chomp (my $string = shift);
  my $context = $handler->{Context}->[-1];

  if ($handler->{Context}->[-2] =~ /^(library|current_release|messages)$/) {
      $tmp_hash{$context} .= $string if $string;
  }

}

###############################################################################
# end_xml_element
#
#  Internal SAX callback function to end tags
###############################################################################
sub end_xml_element {
  my $handler = shift;
  my $element = shift;

  if ($element eq 'library') {

      my $txt_class = ($tmp_hash{status} eq 'archive') ? 'class=gray' : '';

      print
	  "<tr>\n",
	  "<td>",
	  checkbox(-name=>'splib_getfile',
		   -value=>"$tmp_hash{filename}::$tmp_hash{md5}",
		   -label=>'',
		   ),
	  "</td>",
	  "<td $txt_class><b onClick=\"javascript:toggleTR('tr_$tmp_hash{md5}')\">$tmp_hash{filename}</b></td>",
	  "<td>$tmp_hash{size}</td>",
	  "<td>$tmp_hash{date}</td>",
	  "</tr>\n",
	  "<tr style='display:none' id='tr_$tmp_hash{md5}'><td>&nbsp;</td>",
	  "<td class=fileentry colspan=3>$tmp_hash{description}<br/><br/></td></tr>\n";

      %tmp_hash = ();

  } elsif ($element eq 'message') {
	push @messages, $tmp_hash{message};
	$tmp_hash{message} = '';
  }


  return;
}


########################################################################
# pageMaRiMba
#
#  For generating MRM transitions
#
########################################################################
sub pageMaRiMba {

    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # files to process (from session)
    my $speclib_there = &showFiles('speclib','marimba','splib','1. Specify Input Spectral Library File','dbase/speclibs/');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for next file select

    # sequence database file (from session)
    my $dbfile_there = &showFiles('searchdb','marimba','*','2. Specify a protein sequence database','dbase/');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for next file select

    # peptide/protein restriction list (from session)
    my $restrict_there = &showFiles('textfile','marimba','txt','3. Specify peptide or protein restriction list (optional)');

    my @res_list = ('--', @residues);

    my $outfile;
    for (@session_data) {
	chomp;
	if (/$proc_types{'lastdir'}:/) {
	    $outfile  = $';    #'
	    last;
	}
    }
    $outfile .= 'MRMlist.txt';

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title=>'4. Specify Output file'),
	"<div class=formentry>",
	"Output Filename: ",
	textfield(-name=>'mrm_outfile',
		  -value=>$outfile,
		  -size=>80,
		  -maxlength=>140),
	"</div>\n";


    print
	br,
	&printTitle(title  => '5. Filtering Options',
		    class  => 'formentryhead',
		    div_id => 'opts'),
	"<div id=opts class=formentry>";

    if ($restrict_there) {
	my %labels = (
		      'prots' => ' Proteins or',
		      'peps'  => ' Peptides'
		      );
	print
	    "Please specify whether the restriction file listed in step 3 above contains ",
	    radio_group(-name=>'mrm_restrict',
			-values=>['prots','peps'],
			-default=>'prots',
			-labels=>\%labels),
	    br,br;
    }

    print
	b("Precursor peptide"),
	br,
	"Minimum Number of Transitions per Peptide: ",
	popup_menu(-name=>'min_mrm_numtrans',
		   -values => [1..30],
		   -default => '3'),
	br,
	"Maximum Number of Transitions per Peptide: ",
	popup_menu(-name=>'mrm_numtrans',
		   -values => [1..30],
		   -default => '10'),
	br,
	"Allowable charge states: ";

    for (1..5) {
	my $checked = ($_ == 2 || $_ == 3) ? 'checked' : '';
	print
	    checkbox(-name=>"mrm_prec$_",
		     -value=>'true',
		     -checked=>$checked,
		     -label=>"+$_ ");
    }

    print
	br,
	"<table cellspacing=0 cellpadding=0>",
	"<tr><td>m/z :</td><td>&nbsp;&nbsp;&nbsp;&nbsp;Min ",
	textfield(-name=>'mrm_mzmin',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	"&nbsp;&nbsp;&nbsp;&nbsp;Max ",
	textfield(-name=>'mrm_mzmax',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	"</td></tr>\n",
	"<tr><td>pI :</td><td>&nbsp;&nbsp;&nbsp;&nbsp;Min ",
	textfield(-name=>'mrm_pimin',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	"&nbsp;&nbsp;&nbsp;&nbsp;Max ",
	textfield(-name=>'mrm_pimax',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	"</td></tr></table>\n",
	"Exclude Residues: ",
	popup_menu(-name=>'mrm_nores1',
		   -default => '--',
		   -values => \@res_list),
	popup_menu(-name=>'mrm_nores2',
		   -default => '--',
		   -values => \@res_list),
	popup_menu(-name=>'mrm_nores3',
		   -default => '--',
		   -values => \@res_list),
	popup_menu(-name=>'mrm_nores4',
		   -default => '--',
		   -values => \@res_list),
	popup_menu(-name=>'mrm_nores5',
		   -default => '--',
		   -values => \@res_list),
	popup_menu(-name=>'mrm_nores6',
		   -default => '--',
		   -values => \@res_list),
	br,
	"Exclude N-terminal Residues: ",
	checkbox(-name=>'mrm_xclnQ',
		 -value=>'true',
		 -checked=>1,
		 -label=>' Q '),
	checkbox(-name=>'mrm_xclnE',
		 -value=>'true',
		 -label=>' E '),
	br,
	checkbox(-name=>'mrm_xclmods',
		 -value=>'true',
		 -label=>'Exclude all modifications except carbamidomethyl Cys C[160]'),
	br,
	checkbox(-name=>'mrm_usemods',
		 -value=>'true',
		 -label=>'Add transitions for modified (labeled) residues: '),


	br,
	"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Residue / Mass Difference: ",
	popup_menu(-name=>'mrm_label1',
		   -values => \@res_list),
	textfield(-name=>'mrm_deltamz1',
		  -value=>'0.0',
		  -size=>10,
		  -maxlength=>10),
	"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Residue / Mass Difference: ",
	popup_menu(-name=>'mrm_label2',
		   -values => \@res_list),
	textfield(-name=>'mrm_deltamz2',
		  -value=>'0.0',
		  -size=>10,
		  -maxlength=>10),
	br,
	"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Residue / Mass Difference: ",
	popup_menu(-name=>'mrm_label3',
		   -values => \@res_list),
	textfield(-name=>'mrm_deltamz3',
		  -value=>'0.0',
		  -size=>10,
		  -maxlength=>10),
	"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Residue / Mass Difference: ",
	popup_menu(-name=>'mrm_label4',
		   -values => \@res_list),
	textfield(-name=>'mrm_deltamz4',
		  -value=>'0.0',
		  -size=>10,
		  -maxlength=>10),
	br,
	"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Residue / Mass Difference: ",
	popup_menu(-name=>'mrm_label5',
		   -values => \@res_list),
	textfield(-name=>'mrm_deltamz5',
		  -value=>'0.0',
		  -size=>10,
		  -maxlength=>10),
	"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Residue / Mass Difference: ",
	popup_menu(-name=>'mrm_label6',
		   -values => \@res_list),
	textfield(-name=>'mrm_deltamz6',
		  -value=>'0.0',
		  -size=>10,
		  -maxlength=>10),
	br;

	my %mrm_tryptic_labels = (
		      'all-cont'  => ' Consider peptides in all contexts or',
		      'try-cont' => ' consider peptides in a tryptic context only'
		      );
        my %mrm_proteotypic_labels = (
                      'proteo'      => ' Retain proteotypic peptides only or',
		      'non-proteo'  => ' allow non-proteotypic peptides'
		      );
      print 
	  "When mapping peptides to proteins... ",
	br,
	    radio_group(-name=>'mrm_tryptic',
			-values=>['all-cont','try-cont'],
			-default=>'all-cont',
			-labels=>\%mrm_tryptic_labels),
        br,
            radio_group(-name=>'mrm_nounique',
			-values=>['proteo','non-proteo'],
			-default=>'proteo',
			-labels=>\%mrm_proteotypic_labels),
	br,br,


	b("Product Ion"),
	br,
	"Allowable charge states: ";

    for (1..5) {
	my $checked = ($_ == 1 || $_ == 2) ? 'checked' : '';
	print
	    checkbox(-name=>"mrm_ion$_",
		     -value=>'true',
		     -checked=>$checked,
		     -label=>"+$_ ");
    }

    print
	br,
	"Allowable Ion Types: ",
	checkbox(-name=>'mrm_ionb',
		 -value=>'true',
		 -label=>' b '),
	checkbox(-name=>'mrm_iony',
		 -value=>'true',
		 -label=>' y '),
	br,
	checkbox(-name=>'mrm_nloss',
		 -value=>'true',
		 -label=>'Allow Neutral Losses'),
	br,
	checkbox(-name=>'mrm_secloss',
		 -value=>'true',
		 -label=>'Allow Secondary Small Neutral Losses (e.g. water or ammonia)'),
	br,
	checkbox(-name=>'mrm_nonmono',
		 -value=>'true',
		 -label=>'Allow non monoisotopic peaks'),
	br,
	checkbox(-name=>'mrm_massshift',
		 -value=>'true',
		 -label=>'Allow mass-shifted ions'),
	br,
	"Fragment ion lengths to exclude: ";

    for (1..10) {
	print
	    checkbox(-name=>"mrm_length$_",
		     -value=>'true',
		     -label=>"$_ ");
    }

    print
	br,
	"</div>\n";


    print
	br,
	&printTitle(title=>'6. Generate MRM transition list'),
	"<div class=formentry>";

    if ($speclib_there && $dbfile_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runMaRiMba'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageCreateSpecLib
#
# Front-end to run Henry's SpectraST spectrum library search in 'Create'
# mode: to create indexed files
#
########################################################################
sub pageCreateSpecLib{

    my %labels = ( 'none'    => '-- Please choose --',
		   'msp'     => '.msp (NIST Library Format)',
		   'pepxml'  => '.pepXML (Sequence Search Results)',
		   'splib'   => '.splib (perform join/build actions on SpectraST libraries)' );

    # auto-select appropriate section (div) to be visible
    my $select_default = 'none';
    my %div_style = ( 'msp'     => 'hideit',
		      'pepxml'  => 'hideit',
		      'splib'   => 'hideit' );

    if (param('sil_fileformat')){
	$select_default = param('sil_fileformat');
	$div_style{$select_default} = '';
    } elsif (param('file_ext')) {
	$select_default = (param('file_ext') eq 'xml') ? 'pepxml' : param('file_ext');
	$div_style{$select_default} = '';
    }


    print
	br,
	&printTitle(title=>'1. Specify File Format'),
	"<div class=formentry>",
	"Specify File Format to Import: ",
	popup_menu(-name=>'sil_iff',
		   -values => [qw'none msp pepxml splib'],
		   -labels=>\%labels,
		   -default => $select_default,
		   -onChange => 'showForm(this);'),
	"</div>\n";


    # need some javascript
    print <<"EOSCRIPT";

    <script language='JavaScript'>
    function showForm(event) {
	var divs = new Array();
	divs[0] = 'msp';
	divs[1] = 'pepxml';
	divs[2] = 'splib';

	var divId = event.value;
	var x;
	for (x in divs) {
	    if (document.getElementById(divs[x]))
		document.getElementById(divs[x]).style.display='none';
	}
	if (document.getElementById(divId)) {
	    document.getElementById(divId).style.display='block';
	}
	return true;
    }

    function showFilter(event) {
	if (event.value == 'Quality_Filter') {
	    document.getElementById('qualityfilterdiv').style.display='block';
	    document.getElementById('join_action').disabled = true;
	}
	else {
	    document.getElementById('qualityfilterdiv').style.display='none';
	    document.getElementById('join_action').disabled = false;
	}
    }

    function showSubOpts(event) {
	if (event.value == 'Subtract' || event.value == 'Subtract_homologs') {
	    document.getElementById('primary_file').style.display='inline';
	}
	else {
	    document.getElementById('primary_file').style.display='none';
	}
    }


    </script>
EOSCRIPT


    Delete('sil_fileformat');
    Delete('file_ext');

    ### NIST SECTION ###
    print
        "<div id='msp' class=$div_style{'msp'}>\n";

    # files to process (from session)
    my $any_files_there = &showFiles('nistspeclib','spectrastlib','msp','2. Specify files to import','dbase/speclibs/');

    print
	br,
        start_form('POST',$tpp_url),
	&printTitle(title=>'3. General Options'),
	"<div class=formentry>",
	"Enter name of output file (optional): ",
	textfield(-name=>'sil_outf',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	".splib",
	br,
	"Enter name of log file (optional): ",
	textfield(-name=>'sil_logf',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	"</div>\n",
	br;

    print
	&printTitle(title=>'4. Advanced Options'),
	"<div class=formentry>",
	"Filter library spectra for criterion: ",
	br,
	popup_menu(-name=>'sil_filterfield',
		   -values => [qw'-- Name MW PrecursorMZ LibID Charge Mods Status FullName NumPeaks Protein Spec Pep Nreps Prob'],
		   -default => '--'),
	popup_menu(-name=>'sil_filterop',
		   -values => [qw'== != >= > <= < =~ !~'],
		   ),
	textfield(-name=>'sil_filterval',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'sil_extraopts',
		  -value=>'',
		  -size=>25,
		  -maxlength=>100),
	"</div>\n";

    print
	br,
	&printTitle(title=>'5. Import Libraries'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    hidden(-name=>'sil_fileformat',
		   -value=>'msp'),
	    submit(-name=>'Action',
		   -value=>$web_actions{'createidx'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form,
	"</div>\n";


    ### PEPXML SECTION ###
    print
	"<div id='pepxml' class=$div_style{'pepxml'}>\n";

    # files to process (from session)
    $any_files_there = &showFiles('nistspeclib','spectrastlib','xml','2. Specify files to import');

    print
	br,
	start_form('POST',$tpp_url),
	&printTitle(title=>'3. General Options'),
	"<div class=formentry>",
	"Enter name of output file (optional): ",
	textfield(-name=>'sil_outf',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	".splib",
	br,
	"Enter name of log file (optional): ",
	textfield(-name=>'sil_logf',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	br,
	"Specify a dataset identifier (optional): ",
	textfield(-name=>'sil_dataident',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	br,
	"Specify a minimum probability to import: ",
	textfield(-name=>'sil_minprob',
		  -value=>'0.9',
		  -size=>5,
		  -maxlength=>5),
	"</div>\n",
	br;

    print
	&printTitle(title=>'4. Advanced Options'),
	"<div class=formentry>",
	"Filter library spectra for criterion: ",
	br,
	popup_menu(-name=>'sil_filterfield',
		   -values => [qw'-- Name MW PrecursorMZ LibID Charge Mods Status FullName NumPeaks Protein Spec Pep Nreps Prob'],
		   -default => '---'),
	popup_menu(-name=>'sil_filterop',
		   -values => [qw'== != >= > <= < =~ !~'],
		   ),
	textfield(-name=>'sil_filterval',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'sil_extraopts',
		  -value=>'',
		  -size=>25,
		  -maxlength=>100),
	"</div>\n";

    print
	br,
	&printTitle(title=>'5. Import Libraries'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    hidden(-name=>'sil_fileformat',
		   -value=>'pepxml'),
	    submit(-name=>'Action',
		   -value=>$web_actions{'createidx'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form,
	"</div>\n";


    ### SPLIB SECTION ###
    # retrieve splib files
    my @splib_files;
    for (@session_data) {
	chomp;
	if (/^$proc_types{'nistspeclib'}:/) { push @splib_files, $'; } #'
    }

    print
	"<div id='splib' class=$div_style{'splib'}>\n";

    # files to process (from session)
    $any_files_there = &showFiles('nistspeclib','spectrastlib','splib','2. Specify files to build/join','dbase/speclibs/');

    print
	br,
	start_form('POST',$tpp_url),
	&printTitle(title=>'3. Select Actions'),
	"<div class=formentry>",
	"Select Build Action: ",
	popup_menu(-name=>'sil_build',
		   -values => [qw'None Consensus Best_Replicate Quality_Filter Decoy_Generation'],
		   -default => 'None',
		   -onChange => 'showFilter(this);'),
	br,
	"Select Join Action: ",
	popup_menu(-name=>'sil_join',
		   -id=>'join_action',
		   -values => [qw'Union Intersection Subtract Subtract_homologs Append'],
		   -onChange => 'showSubOpts(this);'),
	"<span id=primary_file class=hideit>\n",
	"--> Please specify file to subtract from: ",
	popup_menu(-name=>'sil_primaryfile',
		   -values => \@splib_files,
		   ),
	"</span>\n",
	"</div>\n";

    print
	br,
	&printTitle(title=>'4. General Options'),
	"<div class=formentry>",
	"Enter name of output file (optional): ",
	textfield(-name=>'sil_outf',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	".splib",
	br,
	"Enter name of log file (optional): ",
	textfield(-name=>'sil_logf',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	"</div>\n",
	br;


    print
	&printTitle(title=>'5. Advanced Options'),
	"<div class=formentry>",
	"Specify minimum number of replicates required: ",
	textfield(-name=>'sil_minreplicates',
		  -value=>'1',
		  -size=>5,
		  -maxlength=>5),
	br,br,
	"Specify number of artificial decoy spectrum generated per real spectrum: ",
	textfield(-name=>'sil_decoynum',
		  -value=>'1',
		  -size=>5,
		  -maxlength=>5),
	br,br,
	"Filter library spectra for criterion: ",
	br,
	popup_menu(-name=>'sil_filterfield',
		   -values => [qw'-- Name MW PrecursorMZ LibID Charge Mods Status FullName NumPeaks Protein Spec Pep Nreps Prob'],
		   -default => '---'),
	popup_menu(-name=>'sil_filterop',
		   -values => [qw'== != >= > <= < =~ !~'],
		   ),
	textfield(-name=>'sil_filterval',
		  -value=>'',
		  -size=>20,
		  -maxlength=>50),
	br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'sil_extraopts',
		  -value=>'',
		  -size=>25,
		  -maxlength=>100),
	"</div>\n",
	br;


    %labels = ( '0' => '0: No Filter',
		'1' => '1: Impure Spectra',
		'2' => '2: ... + spectra with look-alike having conflicting IDs',
		'3' => '3: ... + singleton spectra with unconfirmed peptide sequences',
		'4' => '4: ... + singleton spectra',
		'5' => '5: ... + inquorate spectra' );

    print
	&printTitle(title=>'6. Quality Filter Options'),
	"<div id=qualityfilterdiv class=formentry>",
	"Quality Level to Remove: ",
	popup_menu(-name=>'sil_qforemove',
		   -values => [qw'0 1 2 3 4 5'],
		   -labels=>\%labels,
		   -default => '0'),
	br,
	"Quality Level to Flag: ",
	popup_menu(-name=>'sil_qfoflag',
		   -values => [qw'0 1 2 3 4 5'],
		   -labels=>\%labels,
		   -default => '5'),

	"</div>\n";

    print
	br,
	&printTitle(title=>'7. Build/Join Libraries'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    hidden(-name=>'sil_fileformat',
		   -value=>'splib'),
	    submit(-name=>'Action',
		   -value=>$web_actions{'createidx'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form,
	"</div>\n";


    print
	"</div>\n";

}


########################################################################
# pageRunSpectraST
#
# Front-end to run Henry's SpectraST spectrum library search
#
########################################################################
sub pageRunSpectraST{

    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # files to process (from session)
    my $any_files_there = &showFiles('runsearch','runspectrast','mzxml','1. Specify mz[X]ML Input Files');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for next file select

    # spectral library file (from session)
    my $lib_file_there = &showFiles('speclib','runspectrast','splib','2. Specify Library File (not required if specified in params file below)','dbase/speclibs/');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for next file select

    # sequence database file (from session)
    my $dbfile_there = &showFiles('searchdb','runspectrast','*','3. Specify a sequence database to be printed to the output file for downstream processing (optional)','dbase/');

    # spectrast options

#-sF<file>   Read search options from <file>. If <file> is blank, spectrast.params is assumed.
#-sL<file>   Specify library file. (this is actually mandatory if the library is not specified in the params file)
#-sC<type>   Specify the expected modification on cysteines. <type> can be ICAT_cl (cleavable ICAT), ICAT_uc (uncleavable ICAT) or CAM.
#-sM<tol>    Specify m/z tolerance
#-sD<file>   Specify a sequence database to be printed to the output file for downstream processing.

    print
	br,
	start_form('POST',$tpp_url),
	&printTitle(title  => '4. SpectraST Options',
		    class  => 'formentryhead',
		    div_id => 'main'),
	"<div id=main class=formentry>",

	# switch to a file chooser for this? - HENRY: Yes, please do this...
	checkbox(-name=>'sst_useoptsfile',
		 -value=>'true',
		 -label=>'Read search options from file:'),
	textfield(-name=>'sst_optsfile',
		  -value=>'spectrast.params',
		  -size=>30,
		  -maxlength=>50),
	br,
	"Specify precursor m/z tolerance: ",
	textfield(-name=>'sst_mztolerance',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	" Th",
	br,
	checkbox(-name=>'sst_useavgmass',
		 -value=>'true',
		 -label=>'Use average mass'),
	br,
	"Specify sequence database type: ",
	popup_menu(-name=>'sst_dbtype',
		   -values => [qw'AA DNA'],
		   -default => 'AA'),
	br,
	"</div>\n";

# Advanced options:
#GENERAL
#-sR   Cache entire library in memory

    print
	br,
	&printTitle(title  => '5. Advanced Options',
		    class  => 'formentryhead',
		    div_id => 'adv'),
	"<div id=adv class=formentry>",
	h2('General'),
	checkbox(-name=>'sst_usecached',
		 -value=>'true',
		 -label=>'Cache all entries in memory'),
	br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'sst_extraopts',
		  -value=>'',
		  -size=>25,
		  -maxlength=>100),
	br,br;

#SELECTION AND SCORING
#-s2   Ignore all +1 spectra in the library
#-sk   Save query and matched library spectra as dta's
#-sz   Tgz the saved query and matched library spectra to save space
#-sW X Ignore all spectra marked New_ID
#-sG X Ignore all spectra marked Bad_ID  (default is already -sG), turn off by -sG!
    my %labels = ( 'none'    => '-- None --',
		   'ICAT_cl' => 'Cleavable ICAT',
		   'ICAT_uc' => 'Uncleavable ICAT',
		   'CAM'     => 'CAM' );

    print
	h2('Candidate Selection and Scoring'),
	"Detect identical/homologous lower hits up to rank = ",
	textfield(-name=>'sst_gethomohits',
		  -value=>'4',
		  -size=>4,
		  -maxlength=>4),
	br,
	"Specify expected Cysteine modification: ",
	popup_menu(-name=>'sst_cysmod',
		   -values => [qw'none ICAT_cl ICAT_uc CAM'],
		   -labels=>\%labels,
		   -default => 'none'),
	br,
	checkbox(-name=>'sst_ignoreplusone',
		 -value=>'true',
		 -label=>'Ignore all +1 spectra in the library'),
	br,
	checkbox(-name=>'sst_ignoreabnormal',
		 -value=>'true',
		 -label=>'Ignore spectra with abnormal status'),
	br,br;


#OUTPUT CONTROL
#-sE<ext>    Specify output format. <ext> can be txt (fixed-width text file), xls (tab-delimited text file), pepXML, and xml (same as pepXML)
#-sV<thres>  Minimum F value threshold for top hit to be displayed
#-sv<thres>  Minimum F value threshold for lower hit to be displayed
#-sO<dir>    Output directory
#-s1         only display top hits
#-s0         exclude query for which there is no match above F value threshold


    %labels = ( 'txt' => 'Fixed-width text',
		'xls' => 'Tab-delimited text',
		'pep.xml' => 'pepXML' );

    # get path from first xml file in list
    my $fdir;
    for (@session_data) {
	chomp;
	if (/$proc_types{'runsearch'}:/) {
	    $fdir  = dirname($');    #'}
	    last;
	}
    }

    print
	h2('Output Control'),
	"Specify output format: ",
	popup_menu(-name=>'sst_outformat',
		   -values => [qw'txt xls pep.xml'],
		   -labels=>\%labels,
		   -default => 'pep.xml'),
	br,
	"Write search results in directory: ",
	textfield(-name=>'sst_outdir',
		  -value=>$fdir,
		  -size=>70,
		  -maxlength=>200),
	br,
	"Minimum F value threshold for top hit to be displayed: ",
	textfield(-name=>'sst_tophitminf',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	br,
	"Minimum F value threshold for lower hit to be displayed: ",
	textfield(-name=>'sst_lowhitminf',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	br,
	checkbox(-name=>'sst_onlytop',
		 -value=>'true',
		 -label=>'Only display top hits'),
	br,
	checkbox(-name=>'sst_excludebelowf',
		 -value=>'true',
		 -label=>'Exclude query for which there is no match above F value threshold'),
	br,
	"</div>\n";

    print
	br,
	&printTitle(title=>'6. Search!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runspectrast'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageRunMascot
#
# Just a link to the local Mascot MS/MS Ions Search page
#
# Fetches "search log" page from Mascot server; displays page with links
# to download results (.dat) file to TPP-accessible area
#
########################################################################
sub pageRunMascot{

    my $search_url = "${mascot_server}cgi/search_form.pl?SEARCH=MIS";
    # contributed by C.Dantec
    my $mascotJob   = param('mascot_job');
    my $mascotDb    = param('mascot_db');
    my $mascotName  = param('mascot_name');
    my $mascotTi    = param('mascot_ti');

    Delete('Action');

    print
	&printTitle(title=>'1. Mascot Server URL'),
	"<div class=formentry>",
	start_form('POST',$tpp_url),
	p("The currently selected Mascot server is <b>$mascot_server</b>."),
	"Use a new server: ",
	textfield(-name=>'new_mascot_server',
		  -value=>"http://",
		  -size=>30,
		  -maxlength=>100),
	hidden(-name=>'Action',
	       -value=>$web_actions{'newMascotURL'})."\n",
	submit(-name=>'Action',
	       -value=>$web_actions{'newMascotURL'})."\n",
	end_form,
	"</div>\n";

    print
	br,
	&printTitle(title=>'2. Mascot Search'),
	"<div class=formentry>",
	"<p>Please click ",
	a({-href  => "$search_url",
	   -target=> "_blank"},
	  "here"),
	" to go to the Mascot search interface at <b>$mascot_server</b>.",
	"</p></div>\n";

    my $search_log_url =  "${mascot_server}x-cgi/ms-review.exe?CalledFromForm=1&logfile=..%2Flogs%2Fsearches.log&start=-1&howMany=50&pathToData=&column=0&s0=1&s1=1&s2=1&s3=1&s4=1&s5=1&s6=1&s7=1&s8=1&s9=1&s10=1&s11=1&s12=1&s14=1&f0=".$mascotJob."&f1=&f2=".$mascotDb."&f3=".$mascotName."&f4=&f5=".$mascotTi."&f6=&f7=&f8=&f9=&f10=&f11=&f12=&f14=";  # contributed by C.Dantec

    my @response = `$command{wget} -q -O - "$search_log_url"`;
#    my @response = `$command{wget} --http-user=mascotUser --http-password=mascotPassword -q -O - "$search_log_url"`; # LM: use this if you need to use a password to your Mascot server; fill in as required
    # check for error! FIXME

    my @mascot_files;
    my $previous = '';
    foreach (@response) {
	if ($_ =~ /ms-status.exe(.*\.dat)/) {
	    my $query = $1;  
	    if ($query =~ /DateDir=.*\&ResJob=.*\.dat/) {
		$previous =~ s|.*<TD>(.*?)</TD>.*|$1|i;
		$query =~ s/DateDir/NAME=$previous\&DateDir/;
		push @mascot_files, "$query";
	    } else {
		push @mascot_files, "Could not retrieve data file name<br>\n";
	    }
	}
	chomp($previous = $_);
    }

    print
	br,
	&printTitle(title  => '3. Choose Mascot "Intermediate File" to download',
		    class  => 'formentryhead',
		    div_id =>'getdat'),
	"<div id=getdat class=formentry>";

    # Contributed by Chris.Dantec
    print
	start_form('POST',$tpp_url),
	" Mascot Job Id ",
	textfield(-name=>'mascot_job',
		  -value=>"$mascotJob",
		  -size=>5,
		  -maxlength=>100),
	" Mascot DataBase ",
	textfield(-name=>'mascot_db',
		  -value=>"$mascotDb",
		  -size=>10,
		  -maxlength=>100),
	" Mascot User Name ",
	textfield(-name=>'mascot_name',
		  -value=>"$mascotName",
		  -size=>15,
		  -maxlength=>100),
	" Mascot Request Title ",
	textfield(-name=>'mascot_ti',
		  -value=>"$mascotTi" ,
		  -size=>5,
		  -maxlength=>100),
	hidden(-name=>'Action',
	       -value=>$web_actions{'filterdat'})."\n",
	hidden(-name=>'mascot_job',
	       -value=>$mascotJob),
	hidden(-name=>'mascot_db',
	       -value=>$mascotDb),
	hidden(-name=>'mascot_name',
	       -value=>$mascotName),
	hidden(-name=>'mascot_ti',
	       -value=>$mascotTi),
	submit(-name=>'Action',
	       -value=>$web_actions{'filterdat'})."\n",
	end_form;


    if (@mascot_files) {

	print
	    p(
	      "Please select a file to transfer from:",
	      b($mascot_server),
	      "&nbsp;&nbsp;&nbsp;[",
	      a({-href=>$search_log_url,
		 -target=> "_blank"},
		"View Search Log"),
	      "]",
	      ),
	    b(scalar(@mascot_files)),
	    "files found",
	    start_form('POST',$tpp_url);

	foreach (@mascot_files) {
	    # modifications contributed by C.Dantec
	    my $query = $_;
	    my $querySplit = $query;
	    if ($query =~ /\&BrowserSafe/){($query, my @temp) = split(/\&BrowserSafe/,$querySplit);}
	    if ($query =~ /NAME=(.*)\&DateDir=(.*)\&ResJob=(.*\.dat)/) {
		my $jn = $1;
		my $dt = $2;
		my $fn = $3;

		$dt =~ s/(\d\d\d\d)(\d\d)(\d\d)/$1-$2-$3/;

		print 
		    '<input type="radio" name="mascotsrcdat" value="',
		    $query,
		    '">',
		    "$dt -- <b>$fn</b> -- $jn</input>\n",  #'
		    br;
	    } else {
		print
		    '___',
		    b("Could not retrieve data file name"),
		    br,
		    "\n <!-- $query -->\n";
	    }
	}

	print
	    br,
	    checkbox(-name=>'mascotrename',
		     -value=>'true',
		     -checked=>1,
		     -label=>'Attempt to rename file to original base name of mz(X)ML file used in the search, post-download'),
	    br,
	    submit(-name=>'Action',
		   -value=>$web_actions{'fetchdat'}),
	    end_form;

    } else {

	print
	    p(
	      "No Mascot Intermediate files were found on ",
	      b($mascot_server),
	      "."
	      ),
	    p("Please verify that the search is done, and that you have configured the URL correctly.");

    }


# save this for a future implementation of a smarter display...
#    my @rowelements;
#
#    foreach (@response) {
#
#	next unless ($_ =~ /ms-status.exe/);
#
#	s|<TR>||g;
#	s|</TR>||g;
#	s|<TD NOWRAP>|<TD>|g;
#	s|</TD>||g;
#	s|\s+\"|\"|g;
#
#	@rowelements = split /<TD>/;
#
#	foreach (@rowelements) {
#	    print "$_ : ";
#	}
#	print br;
#    }

    print
	"</div>\n";
}


########################################################################
# pageMzXml2Other
#
# The MzXML2Search converter page
#
########################################################################
sub pageMzXml2Other {

    # files to process (from session)
    my $any_files_there = &showFiles('mz2other','mzxml2search','mzxml','1. Specify mz[X]ML Input Files');

    # options
    my %so_labels = (
		     'dta' => 'dta format (.dta)',
		     'mgf' => 'Mascot Generic format (.mgf)',
		     'pkl' => 'Micromass format (.pkl)',
		     'xdta'=> 'X! Tandem single file DTA (.xdta)',
		     'odta'=> 'OMSSA merged single file DTA (.odta)',
		     'ms2' => 'Sequest MS2 format (.ms2)',
		     );

    print
	br,
	&printTitle(title=>'2. Choose Output Format and Options'),
	"<div class=formentry>",
	start_form('POST',$tpp_url),
	"Pick an output file format: ",
	br,
	radio_group(-name=>'m2s_format',
		    -values=>['dta','mgf','pkl','xdta','odta','ms2'],
		    -default=>'dta',
		    -linebreak=>'true',
		    -labels=>\%so_labels),
	br,
	"MS Level (or range) to export: ",
	popup_menu(-name=>'m2s_mslevel',
		   -values => [qw'1 2 3'],
		   -default => '2',
		   ),
	" to (range) ",
	popup_menu(-name=>'m2s_mslevel2',
		   -values => ['',qw'2 3 4 5 6'],
		   -default => '',
		   ),
	br,br,

	"Minimum peak count: ",
	textfield(-name=>'m2s_minpeaks',
		  -value=>'5',
		  -size=>5,
		  -maxlength=>5),
	br,
	"Maximum reported charge state for scans that have a precursor charge: ",
	popup_menu(-name=>'m2s_maxcharge',
		   -values => [1..15],
		   -default => '7',
		   ),
	" (useful when scan has a high charge that search engines can't handle)",
	br,
	"Minimum threshold for peak intensity: ",
	textfield(-name=>'m2s_minintensity',
		  -value=>'0.01',
		  -size=>10,
		  -maxlength=>10),
	br,
	"Limit output to only ",
	textfield(-name=>'m2s_maxpeaks',
		  -value=>'0',
		  -size=>5,
		  -maxlength=>5),
	" most intense peaks (set to zero to output all peaks)",
	br,br,

	"<table>\n<tr>",
	"<td align='right'>Scan Range:</td><td align='right'>First ",
	textfield(-name=>'m2s_scanstart',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	"</td><td align='right'>to last ",
	textfield(-name=>'m2s_scanend',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	"</td><td>(default is all)</td></tr>\n",
	"<tr><td align='right'>MH+ Mass Range:</td><td align='right'>Minimum ",
	textfield(-name=>'m2s_massmin',
		  -value=>'600.0',
		  -size=>10,
		  -maxlength=>10),
	"</td><td align='right'>Maximum ",
	textfield(-name=>'m2s_massmax',
		  -value=>'4200.0',
		  -size=>10,
		  -maxlength=>10),
	"</td><td>(Daltons)</td></tr>\n</table>\n",
	br,

	"Output precision -- ",
	"Mass:",
	textfield(-name=>'m2s_precmass',
		  -value=>'4',
		  -size=>4,
		  -maxlength=>4),
	"&nbsp;&nbsp;&nbsp; Intensity:",
	textfield(-name=>'m2s_precint',
		  -value=>'0',
		  -size=>4,
		  -maxlength=>4),
	br,br,

	"Precursor charge (or range) to analyze:",
	popup_menu(-name=>'m2s_charge1',
		   -values => ['',qw'1 2 3 4 5 6'],
		   -default => '',
		   ),
	" to (range) ",
	popup_menu(-name=>'m2s_charge2',
		   -values => ['',qw'1 2 3 4 5 6'],
		   -default => '',
		   ),
	br,
	checkbox(-name=>'m2s_chargedefault',
		 -value=>'true',
		 -label=>'Use charge(s) as default (instead of overriding input file) '),
	br,br,
	checkbox(-name=>'m2s_remprec',
		 -value=>'true',
		 -label=>'Remove charge-reduced precursors from spectra (suitable for ETD) '),
	br,
	checkbox(-name=>'m2s_remitraq',
		 -value=>'true',
		 -label=>'Remove iTRAQ reporter peaks in the range 112-122 Th.) '),
	br,
	checkbox(-name=>'m2s_remtmt',
		 -value=>'true',
		 -label=>'Remove TMT reporter peaks in the range 126-132 Th.) '),
	br,br,
	"Activation method: ",
	popup_menu(-name=>'m2s_activation',
		   -values => [qw'CID ETD HCD all'],
		   -default => 'CID',
		   ),
	" (ignored if activation method not in scans of mz[X]ML file)",
	"</div>\n";

    print
	br,
	&printTitle(title=>'3. Convert!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'mzxml2other'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageDta2MzXml
#
# The dta2mzxml page
#
########################################################################
sub pageDta2MzXml {

    # files to process (from session)
    my $any_files_there = &showFiles('mzxml','dta2mzxml','DIRS','1. Specify Directories with .dta File(s) to convert to mzXML');

    print
	br,
	&printTitle(title=>'2. Conversion Options'),
	"<div class=formentry>",
	start_form('POST',$tpp_url),

	checkbox(-name=>'dta2mz_recount',
		 -value=>'true',
		 -label=>'Override old dta numbering'),
	br,
	checkbox(-name=>'dta2mz_charge',
		 -value=>'true',
		 -label=>'Include the charge value from the dta files into the mzXML file'),
	br,
	checkbox(-name=>'dta2mz_plus2',
		 -checked=>1,
		 -value=>'true',
		 -label=>'Process the +2 charge file only, if scan files exist for multiple charges'),
	br,
	checkbox(-name=>'dta2mz_byname',
		 -checked=>1,
		 -value=>'true',
		 -label=>'Use the basename of the dta files to create mzXML files with the same basenames'),
	br,
	checkbox(-name=>'dta2mz_gzip',
		 -checked=>$doMzXMLGzip,
		 -value=>'true',
		 -label=>'Write output as a gzipped file (.mzxml.gz - saves space, other TPP tools can read it directly)'),
	br,
	"</div>\n";

    print
	br,
	&printTitle(title=>'3. Convert!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'dta2mzxml'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageIndexMzXml
#
# The indexmzXML page
#
########################################################################
sub pageIndexMzXml {

    # files to process (from session)
    my $any_files_there = &showFiles('mz2other','indexmzxml','mzxml','1. Specify mzXML Files');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title=>'2. Run indexmzXML'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runIndexmzxml'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageCreateChargeFile
#
# The createChargeFile page
#
########################################################################
sub pageCreateChargeFile {

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for sequest params file select

    # files to process (from session)
    my $any_files_there = &showFiles('tocharge','chargefile','DIRS','1. Specify Directories with .dta File(s) from which to extract charges');


    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # files to process (from session)
    $any_files_there += &showFiles('tocharge','chargefile','ms2','2. Specify .ms2 files from which to extract charges');


    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title=>'3. Create .charge files!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'chargefile'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageMergeCharges
#
# The mergeCharges page
#
########################################################################
sub pageMergeCharges {

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for sequest params file select

    # files to process (from session)
    my $any_files_there = &showFiles('runsearch','mergecharges','mzxml','1. Specify mz[X]ML File(s) into which to insert updated charges');

    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # files to process (from session)
    my $any_charges_there = &showFiles('chargefile','mergecharges','charge','2. Specify .charge files with updated charge information');


    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title=>'3. Update charge information!'),
	"<div class=formentry>";

    if ($any_files_there && $any_charges_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'mergecharges'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageMzXml
#
# The mzXML converter page (on deprecation road...)
#
########################################################################
sub pageMzXml {

    Delete('Action');

    my %labels = ( 'raw'       => 'Thermo RAW',
		   'rawdir'    => 'Waters .raw Directories',
		   'wiff'      => 'ABI/Agilent .wiff',
		   'ddir'      => 'Agilent .d Directories'
		   );

    print
	&printTitle(title  => '1. Input File Format',
		    class  => 'formentryhead',
		    div_id => 'input'),
	"<div id=input class=formentry>",
	start_form(-method => 'POST',
		   -action => $tpp_url,
		   -name => 'switchRawFile'),
	'Select instrument file type you want to convert: ',
	hidden(-name=>'Action',
	       -value=>$web_actions{'switchRawFile'}),
	hidden(-name=>'refpage',
	       -value=>'mzxml'),
	popup_menu(-name=>'rawfile_type',
		   -values => [qw'wiff ddir raw rawdir'],
		   -default => $rawfile,
		   -labels=>\%labels,
		   -onChange => 'document.forms.switchRawFile.submit();'),
	h5('Please note that these converters will only work on machines that contain the appropriate vendor libraries.'),
	end_form,
	"</div>\n";

    my $exe = ($rawfile eq 'raw')    ? $command{readw} :
	      ($rawfile eq 'rawdir') ? $command{masswolf} :
	      ($rawfile eq 'wiff')   ? $command{mzwiff} :
	      ($rawfile eq 'ddir')   ? $command{trapper} :
	      'NOTVALID';

    if (! -e $exe) {
	print
	    br,
	    &printTitle(title  => ' WARNING ',
			class  => 'messageshead'),
	    "<div class=messages>\n",
	    "<li>It appears that you do not have the legacy mzXML converter ($exe) installed for the type of raw file that you are trying to convert to mzXML.</li>\n",
	    "<li>You may want to:</li><ol>",
	    "<li>Switch to a different raw file type above</li>",
	    "<li>Use the <a href=\"$tpp_url?Action=$web_actions{'showpage'}&page=msconvert\">msconvert utility</a> to do the conversion (recommended)</li>",
	    "<li>Install the appropriate legacy converter (*not* recommended)</li>",
	    "</ol></div><br>\n\n";

	return;
    }

    my $choosetype = ($rawfile =~ /dir$/) ? 'DIRS' : $rawfile;
    my $choosestr  = ($rawfile =~ /dir$/) ? 'Directory(ies)' : 'File(s)';
    # files to process (from session)
    my $any_files_there = &showFiles('mzxml','mzxml',$choosetype,"2. Specify $choosestr to convert to mzXML");

    print
	br,
	&printTitle(title  => '3. Conversion Options',
		    class  => 'formentryhead',
		    div_id => 'conv'),
	"<div id=conv class=formentry>",
	start_form('POST',$tpp_url);

    if ($rawfile eq "raw") {
	print
	    checkbox(-name=>'tomz_centroid',
		     -value=>'true',
		     -label=>'Centroid all scans (MS1 and MS2) -- meaningful only if data were acquired in profile mode'),
	    br;
    } elsif ($rawfile eq "wiff") {
	print
	    "Centroid the following scans: ",
	    checkbox(-name=>'tomz_centroid1',
		     -value=>'true',
		     -label=>'MS1'),
	    checkbox(-name=>'tomz_centroid2',
		     -value=>'true',
		     -label=>'MS2'),
	    " (meaningful only if data were acquired in profile mode)",
	    br;
    }

    print
	checkbox(-name=>'tomz_zlib',
		 -value=>'true',
		 -label=>'Compress peak lists for smaller output file'),br;
   print
	checkbox(-name=>'tomz_gzip',
		 -value=>'true',
		 -checked=>$doMzXMLGzip,
		 -label=>'Write the output as a gzipped file (other TPP tools can read gzipped files directly)'),
	"</div>\n";

    my $step = 4;
    if ($rawfile eq "wiff") {

	my %labels = ( 'analyst'   => 'Assume Analyst library',
		       'analystqs' => 'Assume AnalystQS library',
		       'none'      => 'Use library specified in file (default)' );

	print
	    br,
	    &printTitle(title  => '4. Advanced (wiff) Options',
			class  => 'formentryhead',
			div_id => 'wiffopts'),
	    "<div id=wiffopts class=formentry>",
	    "Ionisation used: ",
	    textfield(-name=>'tomz_ionisation',
		      -value=>'',
		      -size=>10,
		      -maxlength=>10),
	    br,
	    "Mass Spectrometry type: ",
	    textfield(-name=>'tomz_mstype',
		      -value=>'',
		      -size=>10,
		      -maxlength=>10),
	    br,
	    "Detector: ",
	    textfield(-name=>'tomz_detector',
		      -value=>'',
		      -size=>10,
		      -maxlength=>10),
	    br,
	    checkbox(-name=>'tomz_usemzinfo',
		     -value=>'true',
		     -label=>'Use information recorded in wiff file instead (over-rides text above)'),
	    br,br,
	    radio_group(-name=>'tomz_analyst',
			-values=>['analyst','analystqs','none'],
			-default=>'none',
			-linebreak=>'true',
			-labels=>\%labels),
	    br,
	    "* Please note that more options are available via the command-line version of this tool (mzWiff)",
	    br,
	    "Enter additional options to pass directly to the command-line (expert use only!) ",
	    textfield(-name=>'tomz_extraopts',
		      -value=>'',
		      -size=>25,
		      -maxlength=>100),
	    "</div>\n";

	$step++;
    }

    print
	br,
	&printTitle(title=>"$step. Convert!"),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'mzxmlgen'});
    } else {
	print
	    h2('No files selected yet.').br;
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageIdConvert
#
# The idconvert page
#
########################################################################
sub pageIdConvert {

    # protxml files (from session)
    my $any_files_there = &showFiles('protxml','idconvert','prot.xml','1. Specify ProteinProphet ProtXML Files');

    Delete('file_ext');
    Delete('proc_type'); 

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title=>'2. Convert!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'toMzIdentML'}),
	    br;
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	br,
	"</div>\n",
	end_form;

}


########################################################################
# pageMsConvert
#
# The msconvert page
#
########################################################################
sub pageMsConvert {

    Delete('Action');

    my %labels = ( 'raw'    => 'Thermo RAW',
		   'rawdir' => 'Waters .raw Directories',
		   'wiff'   => 'ABI/Agilent .wiff',
		   'baf'    => 'Bruker .baf',
		   'yep'    => 'Bruker .yep',
		   'ddir'   => 'Agilent .d Directories'
		   );

    print
	&printTitle(title  => '1. Input File Format',
		    class  => 'formentryhead',
		    div_id => 'input'),
	"<div id=input class=formentry>",
	start_form(-method => 'POST',
		   -action => $tpp_url,
		   -name => 'switchRawFile'),
	'Select instrument file type you want to convert: ',
	hidden(-name=>'Action',
	       -value=>$web_actions{'switchRawFile'}),
	hidden(-name=>'refpage',
	       -value=>'msconvert'),
	popup_menu(-name=>'rawfile_type',
		   -values => [qw'wiff ddir baf yep raw rawdir'],
		   -default => $rawfile,
		   -labels=>\%labels,
		   -onChange => 'document.forms.switchRawFile.submit();'),
	h5('Please note that, in certain instances, this converter will only work on machines that contain the appropriate vendor libraries.'),
	end_form,
	"</div>\n";

    my $choosetype = ($rawfile =~ /dir$/) ? 'DIRS' : $rawfile;
    my $choosestr  = ($rawfile =~ /dir$/) ? 'Directory(ies)' : 'File(s)';
    # files to process (from session)
    my $any_files_there = &showFiles('mzml','msconvert',$choosetype,"2. Specify $choosestr to convert to mzML");

    print
	br,
	&printTitle(title  => '3. Conversion Options',
		    class  => 'formentryhead',
		    div_id => 'conv'),
	"<div id=conv class=formentry>",
	start_form('POST',$tpp_url),
	checkbox(-name=>'msconvert_centroid',
		 -value=>'true',
		 -label=>'Centroid all scans (MS1 and MS2) -- meaningful only if data was acquired in profile mode'),
	br,
	checkbox(-name=>'msconvert_zlib',
		 -value=>'true',
		 -label=>'Compress peak lists for smaller output file'),
	br,
	checkbox(-name=>'msconvert_gzip',
		 -value=>'true',
		 -checked=>$doMzXMLGzip,
		 -label=>'Write the output as a gzipped file (other TPP tools can read gzipped files directly)'),
	br,br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'msconvert_extraopts',
		  -value=>'',
		  -size=>50,
		  -maxlength=>100),
	br,
	checkbox(-name=>'msconvert_mzxml',
		 -value=>'true',
		 -label=>'Convert to mzXML instead of mzML'),
	"</div>\n";

    print
	br,
	&printTitle(title=>'3. Convert!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'mzmlgen'});
    } else {
	print
	    h2('No files selected yet.').br;
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageRunSearch
#
# The database search (Sequest) page
#
########################################################################
sub pageRunSearch{

    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # files to process (from session)
    my $any_files_there = &showFiles('runsearch','runsearch','mzxml','1. Specify mz[X]ML Input Files');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for sequest params file select

    # sequest.params file (from session)
    my $params_file_there = &showFiles('sequest','runsearch','*params*','2. Specify Sequest Parameters File');


    # runsearch options
    my %labels = ( 'all'       => 'Analyze all peptides',
		   'plusone'   => 'Analyze +1 peptides only',
		   'plustwo'   => 'Analyze +2 peptides only',
		   'plusthree' => 'Analyze +3 peptides only' );
    print
	br,
	&printTitle(title  => '3. RunSearch Options',
		    class  => 'formentryhead',
		    div_id => 'run'),
	"<div id=run class=formentry>",
	start_form('POST',$tpp_url),
	"Start at scan number: ",
	textfield(-name=>'run_scanstart',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	br,
	"End at scan number: ",
	textfield(-name=>'run_scanend',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	br,
	checkbox(-name=>'run_research',
		 -value=>'true',
		 -label=>'Force search (even if output file exists)'),
	br,
	"Minimum number of peaks required to create .dta file: ",
	textfield(-name=>'run_numpeaks',
		  -value=>'15',
		  -size=>10,
		  -maxlength=>10),
	br,
	radio_group(-name=>'run_peptides',
		  -values=>['all','plusone','plustwo','plusthree'],
		  -default=>'all',
		  -linebreak=>'true',
		  -labels=>\%labels),
	br,
	"</div>\n";


    print
	br,
	&printTitle(title=>'4. Search!'),
	"<div class=formentry>";

    if ($any_files_there && $params_file_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runsearch'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageRunTandem
#
# The database search (X-Tandem-k) page
#
########################################################################
sub pageRunTandem{

    Delete('file_ext');
    Delete('proc_type');   # delete these...

    # files to process (from session)
    my $any_files_there = &showFiles('runsearch','runtandem','mzxml','1. Specify mz[X]ML Input Files');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for next file select

    # tandem params file (from session)
    my $params_file_there = &showFiles('sequest','runtandem','*params*','2. Specify Tandem Parameters File');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for next file select

    # sequence database file (from session)
    my $dbfile_there = &showFiles('searchdb','runtandem','*','3. Specify a sequence database','dbase/');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title=>'4. Options'),
	"<div class=formentry>",
	checkbox(-name=>'tan_2pep',
		 -checked=>1,
		 -value=>'true',
		 -label=>'Convert output files to pepXML'),
	"</div>\n";

    print
	br,
	&printTitle(title=>'5. Search!'),
	"<div class=formentry>";

    if ($any_files_there && $params_file_there && $dbfile_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runtandem'});

	# add "run on the cloud" option
	&printClusterOptions();

    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageRunComet
#
# The Comet database search page
#
########################################################################
sub pageRunComet{
    Delete('file_ext');
    Delete('proc_type');   # delete these...

    print
	"<p>Need more ",
	a({target=>'comet', href=>"http://comet-ms.sourceforge.net"},'information on Comet or its parameters'),
	'?</p>';

    # files to process (from session)
    my $any_files_there = &showFiles('runsearch','runcomet','mzxml','1. Specify mz[X]ML Input Files');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for next file select

    # comet params file (from session)
    my $params_file_there = &showFiles('sequest','runcomet','*params*','2. Specify Comet Parameters File');

    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for next file select

    # sequence database file (from session)
    my $dbfile_there = &showFiles('searchdb','runcomet','*','3. Specify a sequence database','dbase/');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title=>'4. Search!'),
	"<div class=formentry>";

    if ($any_files_there && $params_file_there && $dbfile_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runcomet'});

	# And allow running on cluster
	&printClusterOptions();
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}


########################################################################
# pageConverters
#
# The pepXML converters page
#
########################################################################
sub pageConverters {
    my $any_files_there;

    if ($pipeline eq 'Mascot') {
	Delete('file_ext');
	Delete('proc_type');   # clear these...

	# files to process (from session)
	$any_files_there = &showFiles('converters','topepxml','dat','1. Files to convert to pepXML');

	Delete('file_ext');
	Delete('proc_type');   # otherwise it gets set wrong for database file select

	# search database file (from session)
	my $database_file_there = &showFiles('searchdb','topepxml','*','2. Specify Database Used in MASCOT Search','dbase/');
	$any_files_there &&= $database_file_there;

    } elsif ($pipeline eq 'Tandem') {

	# files to process (from session)
	$any_files_there = &showFiles('converters','topepxml','tandem','1. Files to convert to pepXML');


    } else {   # Sequest
	Delete('file_ext');
	Delete('proc_type');   # clear these...

	# files to process (from session)
	$any_files_there = &showFiles('converters','topepxml','DIRS','1. Directory with .out files to convert to pepXML');

	Delete('file_ext');
	Delete('proc_type');   # otherwise it gets set wrong for sequest params file select

	# sequest.params file (from session)
	my $params_file_there = &showFiles('sequest','topepxml','params','2. (Optional) Specify Sequest Parameters File');
    }


    if ($pipeline eq 'Mascot' || $pipeline eq 'Sequest') {

	# Out2XML options
	my %labels = ( 'default' => 'Use default masses (as specified in sequest.params)',
		       'mono'    => 'Use monoisotopic masses',
		       'average' => 'Use average masses' );

	print
	    start_form('POST',$tpp_url),
	    br,
	    &printTitle(title=>'3. Options'),
	    "<div class=formentry>",
	    "Enzyme: ",
	    popup_menu(-name=>'c_enzyme',
		       -values => [qw'trypsin nonspecific argc aspn chymotrypsin clostripain cnbr elastase formicacid gluc gluc_bicarb iodosobenzoate lysc lysc-p lysn lysn_promisc pepsina protein_endopeptidase ralphtrypsin staph_protease stricttrypsin tca trypsin/cnbr trypsin_gluc trypsin_k trypsin_r thermolysin']),
	    checkbox(-name=>'c_semi',
		     -value=>'true',
		     -label=>'Semi'),
	    br,
	    checkbox(-name=>'c_pI',
		     -value=>'true',
		     -label=>'Compute peptide pI values'),
	    br,br;

	if ($pipeline eq 'Sequest') {
	    print
		"<div class=gray>Sequest files only:</div>",
		"Number of top hits to report: ",
		popup_menu(-name=>'c_hits',
		       -values => [1..10]),
		br,
		checkbox(-name=>'c_allpeps',
			 -value=>'true',
			 -label=>'Output all peptides, don\'t filter out X containing peptides'),
		br,
		checkbox(-name=>'c_maldi',
			 -value=>'true',
			 -label=>'MALDI Mode'),
		br,
		radio_group(-name=>'c_masses',
			    -values=>['default','mono','average'],
			    -default=>'default',
			    -linebreak=>'true', 	 
			    -labels=>\%labels);

	} elsif ($pipeline eq 'Mascot') {
	    print
		"<div class=gray>Mascot files only:</div>",
		checkbox(-name=>'c_notgz',
			 -value=>'true',
			 -label=>'Do not generate (compressed) archive of .dta and .out'),
		br,
		checkbox(-name=>'c_desc',
			 -value=>'true',
			 -label=>'Generate protein description in pepXML output'),
		br,
		checkbox(-name=>'c_shortid',
			 -value=>'true',
			 -label=>'Use short protein id as per Mascot result (instead of full protein ids in fasta file)');
	}

	print
	    br,
	    "</div>\n";

	print
	    br,
	    &printTitle(title=>'4. Convert!'),
	    "<div class=formentry>";
    } else {
	print
	    start_form('POST',$tpp_url),
	    br,
	    &printTitle(title=>'2. Convert!'),
	    "<div class=formentry>";
    }

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'toPepXML'}),
	    br;
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	br,
	"</div>\n",
	end_form;

}


########################################################################
# pageXInteract
#
# The XInteract parameters page
#
########################################################################
sub pageXInteract {

    # files to process (from session)
    my $pepxml_ext = `$command{tpp_hostname} GET_PEPXML_EXT!`; # get TPP's idea of canonical pepxml file ext

    my $any_files_there = &showFiles('xinteract','xinteract','xml','Select File(s) to Analyze');


    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => 'Output File and Filter Options',
		    class  => 'formentryhead'),
	"<div class=formentry>";

    # output file
    if ($any_files_there) {
	my $fdir = '';

	# get path from first xml file in list
	for (@session_data) {
	    chomp;
	    if (/$proc_types{'xinteract'}:/) {
		$fdir  = dirname($');    #'}
		last;
	    }
	}

	print
	    "File path (folder): ",
	    textfield(-name=>'xinter_outd',
		      -value=>"$fdir",
		      -size=>70,
		      -maxlength=>200),
	    br,
	    "Write output to file: ",
	    textfield(-name=>'xinter_outf',
		      -value=>"interact$pepxml_ext", # default was .xml before Jan 2008
		      -size=>70,
		      -maxlength=>100);

	if ($any_files_there > 1) {
	    print
		checkbox(-name=>'xinter_multfiles',
			 -value=>'true',
			 -label=>'Do not merge into single analysis file (process each input file independently)');
	}

	print
	    br,br,
	    "Filter out results below this PeptideProphet probability: ",
	    textfield(-name=>'xinter_pppfilter',
		      -value=>'0.05',
		      -size=>10,
		      -maxlength=>9),
	    br,
	    "Minimum peptide length considered in the analysis: ",
	    textfield(-name=>'xinter_pppeplen',
		      -value=>'7',
		      -size=>3,
		      -maxlength=>2);
    } else {
	print
	    h2('Please select input files first.');
    }

    print "</div>\n";

    # peptideprophet options
    print
	br,
	&printTitle(title  => 'PeptideProphet Options',
		    class  => 'formentryhead',
		    div_id => 'pep'),
	"<div id=pep class=formentry>",
	checkbox(-name=>'pep_run',
		 -checked=>1,
		 -value=>'true',
		 -label=>'RUN PeptideProphet'),
	br,
	checkbox(-name=>'pep_accmass',
		 -value=>'true',
		 -label=>'Use accurate mass binning, using: '),
	popup_menu(-name=>'pep_accmassunit',
		   -values => [qw'PPM Daltons']),
	br,
	checkbox(-name=>'pep_pI',
		 -value=>'true',
		 -label=>'Use pI information'),
	br,
	checkbox(-name=>'pep_phospho',
		 -value=>'true',
		 -label=>'Use Phospho information'),
	br,
	checkbox(-name=>'pep_nglyc',
		 -value=>'true',
		 -label=>'Use N-glyc motif information'),
	br,
	checkbox(-name=>'pep_hydro',
		 -value=>'true',
		 -label=>'Use Hydrophobicity / RT information'),
	br,
	checkbox(-name=>'pep_icat',
		 -value=>'true',
		 -label=>'Use icat information'),
	br,
	checkbox(-name=>'pep_noicat',
		 -value=>'true',
		 -label=>'Do not use icat information'),
	br,
	checkbox(-name=>'pep_nontt',
		 -value=>'true',
		 -label=>'Do not use the NTT model'),
	br,
	checkbox(-name=>'pep_nonmc',
		 -value=>'true',
		 -label=>'Do not use the NMC model'),
	br,
	checkbox(-name=>'pep_maldi',
		 -value=>'true',
		 -label=>'MALDI data'),
	br;

    if ( ($pipeline eq "Tandem") || ($pipeline eq "Comet") ) {
	print
	    checkbox(-name=>'pep_expect',
		     -value=>'true',
		     -label=>'Only use Expect Score as the discriminant - helpful for data with homologous top hits, e.g. phospho or glyco (Tandem and Comet only)'),
	    br,
	    checkbox(-name=>'pep_neggamma',
		     -value=>'true',
		     -label=>'Use Gamma distribution to model the negatives (Tandem only)'),
	    br;

    } else {
	print
	    checkbox(-name=>'pep_xclaster',
		     -value=>'true',
		     -label=>'Exclude all entries with asterisked score values'),
	    br,
	    checkbox(-name=>'pep_nclaster',
		     -value=>'true',
		     -label=>'Leave alone all entries with asterisked score values'),
	    br;
    }

    print
	checkbox(-name=>'pep_force',
		 -value=>'true',
		 -label=>'Force the fitting of the mixture model (bypass automatic mixture model checks)'),
	br,
	checkbox(-name=>'pep_usedecoy',
		 -value=>'true',
		 -label=>'Use decoy hits to pin down the negative distribution.'),
	"Decoy Protein names begin with: ",
	textfield(-name=>'pep_decoystr',
		  -value=>'',
		  -size=>15,
		  -maxlength=>50),
	"(whitespace not allowed)",
	br,
	'&nbsp;'x5,
	checkbox(-name=>'pep_nonparam',
		 -value=>'true',
		 -label=>'Use Non-parametric model (can only be used with decoy option)'),
	br,
	checkbox(-name=>'pep_decoyprobs',
		 -value=>'true',
		 -label=>'Report decoy hits with a computed probability (based on the model learned).'),
	br,br,
	"Ignore charge states: ";

    for (1..5) {
	print
	    checkbox(-name=>"pep_ign$_",
		     -value=>'true',
		     -label=>"+$_ ");
    }

    print
	br,br,
	checkbox(-name=>'pep_prot',
		 -value=>'true',
		 -label=>'Run ProteinProphet afterwards'),
	br,br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'pep_extraopts',
		  -value=>'',
		  -size=>35,
		  -maxlength=>100),
	"</div>\n";


    # interprophet options
    print
	br,
	&printTitle(title  => 'InterProphet Options',
		    class  => 'formentryhead',
		    div_id => 'ipro'),
	"<div id=ipro class=formentry>",

	checkbox(-name=>'ipro_run',
		 -value=>'true',
		 -label=>'RUN InterProphet'),
	br,
	checkbox(-name=>'ipro_prot',
		 -value=>'true',
		 -label=>'Also run ProteinProphet on these results'),
	br,br,
	checkbox(-name=>'ipro_nonrs',
		 -value=>'true',
		 -label=>' do NOT use number of replicate spectra (NRS) model'),
	br,
	checkbox(-name=>'ipro_nonsi',
		 -value=>'true',
		 -label=>' do NOT use number of sibling ions (NSI) model'),
	br,
	checkbox(-name=>'ipro_nonsm',
		 -value=>'true',
		 -label=>' do NOT use number of sibling modifications (NSM) model'),
	br,
	checkbox(-name=>'ipro_nonss',
		 -value=>'true',
		 -label=>' do NOT use number of sibling searches (NSS) model'),
	br,
	checkbox(-name=>'ipro_nonse',
		 -value=>'true',
		 -label=>' do NOT use number of sibling experiments (NSE) model'),
	br,
	checkbox(-name=>'ipro_nonsp',
		 -value=>'true',
		 -label=>' do NOT use number of sibling peptides (NSP) model'),
	"</div>\n";


    # ptmprophet options
    print
	br,
	&printTitle(title  => 'PTMProphet Options',
		    class  => 'formentryhead',
		    div_id => 'ptmpro'),
	"<div id=ptmpro class=formentry>",

	checkbox(-name=>'ptmproph_run',
		 -value=>'true',
		 -label=>'RUN PTMProphet'),
	br,
	"Specify modifications:",
	br,
	"Residue(s): ",
	textfield(-name=>'ptmproph_res1',
		  -value=>'STY',
		  -size=>5,
		  -maxlength=>10),
	" Mass shift: ",
	textfield(-name=>'ptmproph_res1md',
		  -value=>'79.966',
		  -size=>5,
		  -maxlength=>10),
	br;

    for (2..5) {
	print
	    "Residue(s): ",
	    textfield(-name=>"ptmproph_res$_",
		      -value=>'',
		      -size=>5,
		      -maxlength=>10),
	    " Mass shift: ",
	    textfield(-name=>"ptmproph_res${_}md",
		      -value=>'',
		      -size=>5,
		      -maxlength=>10),
	    br;
    }

    print
	"m/z tolerance: ",
	textfield(-name=>'ptmproph_mztol',
		  -value=>'0.1',
		  -size=>5,
		  -maxlength=>5),
	" daltons",
	br,
	checkbox(-name=>'ptmproph_noupdate',
		 -value=>'true',
		 -label=>'Do not update modification_info tags in pepXML'),
	"</div>\n";


    # xpress options
    my %labels      = ( 'light' => 'For ratio, set/fix light to 1, vary heavy',
			'heavy' => 'For ratio, set/fix heavy to 1, vary light',
			'-'     => 'Default (set less abundant in ratio to 1)');
    my %m_labels    = ( 'light' => '...assume IDs are normal and quantify w/corresponding 15N or 13C heavy pair',
			'heavy' => '...assume IDs are 15N or 13C heavy and quantify w/corresponding 14N or 12C light pair' );
    my %meta_labels = ( 'N' => '14/15 N',
			'C' => '12/13 C' );
    my @res_list    = ( '--', @residues, 'n','c');


    print
	br,
	&printTitle(title  => 'XPRESS Options',
		    class  => 'formentryhead',
		    div_id => 'xpress'),
	"<div id=xpress class=formentry>",
	checkbox(-name=>'xp_run',
		 -value=>'true',
		 -label=>'RUN XPRESS'),
	br,
	"Mass tolerance: ",
	textfield(-name=>'xp_mass',
		  -value=>'0.5',
		  -size=>10,
		  -maxlength=>10),
	popup_menu(-name=>'xp_massunit',
		   -values => [qw'Daltons PPM']),
	br,
	radio_group(-name=>'xp_fix',
		    -values=>['light','heavy','-'],
		    -default=>'-',
		    -linebreak=>'true',
		    -labels=>\%labels),
	br,
	checkbox(-name=>'xp_heavy',
		 -value=>'true',
		 -label=>'Heavy labeled peptide elutes before light labeled partner'),
	br,
	"Residue 1 and mass difference: ",
	popup_menu(-name=>'xp_res1',
		   -values => \@res_list),
	textfield(-name=>'xp_res1md',
		  -value=>'9.0',
		  -size=>10,
		  -maxlength=>10),
	br,
	"Residue 2 and mass difference: ",
	popup_menu(-name=>'xp_res2',
		   -values => \@res_list),
	textfield(-name=>'xp_res2md',
		  -value=>'9.0',
		  -size=>10,
		  -maxlength=>10),
	br,
	"Residue 3 and mass difference: ",
	popup_menu(-name=>'xp_res3',
		   -values => \@res_list),
	textfield(-name=>'xp_res3md',
		  -value=>'9.0',
		  -size=>10,
		  -maxlength=>10),
	br,br,

	checkbox(-name=>'xp_fixscan',
		 -value=>'true',
		 -label=>'Fix elution peak area as +/- '),
	textfield(-name=>'xp_fixscanrange',
		  -value=>'5',
		  -size=>3,
		  -maxlength=>3),
	" scans from peak apex",
	br,
	'Minimum number of chromatogram points needed for quantitation: ',
	textfield(-name=>'xp_minquantscans',
		  -value=>'5',
		  -size=>3,
		  -maxlength=>3),
	br,
	'Number of isotopic peaks to sum, use narrow tolerance: ',
	textfield(-name=>'xp_numisopeaks',
		  -value=>'1',
		  -size=>3,
		  -maxlength=>3),
	br,br,
	"<hr noshade size='1' />\n",
	checkbox(-name=>'xp_metalabel',
		 -value=>'true',
		 -label=>'Metabolic Labeling '),
	popup_menu(-name=>'xp_metatype',
		   -values => [qw'N C'],
		   -labels => \%meta_labels ),
	' -- ignore all other parameters, and:',
	br,
	radio_group(-name=>'xp_metaquant',
		    -values=>['light','heavy'],
		    -default=>'-',
		    -linebreak=>'true',
		    -labels=>\%m_labels),
	"</div>\n";


    # asap options
    print
	br,
	&printTitle(title  => 'ASAPRatio Options',
		    class  => 'formentryhead',
		    div_id => 'asap'),
	"<div id=asap class=formentry>",
	checkbox(-name=>'as_run',
		 -value=>'true',
		 -label=>'RUN ASAPRatio'),
	br,
	checkbox(-name=>'as_static',
		 -value=>'true',
		 -label=>'Static modification quantification (i.e. each run is either all light or all heavy)'),
	br,br,
	"Labeled residues: ",
	popup_menu(-name=>'as_labres1',
		   -values => \@res_list,
		   -default => 'C'),
	popup_menu(-name=>'as_labres2',
		   -values => \@res_list,
		   -default => '--'),
	popup_menu(-name=>'as_labres3',
		   -values => \@res_list,
		   -default => '--'),
	popup_menu(-name=>'as_labres4',
		   -values => \@res_list,
		   -default => '--'),
	popup_menu(-name=>'as_labres5',
		   -values => \@res_list,
		   -default => '--'),
	br,
	checkbox(-name=>'as_heavy',
		 -value=>'true',
		 -label=>'Heavy labeled peptide elutes before light labeled partner'),
	br,
	checkbox(-name=>'as_fixedscan',
		 -value=>'true',
		 -label=>'Use fixed scan range for Light and Heavy'),
	br,
	checkbox(-name=>'as_cidonly',
		 -value=>'true',
		 -label=>'Quantitate only the charge state where the CID was made'),
	br,
	"Set areaFlag to ",
	textfield(-name=>'as_area',
		  -size=>10,
		  -maxlength=>10),
	" (ratio display option) ",
	br,br,
	checkbox(-name=>'as_zerobg',
		 -value=>'true',
		 -label=>'Zero out all background'),
	br,
	checkbox(-name=>'as_highbgok',
		 -value=>'true',
		 -label=>'Quantitate despite high background'),
	br,
	"m/z range to include in summation of peak: ",
	textfield(-name=>'as_mzpeak',
		  -value=>'0.5',
		  -size=>10,
		  -maxlength=>10),
	br,br,
	# now 5 residue mass modifications allowed. Need more?
	"Residue 1 mass: ",
	popup_menu(-name=>'as_res1',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res1mass',
		  -size=>10,
		  -maxlength=>10),
	" *",
	br,
	"Residue 2 mass: ",
	popup_menu(-name=>'as_res2',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res2mass',
		  -size=>10,
		  -maxlength=>10),
	" *",
	br,
	"Residue 3 mass: ",
	popup_menu(-name=>'as_res3',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res3mass',
		  -size=>10,
		  -maxlength=>10),
	" *  specify monoisotopic masses",
	br,
	"Residue 4 mass: ",
	popup_menu(-name=>'as_res4',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res4mass',
		  -size=>10,
		  -maxlength=>10),
	" *",
	br,
	"Residue 5 mass: ",
	popup_menu(-name=>'as_res5',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res5mass',
		  -size=>10,
		  -maxlength=>10),
	" *",
	"</div>\n";


    # libra options
    print
	br,
	&printTitle(title  => 'Libra Quantification Options',
		    class  => 'formentryhead',
		    div_id => 'libra'),
	"<div id=libra class=formentry>",
	checkbox(-name=>'lb_run',
		 -value=>'true',
		 -label=>'RUN Libra'),
	br,
	"Condition File: ",
	textfield(-name=>'lb_condition',
		  -value=>'condition.xml',
		  -size=>40,
		  -maxlength=>100),
	"&nbsp;"x15,
	a({href=>"$tpp_url?Action=$web_actions{'showpage'}&page=conditionxml"},'Click here'),
	' to generate a condition file',
	"</div>\n";


    # submit button
    print
	"<br>\n",
	&printTitle(title=>'Run Analysis!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'xinteract'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	br,
	"</div>\n",
	end_form;

}


########################################################################
# pageInterProphet
#
# The interprophet parameters page
#
########################################################################
sub pageInterProphet {
    # files to process (from session)
    my $pepxml_ext = `$command{tpp_hostname} GET_PEPXML_EXT!`; # get TPP's idea of canonical pepxml file ext
    my $protxml_ext = `$command{tpp_hostname} GET_PROTXML_EXT!`;
    my $any_files_there = &showFiles('runprophet','iprophet','xml','Select File(s) to Analyze');


    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => 'Output File and Location',
		    class  => 'formentryhead'),
	"<div class=formentry>";

    # output file
    my $fdir = '';
	if ($any_files_there) {
	$fdir = '';


	# get path from first xml file in list
	for (@session_data) {
	    chomp;
	    if (/$proc_types{'runprophet'}:/) {
		$fdir  = dirname($');    #'}
		last;
	    }
	}

	print
	    "File path (folder): ",
	    textfield(-name=>'iproph_outdir',
		      -value=>"$fdir",
		      -size=>70,
		      -maxlength=>200),
	    br,
	    "Write output to file: ",
	    textfield(-name=>'iproph_outfile',
		      -value=>"interact.ipro$pepxml_ext", # default was .xml before Jan 2008
		      -size=>70,
		      -maxlength=>100);
    } else {
	print
	    h2('Please select input files first.');
    }

    print "</div>\n";

    # interprophet options
    print
	br,
	&printTitle(title  => 'InterProphet Parameters',
		    class  => 'formentryhead',
		    div_id => 'ipro'),
	"<div id=ipro class=formentry>",
	checkbox(-name=>'iproph_nonrs',
		 -value=>'true',
		 -label=>' do NOT use number of replicate spectra (NRS) model'),
	br,
	checkbox(-name=>'iproph_nonsi',
		 -value=>'true',
		 -label=>' do NOT use number of sibling ions (NSI) model'),
	br,
	checkbox(-name=>'iproph_nonsm',
		 -value=>'true',
		 -label=>' do NOT use number of sibling modifications (NSM) model'),
	br,
	checkbox(-name=>'iproph_nonss',
		 -value=>'true',
		 -label=>' do NOT use number of sibling searches (NSS) model'),
	br,
	checkbox(-name=>'iproph_nonse',
		 -value=>'true',
		 -label=>' do NOT use number of sibling experiments (NSE) model'),
	br,
	checkbox(-name=>'iproph_nonsp',
		 -value=>'true',
		 -label=>' do NOT use number of sibling peptides (NSP) model'),
	br,
	checkbox(-name=>'iproph_nofpkm',
		 -value=>'true',
		 -label=>' do NOT use FPKM model'),
	br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'iproph_extraopts',
		  -value=>'',
		  -size=>35,
		  -maxlength=>100),
	br,br,
	checkbox(-name=>'pr_run',
		 -value=>'true',
		 -label=>' Run ProteinProphet on these results'),
	"</div>\n";

    # xpress options
    my %labels      = ( 'light' => 'For ratio, set/fix light to 1, vary heavy',
			'heavy' => 'For ratio, set/fix heavy to 1, vary light',
			'-'     => 'Default (set less abundant in ratio to 1)');
    my %m_labels    = ( 'light' => '...assume IDs are normal and quantify w/corresponding 15N or 13C heavy pair',
			'heavy' => '...assume IDs are 15N or 13C heavy and quantify w/corresponding 14N or 12C light pair' );
    my %meta_labels = ( 'N' => '14/15 N',
			'C' => '12/13 C' );
    my @res_list    = ( '--', @residues, 'n','c');


    print
	br,
	&printTitle(title  => 'XPRESS Options',
		    class  => 'formentryhead',
		    div_id => 'xpress'),
	"<div id=xpress class=formentry>",
	checkbox(-name=>'xp_run',
		 -value=>'true',
		 -label=>'RUN XPRESS'),
	br,
	"Mass tolerance: ",
	textfield(-name=>'xp_mass',
		  -value=>'0.5',
		  -size=>10,
		  -maxlength=>10),
	popup_menu(-name=>'xp_massunit',
		   -values => [qw'Daltons PPM']),
	br,
	radio_group(-name=>'xp_fix',
		    -values=>['light','heavy','-'],
		    -default=>'-',
		    -linebreak=>'true',
		    -labels=>\%labels),
	br,
	checkbox(-name=>'xp_heavy',
		 -value=>'true',
		 -label=>'Heavy labeled peptide elutes before light labeled partner'),
	br,
	"Residue 1 and mass difference: ",
	popup_menu(-name=>'xp_res1',
		   -values => \@res_list),
	textfield(-name=>'xp_res1md',
		  -value=>'9.0',
		  -size=>10,
		  -maxlength=>10),
	br,
	"Residue 2 and mass difference: ",
	popup_menu(-name=>'xp_res2',
		   -values => \@res_list),
	textfield(-name=>'xp_res2md',
		  -value=>'9.0',
		  -size=>10,
		  -maxlength=>10),
	br,
	"Residue 3 and mass difference: ",
	popup_menu(-name=>'xp_res3',
		   -values => \@res_list),
	textfield(-name=>'xp_res3md',
		  -value=>'9.0',
		  -size=>10,
		  -maxlength=>10),
	br,br,

	checkbox(-name=>'xp_fixscan',
		 -value=>'true',
		 -label=>'Fix elution peak area as +/- '),
	textfield(-name=>'xp_fixscanrange',
		  -value=>'5',
		  -size=>3,
		  -maxlength=>3),
	" scans from peak apex",
	br,
	'Minimum number of chromatogram points needed for quantitation: ',
	textfield(-name=>'xp_minquantscans',
		  -value=>'5',
		  -size=>3,
		  -maxlength=>3),
	br,
	'Number of isotopic peaks to sum, use narrow tolerance: ',
	textfield(-name=>'xp_numisopeaks',
		  -value=>'1',
		  -size=>3,
		  -maxlength=>3),
	br,br,
	"<hr noshade size='1' />\n",
	checkbox(-name=>'xp_metalabel',
		 -value=>'true',
		 -label=>'Metabolic Labeling '),
	popup_menu(-name=>'xp_metatype',
		   -values => [qw'N C'],
		   -labels => \%meta_labels ),
	' -- ignore all other parameters, and:',
	br,
	radio_group(-name=>'xp_metaquant',
		    -values=>['light','heavy'],
		    -default=>'-',
		    -linebreak=>'true',
		    -labels=>\%m_labels),
	"</div>\n";


    # asap options
    print
	br,
	&printTitle(title  => 'ASAPRatio Options',
		    class  => 'formentryhead',
		    div_id => 'asap'),
	"<div id=asap class=formentry>",
	checkbox(-name=>'as_run',
		 -value=>'true',
		 -label=>'RUN ASAPRatio'),
	br,
	checkbox(-name=>'as_static',
		 -value=>'true',
		 -label=>'Static modification quantification (i.e. each run is either all light or all heavy)'),
	br,br,
	"Labeled residues: ",
	popup_menu(-name=>'as_labres1',
		   -values => \@res_list,
		   -default => 'C'),
	popup_menu(-name=>'as_labres2',
		   -values => \@res_list,
		   -default => '--'),
	popup_menu(-name=>'as_labres3',
		   -values => \@res_list,
		   -default => '--'),
	popup_menu(-name=>'as_labres4',
		   -values => \@res_list,
		   -default => '--'),
	popup_menu(-name=>'as_labres5',
		   -values => \@res_list,
		   -default => '--'),
	br,
	checkbox(-name=>'as_heavy',
		 -value=>'true',
		 -label=>'Heavy labeled peptide elutes before light labeled partner'),
	br,
	checkbox(-name=>'as_fixedscan',
		 -value=>'true',
		 -label=>'Use fixed scan range for Light and Heavy'),
	br,
	checkbox(-name=>'as_cidonly',
		 -value=>'true',
		 -label=>'Quantitate only the charge state where the CID was made'),
	br,
	"Set areaFlag to ",
	textfield(-name=>'as_area',
		  -size=>10,
		  -maxlength=>10),
	" (ratio display option) ",
	br,br,
	checkbox(-name=>'as_zerobg',
		 -value=>'true',
		 -label=>'Zero out all background'),
	br,
	checkbox(-name=>'as_highbgok',
		 -value=>'true',
		 -label=>'Quantitate despite high background'),
	br,
	"m/z range to include in summation of peak: ",
	textfield(-name=>'as_mzpeak',
		  -value=>'0.5',
		  -size=>10,
		  -maxlength=>10),
	br,
	"Min. iProphet probability to attempt quantitation: ",
	textfield(-name=>'as_miniprob',
		  -value=>'0.5',
		  -size=>10,
		  -maxlength=>10),
	br,br,
	# now 5 residue mass modifications allowed. Need more?
	"Residue 1 mass: ",
	popup_menu(-name=>'as_res1',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res1mass',
		  -size=>10,
		  -maxlength=>10),
	" *",
	br,
	"Residue 2 mass: ",
	popup_menu(-name=>'as_res2',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res2mass',
		  -size=>10,
		  -maxlength=>10),
	" *",
	br,
	"Residue 3 mass: ",
	popup_menu(-name=>'as_res3',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res3mass',
		  -size=>10,
		  -maxlength=>10),
	" *  specify monoisotopic masses",
	br,
	"Residue 4 mass: ",
	popup_menu(-name=>'as_res4',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res4mass',
		  -size=>10,
		  -maxlength=>10),
	" *",
	br,
	"Residue 5 mass: ",
	popup_menu(-name=>'as_res5',
		   -values => \@res_list,
		   -default => '--'),
	textfield(-name=>'as_res5mass',
		  -size=>10,
		  -maxlength=>10),
	" *",
	"</div>\n";


    # libra options
    print
	br,
	&printTitle(title  => 'Libra Quantification Options',
		    class  => 'formentryhead',
		    div_id => 'libra'),
	"<div id=libra class=formentry>",
	checkbox(-name=>'lb_run',
		 -value=>'true',
		 -label=>'RUN Libra'),
	br,
	"Condition File: ",
	textfield(-name=>'lb_condition',
		  -value=>'condition.xml',
		  -size=>40,
		  -maxlength=>100),
	"&nbsp;"x15,
	a({href=>"$tpp_url?Action=$web_actions{'showpage'}&page=conditionxml"},'Click here'),
	' to generate a condition file',
	"</div>\n";


    # submit button
    print
	"<br>\n",
	&printTitle(title=>'Run iProphet Analysis!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runinterprophet'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	br,br,
	"</div>\n",
	end_form;

}



########################################################################
# pagePTMProphet
#
# The PTMProphet parameters page
#
########################################################################
sub pagePTMProphet {
    # files to process (from session)
    my $pepxml_ext = `$command{tpp_hostname} GET_PEPXML_EXT!`; # get TPP's idea of canonical pepxml file ext

    my $any_files_there = &showFiles('runprophet','ptmprophet','xml','Select File(s) to Analyze');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => 'Output File and Location',
		    class  => 'formentryhead'),
	"<div class=formentry>";

    # output file
    if ($any_files_there) {
	my $fdir = '';

	# get path from first xml file in list
	for (@session_data) {
	    chomp;
	    if (/$proc_types{'runprophet'}:/) {
		$fdir  = dirname($');    #'}
		last;
	    }
	}

	print
	    "File path (folder): ",
	    textfield(-name=>'ptmproph_outdir',
		      -value=>"$fdir",
		      -size=>70,
		      -maxlength=>200),
	    br,
	    "Write output to file: ",
	    textfield(-name=>'ptmproph_outfile',
		      -value=>"interact.ptm$pepxml_ext", # default was .xml before Jan 2008
		      -size=>70,
		      -maxlength=>100);
    } else {
	print
	    h2('Please select input files first.');
    }

    print "</div>\n";

    # ptmprophet options
    print
	br,
	&printTitle(title  => 'PTMProphet Parameters',
		    class  => 'formentryhead',
		    div_id => 'ptmpro'),
	"<div id=ptmpro class=formentry>",
	"Specify modifications:",
	br,
	"Residue(s): ",
	textfield(-name=>'ptmproph_res1',
		  -value=>'STY',
		  -size=>5,
		  -maxlength=>10),
	" Mass shift: ",
	textfield(-name=>'ptmproph_res1md',
		  -value=>'79.966',
		  -size=>5,
		  -maxlength=>10),
	br;

    for (2..5) {
	print
	    "Residue(s): ",
	    textfield(-name=>"ptmproph_res$_",
		      -value=>'',
		      -size=>5,
		      -maxlength=>10),
	    " Mass shift: ",
	    textfield(-name=>"ptmproph_res${_}md",
		      -value=>'',
		      -size=>5,
		      -maxlength=>10),
	    br;
    }

    print
	"m/z tolerance: ",
	textfield(-name=>'ptmproph_mztol',
		  -value=>'0.1',
		  -size=>5,
		  -maxlength=>5),
	" (daltons)",
	br,
	checkbox(-name=>'ptmproph_noupdate',
		 -value=>'true',
		 -label=>'Do not update modification_info tags in pepXML'),
	br,br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'ptmproph_extraopts',
		  -value=>'',
		  -size=>35,
		  -maxlength=>100),
	"</div>\n";

    # submit button
    print
	"<br>\n",
	&printTitle(title=>'Run PTMProphet Analysis!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runptmprophet'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	br,br,
	"</div>\n",
	end_form;

}


########################################################################
# pageRunProphet
#
# The runprophet parameters page
#
########################################################################
sub pageRunProphet {

    # files to process (from session)
    my $protxml_ext = `$command{tpp_hostname} GET_PROTXML_EXT!`; # get TPP's idea of canonical protxml file ext

    my $any_files_there = &showFiles('runprophet','runprophet','xml','Select File(s) to Analyze');

    print
	start_form('POST',$tpp_url),
	br,
	&printTitle(title  => 'Choose Output File Name and Location',
		    class  => 'formentryhead'),
	"<div class=formentry>";

    # output file
    if ($any_files_there) {
	my $fdir;

	# get path from first xml file in list
	for (@session_data) {
	    chomp;
	    if (/$proc_types{'runprophet'}:/) {
		$fdir  = dirname($');    #'}
		last;
	    }
	}

	print
	    "Output Directory: ",
	    textfield(-name=>'prt_outdir',
		      -value=>"$fdir",
		      -size=>50,
		      -maxlength=>200),
	    br,
	    "Output Filename: ",
	    textfield(-name=>'prt_outfile',
		      -value=>'interact'.$protxml_ext,  # default was interact-prot.xml before Jan 2008  
		      -size=>50,
		      -maxlength=>100),
	    br,
	    checkbox(-name=>'prt_dohtml',
		     -value=>'true',
		     -label=>'Pre-generate static html file for loading to browser (* warning: may produce a very large file)');

    } else {
	print
	    h2('Please select input files first.');
    }

    print "</div>\n";

    # proteinprophet options
    print
	br,
	&printTitle(title  => 'ProteinProphet Parameters',
		    class  => 'formentryhead',
		    div_id => 'prt'),
	"<div id=prt class=formentry>",
	checkbox(-name=>'prt_ipro',
		 -value=>'true',
		 -label=>'Input is from iProphet'),
	br,
	checkbox(-name=>'prt_icat',
		 -value=>'true',
		 -label=>'icat data (color Cysteines)'),
	br,
	checkbox(-name=>'prt_nglyc',
		 -value=>'true',
		 -label=>'N-glycosylation data (color NXS/T)'),
	br,
	checkbox(-name=>'prt_xpress',
		 -value=>'true',
		 -label=>'Import XPRESS protein ratios'),
	br,
	checkbox(-name=>'prt_asapget',
		 -value=>'true',
		 -label=>'Import ASAPRatio protein ratios and pvalues'),
	br,
	checkbox(-name=>'prt_libra',
		 -value=>'true',
		 -label=>'Import Libra protein ratios'),
	br,
	checkbox(-name=>'prt_nozero',
		 -value=>'true',
		 -label=>'Do not include zero probability protein entries in output'),
	br,
	checkbox(-name=>'prt_protlen',
		 -value=>'true',
		 -label=>'Report protein length'),
	br,
	checkbox(-name=>'prt_molwt',
		 -value=>'true',
		 -label=>'Report (calculated) protein molecular weight'),
	br,
	"</div>\n",
	br,
	&printTitle(title  => 'Advanced ProteinProphet Options',
		    class  => 'formentryhead',
		    div_id => 'adv'),
	"<div id=adv class=formentry>",
	"<b>These are advanced ProteinProphet options. You might want to leave these unchanged unless you are an expert user.</b>",
	br,br,
	checkbox(-name=>'prt_delude',
		 -value=>'true',
		 -label=>'Delude (do not look up ALL proteins corresponding to shared peps)'),
	br,
	checkbox(-name=>'prt_noccam',
		 -value=>'true',
		 -label=>'Do not use Occam\'s razor for shared peps (get max. protein list, including many false positives)'),
	br,
	checkbox(-name=>'prt_nogroups',
		 -value=>'true',
		 -label=>'Do not assemble protein groups'),
	br,
	checkbox(-name=>'prt_nspnorm',
		 -value=>'true',
		 -label=>'Normalize NSP using protein length'),
	br,
	checkbox(-name=>'prt_instances',
		 -value=>'true',
		 -label=>'Use expected number of ion instances to adjust the peptide probabilities prior to NSP adjustment'),
	br,
	checkbox(-name=>'prt_grpwts',
		 -value=>'true',
		 -label=>"Check peptide's total weight in the Protein Group against the threshold (default: check peptide's actual weight against threshold)"),
	br,br,
	"Enter additional options to pass directly to the command-line (expert use only!) ",
	textfield(-name=>'prt_extraopts',
		  -value=>'',
		  -size=>35,
		  -maxlength=>100),
	"</div>\n";


    # submit button
    print
	"<br>\n",
	&printTitle(title=>'Run Protein Analysis!'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'runprophet'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	br,br,
	"</div>\n",
	end_form;

}


########################################################################
# pageUpdatePaths
#
# Specify files that need to have internal paths updated (pepXML, protXML, etc...)
#
########################################################################
sub pageUpdatePaths {

    # files to process (from session)
    my $any_files_there = &showFiles('updatefiles','updatepaths','xml','1. Select file(s) with paths to be updated to current location');

    # additional paths
    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for database file select
    my $any_paths_there = &showFiles('extrapaths','updatepaths','DIRS','2. Specify any additional directories to search for file(s) [ optional ]');

    # search database file (from session)
    Delete('file_ext');
    Delete('proc_type');   # otherwise it gets set wrong for database file select
    my $db_file_there = &showFiles('searchdb','updatepaths','*','3. Specify new location of search database [ optional ]','dbase/');

    # options
    print
	br,
	start_form('POST',$tpp_url),
	&printTitle(title  => '4. Additional options',
		    class  => 'formentryhead',
		    div_id => 'main'),
	'<div id=main class=formentry>',
	"Backup Original Files with extension: ",
	textfield(-name=>'backup',
		  -value=>'',
		  -size=>10,
		  -maxlength=>10),
	br,
	checkbox(-name=>'dry_run',
		 -value=>'true',
		 -checked => 0,
		 -label=>" Perform a trial run that doesn't make any changes"),
	br,
	checkbox(-name=>'verbose',
		 -value=>'true',
		 -checked => 0,
		 -label=>" Be extra verbose about what is being done"),
	"</div>\n";

    print
	br,
	&printTitle(title=>'5. Update files'),
	"<div class=formentry>";

    if ($any_files_there) {
	print
	    submit(-name=>'Action',
		   -value=>$web_actions{'updatePaths'});
    } else {
	print
	    h2('No files selected yet.');
    }

    print
	"</div>\n",
	end_form;

}

########################################################################
# pageS3cmd
#
# Form for running s3cmd 
#
########################################################################
sub pageS3cmd {

    my $d_src = param('workdir');
    my @listoffiles = param('src_files');
    my $src = "";

    print
	start_form('POST',$tpp_url),
	br;

    print
	&printTitle(title => '1. Files/Folders to Sync' ),
	"<div class = formentry>";

    my @locPaths = ();
    my @remPaths = ();
    if ( !@listoffiles ) {
        push @locPaths, $d_src;
        $_ = $d_src;
        s/^$data_dir//; s/\/$//;
	$_ = "/$_" if $_;
        push @remPaths, "$amz_s3url$_/";
        print "$locPaths[-1] <=> $remPaths[-1]<br>\n";
    } else {
        foreach ( @listoffiles ) {
            my $d ="$d_src$_";
            $d =~ s/$data_dir//; 
            push @remPaths, "$amz_s3url/$d";
            push @locPaths, $_;
            print escapeHTML("$d_src$locPaths[-1] <=> $remPaths[-1]"), "<br>\n";
        }
    }
    print
	hidden(-name=>'workdir',
	       -value=> "$d_src" ),
	hidden(-name=>'local_paths',
	       -value=> "@locPaths" ),
	hidden(-name=>'remote_paths',
	       -value=> "@remPaths" ),
	"</div>\n";

    print
	br,
	&printTitle(title => '2. Choose Options'),
	"<div class = formentry>",
	checkbox(-name=>'dry_run',
		 -value=>'true',
		 -checked => 0,
		 -label=>' Only show what should be uploaded or downloaded'
                         . " but don't actually do it (--dry-run)"),
	br, br,
	checkbox(-name=>'delete_removed',
		 -checked => 0,
		 -value=>'true',
		 -label=>' Delete remote objects with no corresponding '
                         . ' local file (--delete-removed)'),
	"</div>\n";
    
    print
	br,
	&printTitle(title=>'3. Sync files/folders with S3'),
	"<div class=formentry>",

        "<table valign='bottom'>\n",
        "<tr><td>\n",
        submit(-name=>'Action', -value=>$web_actions{'s3SyncUp'}),
        "</td><td style='vertical-align: middle; padding-left: 8px'>",
        "Upload only the new or changed local files/folders to S3",
        "</td></tr>\n",
        "<tr><td align='center'>-- or --<td></tr>", 
        "<tr><td>\n",
        submit(-name=>'Action', -value=>$web_actions{'s3SyncDown'}),
        "</td><td style='vertical-align: middle; padding-left: 8px'>",
        "Download only new or modified files/folders from S3 to the local filesystem",
        "</td></tr>\n",
        "</table>\n";

    print
	"</div>\n",
	end_form;
}


########################################################################
# printClusterOptions
#
# Options to run a given executable on a cluster, cloud, etc
#
########################################################################
sub printClusterOptions {
    my $aws_file = "${users_dir}$auth_user/.awssecret";

    if (-e $aws_file) {
	print
	    popup_menu(-name=>'runon_cluster',
		       -values => [qw'on_local_machine on_Amazon_cloud']
		       );
    }

}

########################################################################
# checkAmzFiles
#
# Determine if Amazon job bumdle has finished by checking existenece of amz log files
#
########################################################################
sub checkAmzFiles {
    my $cmd_file = shift || "${tmp_dir}cmd_$user_session";
    my $allfiles = 1;

    if (! -f $cmd_file) {
	print
	    &printTitle(title  => ' Cannot Open File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>File not found: $cmd_file</li>\n",
	    "</ul></div><br>\n\n";
	return 0;
    }

    open(FILE, "$cmd_file") || &fatalError("CHECKAMZFILES_CANNOT_OPEN_COMMAND_OUTPUT_FILE:$cmd_file:$!");
    while (<FILE>) {
	last if /BEGIN COMMAND BLOCK/;
	chomp;
	if (/OUTFILE:/) {
	    my $outf = $'; #'
	    $allfiles *= (-e $outf) if ($outf =~ /.log$/);	    
	}
    }
    close (FILE);

    chomp (my $status = `$command{echo} # END COMMAND BLOCK >> $cmd_file`) if $allfiles;

    return 1;
}

########################################################################
# getOutputFiles
#
# Display links to output files
#
########################################################################
sub getOutputFiles {
    my $cmd_file = shift || "${tmp_dir}cmd_$user_session";
    my @outfiles;

    if (! -f $cmd_file) {
	print
	    &printTitle(title  => ' Cannot Open File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>File not found: $cmd_file</li>\n",
	    "</ul></div><br>\n\n";
	return 0;
    }

    open(FILE, "$cmd_file") || &fatalError("SHOWFILE_CANNOT_OPEN_COMMAND_OUTPUT_FILE:$cmd_file:$!");
    while (<FILE>) {
	last if /BEGIN COMMAND BLOCK/;
	chomp;
	if (/OUTFILE:/) {
	    push @outfiles, $'; #'
	}
    }
    close (FILE);

    print
	br,
	&printTitle(title  => "Output Files",
		    class  => 'formentryhead'),
	"<div class=formentry>\n",
	"<ul>\n";
    for my $file (@outfiles) {
	my $file_url = $file;
	my $file_disp = $file;

	next if ($file eq 'NONE');

	if ($file_url =~ s/$www_root/\//) {
	    print &makeFileLink(file_loc => $file,
				file_url => $file_url,
				file_txt => $file_disp,
				extended => 1);

	} else {
	    &readAllowedDirs($auth_user);
	    my $dir_found = 0;
	    for (my $idx = 0; $idx <= $#directories; $idx++) {
		if ($file_url =~ /$directories[$idx]/) {
		    print &makeFileLink(file_loc => $file,
					file_url => $file_url,
					file_txt => $file_disp,
					extended => 1);
		    $dir_found = 1;
		    last;
		}
	    }	
	    unless ($dir_found) {
		print "<li>$file_disp  (file not under web server root; cannot access)</li>\n";
	    }
	}

    }
    print
	"</ul>\n",
	"</div>\n";

    return 1;
}


########################################################################
# pageBrowseFiles
#
# Browse files: old results, databases, etc
#
########################################################################
sub pageBrowseFiles {
    # add a filter box for this parameter?
    my $file_ext = param('file_ext') || '*';

    #display contents of clipboard
    my $clipboard = &showFiles('copyfile','filebrowser','*','Clipboard Contents');

    print br,br if $clipboard;

    &fileChooser('File Browser','browse',$clipboard);

}


########################################################################
# editParams
#
# Edit search engine parameters file (Tandem, for starters...)
#
########################################################################
sub editParams {
    return if ($errors);

    my $filepath = param('workdir').param('file');
    my $filepath_disp = $filepath;

    if (! -f $filepath) {
	print
	    &printTitle(title  => ' Cannot Open File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>File not found: $filepath</li>\n",
	    "</ul></div><br>\n\n";
	return;
    } elsif (! -T $filepath) {
	print
	    &printTitle(title  => ' Cannot Open File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>Cannot open binary file: $filepath   Sorry!</li>\n",
	    "</ul></div><br>\n\n";
	return;

    } elsif (! -w $filepath) {
	print
	    &printTitle(title  => ' Cannot Edit Parameters File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>Cannot edit file: $filepath</li>",
	    "<li>File is write-protected; please check permissions!</li>\n",
	    "</ul></div><br>\n\n";
	return;
    }

    open(FILE, "$filepath") || &fatalError("EDITPARAMS_CANNOT_OPEN_FILE:$filepath:$!");

    print
	start_form('POST',$tpp_url),
	&printTitle(title=>"Editing parameters file: <h3 style='display:inline'>$filepath_disp</h3>"),
	"<div class=formentry>",
	"<table>\n";

    my $search_link = 'http://www.google.com/search?ie=UTF-8&oe=UTF-8&sourceid=navclient&btnI=1&q=gpm';

    my %seen;
    my $order = 100; # this guarantees unique prefixes for 900 parameters...
    while (<FILE>){
	if (/\<note type=\"input\" label=\"(.*)\"\>(.*)\<\/note\>/) {
	    my $param = $1;
	    my $value = $2;
	    my $parid = "SPARAM_" . ++$order;

	    my $parnosp = $param;
	    $parnosp =~ s/\s//g;
	    my $font = $seen{$parnosp} ? 'style="color:red"' : '';

	    print
		"<tr id='$parid'><td $font>$param<a class='text_link' title='Google me!' target='tparams' href='$search_link+$param'> [ ? ]</a></td>",
		"<td><input type=\"text\" name=\"${parid}_$param\" value=\"$value\" size=\"70\" maxlength=\"200\"></td><td>",
		checkbox(-name=>"remparam",
			 -id=>"${parid}_cb",
			 -label=>'',
			 -title=>'remove?',
			 -value=>$parid,
			 -onClick=>"hilight('$parid')"),
		"</td>";

	    if (
		$param eq 'list path, taxonomy information' ||
		$param eq 'protein, taxon' ||
		$param eq 'spectrum, path' ||
		$param eq 'output, path'
		) {
		print "<td class='gray'> * ignored if you are using Petunia to launch X!Tandem</td>";
	    }
	    elsif ($param eq 'list path, default parameters') {
		my $fname = basename($value);
		my $fdir  = dirname($value);

		if (!-f $value) {
		    print "<td style='color:red'> * File does not exist!</td>\n";
		}
		else {
		    print "<td class='gray'> [ <a href=\"$tpp_url?Action=$web_actions{'showpage'}&page=editparams&workdir=$fdir&file=$fname\">Params</a> | <a href=\"$tpp_url?Action=$web_actions{'showpage'}&page=editfile&workdir=$fdir&file=$fname\">Edit</a> ]</td>";
		}
	    }
	    elsif ($seen{$parnosp}) {
		print "<td style='color:red'> * Duplicated parameter!</td>\n";
	    }

	    $seen{$parnosp}++;
	    print "</tr>\n";
	}
#	print escapeHTML($_);
    }

    print "<tr><td class='black'>Add more?&nbsp;&nbsp;&nbsp;&nbsp;<a class='text_link' target='tparams' href='http://thegpm.org/tandem/api/'>[ X!Tandem API index ]</a></td></tr>\n";

    for my $i (10..20) {
	print
	    "<tr><td><input type=\"text\" name=\"XTRA_SPARAM_NAME_$i\" size=\"40\" maxlength=\"200\"></td>",
	    "<td><input type=\"text\" name=\"XTRA_SPARAM_VALUE_$i\" size=\"70\" maxlength=\"200\"></td></tr>\n";
    }

    print
	"</table>\n",
	"</div>\n",
	submit(-name=>'Action',
	       -value=>$web_actions{'saveParams'}),
	textfield(-name=>'file_path',
		  -value=>$filepath,
		  -size => 100,
		  -maxlength => 250),
	hidden(-name=>'workdir'),
	hidden(-name=>'file'),
	end_form;

    print
	br,
	'<form><input type="button" value="Cancel (Go Back)" onclick="history.back()"></form>';

    return;
}


########################################################################
# editFile
#
# Edit file contents
#
########################################################################
sub editFile {
    return if ($errors);

    my $filepath = param('workdir').param('file');
    my $filepath_disp = $filepath;

    if (! -f $filepath) {
	print
	    &printTitle(title  => ' Cannot Open File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>File not found: $filepath</li>\n",
	    "</ul></div><br>\n\n";
	return;
    } elsif (! -T $filepath) {
	print
	    &printTitle(title  => ' Cannot Open File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>Cannot open binary file: $filepath   Sorry!</li>\n",
	    "</ul></div><br>\n\n";
	return;

    } elsif (! -w $filepath) {
	print
	    &printTitle(title  => ' Cannot Edit File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>Cannot edit file: $filepath</li>",
	    "<li>File is write-protected; please check permissions!</li>\n",
	    "</ul></div><br>\n\n";
	return;
    }

    open(FILE, "$filepath") || &fatalError("EDITFILE_CANNOT_OPEN_FILE:$filepath:$!");

    print
	start_form('POST',$tpp_url),
	&printTitle(title=>"Editing file: <h3 style='display:inline'>$filepath_disp</h3>"),
	"<div class=formentry>",
	"<textarea name='file_contents' rows='25' cols='120'>\n";

    while (<FILE>){
	print escapeHTML($_);
    }

    print
	"</textarea>\n",
	"</div>\n",
	submit(-name=>'Action',
	       -value=>$web_actions{'saveFile'}),
	hidden(-name=>'file_path',
	       -value=>$filepath),
	hidden(-name=>'workdir'),
	end_form;

    print
	br,
	'<form><input type="button" value="Cancel (Go Back)" onclick="history.back()"></form>';

    return;
}


########################################################################
# showFile
#
# Display file contents
#
########################################################################
sub showFile {
    print
	'<form><input type="button" value="Go Back" onclick="history.back()"></form>';

    return if ($errors);

    my $filepath = param('workdir').param('file');
    my $filepath_disp = $filepath;

    if (! -f $filepath) {
	print
	    &printTitle(title  => ' Cannot Open File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>File not found: $filepath</li>\n",
	    "</ul></div><br>\n\n";
	return;
    } elsif (! -T $filepath) {
	print
	    &printTitle(title  => ' Cannot Open File ',
			class  => 'messageshead'),
	    "<div class=messages>\n<ul>",
	    "<li>Cannot open binary file: $filepath   Sorry!</li>\n",
	    "</ul></div><br>\n\n";
	return;
    }

    open(FILE, "$filepath") || &fatalError("SHOWFILE_CANNOT_OPEN_FILE:$filepath:$!");

    print
	br,
	&printTitle(title=>"Viewing file: $filepath_disp"),
	"<div class=formentry>",
	"<pre>\n";

    while (<FILE>){
	print escapeHTML($_);
    }

    print
	"</pre>\n",
	"</div>\n";

    print
	'<form><input type="button" value="Go Back" onclick="history.back()"></form>';

    return;
}


########################################################################
# showFiles
#
# Retrieve files available for input from session
# Returns number of files found
#
########################################################################
sub showFiles {
    my $proc_type = shift;
    my $ref_page = shift;
    my $file_ext = shift;
    my $sect_title = shift || 'Files to Process/Analyze';
    my $ref_dir = shift;

    $ref_dir = $ref_dir ? "$data_dir$ref_dir" : param('workdir');

    my $in_clipboard = ($sect_title =~ /^Clipboard/) ? 1 : 0;
    my $file_text = ($file_ext eq 'DIRS') ? 'directories' : 'files';

    $sfnum++;

    my @s_file_list;
    # Retrieve file list from session

    for (@session_data) {
	chomp;
	if (/^$proc_types{$proc_type}:/) {
	    push @s_file_list, $';  #'
	}
    }
    my $s_files_count = scalar(@s_file_list);

    if ($in_clipboard) {
	return 0 unless $s_files_count;  # don't even show the clipboard div
	$sect_title .= " ($s_files_count)";
    }

    print
	br,
	&printTitle(title  => $sect_title,
		    class  => 'formentryhead',
		    div_id => "showfiles$sfnum");

    if ($in_clipboard) {
	print
	    "<div id=showfiles$sfnum class=hideit>\n";
    } else {
	print
	    "<div id=showfiles$sfnum class=formentry>\n";
    }

    if (@s_file_list) {
	my $remfile_prefix = "filestoremove$sfnum";
	Delete('remfiles');

	print
	    start_form('POST',$tpp_url),
	    hidden(-name=>'ref_page',
		   -value=>$ref_page),
	    hidden(-name=>'remfiles',
		   -value=>$remfile_prefix),
	    "<table border=0 cellpadding=0 cellspacing=0 width=100%>\n";

	my $file_count = 0;
	for my $file_path (@s_file_list) {

	    my $rem_file = "$proc_types{$proc_type}:$file_path";
	    my $file_id = "${proc_type}_${sfnum}_".$file_count++;

	    print
		"<tr id=$file_id class=remfile><td align=left>$file_path</td>",
		"<td align=right>",
		checkbox(-name=>"$remfile_prefix",
			 -id=>"${file_id}_cb",
			 -label=>'',
			 -value=>$rem_file,
			 -onClick=>"hilight('$file_id')"),
		"</td></tr>\n";
	}


	if (scalar(@s_file_list) > 1) {
	    my $checkallname = "checkAll$sfnum";

	    Delete("checkAll$sfnum");  # we never want to pre-check this box
	    print
	    "<tr><td colspan=2 align=right>",
		'Select/Unselect All ',
		checkbox(-name=>"checkAll$sfnum",
			 -id=>$checkallname,
			 -value=>'true',
			 -onClick=>"toggleAll('$checkallname','$remfile_prefix','${proc_type}_${sfnum}_')",
			 -label=>''),
		"</td></tr>";
	}
	print
	    "<tr><td colspan=2 align=right>",
	    hidden(-name=>'file_ext',
		   -value=>$file_ext),
	    submit(-name=>'Action',
		   -value=>$web_actions{'removeFiles'}),
	    "</td></tr>",
	    "</table>\n",
	    end_form;


    } else {
	if ($in_clipboard) {
	    print
		h2('Clipboard is empty.');
	} else {
	    print
		h2("No $file_text selected yet.");
	}
    }

    unless ($in_clipboard) {
	print
	    "<hr noshade size='1' />\n",
	    start_form(-method=>'POST',
		       -action=>$tpp_url,
		       -style =>'display: inline;',
		       ),
	    hidden(-name=>'ref_page',
		   -value=>$ref_page);

	# only include proc_type if it does not match ref page
	if ($ref_page ne $proc_type) {
	    print
		hidden(-name=>'proc_type',
		       -value=>$proc_type);
	}

	print
	    hidden(-name=>'file_ext',
		   -value=>$file_ext),
	    submit(-name=>'Action',
		   -value=>$web_actions{'addFiles'}),
	    end_form;


	my @dafiles = ("$ref_dir [ $file_ext ]");   # limit to 100..??
	if ($ref_dir) {
	    if ($file_ext eq 'DIRS') {
		push @dafiles, &listDirs($ref_dir);
	    } elsif ($file_ext =~ /\*(.*)\*/)  {
		push @dafiles, &listFiles($ref_dir,$1,'false');
	    } else {
		push @dafiles, &listFiles($ref_dir,$file_ext);
	    }
	}

	my $form_name = "form$proc_type";

	Delete('Action');
	Delete('src_files');

	print
	    start_form(-method=>'POST',
		       -action=>$tpp_url,
		       -name => $form_name,
		       -style =>'display: inline;',
		       ),
	    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;",
	    "Or choose from list: ",
	    popup_menu(-name     =>'src_files',
		       -values   => \@dafiles,
		       -style    => 'background:#eeeeee;',
		       -onChange => "document.forms.${form_name}.submit();"),
	    hidden(-name=>'proc_type',
		   -value=>$proc_type),
	    hidden(-name=>'file_ext',
		   -value=>$file_ext),
	    hidden(-name=>'ref_page',
		   -value=>$ref_page),
	    "<input type='hidden' name='quickdir' value='$ref_dir' />",
	    hidden(-name=>'Action',
		   -value=>$web_actions{'selectFiles'}),
	    end_form
	    if (scalar @dafiles > 1);

    }
    print
	"</div>\n";

    return $s_files_count;
}


########################################################################
# fileChooser
#
# Generic module for choosing files / directories
#
########################################################################
sub fileChooser {
    my $heading = shift || "Please choose file(s)";
    my $szMode = shift || 'select';   # select or browse
    my $clipboard = shift || 0;

    my $file_ext = param('file_ext') || '*';
    my $ref_page = param('ref_page') || 'filebrowser';   # hmmm...
    my $proc_type= param('proc_type') || '';
    my $d_src = param('workdir') || $data_dir;

    my @dadirs  = &listDirs($d_src);
    my @dafiles;

    my $file_text = 'files';

    if ($file_ext eq 'DIRS') {
	$file_text = 'directories';
	$heading = "Please choose directory(s)";
	@dafiles = @dadirs;
    } elsif ($file_ext =~ /\*(.*)\*/)  {
	@dafiles = &listFiles($d_src,$1,'false');
    } else {
	@dafiles = &listFiles($d_src,$file_ext);
    }

    if ($szMode eq 'select') {
	print
	    &printTitle(title=>$heading, class=>'cmdreadyhead'),
	    "<div class=formentry_n>";
    } else {
	print
	    &printTitle(title=>$heading, class=>'formentryhead'),
	    "<div class=formentry>";
    }

    print
	"<table cellspacing=5 width=100%>\n",
	"<tr>\n";

    # paste box for quick directory cd
    print
	"<td colspan=2 class=file>",
	start_form(-method=>'POST',
		   -action=>$tpp_url,
		   -style =>'display: inline;',
		   ),
	"Current Directory: ",
	textfield(-name=>'workdir',
		  -value=>$d_src,
		  -size=>110,
		  -maxlength=>300),
	submit(-name=>'DispAction',
	       -value=>'Go!')."\n",
	hidden(-name=>'ref_page',
	       -value=>$ref_page)."\n",
	hidden(-name=>'proc_type',
	       -value=>$proc_type)."\n",
	hidden(-name=>'file_ext',
	       -value=>$file_ext)."\n",
	hidden(-name=>'page',
	       -value=>'filechooser')."\n",
	hidden(-name=>'Action',
	       -value=>$web_actions{'showpage'})."\n",
	end_form;

    if ($szMode eq 'browse') {
	print
	    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;",
	    start_form(-method=>'POST',
		       -action=>$tpp_url,
		       -style =>'display: inline;',
		       ),
	    hidden(-name=>'workdir',
		   -default=>$d_src),
	    submit(-name=>'Action',
		   -value=>$web_actions{'setWorkDir'}),
	    end_form;
    }

    print
	"</td>\n</tr>\n",
	"<tr>\n<td style='vertical-align:top' class=file>",
	"<h4 id='filter_here'>",
	uc("$file_text:"),
	" (*.$file_ext)</h4>";

    if (@dafiles) {

	my @fstat;
	my ($fileSize, $fileAge);

	Delete('src_files');  # do not precheck boxes

	print
	    start_form('POST',$tpp_url,'multipart/form-data');

	# add warning if there are too many (>1000 ?) files    FIXME
	if (scalar(@dafiles) > 1) {
	    Delete('checkAll');  # do not precheck

	    print
		checkbox(-name=>'checkAll',
			 -id=>'checkAll',
			 -value=>'true',
			 -onClick=>"toggleAll('checkAll','src_files')",
			 -label=>'Select/Unselect All');
	}

	print
	    "<table class='filterable' border='0' cellspacing='5' width='100%'>\n<thead>\n",
	    "<tr><th class='fileentry'>File Name</th>",
	    "<th class='fileentry' align='middle'>View?</th>",
	    "<th class='fileentry' align='middle'>Size</th>",
	    "<th class='fileentry' align='middle'>Date Modified</th>",
	    "</tr></thead>\n<tbody>\n";

	my $fc = 0;
	my $fclass;

	foreach my $sFile (@dafiles) {
	    my $filepath = $d_src.$sFile;

	    # get stats for each file
	    @fstat = stat $filepath;
	    $fileSize = &human_size($fstat[7]);
	    $fileAge  = scalar localtime($fstat[9]);

	    # when user is looking for search db, let's not show stuff we know isn't searchdb
	    my $maybe_db = 1;
	    if ($filepath =~ /\.(shtml|html|xml|mzml|mzxml|dta|png|gif)$/) {
		$maybe_db = 0;
	    }

	    # links to view files
	    my $file_url = $filepath;
	    $file_url =~ s/$www_root/\//;
	    my $viewlink = &makeFileLink(file_loc => $filepath,
					 file_url => $file_url,
					 extended => 0);

	    # allow edits in filebrowser only for files < 1Mb
	    if ( ($szMode eq 'browse') &&
		 (-T $filepath) &&
		 ($fstat[7] < 1000000) ) {
		chop($viewlink); #remove closing bracket
		$viewlink .= " | <a href=\"$tpp_url?Action=$web_actions{'showpage'}&page=editfile&workdir=$d_src&file=$sFile\">Edit</a> ]";
	    }

	    # don't show things that are known not to be search dbs
	    # if the user is looking for search dbs
	    if (($proc_type ne 'searchdb') || ($maybe_db eq 1)) {
		$fc++;
		$fclass = ($fc % 4) ? "": "class=fileentry";

		print
		    "<tr><td $fclass>",
		    checkbox(-name=>"src_files",
# later...     		 -id=>"${file_id}_cb",
			     -label=>$sFile,
			     -value=>$sFile,
# later...      	 -onClick=>"hilight('$file_id')"
			     ),
		    "</td>",
		    "<td $fclass align=middle>$viewlink</td>",
		    "<td $fclass align='right'>$fileSize</td>",
		    "<td $fclass align='right'>$fileAge</td></tr>\n";
	    }
	}

	print
	    "</tbody></table>\n",
	    "<p><b>$fc</b> $file_text found</p>\n";

	my $jscripts = readconfig( 'web_scripts', '/ISB/html/js' );

	print "<script type='text/javascript' src='$jscripts/filterTable.js'></script>\n" if ($#dafiles > 4);

	print "<div class='file'>\n";

	if ($szMode eq 'browse') {
	    print
		submit(-name=>'Action',
		       -value=>$web_actions{'deleteFiles'}),
		submit(-name=>'Action',
		       -id=>'CopyFiles',
		       -value=>$web_actions{'copyFiles'});

	} else {
	    print
		hidden(-name=>'workdir',
		       -default=>$d_src),
		hidden(-name=>'proc_type',
		       -value=>''),
		submit(-name=>'Action',
		       -value=>$web_actions{'selectFiles'}),
		"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

	}

    } else {
	print
	    br,
	    "There are no $file_text of the requested type ($file_ext) in this directory.",
	    br,br,
	    "<div class='file'>\n",
	    start_form('POST',$tpp_url,'multipart/form-data');
    }

    if ($szMode eq 'browse') {
	print
	    hidden(-name=>'workdir',
		   -default=>$d_src),
	    submit(-name=>'Action',
		   -id=>'PasteFiles',
		   -onclick=>"document.getElementById('PasteFiles').className='running';",
		   -value=>$web_actions{'pasteFiles'}),
	    submit(-name=>'Action',
	           -id=>'S3Sync',
		   (-f "${users_dir}$auth_user/.s3cfg" ? () : (-disabled => undef) ),
		   -value=>$web_actions{'s3cmd'}),
	    submit(-name=>'Action',
		   -id=>'DownloadFiles',
		   -value=>$web_actions{'downloadFiles'}),
	    submit( -name => 'Action', -value => $web_actions{'uploadFile'}),
	    ":",
	    filefield( -name=>'file', -value => '', -size => 30 ),
	    "</div>",
	    end_form,
	    "</td>\n";

    } else {
	print
	    hidden(-name=>'file_ext',
		   -value=>$file_ext),
	    hidden(-name=>'ref_page',
		   -value=>$ref_page),
	    submit(-name=>'Action',
		   -value=>$web_actions{'cancelSelect'}),
	    "</div>",
	    end_form,
	    "</td>\n";
    }

    print "<script type='text/javascript'>document.getElementById('PasteFiles').disabled = true;</script>\n" unless $clipboard;


    # Directories pane
    #
    print
	"<td style='vertical-align:top' class='file'><h4>DIRECTORY TREE:</h4>";

    my $selfurl;
    if ($szMode eq 'browse') {
	$selfurl = "$tpp_url?Action=display&page=filebrowser&file_ext=$file_ext";
    } else {
	$selfurl = "$tpp_url?Action=display&page=filechooser&file_ext=$file_ext&ref_page=$ref_page&proc_type=$proc_type";
    }

    print "<ul>\n";
    my ($go_up, $goto_dir);
    my $indent = "";
    while($d_src =~ m|/|g) {
	$goto_dir = $`."/";
	$go_up = ($goto_dir =~ /$data_dir/) ? 1 : 0;

	if ($go_up && ($goto_dir ne $d_src)) {
	    print
		"<li>",
		"Go up to: $indent";

	    print '|-- ' if ($indent);

	    chop($goto_dir);
	    $goto_dir =~ m|/([^/]+)$|;

	    print
		a({-href=>$selfurl."&workdir=$goto_dir"},
		  "$1"),
		"</li>\n";

	    $indent .= '&nbsp;&nbsp;&nbsp;&nbsp;';
	}
    }

    # print current directory (only if not top-level)
    if ($indent) {
	chop($goto_dir);
	$goto_dir =~ m|/([^/]+)$|;
	print "<li>Current: $indent|--$1</li>\n";
    }
    print "</ul>\n";

    print "<h4>SUB-DIRECTORIES:</h4>";
    print "<ul>\n";
    if (@dadirs) {
	for (@dadirs) {
	    print
		"<li>",
		a({-href=>$selfurl."&workdir=$d_src/$_"},
		  "$_"),
		"</li>\n";
	}
    } else {
	print
	    "<li>No subdirectories found in this folder.</li>\n";
    }
    print "</ul>\n";

    if ($szMode eq 'browse') {
	Delete('Action');

	# Contributed by Bill Nelson
	print
	    start_form('POST',$tpp_url),
	    submit(-name=>'Action',
		   -value=>$web_actions{'newDir'})."\n",
	    textfield(-name=>'new_folder',
		      -value=>"",
		      -size=>15,
		      -maxlength=>30),
	    br,
	    hidden(-name=>'ref_page',
		   -value=>$ref_page)."\n",
	    hidden(-name=>'file_ext',
		   -value=>$file_ext)."\n",
	    hidden(-name=>'workdir',
		   -value=>$d_src)."\n",
	    # if there is only one textfield and enter is hit instead of clicking the button
	    # the submit button's name/value will not be sent. so put this in a hidden field also
	    hidden(-name=>'Action',
		   -value=>$web_actions{'newDir'})."\n",
	    end_form."\n";
    }

    print
	"</td>\n</tr>\n</table>",
	"</div>\n";

}

########################################################################
# makeFileLink
#
# Returns html with link to open/view file with appropriate viewer
#
########################################################################
sub makeFileLink {
    my %args = @_;

    my $file_loc = $args{file_loc};
    my $file_url = $args{file_url};
    my $file_txt = $args{file_txt} || '';
    my $incl_filename = $args{extended} || 0;

    my $html_file = "<li><b>$file_txt</b>";
    my $html_link;

    if (-d $file_loc) {
	$html_link = "[ <b>All files</b> are in <a href=\"$tpp_url?Action=$web_actions{'showpage'}\&page=filebrowser\&file_ext=*\&workdir=$file_loc\">this directory</a></b> ]";
    } elsif (!-f $file_loc) {
	$html_file = "<li>$file_txt";
	$html_link = "(<b>file not found:</b> not created or deleted?)";

    } elsif ( (-T $file_loc) || ($file_loc =~ /\.gz$/i) )  {  # -T means "is it a textfile?"
	my $tpp_ftype = &isTPPXmlFile($file_loc);

	if ($tpp_ftype eq 'pep') {
	    # launch pepXML viewer in external window
	    $html_link = "[ <a target=\"_blank\" href=\"$tpp_bin_url/PepXMLViewer.cgi?xmlFileName=$file_loc\">PepXML</a> ]";
	} elsif ($tpp_ftype eq 'prot') {
	    # launch protXML viewer in external window
	    $html_link = "[ <a target=\"_blank\" href=\"$tpp_bin_url/ProtXMLViewer.pl?file=$file_loc\">ProtXML</a> | <a target=\"_blank\" href=\"$tpp_bin_url/protxml2html.pl?xmlfile=$file_loc&restore_view=yes\">(legacy viewer)</a> ]";
	} elsif ($tpp_ftype eq 'params') {
	    # add link to simple params editor
	    my $fname = basename($file_loc);
	    my $fdir  = dirname($file_loc);
	    $html_link = "[ <a href=\"$tpp_url?Action=$web_actions{'showpage'}&page=editparams&workdir=$fdir&file=$fname\">Params</a> ]";
	} elsif ($tpp_ftype eq 'mz') {
	    # launch pep3d in external window
	    $html_link = "[ <a target=\"_blank\" href=\"$tpp_bin_url/Pep3D_xml.cgi?htmFile=$file_loc\">Pep3D</a> ]";
	} elsif ($tpp_ftype eq 'err') {
	    # could not open file...
	    $html_link = "[ <font color='red'>Unreadable!</font> ]";
	} elsif ($file_loc =~ /\.dta$/i) {
	    # launch plot-msms in external window
	    $html_link = "[ <a target=\"main\" href=\"$tpp_bin_url/plot-msms-js.cgi?Dta=$file_loc\">spectrum</a> ]";
	} elsif ($file_loc =~ /\.gz$/i) {
	    $html_link = "---";
	} elsif ($file_loc =~ /\.prot\.json$/i) {
	    $html_link = "[ <a target=\"pgwin\" href=\"/ISB/html/ProteoGrapher/PG.html?file=$file_url&terms=terms.json\"> ProteoGrapher </a> ]";
	} else {
	    $html_link = "[ <a target=\"_blank\" href=\"$file_url\">View</a> ]";
	}

    } elsif ($file_loc =~ /\.(png|gif)$/) {   # add others?
	$html_link = "[ <a target=\"_blank\" href=\"$file_url\">View</a> ]";
    } else {
	# unidentified binary file; no link
	$html_link = "---";
    }

    return $incl_filename ? "$html_file $html_link</li>\n" : $html_link;
}

########################################################################
# stripPaths
#
# Strip out base path name from a list of files
#
########################################################################
sub stripPaths {
    my %args = @_;
    my @files = @{$args{files}};
    my $base_path = $args{bpath} || dirname($files[0]);

    my @stripped_files;

    foreach my $file (@files) {
	$file =~ s|^$base_path/?||; 
	push @stripped_files, $file;
   }

    return @stripped_files;
}

########################################################################
# Convert a numeric size into a human readable value 
########################################################################
sub human_size {
    my $val = shift;

    # 2**10 (binary) multiplier by default
    my $multiplier = @_ ? shift: 1024;

    my $magnitude = 0;
    my @suffixes  = qw/B KB MB GB TB PB EB/;

    my $rval;

    while ( ( $rval = sprintf( "%.1f", $val ) ) >= $multiplier )
    {
	$val /= $multiplier;
	$magnitude++;
    }

    # Use Perl's numeric conversion to remove trailing zeros
    # in the fraction and the decimal point if unnecessary
    $rval = 0 + $rval;

    return sprintf( "%s %2.2s",  $rval, $suffixes[$magnitude] );
}


########################################################################
# handleAjaxRequest
#
# returns xml response to specific AJAX queries
#
# All xml response documents have the following structure:
#  <response>
#    <date></date>
#    ...
#    <message></message>
#  </response>
#
########################################################################
sub handleAjaxRequest {
    my $xml_response = "Content-type:text/xml\n\n<response>\n";
    $xml_response .= "<date>".scalar(localtime)."</date>\n";

    # this is all very ad-hoc at the moment...
    my $action = param('Action');
    my $param1 = param('p1') if param('p1');
    my $param2 = param('p2') if param('p2');

    my $myusersession = &getSession();
    if ( $action eq 'AJAXPing' ) {
        my $cb = param('Callback');
        if ( $cb !~ /^[A-Z_$][0-9A-Z_]*$/i ) {
            &fatalError('Invalid function name used for javascript callback');
        }
        print "Content-type:text/javascript\n\n";
        print "$cb({ TPPVersionInfo: '$TPPVersionInfo' })";
        return;
    } elsif (!$myusersession) {
	$xml_response .= "<status>ERROR</status>\n";
	$xml_response .= "<message>Could not read valid user session from cookie.</message>\n";

    } elsif ($action eq 'AJAXCheckStatusUpdates') {
	# Sub-elements in response:
	# <command>
	#   <cid>
	#   <status>

	my @jobs_data = &getAllCommandsStatus('all');

	foreach (@jobs_data) {
	    my ($jcmd, $jname, $jloc, $jstatus) = split /\t/, $_;

	    $xml_response .= "<command>\n";
	    $xml_response .= "<cid>$jcmd</cid>\n";
	    $xml_response .= "<status>$jstatus</status>\n";
	    $xml_response .= "</command>\n";
	}
	$xml_response .= "<message>-</message>\n";

    } elsif ($action eq 'AJAXCheckStatus') {
	# Sub-elements in response:
	# <status_summary>
	#   <status>
	#   <count>

	my %retstat = &getAllCommandsStatus('summary');
	if (%retstat) {
	    foreach my $stat (keys %retstat) {
		$xml_response .= "<status_summary>\n";
		$xml_response .= "<status>$stat</status>\n";
		$xml_response .= "<count>$retstat{$stat}</count>\n";
		$xml_response .= "</status_summary>\n";
	    }
	}
	$xml_response .= "<message>-</message>\n";

    } elsif ($action eq 'AJAXGetSessionInfo') {
	# returning html for now...

	my $html_response = "Content-type:text/html\n\n";
	$html_response .= "<p>You requested: <b>$param1</b> for user <b>$param2</b></p>";

	my $user_session_file = "${users_dir}$param2/$param1";
	if (-e $user_session_file) {
	    open(SESSION, "$user_session_file") ||  &fatalError("AJAX_CANNOT_OPEN_SESSION_FILE:$session_file:$!");

	    $html_response .= "<table>";
	    while (<SESSION>){
		chomp;

		$html_response .= "<tr>";
		if (/:/) {
		    my $type = $`;
		    my $file = $';  #'
		    my $class = '';

		    if ($type =~ s/^\@//) {
			$class = 'setting';
		    } else {
			$class = (-e $file) ? 'file' : 'stale';
		    }

		    $html_response .= "<th class=$class>$type</th><td class=$class>$file</td>";

		} else {
		    $html_response .= "<td colspan=2>".escapeHTML($_)."</td>";
		}
		$html_response .= "</tr>\n";

	    }
	    close(SESSION);

	    # check for commands status
	    $param1 =~ /session_(.+)/;
	    my %retstat = &getAllCommandsStatus('summary',$1);
	    my $retstatus;

	    if (%retstat) {
		foreach my $stat (keys %retstat) {
		    $retstatus .= "$retstat{$stat} jobs $stat,";
		}
		chop($retstatus); # get rid of last comma
	    } else {
		$retstatus = 'none';
	    }
	    $html_response .= "<tr><th class=setting>Commands Status</th><td class=setting>$retstatus</td></tr>";
	    $html_response .= "</table><br><br>";

	    $html_response .= "<table>";
	    $html_response .= "<tr><th>Legend</th><td></td></tr>";
	    $html_response .= "<tr><th class=setting>White</th><td class=setting>Config / Setting</td></tr>";
	    $html_response .= "<tr><th class=file>Blue</th><td class=file>Existing file or directory</td></tr>";
	    $html_response .= "<tr><th class=stale>Orange</th><td class=stale>Invalid/missing file or directory</td></tr>";
	    $html_response .= "</table>";

	} else {
	    $html_response .= "<p><b>Session file not found!</b></p>";
	}

	print $html_response;
	return;


    } elsif ($action eq 'AJAXCheckTPPVersion') {
	my $html_response = "Content-type:text/html\n\n";

	return unless ($TPPVersionInfo =~ /(\d{12})/);
	my $build_version = $1;
	my $version_url = "http://tools.proteomecenter.org/tppv.xml?v=".CGI::escape($TPPVersionInfo);
       if ( my ( $aid, $ami ) = getEC2Info() ) {
            $version_url .= '&aid=' . CGI::escape($aid) if ( $aid );
            $version_url .= '&ami=' . CGI::escape($ami) if ( $ami );
        }

	my $http_request = `$command{wget} -q -O - $version_url`;

	return unless ($http_request);

	# Set up the XML parser and parse the returned XML
	my $parser = XML::Parser->new(
				      Handlers => {
					  End  => \&end_xml_element,
					  Char => \&xml_chars,
				      },
				      ErrorContext => 2 );
	eval { $parser->parse( $http_request ); };

	if($@) {
	    &printToLog("---\nERROR_PARSING_XML_RESPONSE:$@\n\n$http_request\n---\n");
	    return;
	}

	return unless ( $tmp_hash{version_build} );

	if ( $tmp_hash{version_build} > $build_version ) {
	    $html_response .= "<ul>".
		"<li>A more recent version ($tmp_hash{version_short}, released on $tmp_hash{release_date}) of the TPP may be available for download.</li>".
		"</ul>";
	}

	$html_response .= "<ul>";
	foreach (@messages) {
	    $html_response .= "<li>$_</li>";
	}
	$html_response .= "</ul>";

	print $html_response;
	return;

    } else {
	$xml_response .= "<status>ERROR</status>\n";
	$xml_response .= "<message>$action is not recognized as a valid action.</message>\n";

    }
    $xml_response .= "</response>";


    print $xml_response;
}

#
# The following code comes from String::ShellQuote. 
#
# Copyright (c) 1997 Roderick Schertler.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.
#
sub shell_quote {
    my @in = @_;

    return '' unless @in;

    my $ret = '';
    foreach (@in) {
	if (!defined $_ or $_ eq '') {
	    $_ = "''";
	    next;
	}

	if (s/\x00//g) {
	    &fatalError( "SHELL_QUOTE: No way to quote string containing null (\\000) bytes" );
	}

	# = does need quoting else in command position it's a program-local
	# environment setting

	if (m|[^\w!%+,\-./:@^]|) {

	    # ' -> '\''
    	    s/'/'\\''/g;

	    # make multiple ' in a row look simpler
	    # '\'''\'''\'' -> '"'''"'
    	    s|((?:'\\''){2,})|q{'"} . (q{'} x (length($1) / 4)) . q{"'}|ge;

	    $_ = "'$_'";
	    s/^''//;
	    s/''$//;
	}
    }
    continue {
	$ret .= "$_ ";
    }

    chop $ret;
    return $ret;
}
