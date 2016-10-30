/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

/*
 * ShowXIC by David Shteynberg (c) Institute for Systems Biology, 12/30/2013
 *
 * Purpose:  Program to plot Extracted Ion Currents
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common/constants.h"
#include "common/util.h"
#include "Visualization/ShowXIC/ShowXIC.h"
#include "common/TPPVersion.h"

#define PROTON_MASS 1.00727646688


using namespace std;



  ShowXIC::ShowXIC(string* file, double mz) {
    file_ = new string(*file);
    mz_ = mz;
  }

  
  Array<double>* ShowXIC::extractXIC() {

    Array<double>* xic  = new Array<double>();
    
    using namespace std;
    using namespace pwiz::msdata;
    
    int i=0;
    int j=0;
    int k=0;
     string run;
    MSDataFile* msd;
    SpectrumListPtr sl;// = *msd.run.spectrumListPtr;
    SpectrumPtr s;
    
    try {
      msd = new MSDataFile(*file_);
    }
    catch (...) {
      cerr << "WARNING: Unable to open file: " << *file_ << endl;
      delete xic;
      return NULL;
    }

    run = *file_;
   
    if (!msd->run.spectrumListPtr.get())
      throw runtime_error("[trackPeptidesXICs] Null spectrumListPtr.");

    
    sl = msd->run.spectrumListPtr;
    
    cerr << "Parsing mzML File: " << *file_  << endl;

    int step = (int)sl->size() / 100;
    int tot = 0;


    for (j=1; j<(int)sl->size(); j++) {
      
      s = sl->spectrum(j, true);

      SpectrumInfo info;
      
      info.update(*s, true);
      
      if (info.msLevel != 1) {
	continue;
      }


      double rt = info.retentionTime;
      double intens = 0;

      std::vector< MZIntensityPair >  data =  info.data;	
      
      std::sort(data.begin(), data.end(), compare_mz);


      size_t id = 0;
      size_t last_id = 0;
      size_t left = 0;
      size_t right = info.dataSize ? info.dataSize-1 : 0;
      double got_mz = -1;
      double look_mz = -1;
     
	  intens = 0;
	
	  look_mz = mz_;
	  id = 0;
	  left = 0;
	  right  = info.dataSize ? info.dataSize-1 : 0;

	  id = (right+left)/2;

	  //Binary Search
	  while (1) {
	    
	    if (right <= left) {
	      if (fabs(look_mz-data[id].mz) < 0.05 && data[id].intensity > intens) {
		intens = data[id].intensity;
		got_mz = data[id].mz;
	      }
	      break;
	    }
	    else if (fabs(look_mz-data[id].mz) < 0.05 && data[id].intensity > intens) {
	      intens = data[id].intensity;
	      got_mz = data[id].mz;
	      break;
	    }
	    else if (look_mz < data[id].mz) {
	      right = id;
	    }
	    else {
	      left = id;
	    }

	    last_id = id;
	    id = (right+left)/2;
	    
	    if (last_id == id) {
	      id = right;
	      right = left;
	    }
	    
	  }
	  
	  double ppm = got_mz > -1 ? 1e6*(got_mz-look_mz)/look_mz : 0;

	  xic->insertAtEnd(rt);
	  xic->insertAtEnd(intens);
	  xic->insertAtEnd(ppm);
	  
	 

  
    }
    xic_ = xic;
    return xic;
  }
  
  void ShowXIC::printXICd3(ostream& out) {

   
    using namespace std;
    string szWebRoot = "/ISB/";

    out << " <title>Extracted Ion Current Viewer</title> " << endl;
    out << " <script type='text/javascript' src='" << szWebRoot << "/html/js/d3.js'></script> " << endl;
out << "  " << endl;
out << " <style type=\"text/css\">  " << endl;
out << " a:link {text-decoration:none;}    /* unvisited link */ " << endl;
out << " a:visited {text-decoration:none;} /* visited link */ " << endl;
out << " a:hover {text-decoration:underline;}   /* mouse over link */ " << endl;
out << " a:active {text-decoration:underline;}  /* selected link */ " << endl;
out << "  " << endl;
out << " svg { " << endl;
out << "   font: 10px sans-serif; " << endl;
out << "   shape-rendering: crispEdges; " << endl;
out << "    background-color: #bcbcbc;	" << endl;
out << " } " << endl;
out << "  " << endl;
out << " .help { " << endl;
out << "   font: 10px sans-serif; " << endl;
out << "   shape-rendering: crispEdges; " << endl;
out << "   color: #DD0000; " << endl;
out << " } " << endl;
out << "  " << endl;
out << " path.dot { " << endl;
out << "   fill: white; " << endl;
out << "   stroke-width: 1.5px; " << endl;
out << " } " << endl;
out << "  " << endl;
out << " td      { " << endl;
out << "     font-family: Helvetica, Arial, sans-serif;  " << endl;
out << "     font-size: 9pt;  " << endl;
out << "     vertical-align: top " << endl;
out << "      " << endl;
out << " } " << endl;
out << "  " << endl;
out << "  " << endl;
out << " button { " << endl;
out << "   left: 275px; " << endl;
out << "   position: absolute; " << endl;
out << "   top: 145px; " << endl;
out << "   width: 80px; " << endl;
out << " } " << endl;
out << "  " << endl;
out << "  " << endl;
out << " #TableDiv { " << endl;
out << "             display: table-cell;  " << endl;
out << "             border: 2px solid black; " << endl;
out << "             margin: 0; " << endl;
out << "             } " << endl;
out << "  " << endl;
out << " .data { " << endl;
out << "         /* borders for the whole table */ " << endl;
out << "         table-layout: auto; " << endl;
out << "         /*border: none;*/ " << endl;
out << "         background: #aaaaaa; " << endl;
out << "         border-collapse: collapse; " << endl;
out << "         border: 1px solid black; " << endl;
out << " 	display: table-cell;  " << endl;
out << "         margin: 0; " << endl;
out << "       } " << endl;
out << " td { " << endl;
out << " 	border-left: 2px solid white; " << endl;
out << " 	border-right: 2px solid white; " << endl;
out << " 	width: 33%; " << endl;
out << "   } " << endl;
out << "  " << endl;
out << " td.ratio { " << endl;
out << "         width: 10%; " << endl;
out << "   } " << endl;
out << "  " << endl;

 out << " .divLoad {  " << endl;
 out << "       width: 200px; " << endl; 
 out << "        height: 100px;  " << endl;
 out << "        margin: 0px auto; " << endl; 
 out << "    } " << endl; 

 out << " td.link  { " << endl;
 out << "         width: 20%; " << endl;
 out << "   } " << endl;
 out << "  " << endl;
 out << "  " << endl;
 out << " td.desc { " << endl;
 out << "         width: 70%; " << endl;
 out << "    " << endl;
 out << "   } " << endl;
 out << "  " << endl;
 out << "  " << endl;
 out << " tr.even { " << endl;
 out << "                 background: #DDE0FF; " << endl;
 out << " 		border: 1px solid steelblue; " << endl;
 out << "         } " << endl;
 out << "  " << endl;
 out << "  " << endl;
 out << "  " << endl;
 out << " tr.odd { " << endl;
 out << "                background: #eeeeee; " << endl;
 out << " 	       border: 1px solid cyan; " << endl;
 out << " 	       	       " << endl;
 out << "        } " << endl;
 out << "  " << endl;
 out << "  " << endl;
 out << "  " << endl;
 out << " table.top { " << endl;
 out << "                 background: #FFE000; " << endl;
 out << " 		border: 1px solid red; " << endl;
 out << " 		width: 100%; " << endl;
 out << " 		text-align: left; " << endl;
 out << "         } " << endl;
 out << "  " << endl;
 out << " table.spacer { " << endl;
 out << "                 background: lightgray; " << endl;
 out << " 		border: 1px solid lightgray; " << endl;
 out << " 		width: 100%; " << endl;
 out << " 		text-align: left; " << endl;
 out << "         } " << endl;
 out << "  " << endl;
 out << "  " << endl;
 out << " table.even { " << endl;
 out << "                 background: #DDE0FF; " << endl;
 out << " 		border: 1px solid steelblue; " << endl;
 out << " 		width: 100%; " << endl;
 out << " 	         " << endl;
 out << "         } " << endl;
 out << "  " << endl;
 out << " path.line { " << endl;
 out << "   fill: none; " << endl;
 out << "   stroke: #000000; " << endl;
 out << "   stroke-width: 1px; " << endl;
 out << " } " << endl;
 out << "  " << endl;
 out << " table.odd { " << endl;
 out << "                background: #eeeeee; " << endl;
 out << " 	       border: 1px solid cyan; " << endl;
 out << " 	       width: 100%; " << endl;
 out << " 	       text-align: left; " << endl;
 out << " 	   } " << endl;
 out << "   " << endl;
 out << "  body { " << endl;
 out << "          background-color: #f0f0f0; " << endl;
 out << "  " << endl;
 out << " 	} " << endl;
 out << " .spacer { " << endl;
 out << "   height: 30px; " << endl;
 out << "   width: 10px; " << endl;
 out << "   fill: none; " << endl;
 out << "   border: 1px solid; " << endl;
 out << "   display: inline; " << endl;
 out << " } " << endl;
 out << "  " << endl;
 out << "     </style> " << endl;
 out << "   </head> " << endl;
 out << "   <body> " << endl;



 out << "      <div id=\"divLoad\" class=\"divLoad\"> " << endl; 
 out << "         Please wait while the XIC data is extracted ... if the page finished loading and your don't see the XIC please check the mzML file !!! " << endl;
 out << "     </div> " << endl;


 out << " <script type=\"text/javascript\"> " << endl;



 out << " document.getElementById(\"divLoad\").style.visibility = \"visible\" " << endl; 


 out << "var xic = [" << endl;
 for (int i=0; i< xic_->size(); i+=3) {
    
      out << "{time: " << (*xic_)[i];
      out << ", intensity: " << (*xic_)[i+1];
      out << ", ppm: " << (*xic_)[i+2] << "}";
      if (i < xic_->size()-3) {
	out << ", " << endl;
      }
    }
 out << "];" << endl;
