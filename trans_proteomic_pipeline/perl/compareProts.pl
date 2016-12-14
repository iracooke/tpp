#!/usr/bin/perl -w

use strict;

$, = ' ';
$\ = "\n";

# grab our tpplib exports from the same directory as this script
use File::Basename;
use Cwd qw(realpath cwd);
use lib realpath(dirname($0));
use FindBin;
use lib "$FindBin::Bin/../cgi-bin";
use tpplib_perl; # exported TPP lib function points


if(@ARGV == 0) {
    print STDERR " usage: compareProts.pl [-D3] <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison to tab delim file 'compare.xls'\n\n";
    print STDERR " or to rename output file: \n";
    print STDERR "        compareProts.pl -Nalternative.xls <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison to user specified excel file 'alternative.xls'\n\n";
    print STDERR " or to specify input columns to be reported (along with protein names): \n";
    print STDERR "        compareProts.pl -Nalternative.xls -h'protein probability' -h'ASAPRatio mean' <tab delim file1><tab delim file2> ...\n";
    print STDERR "             writes comparison with protein probability and ASAPRatio mean columns to user specified tab delim file 'alternative.xls'\n\n";
    print STDERR " or to specify NO columns to be reported (along with protein names): \n";
    print STDERR "        compareProts.pl -Nalternative.xls -h <tab delim file1><tab delim file2> ...\n";

    print STDERR "\n\n";
    exit(1);
}

my $D3 = 0;

my @FILES = (); #@ARGV;

my $minProb = 0.2;

my $outfile = 'compare.xls';

my @info = ();

# check for outfile
for(my $k = 0; $k < @ARGV; $k++) {
    if($ARGV[$k] =~ /^\-N(\S+)$/) {
	$outfile = $1;
    }
    elsif($ARGV[$k] =~ /^\-p(.*)$/) {
	$minProb = $1;
    }
    elsif($ARGV[$k] =~ /^\-h(.*)$/) {
	push(@info, $1);
    }
    elsif($ARGV[$k] =~ /^\-D3$/) {
	$D3 = 1;
    }
    else {
	push(@FILES, $ARGV[$k]);
    }
}


die "no input excel files specified\n" if(@FILES == 0);

my $JS_HOME;

if ( $^O eq 'linux' ) {
    $JS_HOME = 'tpp/html/js/';  
#
# Cygwin Configuration
#
} elsif ( ($^O eq 'cygwin' )||($^O eq 'MSWin32' )) {
    $JS_HOME = 'ISB/html/js/';
}


#my @prots = ();
my @inds_ref = ();
my @inds = (0);
my %all_prots = ();
#@info = ('group_no', 'prot_prob', 'ASAPRatio mean') if(@info == 0); # default
@info = ('entry no.', 'protein probability', 'ratio mean') if(@info == 0); # default
@info = () if(@info == 1 && ! $info[0]);

my %info = ();
foreach(@info) {
    $_ =~ s/'//g; #####'
    my %next = ();
    for(my $f = 0; $f < @FILES; $f++) {
	$next{$FILES[$f]} = -1;
    }
    $info{$_} = \%next;
}

my %prots = ();

my %prot_groups = ();

for(my $f = 0; $f < @FILES; $f++) {
#foreach(@FILES) {
#    print "$_....\n";
    #print STDERR "$ARGV[$f]...\n";
    print STDERR " $FILES[$f]...\n";
    $prots{$FILES[$f]} = extractProts($FILES[$f]);

    $prot_groups{$FILES[$f]} = extractProtGroups($FILES[$f]);
    #$prots{$ARGV[$f]} = extractProts($ARGV[$f]);
    #push(@prots, extractProts($_));
    push(@inds_ref, 0);
} # next file

#exit(1);
# now look at by subset
#print "total: \n";
#foreach(keys %prots) { print "$_ "; } print "\n"; exit(1);


