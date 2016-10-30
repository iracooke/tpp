Ext.define('AmztppSim.controller.Variables', {
    extend: 'Ext.app.Controller',

    settings : null,
    inputs   : new Array(),
    searches : new Array(),
    outputs  : new Array(),

    views:  [ 'VariablesForm', 'TimeChart',
              'ResultsPanel', 'TimelinePanel', 'ResultsGrid' ],
    stores: [ 'Searches', 'TimeSlices' ],
    models: [ 'Search', 'TimeSlice' ],
    
    refs: [ { ref: 'variables',     selector: 'variableform' },
            { ref: 'resultsPanel',  selector: 'resultspanel' },
            { ref: 'timelinepanel', selector: 'timelinepanel' },
            { ref: 'resultsGrid',  selector: 'resultsgrid' },
            { ref: 'timeChart', selector: 'timechart' }
    ],

    init: function() {
        console.log('Initialized controller');
        this.control( { 
            'variableform' : {
               render: function() { console.debug('rendered') }
            },
            'timechart' : {
               render: function() { console.debug('rendered timechart') }
            },
            'button[action=run]' : {  click: function() { this.run() } }
        } );
    },

    run: function() {
        var f = this.getVariables().getForm();
        if ( !f.isValid() ) {
            console.warn( 'invalid form' );
            return;
        }

        console.debug( 'Running simulation' );
        Ext.getBody().mask( 'Simulation running...' );
        this.getStore( 'Searches' ).removeAll();
        
        this.settings = f.getFieldValues();
    
        this.times    = [];
        this.inputs   = [];
        this.searches = [];
        this.outputs  = [];
        this.results  = [];
        
        // Generate input files
        console.debug( 'mzFileCount = ' + this.settings.mzFileCount );
        console.debug( 'mzFileSize  = ' + this.settings.mzFileSize );
        console.debug( '  parallel  = ' + this.settings.parallel );
        for ( var i = 0; i < this.settings.mzFileCount; i++ ) {
            var mzSize  = this.settings.mzFileSize;
            var pepSize = this.settings.pepFileSize;
            if ( this.settings.randomize ) {
                mzSize = rnd( this.settings.mzFileSize, 5 );
                pepSize = rnd( this.settings.pepFileSize, 5 );
            }
            this.inputs[i] = { num: i + 1, mzSize: mzSize, pepSize: pepSize };
        }
        console.debug( 'Generated ' + this.inputs.length + ' input files' );
        
        this.upTask = Ext.TaskManager.newTask( {
                run: this.uploadCycle,
		interval: 0,
		scope: this,
                onError: function( o, msg ) { console.error( 'Error in upTask: ' + msg ) }
	} );
	
        this.searchTask = Ext.TaskManager.newTask( {
                run: this.searchCycle,
		interval: 0,
                scope: this,
                onError: function( o, msg ) { console.error( 'Error in searchTask: ' + msg ) }
        } );
	
        this.downTask = Ext.TaskManager.newTask( {
                run: this.downCycle,
		interval: 0,
                scope: this,
                onError: function( o, msg ) { console.error( 'Error in downTask: ' + msg ) }
        } );

        this.reportTask = Ext.TaskManager.newTask( {
                run: this.reportCycle,
		interval: 0,
                scope: this,
                onError: function( o, msg ) { console.error( 'Error in reportTask: ' + msg ) }
        } );
        
        console.debug( 'Starting upload task' );
        this.upTask.start();
        return;
        Ext.getBody().unmask();
    },

    uploadCycle: function() {
 //       console.debug( 'Upload cycle' );
        for ( var i = 0; i < this.settings.parallel; i++ ) {
            var f = this.inputs.shift();
            if ( !f ) {
                console.debug( 'Uploading finished' );
                this.upTask.destroy();
                
                console.debug( 'Starting search task' );
                this.times = [];
                this.searchTask.start();
                return;
            }
           
            if ( !this.times[i] )
                this.times[i] = 0;

            var speed = this.settings.localUploadSpeed;
            if ( this.settings.randomize )
                speed = rnd( speed, 1 );

            f.localUploadBeg = this.times[i];
            this.times[i] += (f.mzSize/speed);
            f.localUploadEnd = this.times[i];
            this.searches.push( f );
//            console.debug( f.localUploadBeg + ' -> ' + f.localUploadEnd +
//                           ' uploaded file' );
        }
    },

    searchCycle: function() {
 //       console.debug( 'Search cycle' );
        for ( var i = 0; i < this.settings.ec2Max; i++ ) {
            var s = this.searches.shift();
            if ( !s ) {
                console.debug( 'Searches finished' );
                this.searchTask.destroy();
                
                console.debug( 'Starting download task' );
                this.times = [];
                this.downTask.start();
                return;
            }
           
            if ( !this.times[i] )
                this.times[i] = s.localUploadEnd;
            if ( s.localUploadEnd > this.times[i] )
                this.times[i] = s.localUploadEnd;
                
            var speed = this.settings.amzDownloadSpeed;
            if ( this.settings.randomize )
                speed = rnd( speed, 1 );
            s.amzDownloadBeg = this.times[i];
            this.times[i] += (s.mzSize/speed);
            s.amzDownloadEnd = this.times[i];
                
            speed = this.settings.avgSearchTime * 60;
            if ( this.settings.randomize )
                speed = rnd( speed, 1 );
            s.searchBeg = this.times[i];
            this.times[i] += speed;
            s.searchEnd = this.times[i];
                
            speed = this.settings.amzUploadSpeed;
            if ( this.settings.randomize )
                speed = rnd( speed, 1 );
            s.amzUploadBeg = this.times[i];
            this.times[i] += (s.pepSize/speed);
            s.amzUploadEnd = this.times[i];
                
            this.outputs.push( s );
//            console.debug( s.amzDownloadBeg + ' -> ' + s.amzUploadEnd +
//                           ' searched file' );
        }
    },    
    
    downCycle: function() {
 //       console.debug( 'Download cycle' );

        for ( var i = 0; i < this.settings.parallel; i++ ) {
            var s = this.outputs.shift();
            if ( !s ) {
                console.debug( 'Downloading finished' );
                this.downTask.destroy();
                this.getStore( 'Searches' ).loadData( this.results );

                // Update results panel
                this.getResultsPanel().setEC2Values( this.calcEC2Cost() );
                this.getResultsPanel().setS3Values( this.calcS3Cost() );
                this.getResultsPanel().setSQSValues( this.calcSQSCost() );

                var f = this.getResultsPanel().getForm();
                f.setValues( {
                    ec2Count: this.settings.ec2Max,
                    runTime: Math.round(this.getRuntime()/60*100)/100,
                    upBytes: numberWithCommas( this.getUploadBytes() ),
                    downBytes: numberWithCommas( this.getDownloadBytes() ),
                    totalCost: '$' + this.getTotalCost().toFixed(2)
                } );


                // Update timeline chart
                var t = this.getStore( 'TimeSlices' );
                var d = this.reportCycle();
                var c = this.getTimeChart();
                if ( c ) {
                    c.axes.items[0].maximum = Math.ceil(d[1]/10)*10;
                }
                t.loadData( d[0] );
                Ext.getBody().unmask();
                return;
            }
           
            if ( !this.times[i] )
                this.times[i] = s.amzUploadEnd;

            if ( s.amzUploadEnd > this.times[i] )
                this.times[i] = s.amzUploadEnd;
            s.localDownloadBeg = this.times[i];
            this.times[i] += (s.pepSize/this.settings.localDownloadSpeed);
            s.localDownloadEnd = this.times[i];
            
            this.results.push( s );
//            console.debug( s.localDownloadBeg + ' -> ' + s.localDownloadEnd +
//                           ' downloaded file' );
        }
    },
    
    reportCycle: function() {
    
        // Iterate through results to create a timeline of changes
        var t = [];
        for ( i = 0; i < this.results.length; i++ ) {
           var r = this.results[i];
            
//           console.debug( "i = " + i);
           t0 = Math.floor( r.localUploadBeg );
           if ( typeof t[t0] === "undefined" )
              t[t0] = { pendUp: 0, pendSrv: 0, activeSrv: 0, pendDown: 0 };
           t[t0].pendUp--;
//           console.debug( "t[" + t0 + "] " + "pendUp: " + t[t0].pendUp );
           
           t0 = Math.floor( r.localUploadEnd );
           if ( typeof t[t0] === "undefined" )
              t[t0] = { pendUp: 0, pendSrv: 0, activeSrv: 0, pendDown: 0 };
           t[t0].pendSrv++;
//           console.debug( "t[" + t0 + "] " + "pendSrv: " + t[t0].pendSrv );
           
           t0 = Math.floor( r.amzDownloadBeg );
           if ( typeof t[t0] === "undefined" )
              t[t0] = { pendUp: 0, pendSrv: 0, activeSrv: 0, pendDown: 0 };
           t[t0].pendSrv--;
           t[t0].activeSrv++;
//           console.debug( "t[" + t0 + "] " + "pendSrv: " + t[t0].pendSrv +
//                          " activeSrv:" + t[t0].activeSrv );
           
           t0 = Math.floor( r.amzUploadEnd );
           if ( typeof t[t0] === "undefined" )
              t[t0] = { pendUp: 0, pendSrv: 0, activeSrv: 0, pendDown: 0 };
           t[t0].activeSrv--;
           t[t0].pendDown++;
//           console.debug( "t[" + t0 + "] " + "activeSrv: " + t[t0].activeSrv +
//                          " pendDown:" + t[t0].pendDown );
           
           t0 = Math.floor( r.localDownloadBeg );
           if ( typeof t[t0] === "undefined" )
              t[t0] = { pendUp: 0, pendSrv: 0, activeSrv: 0, pendDown: 0 };
           t[t0].pendDown--;
//           console.debug( "t[" + t0 + "] " + "pendDown: " + t[t0].pendDown );
        }
        
        // Now tally into time slices for charting
        var maxY      = 10;
        var pendUp    = this.results.length;
        var pendSrv   = 0;
        var activeSrv = 0;
        var pendDown  = 0;
        var ts = [ { time: 0, pendUp: pendUp, pendSrv: 0, 
                     activeSrv: 0, pendDown: 0 } ];
        var end = Math.ceil(this.results[this.results.length-1].localDownloadEnd); 
        for ( i = 0; i < end; i++ ) {
           if ( typeof t[i] != "undefined" ) {
               pendUp    += t[i].pendUp;
               pendSrv   += t[i].pendSrv;
               activeSrv += t[i].activeSrv;
               pendDown  += t[i].pendDown;
           
               if ( pendUp > maxY )
                   maxY = pendUp;
               if ( pendSrv > maxY )
                   maxY = pendSrv;
               if ( activeSrv > maxY )
                   maxY = activeSrv;
               if ( pendDown > maxY )
                   maxY = pendDown;
              
 /*          console.debug( "time: " + i + 
                         " pendUp: " + pendUp, 
                         " pendSrv: " + pendSrv,
                         " activeSrv: " + activeSrv,
                         " pendDown: " + pendDown 
                        ); 
                        */
           
           // Add previous values (to make chart series step)
           last = ts[ts.length-1];      
           ts.push( { time: i/60, 
                      pendUp:    last.pendUp, 
                      pendSrv:   last.pendSrv, 
                      activeSrv: last.activeSrv, 
                      pendDown:  last.pendDown
               } );
               
           // Add new values
           ts.push( { time: i/60, 
                      pendUp:    pendUp, 
                      pendSrv:   pendSrv, 
                      activeSrv: activeSrv, 
                      pendDown:  pendDown 
               } );
           }
        }
        return [ ts, maxY ];
    },
 
    timeout: function(i) {
        return function() {
           if ( i >= 10 ) {
               Ext.MessageBox.hide();
           } else {
               Ext.MessageBox.updateProgress( i, Math.round(10*i) + '% completed');
           }
        };
    },

    getRuntime: function() {
        return this.results[this.results.length-1].localDownloadEnd;
    },

    getUploadBytes: function() {
        var s = 0;
        for ( i = 0; i < this.results.length; i++ ) {
            s += this.results[i].mzSize;
        }
        return Math.ceil(s);
    },

    getDownloadBytes: function() {
        var s = 0;
        for ( i = 0; i < this.results.length; i++ ) {
            s += this.results[i].pepSize;
        }
        return Math.ceil(s);
    },

    // EC2 price/hr
    // PublicIP-In   free
    // PublicIP-Out  $0.12/GB
    // Regional-In   $0.01/GB
    // Instances-In  free
    // Instances-Out free
    calcEC2Cost: function() {
        var t  = Math.ceil(this.getRuntime()/(60*60));
        var gb = 0.00000095367;                         // kB to GB

        var c = {}; 
        c.instancesCost = t * this.settings.ec2Max * this.settings.ec2Cost;
        c.publicIPIn    = 0 * 10;                       // nominal amount
        c.publicIPOut   = (0.12 * 10 * gb);             // nominal 10K amount
        c.regionalIn    = (0.01 * 10 * gb);             // nominal 10K amount
        c.instancesIn   = 0;
        c.instancesOut  = 0;
        c.total = c.instancesCost + c.publicIPIn + c.publicIPOut
                + c.regionalIn + c.instancesIn + c.instancesOut;

        this.ec2Cost = c;
        return c;
    },

    // PUT,COPY,POST,or List ($0.01/1,000)
    // GET,all other ($0.01/10,000)
    // Data Transfer In (free)
    // Data Transfer Out ($0.12/GB)
    // Storage (GB/month) $0.095/GB month
    calcS3Cost: function() {
        
        var n  = this.results.length;                           // # files
        var up = this.getUploadBytes() * 0.000976563;           // MB to GB 
        var dn = this.getDownloadBytes() * 0.000976563;         // MB to GB 
        
        var c = {}; 
        c.miscOperCost = (0.01/1000) * n * 5;           // use 5 ops/file
        c.getOperCost  = (0.01/10000) * n * 15;         // use 15 ops/file
        c.dataOutCost  = 0.12 * dn;
        
        // Not accurate as it doen't take in effect the increasing use of S3
        c.storageCost = (up + dn);                      // ... GB
        c.storageCost *= (this.getRuntime()/(60*60));   // ... GB byte hrs
        c.storageCost *= (1/744);                       // ... GB-Months
        c.storageCost *= .095;                          // ... assume Tier 1 
        
        c.total = c.miscOperCost + c.getOperCost + c.dataOutCost + c.storageCost;

        this.s3Cost = c;
        return c;
    },
    
    // Requests $0.01/10,000
    // Data transfer In (free)
    // Data transfer Out ($0.12/GB)
    calcSQSCost: function() {
        var n = this.results.length;                    // # files
        var gb = 0.00000095367;                         // kB to GB
        var c = {}; 

        c.requestsCost = (0.01/10000) * n * 15;         // use 15 per file
        c.dataIn       = (0) * n * 5 * gb;              // use 5KB 
        c.dataOut      = (0.12) * n * 6 * gb;           // use 6KB
        c.total = c.requestsCost + c.dataIn + c.dataOut;

        this.sqsCost = c;
        return c;
    },

    getTotalCost: function() {
       return this.ec2Cost.total + this.s3Cost.total + this.sqsCost.total;
    }
});

//
// See http://www.protonfish.com/random.shtml
//
function rnd_bmt() {
    var x = 0, y = 0, rds, c;

    // Get two random numbers from -1 to 1.
    // If the radius is 0 or greater than 1, throw them out and pick 2 new ones
    // Rejection sampling throws away about 20% of the pairs.
    do {
        x = Math.random()*2-1;
        y = Math.random()*2-1;
        rds = x*x + y*y;
    }
    while (rds == 0 || rds > 1)

    // This magic is the Box-Muller Transform
    c = Math.sqrt(-2*Math.log(rds)/rds);

    // It always creates a pair of numbers. I'll return them in an array.
    // This function is quite efficient so don't be afraid to throw one 
    // away if you don't need both.
    return [x*c, y*c];
}

function rnd(mean, stdev) {
    var v;
    do {
        v = rnd_snd()*stdev+mean;
    } while ( v <= 0 );
    return v;
}

function rnd_snd() {
    return (Math.random()*2-1)+(Math.random()*2-1)+(Math.random()*2-1);
}

function numberWithCommas(x) {
    return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}
