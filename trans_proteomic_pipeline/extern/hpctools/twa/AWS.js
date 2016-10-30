/**
* Program: TPP Web Launcher for Amazon
* Author:  Joe Slagel
*
* Copyright (C) 2010-2013 by Joseph Slagel
* 
* This library is free software; you can redistribute it and/or             
* modify it under the terms of the GNU Lesser General Public                
* License as published by the Free Software Foundation; either              
* version 2.1 of the License, or (at your option) any later version.        
*                                                                           
* This library is distributed in the hope that it will be useful,           
* but WITHOUT ANY WARRANTY; without even the implied warranty of            
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         
* General Public License for more details.                                  
*                                                                           
* You should have received a copy of the GNU Lesser General Public          
* License along with this library; if not, write to the Free Software       
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
* 
* Institute for Systems Biology
* 401 Terry Avenue North
* Seattle, WA  98109  USA
* jslagel@systemsbiology.org
*
* $Id: $
*/


/**
 * @constructor
 * 
 * Constructor for class for interacting with Amazon Web Services.
 */
function AWS() {

   // AWS keys
   this.accessKey = null;
   this.secretKey = null;
   
   // General parameters
   this.guestPassword = 'guest';    // Petunia's default user password
   this.uuid          = '';         // Unique session identifier
   this.installDemo   = true;       // Install demo data on startup

   // EC2 parameters
   this.instanceType = 'm3.large'; // EC2 instance type
   this.zone         = null;        // EC2 availability zone
   this.keyName      = null;        // EC2 Key to use
   this.shutdown     = 8;           // Autoshutdown in (n) hours
   
   // S3 parameters
   this.s3Url         = null;       // S3 URL to sync instance with
   this.s3SyncOnStart = false;      // Sync with S3 on startup
   this.s3SyncOnStop  = false;      // Sync with S3 on shutdown
   
   // EBS parameters
   this.ebsAttach     = true;       // Attach EBS block?
   this.ebsSize       = 100;        // Initial size of EBS block (GiB)
   this.ebsVolId      = null;       // EBS volume ID
   this.ebsZone       = null;       // EBS availability zone
   this.ebsStatus     = null;       // EBS status
   
   // Instance info
   this.imageId       = null;
   this.instanceId    = null;
   this.instanceState = null;
   this.instanceUrl   = null;
   this.instanceDNS   = null;

   // Uncomment the following line in order to generate a uuid for each browser
   // session so users can share AWS accounts and get separate instances.
// this.uuid = getUUID();

   // AWS API
   this.apiVersion = '2011-02-28';
   this.amiOwner   = '178066177892';            // Owner of TPP AMI's
   this.region     = 'us-west-2';               // Default EC2 region
   this.securityGroup = 'TPP-TWA';              // EC2 security group to use

   // Proxy server for ec2 aws requests to get around cross-site
   // restrictions. 
   this.server = 'http://tools.proteomecenter.org/twa'; 

   // Normal endpoint for EC2 would be something like
   // https://ec2.us-west-2.amazonaws.com but instead use
   // just the region and the cgi parameters and let the proxy
   // rewrite rule fill in the rest.  Otherwise we're running
   // an open proxy.
//   this.endpoint   = 'ec2.' + this.region + '.amazonaws.com';
   this.endpoint  = this.region;
   this.endpoint  = this.server + '/aws/' + this.endpoint;

   this.amiList     = [];                       // List of TPP TWA AMIs
   this.keyPairList = [];                       // List of EC2 key pairs
}

/**
 * Initialize using the credentials
 */
AWS.prototype.signIn = function( callback ) {
    console.debug( 'AWS signing in' );

    this.signinCallback = callback;

    // Start a chain of asynchronous requests to sign us in
    this._checkCredentials();
};
 
/**
 * Check the credentials
 */
AWS.prototype._checkCredentials = function() {
    console.debug( 'AWS checking credentials' ); 
    
    if ( !this.accessKey && !this.secretKey )
       throw 'Missing needed credentials for signing in';
       
    var params = {};
    var url    = this.generateSignedURL( 'DescribeRegions', params );
    this.getUrl( url, bind ( this, function( xmlhttp, status, msg ) { 
       console.debug( 'AWS check credentials status = ' + status );
       if ( status != 200 ) {
          this.signinCallback( 'Problem with your AWS credentials', msg );
       } else {
          this._initSecurityGroup();
       }
    } ) );
};

/**
 * Initialize our security group 
 */