#foreach(@inds_ref) {
#    print "$_ ";
#}
#print "\n";
my $index = 1;
while(increment(\@inds_ref, $index++)) {
    my @next = ();
    foreach(@inds_ref) {
	push(@next, $_);
    }
    push(@inds, \@next);

#    foreach(@inds_ref) {
#	print "$_ ";
#    }
#    print "\n";

}
$index--;
#print "final index: $index\n"; exit(1);
my @all_prots = keys %all_prots;


# now group each entry according to where it falls
my @grp_members = (0); # null for first group (all zeros)
my %founds = ();
for(my $k = 1; $k < $index; $k++) {
    my @next_group = ();
    for(my $j = 0; $j < @all_prots; $j++) {
	my $f;
	if(! exists $founds{$j}) {
	    for($f = 0; $f < @FILES; $f++) {

#		${$prots {$FILES[$f]}{${$prot_groups{$FILES[$f]}}{$all_prots[$j]}} }

		if(${$inds[$k]}[$f] != exists ${$prot_groups{$FILES[$f]}}{$all_prots[$j]}) { # not a member
		    $f = @FILES + 1; # no match
		}
	    } # next file
	    if($f == @FILES) { # match
		
		push(@next_group, $all_prots[$j]);
		$founds{$j}++;
	    }
	} # if not already allocated to group
    } # next protein
    push(@grp_members, \@next_group);
}
# header



#exit(1);