out << "  var size = 1000; " << endl;
out << "   var pad = 200; " << endl;
out << "   var w = 1; " << endl;
out << "  " << endl;
out << "  " << endl;
out << "   var maxTime = d3.max(xic.map(function(d) { return d.time; } ) ); " << endl;
out << "   var minTime = d3.min(xic.map(function(d) { return d.time; } ) ); " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << "   var maxInt = d3.max(xic.map(function(d) { return d.intensity; } ) ); " << endl;
out << "   var minInt = d3.min(xic.map(function(d) { return d.intensity; } ) ); " << endl;
out << "  " << endl;
out << "  " << endl;
out << "   var maxPPM = d3.max(xic.map(function(d) { return Math.abs(d.ppm); } ) ); " << endl;
out << "   var minPPM = d3.min(xic.map(function(d) { return Math.abs(d.ppm); } ) ); " << endl;
out << "  " << endl;
out << "   var colorLow = \"steelblue\", colorMed = \"lavender\", colorHigh = \"salmon\"; " << endl;
out << "  " << endl;
out << "   if (minPPM <= 0) { " << endl;
out << "           minPPM = 0.01  ; " << endl;
out << "   }	   " << endl;
out << "  " << endl;
out << "   var colorScale = d3.scale.log() " << endl;
out << "                      .domain([minPPM, (maxPPM+minPPM)/2, maxPPM]) " << endl;
out << " 		     .range([colorLow, colorMed, colorHigh]); " << endl;
out << "  " << endl;
out << "  " << endl;
out << "   var w = xic.length * 2; " << endl;
out << "    " << endl;
out << "   w = 1200; " << endl;
out << "  " << endl;
out << "   var size = [w, 600], // width height " << endl;
out << "     padding = [4, 4, 20, 40]; // top right bottom left " << endl;
out << "     tx = function(d) { return \"translate(\" + x(d) + \",0)\"; }, " << endl;
out << "     ty = function(d) { return \"translate(0,\" + y(d) + \")\"; }, " << endl;
out << "     stroke = function(d) { return d ? \"#ccc\" : \"#666\"; }; " << endl;
out << "  " << endl;
out << "    var scale = 1; " << endl;
out << "    var prevscale = -1; " << endl;
out << "    var transl = [0,0]; " << endl;
out << "     " << endl;
out << "    var lastXY = []; " << endl;
out << "    var plotXY = []; " << endl;
out << "    var allXY = []; " << endl;
out << "  " << endl;
out << "    xic.forEach(function(d,i,a) {  " << endl;
out << " 			//console.log(\"time(secs)=\"+d.time+\", Intensity=\"+d.intensity+\", PPM=\"+d.ppm); " << endl;
out << " 			if (d.ppm != 0) { " << endl;
out << " 			   allXY.push({ \"x\": d.time, \"y\": d.intensity, \"ppm\": d.ppm }); " << endl;
out << " 			} " << endl;
out << " 			else { " << endl;
out << " 			   allXY.push({ \"x\": d.time, \"y\": d.intensity, \"ppm\": 1000000 }); " << endl;
out << " 			} " << endl;
out << "  " << endl;
out << " 				  } ); " << endl;
out << "  " << endl;
out << "   var x = d3.scale.linear() " << endl;
out << "     .domain([minTime, maxTime]) " << endl;
out << "     .range([0, size[0]]); " << endl;
out << "  " << endl;
out << "  " << endl;
out << "   var y = d3.scale.linear() " << endl;
out << "     .domain([minInt, maxInt]) " << endl;
out << "     .range([size[1], 0]); " << endl;
out << "  " << endl;
out << "  " << endl;
out << "   var svg =  d3.select(\"body\").append(\"svg\") " << endl;
out << "     .attr(\"width\", size[0] + padding[3] + padding[1]) " << endl;
out << "     .attr(\"height\", size[1] + padding[0] + padding[2]) " << endl;
out << "     .attr(\"pointer-events\", \"all\") " << endl;
out << "   .append(\"g\") " << endl;
out << "     .attr(\"transform\", \"translate(\" + padding[3] + \",\" + padding[0] + \")\") " << endl;
out << "     .call(d3.behavior.zoom() " << endl;
out << "       .extent([[0, size[0]], [0, size[1]], [0, Infinity]]) " << endl;
out << "       .on(\"zoom\", function() {  " << endl;
out << "                                 prevscale = scale; scale = d3.event.scale; transl = d3.event.translate; " << endl;
out << "                                 recalc_plotXY(); " << endl;
out << " 				redraw(); " << endl;
out << " 			      })) " << endl;
out << "     .append(\"g\"); " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << "      " << endl;
out << "     lastXY = allXY; " << endl;
out << "     plotXY = allXY; //to start " << endl;
out << "  " << endl;
out << "  " << endl;
out << " var xfocus = []; " << endl;
out << " var div; " << endl;
out << " var last; " << endl;
out << " var trace = d3.svg.area() " << endl;
out << "                   .x(function(d) { return x(d.x); }) " << endl;
out << " 		  .y(function(d) { return y(d.y); }); " << endl;
out << "  " << endl;
out << "  " << endl;
out << " var barWidth =  size[0] / plotXY.length; " << endl;
out << "    " << endl;
out << "  if (barWidth < 1) { " << endl;
out << "   barWidth = 1; " << endl;
out << "  } " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << " // Generate x-ticks " << endl;
out << " var gx = svg.selectAll(\"g.x\") " << endl;
out << "     .data(x.ticks(10)) " << endl;
out << "     .enter().append(\"g\") " << endl;
out << "     .attr(\"class\", \"x\") " << endl;
out << "     .attr(\"transform\", tx); " << endl;
out << "  " << endl;
out << " gx.append(\"line\") " << endl;
out << "     .attr(\"stroke\", stroke) " << endl;
out << "     .attr(\"y1\", 0) " << endl;
out << "     .attr(\"y2\", size[1]); " << endl;
out << "  " << endl;
out << " gx.append(\"text\") " << endl;
out << "     .attr(\"y\", size[1]) " << endl;
out << "     .attr(\"dy\", \"1em\") " << endl;
out << "     .attr(\"text-anchor\", \"middle\") " << endl;
out << "     .text(x.tickFormat(10)); " << endl;
out << "  " << endl;
out << " // Generate y-ticks " << endl;
out << " var gy = svg.selectAll(\"g.y\") " << endl;
out << "     .data(y.ticks(10)) " << endl;
out << "   .enter().append(\"g\") " << endl;
out << "     .attr(\"class\", \"y\") " << endl;
out << "     .attr(\"transform\", ty); " << endl;
out << "  " << endl;
out << " gy.append(\"line\") " << endl;
out << "     .attr(\"stroke\", stroke) " << endl;
out << "     .attr(\"x1\", 0) " << endl;
out << "     .attr(\"x2\", size[0]); " << endl;
out << "  " << endl;
out << " gy.append(\"text\") " << endl;
out << "     .attr(\"x\", -3) " << endl;
out << "     .attr(\"dy\", \".35em\") " << endl;
out << "     .attr(\"text-anchor\", \"end\") " << endl;
out << "     .text(y.tickFormat(10)); " << endl;
out << "  " << endl;
out << "  " << endl;
out << " svg.append(\"rect\") " << endl;
out << "     .attr(\"width\", size[0]) " << endl;
out << "     .attr(\"height\", size[1]) " << endl;
out << "     .attr(\"stroke\", stroke) " << endl;
out << "     .attr(\"fill\", \"none\"); " << endl;
out << "  " << endl;
out << "  " << endl;
out << " var dots = svg.selectAll(\"circle\") " << endl;
out << "     .data(plotXY) " << endl;
out << "     .enter().append(\"circle\") " << endl;
out << "     .attr(\"class\", \"dot\") " << endl;
out << "     .attr(\"stroke\", function(d) {return colorScale(Math.abs(d.ppm)); } ) " << endl;
out << "     .attr(\"fill\", function(d) {return \"none\"; } ) " << endl;
out << "     .attr(\"transform\", function(d) { //console.log(\"translate(\" + x(d.x) + \",\" + y(d.y) + \")\"); " << endl;
out << "                                    return \"translate(\" + x(d.x) + \",\" + y(d.y) + \")\"; " << endl;
out << " 				    }) " << endl;
out << "     .attr(\"r\", function(d) { if ( Math.abs(d.ppm) < 100) return Math.abs(d.ppm)/5+1; return 0;} ) " << endl;
out << "     .append(\"title\").text(function(d) {  return \"Time = \"+d.x+\", Intensity = \"+d.y+\", PPM = \"+d.ppm; }); " << endl;
out << "  " << endl;
out << "  " << endl;
out << " var bar = svg.selectAll(\"g\") " << endl;
out << "       .data(plotXY) " << endl;
out << "     .enter().append(\"g\") " << endl;
out << "       .attr(\"transform\", function(d, i) {  " << endl;
out << "                                           return \"translate(\" + x(d.x) + \",\" + y(d.y) + \")\"; " << endl;
out << " 					  }) " << endl;
out << "   bar.append(\"rect\") " << endl;
out << "       .attr(\"width\", barWidth) " << endl;
out << "       .attr(\"height\",  function(d) { return size[1] - y(d.y); }) " << endl;
out << "       .attr(\"stroke\", function(d) {return colorScale(Math.abs(d.ppm)); } ) " << endl;
out << "       .attr(\"fill\", \"none\" ); " << endl;
out << "  " << endl;
out << " var line = d3.svg.line() " << endl;
out << "              .interpolate(\"basis\") " << endl;
out << "              .x(function(d) { return x(d.x); }) " << endl;
out << " 	     .y(function(d) { return y(d.y); }); " << endl;
out << "              " << endl;
out << "     // Add the line path. " << endl;
out << "   svg.append(\"path\") " << endl;
out << "       .attr(\"class\", \"line\") " << endl;
out << "       .attr(\"d\", line(plotXY)); " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << " var help = size; " << endl;
out << " help[0] = 10; " << endl;
out << " help[1] = help[1]-help[0]; " << endl;
out << "  " << endl;
out << " var hp1 = d3.select(\"body\").append(\"p\").append(\"text\").attr(\"class\",\"help\") " << endl;
out << "   //  .style(\"font-size\", \"12px\") " << endl;
out << "     .text(\"* DoubleClick to Zoom then Drag to Pan\"); " << endl;
out << "  " << endl;
out << " hp1.append(\"p\").append(\"text\").attr(\"class\",\"help\") " << endl;
out << " //    .style(\"font\", \"1px\") " << endl;
out << "     .text(\"* Hold Shift and DoubleClick to Unzoom\"); " << endl;
out << "  " << endl;
out << " hp1.append(\"hr\"); " << endl;
out << "  " << endl;
out << " recalc_plotXY(); " << endl;
out << " redraw(); " << endl;
out << "  " << endl;
out << " function mouseOut() { " << endl;
out << "  d3.selectAll(\"circle\").data(allXY).transition().duration(100).attr(\"stroke\", function(dd) {return ptcolor(dd.x); } ).attr(\"r\", 5).attr(\"fill\", \"none\"); " << endl;
out << "  recalc_plotXY(); " << endl;
out << "  redraw(); " << endl;
out << " } " << endl;
out << "  " << endl;
out << " function hover() { " << endl;
out << "  d3.selectAll(\"circle\").data(allXY).transition().duration(100).attr(\"stroke\", \"lightgray\" ).attr(\"r\", 3).attr(\"fill\", \"none\"); " << endl;
out << " } " << endl;
out << "  " << endl;
out << " function  refresh() { " << endl;
out << "    if (d3.event) { " << endl;
out << "       d3.event=null; " << endl;
out << "    }    " << endl;
out << "    prevscale = scale; " << endl;
out << "    scale = 1; " << endl;
out << "    transl = [0,0]; " << endl;
out << "    recalc_plotXY(); " << endl;
out << "    redraw(); " << endl;
out << "  " << endl;
out << " }     " << endl;
out << "  " << endl;
out << " function recalc_plotXY() { " << endl;
out << "    var Tdata = []; " << endl;
out << "     svg.attr(\"transform\", " << endl;
out << "       \"translate(\" + transl + \")\" " << endl;
out << "            + \"scale(\" +	scale + \")\");   " << endl;
out << "    if (scale <= 0) scale = 1; " << endl;
out << "  " << endl;
out << "    if (prevscale != scale || transl[0] != ptransl[0] || transl[1] != ptransl[1]  ) {  " << endl;
out << "  " << endl;
out << "                       allXY.forEach(function(d,i,a) {  " << endl;
out << " 		         	//console.log(\"time(secs)=\"+d.time+\", Intensity=\"+d.intensity+\", PPM=\"+d.ppm); " << endl;
out << " 				 var tX = x(d.x)*scale+transl[0]; " << endl;
out << " 				 var tY = y(d.y)*scale+transl[1]; " << endl;
out << " 				 if (tX >= 0 && tY >= 0 ) {		  " << endl;
out << " 					Tdata.push(d); " << endl;
out << " 				  } " << endl;
out << " 		            }); " << endl;
out << " 			    lastXY = plotXY; " << endl;
out << " 			    plotXY = Tdata; " << endl;
out << "    } " << endl;
out << "    else { " << endl;
out << " 	 plotXY = lastXY;; " << endl;
out << "    } " << endl;
out << "    prevscale = scale; " << endl;
out << "    ptransl = transl; " << endl;
out << " } " << endl;
out << "  " << endl;
out << "  " << endl;
out << "  " << endl;
out << " function redraw() { " << endl;
out << "  " << endl;
out << "    " << endl;
out << "  " << endl;
out << "  " << endl;
out << " div = d3.select(\"body\").selectAll(\"div\").data([], function(d) {  " << endl;
out << "                                                                     return d;  " << endl;
out << " 								   }).exit().remove(); " << endl;
out << "  div = d3.select(\"body\").selectAll(\"div\") " << endl;
out << "    .data(xfocus, function(d) { return d.x; } ) " << endl;
out << "    .enter().append(\"div\").style(\"position\",\"relative\") " << endl;
out << "    .style(\"left\", window.scrollLeft).style(\"bottom\", \"5px\"); " << endl;
out << " document.getElementById(\"divLoad\").style.visibility = \"hidden\" " << endl; 
out << " } " << endl;
out << "  " << endl;
out << "     </script> " << endl;
out << "   </body> " << endl;
out << " </html> " << endl;
out << "  " << endl;
 

  }