AWS.prototype._initSecurityGroup = function() {
    console.debug( 'AWS checking for security group' ); 
    
    var params = { 'Filter.1.Name' : 'group-name',
                   'Filter.1.Value': this.securityGroup
                 };
    var url    = this.generateSignedURL( 'DescribeSecurityGroups', params );
    this.getUrl( url, bind ( this, function( xmlhttp, status, msg ) {

       console.debug('AWS init security group status = ' + status );
       if ( status != 200 ) {
          this.signinCallback( 'Error creating TPP-TWA security group', msg );
          return;
       }

       if ( !xmlhttp.getElementsByTagName('groupName')[0] ) {
          this._createSecurityGroup();
          return;
          }
          
       console.debug( 'AWS TPP-TWA security group already exists' );
       this._getAMIs();
       return;
       
    } ) );
};

/**
 * Create the TPP-TWA security group
 */
AWS.prototype._createSecurityGroup = function() {
    console.debug( 'AWS creating TPP-TWA security group' ); 

    var params = { GroupName        : this.securityGroup,
                   GroupDescription : 'Security group used by the TPP web launcher for Amazon.'
                 };
    var url = this.generateSignedURL( 'CreateSecurityGroup', params );
    this.getUrl( url, bind( this, function( xmlhttp, status, msg ) { 
        
       console.debug( 'TWA create security group status = ' + status ); 
       if ( status != 200 ) {
          this.signinCallback( 'Error creating TPP-TWA security group', msg );
          return;
       }
       this._enableIngress();
       return;
       
    } ) );
};

/**
 * Enable ingress on the TPP-TWA security group
 */
AWS.prototype._enableIngress = function() {
    
    console.debug( 'enabling ingress on TPP-TWA security group' ); 

    var params = { 'GroupName'                  : 'TPP-TWA',
                   'IpPermissions.1.IpProtocol' : 'tcp',
                   'IpPermissions.1.FromPort'   : '80',
                   'IpPermissions.1.ToPort'     : '80',
                   'IpPermissions.1.IpRanges.1.CidrIp' : '0.0.0.0/0',
                   'IpPermissions.2.IpProtocol' : 'tcp',
                   'IpPermissions.2.FromPort'   : '22',
                   'IpPermissions.2.ToPort'     : '22',
                   'IpPermissions.2.IpRanges.1.CidrIp' : '0.0.0.0/0'
                 };
    var url = this.generateSignedURL( 'AuthorizeSecurityGroupIngress', params );
    this.getUrl( url, bind( this, function( xmlhttp, status, msg ) { 
       console.debug( 'TPP-TWA security group engress status = ' + status ); 
       if ( status != 200 ) {
          this.signinCallback( 'Error enabling TPP-TWA security group', msg );
          return;
       }
       this._getAMIs();
       return;
       
    } ) );
};

/**
 *  Lookup list of TWA AMIs 
 */
AWS.prototype._getAMIs = function() {

    console.debug( 'AWS getting list of TPP-TWA ami images' );
        
    // Note: AWS tags can't be used to identify TWA images as they are 
    //       are stored per AWS account
    var params = { 'Owner.1'          : this.amiOwner,
                   'Filter.1.Name'    : 'state',
                   'Filter.1.Value.1' : 'available'
                 };
    var url = this.generateSignedURL( 'DescribeImages', params );
    this.getUrl( url, bind( this, this._getAMIsResponse ) );
};

AWS.prototype._getAMIsResponse = function( xmlhttp, status, msg ) {
    console.debug( 'TPP-TWA get ami status = ' + status ); 
    if ( status != 200 ) {
        this.signinCallback( 'Error getting TWA Amazon Machine Images', msg );
        return;
    }
          
    // Iterate through AMIs
    this.amiList = [];
    var items = xmlhttp.getElementsByTagName('item'); 
    for ( var i = 0; i < items.length; i++ ) {
        if ( !items[i].getElementsByTagName('imageId')[0] )
            continue; // images tagged have additional <item> elements, ignore

        id   = items[i].getElementsByTagName('imageId')[0].childNodes[0].nodeValue;
        desc = items[i].getElementsByTagName('description')[0].childNodes[0].nodeValue;
        name = items[i].getElementsByTagName('name')[0].childNodes[0].nodeValue;
        console.debug( 'TPP-TWA found ami id: ' + id + ' name: ' + name + ' desc: ' + desc );

        // which AMIs work with TWA? FIXME: for now just hardcode
        if ( desc.indexOf('TPP v4.8') !== -1 )
           this.amiList.push( { id: id, name: name, desc: desc } );
        if ( desc.indexOf('TPP v4.7') !== -1 )
           this.amiList.push( { id: id, name: name, desc: desc } );
        if ( desc.indexOf('TPP v4.6 OCCUPY rev 3') !== -1 )
           this.amiList.push( { id: id, name: name, desc: desc } );
        if ( desc.indexOf('TPP v0.0') !== -1 )
           this.amiList.push( { id: id, name: name, desc: desc } );
    }
    this.amiList.sort( function(a,b) { return a.name < b.name } );
    this._getKeyPairs();
    return;
};

/**
 *  Lookup list of EC2 Keys
 */