for(my $k = 1; $k < $index; $k++) {
    #print "k: $k (vs $index)\n";
    my @files = @{getFiles($inds[$k], \@FILES)};
    my $flat_file = join('_', @files);
    $flat_file =~ s/\//-/g;
	
	if ($outfile eq "") {
		$outfile = "COMPARE_".$flat_file ;
    }
# output
    open(OUT, ">$outfile") or die "cannot open $outfile $!\n";
    
    my $get_rat = 0;
    print OUT "file_combo\tprotein";
    for(my $k = 0; $k < scalar(@files); $k++) {
	for(my $i = 0; $i < scalar(@info); $i++) {
	    print OUT "\t$info[$i]";
	    if($info[$i] eq "max group pct share of spectrum ids") {
		$get_rat = 1;
	    }
	}
    }
    
	if (scalar(@files)==2 && $get_rat == 1 ) {
		
########### D3 OUTPUT START ######################
	if ($D3==1)  { 
my $TPPhostname = tpplib_perl::get_tpp_hostname();
	
my $d3outfile = $outfile.".d3.html";
print "D3 Output File: $d3outfile \n";
open(D3OUT, ">$d3outfile") or die "cannot open $outfile $!\n";;

print D3OUT ' 

<!DOCTYPE html>
<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html;charset=utf-8">
    <title>Zoom + Pan</title>
    <script type="text/javascript" src="';

print D3OUT '/'.$JS_HOME.'/d3.js">
    </script>
    <style type="text/css">


a:link {text-decoration:none;}    /* unvisited link */
a:visited {text-decoration:none;} /* visited link */
a:hover {text-decoration:underline;}   /* mouse over link */
a:active {text-decoration:underline;}  /* selected link */

svg {
  font: 10px sans-serif;
  shape-rendering: crispEdges;
}

.help {
  font: 10px sans-serif;
  shape-rendering: crispEdges;
  color: #DD0000;
}

path.dot {
  fill: white;
  stroke-width: 1.5px;
}

td      {
    font-family: Helvetica, Arial, sans-serif; 
    font-size: 9pt; 
    vertical-align: top
    
}


button {
  left: 275px;
  position: absolute;
  top: 145px;
  width: 80px;
}


#TableDiv {
            display: table-cell; 
            border: 2px solid black;
            margin: 0;
            }

.data {
        /* borders for the whole table */
        table-layout: auto;
        /*border: none;*/
        background: #aaaaaa;
        border-collapse: collapse;
        border: 1px solid black;
	display: table-cell; 
        margin: 0;
      }
td {
	border-left: 2px solid white;
	border-right: 2px solid white;
	width: 33%;
  }

td.ratio {
        width: 10%;
  }

td.link  {
        width: 20%;
  }


td.desc {
        width: 70%;
  
  }


tr.even {
                background: #DDE0FF;
		border: 1px solid steelblue;
        }



tr.odd {
               background: #FFFFFF;
	       border: 1px solid cyan;
	       	      
       }



table.top {
                background: #FFE000;
		border: 1px solid red;
		width: 100%;
		text-align: left;
        }

table.spacer {
                background: lightgray;
		border: 1px solid lightgray;
		width: 100%;
		text-align: left;
        }


table.even {
                background: #DDE0FF;
		border: 1px solid steelblue;
		width: 100%;
	        
        }



table.odd {
               background: #FFFFFF;
	       border: 1px solid cyan;
	       width: 100%;
	       text-align: left;
	   }

.spacer {
  height: 30px;
  width: 10px;
  fill: none;
  border: 1px solid;
  display: inline;
}

    </style>
  </head>
  <body>
    <script type="text/javascript">

';

	print D3OUT "var data = [";
}
########### D3 OUTPUT ######################

	print OUT "\tratio of max group spectrum share pct";
    }
    print OUT "\n";

    #print "proteins found in ", getFileCombo($inds[$k], \@ARGV), " (total: ", scalar @{$grp_members[$k]}, ")\n";
    for(my $j = 0; $j < @{$grp_members[$k]}; $j++) {
	print OUT "$flat_file\t${$grp_members[$k]}[$j]";
	for(my $f = 0; $f < @files; $f++) {
	    my @parsed = split("\t", ${$prots{$files[$f]}}{${$grp_members[$k]}[$j]});
	    for(my $i = 0; $i < @info; $i++) {
		print OUT "\t";
		if(${$info{$info[$i]}}{$files[$f]} >= 0) {
		    print OUT "$parsed[${$info{$info[$i]}}{$files[$f]}]";
		}
		else {
		    print OUT "?";
		}
	    } # next info
	} # next file
	if (scalar(@files)==2) {
	    my @parsed0 = split("\t", ${$prots{$files[0]}}{${$grp_members[$k]}[$j]});
	    my @parsed1 = split("\t", ${$prots{$files[1]}}{${$grp_members[$k]}[$j]});
	    
	    for(my $i = 0; $i < @info; $i++) {
		print OUT "\t";

		if($info[$i] eq "max group pct share of spectrum ids") {
		    if ($parsed0[${$info{$info[$i]}}{$files[0]}] == 0 && $parsed1[${$info{$info[$i]}}{$files[1]}] == 0) {
			print OUT "?";
		    }
		    elsif ($parsed0[${$info{$info[$i]}}{$files[0]}] == 0) {
			print OUT "-inf";
		    }
		    elsif ($parsed1[${$info{$info[$i]}}{$files[1]}] == 0) {
			print OUT "inf";
		    }
		    else {
			if ($D3==1) {
########### D3 OUTPUT ######################
                             print D3OUT '{"proteinDesc": "' . $parsed0[${$info{"description"}}{$files[0]}] . '", "proteinLink": "' . $parsed0[${$info{"uniprot link"}}{$files[0]}] . '", '
			                 . '"logRatio": ' . log($parsed0[${$info{$info[$i]}}{$files[0]}]/$parsed1[${$info{$info[$i]}}{$files[1]}]) . '}, ';
                        }
			print OUT log($parsed0[${$info{$info[$i]}}{$files[0]}]/$parsed1[${$info{$info[$i]}}{$files[1]}]);
		
		    }
		}
		
	    } # next info
	}
	
	print OUT "\n";
    } # next protein in that combo
    if (scalar(@files)==2 && $get_rat == 1 && $D3==1) {
########### D3 OUTPUT ######################
            print D3OUT "];";
            print D3OUT '

var maxRatio = d3.max(data.map(function(d) { return d.logRatio; } ) );
var minRatio = d3.min(data.map(function(d) { return d.logRatio; } ) );
var ratioCount = data.length;

var prevscale = -1;
var scale = 1;
var transl = [0,0];
var ptransl = [-1,-1];

data.sort(function(a, b) { return a.logRatio- b.logRatio; } );

var lastXY = [];
var plotXY = [];
var allXY = [];

data.forEach(function(d,i,a) { 
			//console.log("X="+i+",Y="+d.logRatio);
			allXY.push({ "x": i, "y": d.logRatio, "link": d.proteinLink , "desc": d.proteinDesc });
				  } );
var w = ratioCount*2;

if (w > window.innerWidth) {
  w = window.innerWidth;
}
var size = [w, 500], // width height
    padding = [4, 4, 20, 20], // top right bottom left
    tx = function(d) { return "translate(" + x(d) + ",0)"; },
    ty = function(d) { return "translate(0," + y(d) + ")"; },
    stroke = function(d) { return d ? "#ccc" : "#666"; };

// x-scale (1.42 = 710/500)
var x = d3.scale.linear()
    .domain([0, ratioCount])
    .range([0, size[0]]);

// y-scale (inverted domain)class

var y = d3.scale.linear()
    .domain([minRatio, maxRatio])
    .range([size[1], 0]);


var svg =  d3.select("body").append("svg")
    .attr("width", size[0] + padding[3] + padding[1])
    .attr("height", size[1] + padding[0] + padding[2])
    .attr("pointer-events", "all")
  .append("g")
    .attr("transform", "translate(" + padding[3] + "," + padding[0] + ")")
    .call(d3.behavior.zoom()
      .extent([[0, size[0]], [0, size[1]], [0, Infinity]])
      .on("zoom", function() { 
                                prevscale = scale; scale = d3.event.scale; transl = d3.event.translate;
                                recalc_plotXY();
				redraw();
			      }))
    .append("g");

						

// Generate x-ticks
var gx = svg.selectAll("g.x")
    .data(x.ticks(10))
  .enter().append("g")
    .attr("class", "x")
    .attr("transform", tx);

gx.append("line")
    .attr("stroke", stroke)
    .attr("y1", 0)
    .attr("y2", size[1]);

gx.append("text")
    .attr("y", size[1])
    .attr("dy", "1em")
    .attr("text-anchor", "middle")
    .text(x.tickFormat(10));

// Generate y-ticks
var gy = svg.selectAll("g.y")
    .data(y.ticks(10))
  .enter().append("g")
    .attr("class", "y")
    .attr("transform", ty);

gy.append("line")
    .attr("stroke", stroke)
    .attr("x1", 0)
    .attr("x2", size[0]);

gy.append("text")
    .attr("x", -3)
    .attr("dy", ".35em")
    .attr("text-anchor", "end")
    .text(y.tickFormat(10));


svg.append("rect")
    .attr("width", size[0])
    .attr("height", size[1])
    .attr("stroke", stroke)
    .attr("fill", "none");

lastXY = allXY;
plotXY = allXY; //to start

var xfocus = [];
var div;
var last;
var dots = svg.selectAll("circle")
    .data(plotXY, function(d) { return d.link; } )
    .enter().append("circle")
    .attr("class", "dot")
    .attr("stroke", function(d) {return ptcolor(d.x); } )
    .attr("fill", function(d) {return "none"; } )
    .attr("transform", function(d) { //console.log("translate(" + x(d.x) + "," + y(d.y) + ")");
                                    return "translate(" + x(d.x) + "," + y(d.y) + ")";

				    })
    .on("click", function(d) { return window.open(d.link); } )
    .on("mouseover", function(d) { 
                                    xfocus = [d];
                                    mouseOut();
				    hover();
				    d3.select(this).transition().duration(100).attr("stroke","red").attr("fill", "yellow").attr("r", 10); //this.focus();	    

				    div = d3.select("body").selectAll("div").data([], function(dd) { 
				                                                                          return dd.x; 
													} ).exit().remove();

				    //TABLE
				    div =d3.select("body").selectAll("div")//.style("background", even_odd(d.x,1))
				                          .data([d], function(dd) { 
				                                                   return dd.x; 
										} )
							  .enter().append("div").style("position","relative").style("left", window.scrollLeft).style("bottom", "5px")
							  .append("table").attr("class", "top").append("tr")

				    div.append("td").attr("class","ratio").text(d.y);
   
                                    div.append("td").attr("class","link")
				         .append("a")
					 .attr("xlink:href",d.link)
					 .text(d.link)
					 .on("click", function() { return window.open(d.link); } );

				    div.append("td").attr("class","desc")
				          .text(function() { return d.desc; })
					  .on("click", function() { return window.open(d.link); } );
     



                                    d3.selectAll("div").attr("index", function(dd, i) { return i; });

				    
				  })
    .on("mouseout", function(d) {
                                     mouseOut();	
				     d3.select(this).transition().duration(100).attr("stroke","red").attr("r",10);
				})
				     
    .attr("r", 5)
    .append("title").text(function(d) { return d.link+"\n"+d.desc; });

dots.data(plotXY, function(d) { return d.link; } ).exit().remove();

var help = size;
help[0] = 10;
help[1] = help[1]-help[0];

var hp1 = d3.select("body").append("p").append("text").attr("class","help")
  //  .style("font-size", "12px")
    .text("* DoubleClick to Zoom then Drag to Pan");

hp1.append("p").append("text").attr("class","help")
//    .style("font", "1px")
    .text("* Hold Shift and DoubleClick to Unzoom");

hp1.append("hr");

recalc_plotXY();
redraw();

function mouseOut() {
 d3.selectAll("circle").data(allXY).transition().duration(100).attr("stroke", function(dd) {return ptcolor(dd.x); } ).attr("r", 5).attr("fill", "none");
 recalc_plotXY();
 redraw();
}

function hover() {
 d3.selectAll("circle").data(allXY).transition().duration(100).attr("stroke", "lightgray" ).attr("r", 3).attr("fill", "none");
}

function  refresh() {
   if (d3.event) {
      d3.event=null;
   }   
   prevscale = scale;
   scale = 1;
   transl = [0,0];
   recalc_plotXY();
   redraw();

}    

function recalc_plotXY() {
   var Tdata = [];
    svg.attr("transform",
      "translate(" + transl + ")"
           + "scale(" +	scale + ")");  
   if (scale <= 0) scale = 1;

   if (prevscale != scale || transl[0] != ptransl[0] || transl[1] != ptransl[1]  ) { 

                      allXY.forEach(function(d,i,a) { 
			         //console.log("X="+i+",Y="+d.logRatio);
				 var tX = x(d.x)*scale+transl[0];
				 var tY = y(d.y)*scale+transl[1];
				 if (tX >= 0 && tY >= 0 ) {		 
					Tdata.push(d);
				  }
		            });
			    lastXY = plotXY;
			    plotXY = Tdata;
   }
   else {
	 plotXY = lastXY;;
   }
   prevscale = scale;
   ptransl = transl;
}



function redraw() {

  


div = d3.select("body").selectAll("div").data([], function(d) { 
                                                                    return d.x; 
								   }).exit().remove();
 div = d3.select("body").selectAll("div")
   .data(xfocus, function(d) { return d.x; } )
   .enter().append("div").style("position","relative").style("left", window.scrollLeft).style("bottom", "5px")
   .append("table").attr("class", "top").append("tr"); 

   div.append("td").attr("class","ratio").text(function(d) { return d.y; });
   
   div.append("td").attr("class","link")
            .append("a")
	    .attr("xlink:href",function(d) { return d.link; })
	    .text(function(d) { return d.link; })
	    .on("click", function(d) { return window.open(d.link); } );

   div.append("td").attr("class","desc")
            .text(function(d) { return d.desc; })
	    .on("click", function(d) { return window.open(d.link); } );

//SPACER
if (xfocus.length > 0) {
 div = d3.select("body").selectAll("div")
   .data(["..."], function(d) { return d; })
   .enter().append("div").style("position","relative").style("left", window.scrollLeft).style("bottom", "5px")
   .append("table").attr("class", "spacer").append("tr"); 

   div.append("td").text(function(d) { return d; });
}

 //TABLE
 div = d3.select("body").selectAll("div")
   .data(plotXY, function(d) { return d.x; } )
   .enter().append("div").style("position","relative").style("left", window.scrollLeft).style("bottom", "5px")
   .append("table").attr("class", function(d) { return even_odd(d.x); }).append("tr"); 

   div.append("td").attr("class","ratio").text(function(d) { return d.y; });
   
   div.append("td").attr("class","link")
            .append("a")
	    .attr("xlink:href",function(d) { return d.link; })
	    .text(function(d) { return d.link; })
	    .on("click", function(d) { return window.open(d.link); } );

   div.append("td").attr("class","desc")
            .text(function(d) { return d.desc; })
	    .on("click", function(d) { return window.open(d.link); } );
     



    d3.selectAll("div").attr("index", function(d, i) { return i; });

    
}

function ptcolor(x) {
 // if (f) return "red";
  if (x % 2 == 0) return "steelblue"; else return "cyan";
}

function even_odd(x) {
  if (x % 2 == 0) return "even"; else return "odd";
}
    </script>
  </body>
</html>


';

close(D3OUT);
########### D3 OUTPUT END ######################
}