ShowXIC::~ShowXIC() {
    delete file_;
  }




int main(int argc, char **argv)
{
  
 
  string* file;
  double mz;
  ShowXIC* xic;
  int testarg = 0;
  
  bool CGI = false;

  char* envTest = getenv("REQUEST_METHOD");

  string* currName = new string();
  string* currVal =  new string();
  char buf[8192];
  size_t poslast = 0;
  size_t pos1 = 0;
  size_t pos2 = 0;

  if (envTest) {
    CGI = true;
  }


  if (!CGI) {
      //TODO: Add input error handling
    if(argc < 3) {
      cerr <<  argv[0] << " (" << szTPPVersionInfo << ")" << endl;
      cerr << "USAGE: " << argv[0] << " <mzfile> <mz>" << endl;
      exit(1);
    }
    
    //TODO Param passing needs work!!!
    for (int argidx = testarg+1; argidx < argc; argidx++) {
      string arg = argv[argidx];
      
      if (argidx == testarg+1) {
	file = new string(argv[argidx]);
      }
      if (argidx == testarg+2) {
	mz = atof(argv[argidx]);
      }
      
    }
  }
  else {
    envTest = getenv("QUERY_STRING");
   
    if (envTest) {
      
      string* qs = new string(envTest); 
      

      pos1 = qs->find('&', poslast);
      
      while (poslast != string::npos) {

	
	if (poslast) {
	  pos2 = qs->find('=', poslast);
	  *currName = qs->substr(poslast, pos2-(poslast));

	}
	else {
	  pos2 = qs->find('=', 0);
	  *currName = qs->substr(0, pos2);

	}
	if (pos1 != string::npos) {
	  *currVal = qs->substr(pos2+1, pos1-pos2-1);	
	}
	else {
	  *currVal = qs->substr(pos2+1, pos1-pos2-1);	
	}

	strcpy(buf, currVal->c_str());
	plustospace(buf);
	unescape_url(buf);
	*currVal = buf;

	if (*currName == "FILE") {
	  file = new string(*currVal);
	}
	if (*currName == "MZ") {
	  mz = atof(currVal->c_str());	  
	}
       
	poslast = pos1 != string::npos ?  pos1+1 : pos1;
	pos1 = qs->find('&', poslast);


      }

      
    }
    delete currName;
    delete currVal;
  }
  
  cout << "Content-type: text/html" << endl << endl; 


  xic = new ShowXIC(file, mz);
  xic->extractXIC();
  xic->printXICd3(cout);

  delete file;
  delete xic;

}