AWS.prototype._getKeyPairs = function() {

    console.debug( 'AWS getting list of key pairs' );
        
    var params = {};
    var url = this.generateSignedURL( 'DescribeKeyPairs', params );
    this.getUrl( url, bind( this, this._getKeyPairsResponse ) );
};

AWS.prototype._getKeyPairsResponse = function( xmlhttp, status, msg ) {
    console.debug( 'TPP-TWA get key pairs status = ' + status ); 
    if ( status != 200 ) {
        this.signinCallback( 'Error getting EC2 key pairs', msg );
        return;
    }
          
    // Iterate through key pairs
    this.keyPairList = [];
    var items = xmlhttp.getElementsByTagName('item'); 
    for ( var i = 0; i < items.length; i++ ) {
        name  = items[i].getElementsByTagName('keyName')[0].childNodes[0].nodeValue;
        console.debug( 'TPP-TWA found keypair ' + name );
        this.keyPairList[i] = { name: name };
    }
    
    this._getEBSVolume();
    return;
};

/**
 *  Lookup any EBS volume
 */
AWS.prototype._getEBSVolume = function() {

    console.debug( 'AWS looking for a TWA EBS volume' );
        
    var params = { 'Filter.1.Name'    : 'tag:Name',
                   'Filter.1.Value.1' : 'TPP-TWA',
                   'Filter.2.Name'    : 'status',
                   'Filter.2.Value.2' : 'available'
                 };
    if ( this.uuid )
       params['Filter.1.Value.1'] += "-" + this.uuid;
    
    var url = this.generateSignedURL( 'DescribeVolumes', params );
    this.getUrl( url, bind( this, this._getEBSVolumeResponse ) );
};

AWS.prototype._getEBSVolumeResponse = function( xmlhttp, status, msg ) {
    console.debug( 'TPP-TWA get EBS volume status = ' + status ); 
    if ( status != 200 ) {
        this.signinCallback( 'Error looking up EBS volume', msg );
        return;
    }
          
    var items = xmlhttp.getElementsByTagName('volumeSet'); 
    items = items[0].getElementsByTagName('item');
    if ( items.length ) {
        this.ebsVolId  = items[0].getElementsByTagName('volumeId')[0].childNodes[0].nodeValue;
        this.ebsZone   = items[0].getElementsByTagName('availabilityZone')[0].childNodes[0].nodeValue;
        this.ebsStatus = items[0].getElementsByTagName('status')[0].childNodes[0].nodeValue;
        console.debug( 'TPP-TWA found EBS zone' +
                       ' volId: '  + this.ebsVolId +
                       ' zone: '   + this.ebsZone +
                       ' status: ' + this.ebsStatus );
    }
    
    this._getInstanceId();
    return;
};

//FIX: Nice to load zones dynamically
AWS.prototype.getAvailabilityZones = function() {
    return [ 'us-west-2a', 'us-west-2b', 'us-west-2c' ];
};

/**
 * Get any TWA instance
 */
AWS.prototype._getInstanceId = function() {
    console.debug( 'AWS checking for a live instance' );
    this.instanceId = null;

    var tag = 'TPP-TWA';
    if ( this.uuid )
        tag += '-' + this.uuid;

    var params = { 
       'Filter.1.Name'    : 'tag:' + tag,
       'Filter.1.Value.1' : aws.accessKey,
       'Filter.2.Name'    : 'instance-state-code',
       'Filter.2.Value.1' : 16,         // active
       'Filter.2.Value.2' : 0,          // pending
       'Filter.2.Value.3' : 32          // shutting down
       };
    var url  = this.generateSignedURL( 'DescribeInstances', params );
    this.getUrl( url, bind( this, function( xmlhttp, status, msg ) { 
        
       if ( status != 200 ) {
          this.signinCallback( 'Error checking for EC2 instance', msg );
          return;
       }
       
       this.instanceId = elemText( xmlhttp, "instanceId" );
       this.signinCallback();
       return;

    } ) );
};


/**
 * Start a new instance
 */
AWS.prototype.startInstance = function( callback ) {

    if ( this.instanceId && this.instanceState != 16  ) {
        callback( "Error starting instance", 
                  "nInstance ID " + this.instanceId + 
                  " state " + this.instanceState +
                  " is already running." );
        return;
    }

    console.debug( 'startInstance()' );

    this.instanceId    = null;
    this.instanceState = null;
    this.instanceUrl   = null;
    this.instanceDNS   = null;

    // Generate request
    var params = { ImageId  : this.imageId,
                   MinCount : 1,
                   MaxCount : 1,
                   'SecurityGroup.1' : this.securityGroup,
                   InstanceType      : this.instanceType,
                   UserData          : this._bootstrap()
    };
    if ( this.keyName )
       params.KeyName = this.keyName;
    if ( this.ebsAttach && this.ebsVolId )
       this.zone = this.ebsZone;
    if ( this.zone )
       params['Placement.AvailabilityZone'] = this.zone;

    // Send the request
    var url = this.generateSignedURL( 'RunInstances', params );
    this.getUrl( url, bind( this, function( xmlhttp, status, msg ) { 

       console.debug( "_startResponse() status = " + status );
       if ( status != 200 ) {
           callback( "Error starting instance", msg );
       }

       this.instanceId = elemText( xmlhttp, "instanceId" );
       console.debug( "_startResponse() instanceId: " + this.instanceId );
       if ( !this.instanceId ) {
           console.error( "_startResponse(): missing instance id " + msg );
           callback( "Missing EC2 instance ID", msg );
       }

       this._createTag( callback );
    } ) );
};