close(OUT);
}


print STDERR " comparison results written to file $outfile\n\n";

exit(0);

# take in tab delimited files for 2 protein prophet outputs, and compile 3 lists:
# output1, output2, and both

my $first = $ARGV[0];
my $second = $ARGV[1];
my %first_prots = %{extractProts($first)};
my %second_prots = %{extractProts($second)};

my @firsts = ();
my @seconds = ();
my @boths = ();

foreach(keys %first_prots) {
    if(exists $second_prots{$_}) {
	push(@boths, $_);
    }
    else {
	push(@firsts, $_);
    }
}
foreach(keys %second_prots) {
    if(! exists $first_prots{$_}) {
	push(@seconds, $_);
    }
}

printf "only in $first: (%d)\n", scalar @firsts;
foreach(@firsts) {
    print "$_\n";
}
print "\n\n";
printf "only in $second: (%d)\n", scalar @seconds;
foreach(@seconds) {
    print "$_\n";
}
print "\n\n";
printf "in both $first and $second: (%d)\n", scalar @boths;
foreach(@boths) {
    print "$_\n";
}
print "\n\n\n";

sub getFileCombo {
(my $inds, my $fileptr) = @_;
my $output = '';
die "have ", scalar @{$inds}, " inds and ", scalar @{$fileptr}, " files\n" if(@{$inds} != @{$fileptr});
for(my $k = 0; $k < @{$inds}; $k++) {
    #print "${$inds}[$k] for ${$fileptr}[$k]...\n";
    if(${$inds}[$k]) {
	$output .= ' ' if(! ($output eq ''));
	$output .= ${$fileptr}[$k];
    }
} # next index
return $output;
}