/**
 * Return a bash script to run on bootup
 */
AWS.prototype._bootstrap = function() {

    var bootstrap = "#!/bin/sh\n";
    bootstrap += "chmod a+rwxt /tmp; cd /tmp\n";

    // ...setup shutdown period
    if ( this.shutdown )
        bootstrap += 'start deadman TIMEOUT=+' + ((this.shutdown * 60) - 1) + "m || true\n";

    // ...set the guest user's password using some escape magic on it
    if ( aws.guestPassword !== "" ) {
       bootstrap += "cd /opt/tpp/users/guest\n";
       bootstrap += "perl -e 'print crypt($ARGV[0],\"isbTPPspc\")' '" +
          aws.guestPassword.replace(/'/,'\'"\'"\'') +
             "' > .password\n";
    }

    // ...write s3cfg file for s3cmd upstart scripts
    if ( this.s3Url ) {
        bootstrap += "\n";
        bootstrap += "echo '# TWA autogenerated'                  > /opt/tpp/users/.s3cfg\n"; 
        bootstrap += "echo '[default]'                           >> /opt/tpp/users/.s3cfg\n"; 
        bootstrap += "echo 'access_key = " + aws.accessKey + "'  >> /opt/tpp/users/.s3cfg\n"; 
        bootstrap += "echo 'bucket_location = us-west-2'         >> /opt/tpp/users/.s3cfg\n"; 
        bootstrap += "echo 'secret_key = " + aws.secretKey + "'  >> /opt/tpp/users/.s3cfg\n"; 
        bootstrap += "echo 'use_https = True'                    >> /opt/tpp/users/.s3cfg\n";
        bootstrap += "echo 'tpp_s3url = s3://" + aws.s3Url + "'  >> /opt/tpp/users/.s3cfg\n";
        bootstrap += "chown www-data:www-data /opt/tpp/users/.s3cfg\n";
        bootstrap += "cp /opt/tpp/users/.s3cfg /opt/tpp/users/guest\n";
        bootstrap += "s3cmd  --config /opt/tpp/users/.s3cfg mb s3://" 
                   + aws.s3Url + " || true\n";
    }

   // ...write amztpp/petunia awssecret file 
//bootstrap += "echo '" + aws.accessKey + "' >  /opt/tpp/users/guest/.awssecret";
//bootstrap += "echo '" + aws.secretKey + "' >> /opt/tpp/users/guest/.awssecret";
//bootstrap += "echo '" + aws.region    + "' >> /opt/tpp/users/guest/.awssecret";
//bootstrap += "echo '" + aws.bucket    + "' >> /opt/tpp/users/guest/.awssecret";
//bootstrap += "chmod 600 /opt/tpp/users/guest/.awssecret\n";
//bootstrap += "chown www-data:www-data /opt/tpp/users/guest/.awssecret\n";

    // ...enable S3 sync on start
    if ( this.s3SyncOnStart && this.s3Url ) {
        bootstrap += "rm -f /etc/init/tpp-s3-get.override\n";
        // ...updating password so it doesn't get stomped
        if ( aws.guestPassword !== "" ) {
           bootstrap += "s3cmd -v -c /opt/tpp/users/.s3cfg put " +
              "/opt/tpp/users/guest/.password " +
              "s3://" + aws.s3Url + "/tppusers/guest/.password || true\n";
        }
    }

    // ...enable S3 sync on shutdown
    if ( this.s3SyncOnStop && this.s3Url ) {
        bootstrap += "rm -f /etc/init/tpp-s3-put.override\n";
    }

    // ...start web server
    // FIX: this sed command was in the AMI setup script but didn't work?!?
    bootstrap += "sed -i '/Alias \\/tppdata/i Alias /ISB /opt/tpp' /etc/apache2/sites-available/*default.conf\n";
//    bootstrap += "perl -pi -e 's#DocumentRoot .*#DocumentRoot /mnt/tppdata#' /etc/apache2/sites-available/default\n";
    bootstrap += 'update-rc.d apache2 enable\n';
    bootstrap += '/etc/init.d/apache2 restart\n';

    // ...add some demo data
    if ( this.installDemo ) {
       bootstrap += "cd /mnt/tppdata/local\n";

       bootstrap += "(wget -q http://s3.amazonaws.com/spctools-twa/DemoData.zip &&";
       bootstrap += " unzip DemoData.zip && ";
       bootstrap += " chown -R www-data:www-data demo &&";
       bootstrap += " rm -f DemoData.zip) &\n";

       bootstrap += "(wget -q http://s3.amazonaws.com/spctools-twa/CourseData-AWS.zip &&";
       bootstrap += " unzip CourseData-AWS.zip && ";
       bootstrap += " chown -R www-data:www-data class &&";
       bootstrap += " rm -f CourseData-AWS.zip) &\n";

/*
       bootstrap += "(wget -q http://s3.amazonaws.com/spctools-twa/CourseData-dbase.zip &&";
       bootstrap += " unzip CourseData-dbase.zip &&";
       bootstrap += " chown -R www-data:www-data dbase &&";
       bootstrap += " rm -f CourseData-dbase.zip) &\n";

       bootstrap += "(wget -q http://s3.amazonaws.com/spctools-twa/CourseData-ProteinProphet.zip &&";
       bootstrap += " unzip CourseData-ProteinProphet.zip &&";
       bootstrap += " chown -R www-data:www-data class &&";
       bootstrap += " rm -f CourseData-ProteinProphet.zip ) &\n";

       bootstrap += "(wget -q http://s3.amazonaws.com/spctools-twa/CourseData-SpectraST.zip &&";
       bootstrap += " unzip CourseData-SpectraST.zip &&";
       bootstrap += " chown -R www-data:www-data class &&";
       bootstrap += " rm -f CourseData-SpectraST.zip ) &\n";

       bootstrap += "(wget -q http://s3.amazonaws.com/spctools-twa/CourseData-Search.zip &&";
       bootstrap += " unzip CourseData-Search.zip &&";
       bootstrap += " mv Search class &&";
       bootstrap += " chown -R www-data:www-data class &&";
       bootstrap += " rm CourseData-Search.zip ) &\n";
*/
    }

    return rstr2b64( bootstrap );
};

/**
* Create the TPP-TWA tag for identifying instances
*/
AWS.prototype._createTag = function( callback ) {

    var params = { 'ResourceId.1' : aws.instanceId,
                   'Tag.1.Key'    : 'TPP-TWA',
                   'Tag.1.Value'  : aws.accessKey
    };
    if ( this.uuid )
        params['Tag.1.Key'] += '-' + this.uuid;

    var url = this.generateSignedURL( 'CreateTags', params );
    this.getUrl( url, function(xmlhttp, status, msg) { 

    console.debug( "_createTag() status = " + status );
        if ( status != 200 ) {
            callback( 'Error tagging instance', msg );
        } else {
            callback();
        }

    } );
};


/**
 * Asynchronous request to update the state of the instance
 */
AWS.prototype.getState = function( callback ) {
    console.debug( "getState() id: " + this.instanceId + ' state: ' + this.instanceState );
    if ( !this.instanceId ) {
       callback( "Attempt to get state of non-instance" );
       return;
    }

    this.getStateCallback = callback;

    var params = { InstanceId : this.instanceId };
    var url = this.generateSignedURL( 'DescribeInstances', params );
    this.getUrl( url, bind( this, this._stateResponse )  );
};

AWS.prototype._stateResponse = function( resp, status, msg ) {

    console.debug( '_stateResponse() status = ' + status );

    this.instanceId    = null;
    this.instanceState = null;
    this.instanceUrl   = null;
    this.instanceDNS   = null;

    if ( status != 200 ) {
        this.getStateCallback( 'Error getting instance state', msg );
        return;
    }

    this.instanceId = elemText( resp, "instanceId" );
    console.debug( "_stateResponse() instanceId: " + this.instanceId );
    if ( !this.instanceId ) {
        console.error( "_stateResponse() no instance Id received from AWS" );
        this.getStateCallback( 'Error getting instance state', 
                               'No instance id reported by AWS' );
        return;
    }

    this.instanceState = elemText( resp, "code" );
    console.debug( "_stateResponse() instanceState: " + this.instanceState );
    if ( this.instanceState === null ) {
        console.error( "_stateResponse() no instance State received from AWS" );
        this.getStateCallback( 'Error getting instance state', 
                               'No instance state reported by AWS' );
    } 

    // Is the instance running?
    if ( this.instanceState == 16 ) {
        var dns = elemText( resp, "dnsName" );
        if ( dns ) {
           this.instanceDNS = dns;
           this.instanceUrl = "http://" + dns + '/tpp/cgi-bin/tpp_gui.pl';
        } else {
           this.getStateCallback( 'Error getting instance state',
                                  'No DNS found for instance ');
           return;
        }
        console.debug( "_stateResponse() url: " + this.instanceUrl );

        if ( this.ebsAttach && this.ebsStatus == 'available') {
            this._attachEBS();
            return;
        }
    }

    this.getStateCallback();
};