sub getFiles {
(my $inds, my $fileptr) = @_;
my @output = ();
die "have ", scalar @{$inds}, " inds and ", scalar @{$fileptr}, " files\n" if(@{$inds} != @{$fileptr});
for(my $k = 0; $k < @{$inds}; $k++) {
    #print "${$inds}[$k] for ${$fileptr}[$k]...\n";
    if(${$inds}[$k]) {
	push(@output, ${$fileptr}[$k]);
    }
} # next index
return \@output;
}


sub increment {
(my $init, my $index) = @_;
my $factor;
my $check = 1;
for(my $k = 0; $k < @{$init}; $k++) {
    $check *= 2;
}
return 0 if($index > $check - 1);

for(my $k = 0; $k < @{$init}; $k++) {
    $factor = 1;
    for(my $j = 0; $j < $k; $j++) {
	$factor *= 2;
    }
    if($index % $factor == 0) { # switch
	if(${$init}[$k] == 1) {
	    ${$init}[$k] = 0;
	}
	else {
	    ${$init}[$k] = 1;
	}
    }
} # next dataset
return 1;
}

sub extractProtGroups {
(my $file) = @_;
my %output = ();
open(FILE, $file) or die "cannot open $file $! \n";
# get annotation line for protein name
my @parsed;
my $prot_index = -1;
my $entry_index = -1;
my $protProb_index = -1;
my $protGrpProb_index = -1;
my $first = 1;

while(<FILE>) {
    chomp();
    @parsed = split("\t");
    if($first) {
	if(! /^\s*[a-z,A-Z,\',\"]/ || ! /[a-z,A-Z,\',\"]\s*$/) {
	    print STDERR " Warning: Make sure $file is a tab delimited file, NOT a real Excel file\n\n";
	    exit(1);
	}
	
	for(my $k = 0; $k < @parsed; $k++) {
	    if ($parsed[$k] eq 'entry no.') {
		$entry_index = $k;
	    }
	    if ($parsed[$k] eq 'protein probability') {
		$protProb_index = $k;
	    }
	    if ($parsed[$k] eq 'group probability') {
		$protGrpProb_index = $k;
	    }
	    if($parsed[$k] eq 'protein') {
		$prot_index = $k;
		#$k = @parsed;
	    }
	    else {
		for(my $i = 0; $i < @info; $i++) {
		    if($parsed[$k] eq $info[$i]) {
			#print "info $info[$i] for $file: $k\n";
			${$info{$info[$i]}}{$file} = $k;
		    }
		}
	    }
	}
	$first = 0;
	if($prot_index == -1) {
	    die "could not find column for protein name in $file\n";
	}
    }
    elsif ($parsed[$protGrpProb_index]>$minProb) {
	my @arr = split "\t", $_;
	my @grp = split /[a-z]/, $arr[$entry_index];
	$output{$parsed[$prot_index]} = $grp[0]; # store it
	#print "adding $parsed[$prot_index]...\n";
	#$all_prots{$parsed[$prot_index]}++;
    }
	
} # next entry
close(FILE);
return \%output;
}


sub extractProts {
(my $file) = @_;
my %output = ();
open(FILE, $file) or die "cannot open $file $! \n";
# get annotation line for protein name
my @parsed;
my $prot_index = -1;
my $entry_index = -1;
my $protProb_index = -1;
my $protGrpProb_index = -1;
my $first = 1;

while(<FILE>) {
    chomp();
    @parsed = split("\t");
    if($first) {
	if(! /^\s*[a-z,A-Z,\',\"]/ || ! /[a-z,A-Z,\',\"]\s*$/) {
	    print STDERR " Warning: Make sure $file is a tab delimited file, NOT a real Excel file\n\n";
	    exit(1);
	}
	
	for(my $k = 0; $k < @parsed; $k++) {
	    if ($parsed[$k] eq 'entry no.') {
		$entry_index = $k;
	    }
	    if ($parsed[$k] eq 'protein probability') {
		$protProb_index = $k;
	    }
	    if ($parsed[$k] eq 'group probability') {
		$protGrpProb_index = $k;
	    }
	    if($parsed[$k] eq 'protein') {
		$prot_index = $k;
		#$k = @parsed;
	    }
	    else {
		for(my $i = 0; $i < @info; $i++) {
		    if($parsed[$k] eq $info[$i]) {
			#print "info $info[$i] for $file: $k\n";
			${$info{$info[$i]}}{$file} = $k;
		    }
		}
	    }
	}
	$first = 0;
	if($prot_index == -1) {
	    die "could not find column for protein name in $file\n";
	}
    }
    elsif ($parsed[$protGrpProb_index]>$minProb) {
	my @arr = split "\t", $_;
	my @grp = split /[a-z]/, $arr[$entry_index];
	$output{$parsed[$prot_index]} = $_; # store it
	#print "adding $parsed[$prot_index]...\n";
	$all_prots{$parsed[$prot_index]}++;
    }
	
} # next entry
close(FILE);
return \%output;
}