/**
 *  Asynchronous request to attach a EBS volume to a instance
 */
AWS.prototype._attachEBS = function() {

    console.debug( "_attachEBS() attaching EBS volume" );
    var params = { 'VolumeId'   : aws.ebsVolId,
                   'InstanceId' : aws.instanceId,
                   'Device'     : '/dev/sdf' 
                 };

    var url = this.generateSignedURL( 'AttachVolume', params );
    this.getUrl( url, bind( this, function( xmlhttp, status, msg ) { 
        console.debug( "attach EBS response status = " + status );
    
        if ( status != 200 ) {
            this.getStateCallback( 'Error attaching instance', msg );
            return;
        }

        this.ebsStatus = elemText( xmlhttp, "status" );
        console.debug( "attach EBS state = " + this.ebsStatus );

        this.getStateCallback();
    } ) );
};

/**
 * Stop the instance
 */
AWS.prototype.stopInstance = function() {
    if ( !this.instanceId ) {
       console.error( "No instance detected to stop" );
       this.instanceState = null;
       return false;
    }

    var params = { InstanceId : this.instanceId };
    var url = this.generateSignedURL( 'TerminateInstances', params );
    this.getUrl( url, bind( this, this._stateResponse )  );
    return true;
};


/**
 * Create a TPP-TWA specific EBS volume
 */
AWS.prototype.createEBSVolume = function( zone, size, callback ) {
    var params = { 'AvailabilityZone' : zone,
                   'Size'             : size
                 };
    
    var url = this.generateSignedURL( 'CreateVolume', params );
    this.getUrl( url, bind( this, function( xmlhttp, status, msg ) { 
        console.debug( "create EBS response: " + status );
        if ( status == 200 ) {
            this.ebsVolId  = xmlhttp.getElementsByTagName('volumeId')[0].childNodes[0].nodeValue;
            this.ebsZone   = xmlhttp.getElementsByTagName('availabilityZone')[0].childNodes[0].nodeValue;
            this.ebsStatus = xmlhttp.getElementsByTagName('status')[0].childNodes[0].nodeValue;
            console.debug( 'Created EBS volume ' +
                           ' volId: '  + this.ebsVolId +
                           ' zone: '   + this.ebsZone +
                           ' status: ' + this.ebsStatus );
           
            // Tag it as ours
            var tag = 'TPP-TWA';
            if ( this.uuid )
               tag += '-' + this.uuid;
            var params = { 'ResourceId.1' : this.ebsVolId,
                              'Tag.1.Key' : 'Name',
                            'Tag.1.Value' : tag
            };
            var url = this.generateSignedURL( 'CreateTags', params );
            this.getUrl( url, function(hdr, status, result) { console.debug( "tagged volume: " + status );} );
        }
        callback( status, msg );
    } ) );
};

/**
 * Deletes a existing EBS volume
 */
AWS.prototype.deleteEBSVolume = function( callback ) {
    
    if ( aws.instanceId ) {
       console.error( "deleteDBSVolume() potentially running instance found" );
       callback( null, "potentially running instance found" );
       return;
    }
    
    if ( !aws.ebsVolId ) {
       console.error( "deleteDBSVolume() no TWA volume to delete" );
       callback( null, "no TWA volume to delete" );
       return;
    }
    
    var params = { 'VolumeId' : aws.ebsVolId };
    
    var url = this.generateSignedURL( 'DeleteVolume', params );
    this.getUrl( url, bind( this, function( xmlhttp, status, msg ) { 
        console.debug( "delete EBS response: " + status );
        if ( status == 200 )
            aws.ebsVolId = null;
        callback( status, msg );
    } ) );
    
};


/**                                                                           */
/********************   Utility Functions   ***********************************/
/**                                                                           */

// only used for sharing accounts -- should be disabled by default
function getUUID()
{
   var uuid = Ext.util.Cookies.get( 'TPP-TWA-uuid' );
   if ( uuid )
      {
      console.debug( 'uuid found in cookie: ' + uuid );
      return uuid;
      }

   uuid = Math.uuid(15);
   console.debug( 'uuid created: ' + uuid );
   Ext.util.Cookies.set( 'TPP-TWA-uuid', uuid, 
      new Date(new Date().getTime() + (1000*60*60*24*7) ) );
   return uuid;
}

// Fix standard Date function
Date.prototype.toISODate =
        new Function("with (this)\n    return " +
           "getFullYear()+'-'+addZero(getMonth()+1)+'-'" +
           "+addZero(getDate())+'T'+addZero(getHours())+':'" +
           "+addZero(getMinutes())+':'+addZero(getSeconds())+'.000Z'");

function generateV1Signature(url, key) {
        var stringToSign = getStringToSign(url);
        var signed =   b64_hmac_sha1(key, stringToSign);
        return signed;
}

function addZero(n) {
    return ( n < 0 || n > 9 ? "" : "0" ) + n;
}

function getNowTimeStamp() {
    var time = new Date();
    var gmtTime = new Date(time.getTime() + (time.getTimezoneOffset() * 60000));
    return gmtTime.toISODate() ;
}

function ignoreCaseSort(a, b) {
    var ret = 0;
    a = a.toLowerCase();
    b = b.toLowerCase();
    if(a > b) ret = 1;
    if(a < b) ret = -1;
    return ret;
}

function getStringToSign(url) {

    var stringToSign = "";
    var query = url.split("?")[1];

    var params = query.split("&");
    params.sort(ignoreCaseSort);
    for (var i = 0; i < params.length; i++) {
        var param = params[i].split("=");
        var name =   param[0];
        var value =  param[1];
        if (name == 'Signature' || undefined  == value) continue;
            stringToSign += name;
            stringToSign += decodeURIComponent(value);
         }

    return stringToSign;
}

/**
 * Sign AWS url request
 */
AWS.prototype.generateSignedURL = function( actionName, params )  {
   var url = this.endpoint + "?SignatureVersion=1" +
             "&Action=" + actionName +
             "&Version=" + encodeURIComponent( this.apiVersion );
           
   console.log( "generateSignedURL() url: " + url );
   
   // Add params
   for ( prop in params )
      {
      if ( !params.hasOwnProperty( prop ) ) continue;
      
      console.log( " param: " + prop + " value: " + params[prop] );
      url += "&" + prop + "=" + encodeURIComponent( params[prop] );
      }
    
   // Add access components
   var timestamp = getNowTimeStamp();
   url += "&Timestamp=" + encodeURIComponent( timestamp );
   url += "&AWSAccessKeyId=" + encodeURIComponent( this.accessKey );
            
   // Sign url
//   console.debug( '   url: ' + url );
   var signature = generateV1Signature( url, this.secretKey );
   url += "&Signature=" + encodeURIComponent( signature ); 
   
//   console.log( "final: " + url );
   return url;
};

//
// Adapted from http://jibbering.com/2002/4/httprequest.html
//
function getXmlHttp() {
  var xmlhttp;
  
  try {
     xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
  } catch (e) {
     try {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
     } catch (E) {
        xmlhttp = false;
     }
  }
  
  if (!xmlhttp && typeof XMLHttpRequest!='undefined') {
    xmlhttp = new XMLHttpRequest();
  }
  
  return xmlhttp;
}

AWS.prototype.getUrl = function ( url, callback )  {
   var xmlhttp = getXmlHttp();
   xmlhttp.open( "GET", url, true );
   xmlhttp.onreadystatechange = function()  {
      console.debug( "getUrl() readyState: " + xmlhttp.readyState );
      if ( xmlhttp.readyState != 4 ) 
         return;
      
      console.debug( "getUrl() status = " + xmlhttp.status );
      var result;
      var regerr = /<Errors>/m;
      if ( xmlhttp.status == 200 && xmlhttp.responseText == 'ERROR: invalid url' ) {
         console.debug( 'getUrl() cross-site proxy failed' ); 
         callback( xmlhttp, -100, xmlhttp.responseText );
         return;
      // TODO: Fix proxy code/xml issue for now brute force regex
//     else if ( xmlhttp.status == 200 && xmlhttp.responseText.match(regerr) )
      } else if ( xmlhttp.responseText.match(regerr) ) {
         console.debug( 'getUrl() AWS error response received ' );
         var xmlDoc;
         if (window.DOMParser) {
            parser=new DOMParser();
            xmlDoc=parser.parseFromString(xmlhttp.responseText,"text/xml");
            try { error = xmlDoc.getElementsByTagName("Message")[0].textContent; } catch (e) {}
         } else  { // Internet Explorer
            console.debug( 'getUrl() other error' );
            xmlDoc=new ActiveXObject("Microsoft.XMLDOM");
            xmlDoc.async="false";
            xmlDoc.loadXML(xmlhttp.responseText);
            try { error = xmlDoc.getElementsByTagName("Message")[0].text; } catch (e) {}
         } 
         console.debug( "getUrl() error = " + error );
         if ( !error )
            error = xmlhttp.responseText;
         callback( xmlhttp, -200, error );
         return;
      } else if ( xmlhttp.status != 200 ) {
         error = "Network Error: (" + xmlhttp.status + ") " + xmlhttp.statusText;
         console.debug( "getUrl() error = " + error );
         callback( xmlhttp, xmlhttp.status, error );
         return;
      }
     
// FIX: if you start a instance, and terminate it outside of twa then
//      start another one then you get a exception 'attempt to start 2nd'
      
//     try { result = xmlhttp.responseXML.getElementsByTagName("Message")[0].textContent; } catch (e) {}
//     if ( result ) {
//        callback( result, -200 );
//        return;
//     }
     
      callback( xmlhttp.responseXML, xmlhttp.status );
   };
   xmlhttp.send( null );
};

// Bind scope of "this" so its accessible in a callback
function bind(scope, fn) { return function () { fn.apply(scope, arguments); }; }

// Replace above with this?  Allows for a cleaner bind syntax
//
//Function.prototype.bind = function(scope) {
//  var _function = this;
  
//  return function() {
//    return _function.apply(scope, arguments);
//  }
//}

// Cross browser XML DOM lookup
function elemText( xml, tag ) {
   var txt = null;
   try { 
      var e = xml.getElementsByTagName(tag)[0];
      if ( !e )
         return null;
      if ( 'textContent' in e )
         txt = e.textContent; 
      else
         txt = e.text; 
   } catch (e) { // do nothing
   }
   return txt;
}

/*!
Math.uuid.js (v1.4)
http://www.broofa.com
mailto:robert@broofa.com

Copyright (c) 2010 Robert Kieffer
Dual licensed under the MIT and GPL licenses.
*/

/*
 * Generate a random uuid.
 *
 * USAGE: Math.uuid(length, radix)
 *   length - the desired number of characters
 *   radix  - the number of allowable values for each character.
 *
 * EXAMPLES:
 *   // No arguments  - returns RFC4122, version 4 ID
 *   >>> Math.uuid()
 *   "92329D39-6F5C-4520-ABFC-AAB64544E172"
 *
 *   // One argument - returns ID of the specified length
 *   >>> Math.uuid(15)     // 15 character ID (default base=62)
 *   "VcydxgltxrVZSTV"
 *
 *   // Two arguments - returns ID of the specified length, and radix. (Radix must be <= 62)
 *   >>> Math.uuid(8, 2)  // 8 character ID (base=2)
 *   "01001010"
 *   >>> Math.uuid(8, 10) // 8 character ID (base=10)
 *   "47473046"
 *   >>> Math.uuid(8, 16) // 8 character ID (base=16)
 *   "098F4D35"
 */
(function() {
  // Private array of chars to use
  var CHARS = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'.split('');

  Math.uuid = function (len, radix) {
    var chars = CHARS, uuid = [], i;
    radix = radix || chars.length;

    if (len) {
      // Compact form
      for (i = 0; i < len; i++) uuid[i] = chars[0 | Math.random()*radix];
    } else {
      // rfc4122, version 4 form
      var r;

      // rfc4122 requires these characters
      uuid[8] = uuid[13] = uuid[18] = uuid[23] = '-';
      uuid[14] = '4';

      // Fill in random data.  At i==19 set the high bits of clock sequence as
      // per rfc4122, sec. 4.1.5
      for (i = 0; i < 36; i++) {
        if (!uuid[i]) {
          r = 0 | Math.random()*16;
          uuid[i] = chars[(i == 19) ? (r & 0x3) | 0x8 : r];
        }
      }
    }

    return uuid.join('');
  };

  // A more performant, but slightly bulkier, RFC4122v4 solution.  We boost performance
  // by minimizing calls to random()
  Math.uuidFast = function() {
    var chars = CHARS, uuid = new Array(36), rnd=0, r;
    for (var i = 0; i < 36; i++) {
      if (i==8 || i==13 ||  i==18 || i==23) {
        uuid[i] = '-';
      } else if (i==14) {
        uuid[i] = '4';
      } else {
        if (rnd <= 0x02) rnd = 0x2000000 + (Math.random()*0x1000000)|0;
        r = rnd & 0xf;
        rnd = rnd >> 4;
        uuid[i] = chars[(i == 19) ? (r & 0x3) | 0x8 : r];
      }
    }
    return uuid.join('');
  };

  // A more compact, but less performant, RFC4122v4 solution:
  Math.uuidCompact = function() {
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
      var r = Math.random()*16|0, v = c == 'x' ? r : (r&0x3|0x8);
      return v.toString(16);
    });
  };
})();

/**
 * Disable browser's same origin security policy to allow access to Amazon's Web
 * Services.
 *
 * NOTE: This doesn't work as you need to do this within the scope of the same
 *       origin "violation".
 */
AWS.prototype.enableAccess = function() {
   try 
      {
      netscape.security.PrivilegeManager.enablePrivilege( "UniversalBrowserRead" );
      console.log( "User allowed cross site privileges" );
      }
   catch( err )
      {
      console.warn( "User choose not to allow cross site privileges" );
      console.warn( "getUrl: script does not have cross site privileges" );
      this.instanceState = -1;
      this.error = err.toString();
      }
};
