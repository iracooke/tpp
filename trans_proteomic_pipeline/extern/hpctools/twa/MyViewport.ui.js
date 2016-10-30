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

var version = 'v2.0.3 Beta';


// Globals
var optionsWindow;                      // options dialog window
var ebsWindow;                          // create EBS volume window

/**
 * Create UI toolbar
 */
MyViewportUi = Ext.extend(Ext.Viewport, {
   initComponent: function() {
       
      var key  = { akey: '', skey: '' };
      var ckey = Ext.util.Cookies.get( 'TPP-TWA-session' );
      if ( ckey )
         key = Ext.JSON.decode( ckey );

      optionsWindow = optionsWindow();
      ebsWindow = createEBSWindow();
      
      this.items = [
         {
            xtype: 'toolbar',
            items : [
                     {
                        xtype: 'label',
                        html: '<a href="" noborder><img src="images/sm_tpp_logo.png" height=32></a>'
                     },
                     { xtype: 'tbspacer' },
                     { xtype: 'label', text : 'TPP Web Launcher for Amazon ', style: 'font-size: larger'},
                     { xtype: 'tbspacer', width: 5 },
                     { xtype: 'label', text : version, style: 'font-size: smaller' },
                     { xtype: 'tbspacer', width: 50 },
                     { xtype: 'label', text : 'Key ID:' },
                     { xtype: 'tbspacer' },
                     {
                         id: 'accessKeyId',
                         xtype: 'textfield',
                         allowBlank: false,
                         width: 140,
                         value: key.akey
                     },
                     '   ',
                     { xtype: 'label', text : 'Secret:' },
                     { xtype: 'tbspacer' },
                     {
                        id: 'secretKeyId',
                        xtype: 'textfield',
                        allowBlank: false,
                        inputType: 'password',
                        width: 140,
                        value: key.skey
                     },
                     {  xtype: 'button', 
                        icon: 'images/reload.png',
                        iconAlign: 'right',
                        scale: 'small',
                        handler :  function () {
                            checkCredentialsChange( true );
                        }
                     },
                     { xtype: 'tbspacer' },
                     toolsMenu(),
                     shortcutsMenu(),
                     '->',
                     { xtype: 'tbseparator' },
                     {
                        id: 'controlButtonId',
                        xtype: 'button',
                        text: 'Start Instance',
                        icon: 'images/start.png',
                        handler: startButtonClicked,
                        disabled: true
                     },
                     { xtype: 'tbspacer' }
            ]
         }
      ];
      MyViewportUi.superclass.initComponent.call(this);
   }
});
  
/*
 * Returns structure for creating tools menu in control bar
 */
function toolsMenu() {
   return {
      xtype: 'button',
      text: 'Tools',
      menu: {
         xtype: 'menu',
         items: [{
             xtype: 'menuitem',
             text: 'Reload TPP interface',
             id: 'reloadTPPMenuId',
             disabled: true, 
             handler: function() { 
                if ( !aws.instanceId ) {
                    Ext.MessageBox.alert( 'Error connecting to instance', 
                        'No EC2 instance started by TWA was found. Try ' +
                        'reloading your AWS credentials.');
                    return;
                }
                showBusyBox( 'Sign In', 'Connecting to instance...' );
                aws.connected = false; 
                loadPetunia( 10 );
             }
         },{
             xtype: 'menuseparator'
         },{
             xtype: 'menuitem',
             text: 'Create EBS volume...',
             id: 'createEBSMenuId',
             disabled: true, 
             handler: createEBSClicked
         },{
             xtype: 'menuitem',
             text: 'Delete EBS volume...',
             id: 'deleteEBSMenuId',
             disabled: true, 
             handler: deleteEBSClicked
         },{
             xtype: 'menuseparator'
         },{
             xtype: 'menuitem',
             id: 'optionsMenuId',
             text: 'Start Options...',
             disabled: true, 
             handler: function() { optionsWindow.show(); }
         }]
      }
   };
}

/*
 * Return structure for creating shortcuts menu in control bar
 */
function shortcutsMenu() {
   return {
      xtype: 'button',
      text: 'AWS Shortcuts',
      style: 'left-margin: 20px',
      menu: {
         xtype: 'menu',
         items: [
            {
               xtype: 'menuitem',
               text: 'Amazon Sign In/Register',
               icon: 'images/aws_logo.png',
               handler: function() { open( 'https://portal.aws.amazon.com/gp/aws/developer/registration/index.html', '_twa_aws' ); }
            },
            {
               xtype: 'menuitem',
               text: 'Amazon Security Credentials',
               icon: 'images/aws_logo.png',
               handler: function() { open( 'https://portal.aws.amazon.com/gp/aws/securityCredentials', '_twa_aws' ); }
            },
            {
               xtype: 'menuitem',
               text: 'EC2 Management Console',
               icon: 'images/aws_logo.png',
               handler: function() { open( 'https://console.aws.amazon.com/ec2/v2/home?region=us-west-2', '_twa_aws' ); }
            },
            {
               xtype: 'menuitem',
               text: 'S3 Management Console',
               icon: 'images/aws_logo.png',
               handler: function() { open( 'https://console.aws.amazon.com/s3/home', '_twa_aws' ); }
            }
        ]
     }
  };
}

/**
 * Enable/disable TWA UI elements
 */
function twaEnable( enable, button ) {
   
   var b = Ext.ComponentMgr.get('controlButtonId');
   b.setDisabled( !enable ); 
   if ( b.getText() == 'Start Instance' && button == 'Stop Instance') { 
      b.setText( 'Stop Instance' );
      b.setIcon( 'images/stop.png' );
      b.handler = stopButtonClicked;
   } else if ( b.getText() == 'Stop Instance' && button == 'Start Instance') { 
      b.setText( 'Start Instance' );
      b.setIcon( 'images/start.png' );
      b.handler = startButtonClicked;
   }

   if ( button == 'Stop Instance' ) { 
      Ext.ComponentMgr.get('reloadTPPMenuId').setDisabled( false );
      Ext.ComponentMgr.get('optionsMenuId').setDisabled( true );
      Ext.ComponentMgr.get('createEBSMenuId').setDisabled( true );
      Ext.ComponentMgr.get('deleteEBSMenuId').setDisabled( true );
   } else if ( button == 'Start Instance' ) {
      Ext.ComponentMgr.get('reloadTPPMenuId').setDisabled( true );
      Ext.ComponentMgr.get('optionsMenuId').setDisabled( false );
      Ext.ComponentMgr.get('ebsAttachId').setDisabled( !aws.ebsVolId );
      Ext.ComponentMgr.get('createEBSMenuId').setDisabled( false );
      Ext.ComponentMgr.get('deleteEBSMenuId').setDisabled( false );
   } else {
      Ext.ComponentMgr.get('reloadTPPMenuId').setDisabled( true );
      Ext.ComponentMgr.get('optionsMenuId').setDisabled( true );
      Ext.ComponentMgr.get('createEBSMenuId').setDisabled( true );
      Ext.ComponentMgr.get('deleteEBSMenuId').setDisabled( true );
   }
}

/**
 * Check user provided credentials by querying Amazon through the API. Called 
 * when the access key or secret key loses focus.
 */
function checkCredentialsChange( force ) {

   force = typeof(force) != 'undefined' ? force : false;
   
   console.debug( "checkCredentialsChange()" );
   
   var newAccess = Ext.ComponentMgr.get( 'accessKeyId' ).getValue();
   var newSecret = Ext.ComponentMgr.get( 'secretKeyId' ).getValue();
   
   // Assign new credentials
   aws.instanceId = null;
   aws.accessKey  = (newAccess === '') ? null : newAccess;
   aws.secretKey  = (newSecret === '') ? null : newSecret;
   
   // Store client side using a session cookie with no expires so it stays
   // only in browser memory.  Not the best approach but... var ckey = Ext.JSON.encode( { akey: aws.accessKey, skey: aws.secretKey } ); var key  = Ext.util.Cookies.set( 'TPP-TWA-session', ckey ); // Sign in 
   twaEnable( false );
   if ( aws.accessKey && aws.secretKey )
      {
      showBusyBox( 'Sign In', 'Signing into AWS...' );
      aws.signIn( signInResponse );
      }
   else
      {
      return Ext.MessageBox.show( { title: 'Problem with you AWS credentials',  
                                    msg: 'Missing access or secret key', 
                                    buttons: Ext.MessageBox.OK, 
                                    icon: Ext.MessageBox.ERROR } );
      }
   return null;
}

/*
 * Callback when finished signing in
 */
function signInResponse( error, reason ) { 
   console.debug( 'signInResponse(): finishing up sign in' );
   
   if ( error ) {
      showErrorBox( error, reason );
      twaEnable( false );
      return null;
   }

   // Fill in ami image ids
   var c = Ext.ComponentMgr.get('amiComboId');
   c.clearValue();
   c.store.removeAll();
   for ( var i = 0; i < aws.amiList.length; i++ ) {
      var id   = aws.amiList[i].id;
      var desc = aws.amiList[i].desc;
      c.store.add( { id: id, desc : '(' + id + ') ' + desc } );
   }
   c.store.sync();
   if ( aws.amiList.length ) {
      c.select( aws.amiList[0].id );
      aws.imageId = aws.amiList[0].id;
   }
   
   // Fill in key names 
   c = Ext.ComponentMgr.get('keyPairComboId');
   c.clearValue();
   c.store.removeAll();
   c.store.add( { id: 0, name: 'No Key' } );
   for ( i = 0; i < aws.keyPairList.length; i++ ) {
      var name = aws.keyPairList[i].name;
      c.store.add( { id: name, name: name } );
   }
   c.store.sync();
   if ( aws.keyPairList.length ) {
      c.select( aws.keyPairList[0].name );
      aws.keyName = aws.keyPairList[0].name;
   }
   
   // Fill in S3 URLs
   c = Ext.ComponentMgr.get('s3UrlId');
   if ( !c.getValue() )
      c.setValue( 'tpp-' + aws.accessKey.toLowerCase() );
   aws.s3Url = c.getValue();
   
   // Was there already a instance?
   if ( aws.instanceId ) {
      aws.getState( getStateResponse );
   } else {
      twaEnable( true, 'Start Instance' );
      loadWelcome();
   }
   return null;
}

/*
 * Invoked when user selects the control button and its in the start state
 */
function startButtonClicked( button ) {
   if ( !aws.accessKey || !aws.secretKey ) {
      Ext.MessageBox.alert( 'Error starting instance', 
          'You must first provide both a AWS access and secret key' );
      return;
   }
   
   if ( Ext.ComponentMgr.get('optionsFormId').getForm().hasInvalidField() ) {
      Ext.MessageBox.alert( 'Error starting instance', 
          'One or more options are invalid and must first be corrected' );
      return;
   }
   
   showBusyBox( 'Start Instance', 'Requesting start of instance...' );
   
   aws.startInstance( function( error, reason ) {
      if ( error ) {
         showErrorBox( error, 'The following error occurred while trying to ' +
                 'start the instance:<br><br>' + reason + '<br><br>' +
                 'The instance may/may not have started. Please reload TWA ' +
                 'or use the EC2 console to check for any running instances.' );
      } else {
         Ext.MessageBox.updateText( 'Instance is now launching. It may take a few minutes to start.' );
         aws.getState( getStateResponse );
      }
      
   } );
}

/*
 * Invoked when user selects the control button and its in the stop state 
 */
function stopButtonClicked( button ) {
   if ( !aws.accessKey || !aws.secretKey ) {
      Ext.MessageBox.alert( 'Missing Key(s)', 
          'You must first provide both a AWS access and secret key' );
      return;
   }

   if ( !aws.s3SyncOnStop && !aws.ebsAttach ) {
          Ext.MessageBox.confirm( 
              'Confirm shutdown', 
              'Are you sure you want to do that? You may have unsaved changes.',
              function(b) { if ( b === 'yes' ) stopButtonAction(); } );
      return;
   } 

   showBusyBox( 'Stop Instance', 'Requesting stop of instance...' );
   if ( !aws.stopInstance() ) {
      showErrorBox( 'Error stopping instance', 
                    'An error occurred when trying to stop the instance. ' +
                    'Please try signing in again to check the state of AWS.' );
   }
}

/*
 * Called when a state response is received
 */
function getStateResponse( error, reason ) { 
    
   console.debug( "getStateResponse() state: ", aws.instanceState );
   
   if ( error ) {
      showErrorBox( 'Error getting instance state', 
                    'The following error occurred while trying to ' +
              'get the instance state:<br><br>' + reason + '<br><br>' +
              'Please try reloading your credentials or reloading TWA to ' +
              'recheck the instance state. You may also have to use check ' +
              'using the EC2 console.' );
      twaEnable( false );
      return;
   }
                                                                /*jsl:ignore*/
   if ( aws.instanceState == null ) {                           // No instance
      twaEnable( true, 'Start Instance' );
      Ext.MessageBox.hide();
   } else if ( aws.instanceState == 0 )  {                      // Pending...
      var msg = 'Instance is pending. It may take a few minutes to start.';
      if ( Ext.MessageBox.isVisible() )
         Ext.MessageBox.updateText( msg );                      /*jsl:end*/
      aws.timeout = setTimeout( function()  { aws.getState( getStateResponse ); }, 5000 );
   } else if ( aws.instanceState == 16 ) {                      // Running...
      msg = 'Instance ' + aws.instanceId + ' is running. Connecting...';
      if ( Ext.MessageBox.isVisible() )
         Ext.MessageBox.updateText( msg );
      twaEnable( true, 'Stop Instance' );
      aws.timeout = setTimeout( function() {  aws.connected = false; loadPetunia( 10 ); }, 1500 );
   } else if ( aws.instanceState == 32 ) {                      // Shutdown...
      if ( Ext.MessageBox.isVisible() )
         Ext.MessageBox.updateText( 'Instance is shutting down...' );
      aws.timeout = setTimeout( function()  { aws.getState( getStateResponse ); }, 5000 );
   } else if ( aws.instanceId && aws.instanceState == 48 ) {    // Terminated...
      aws.instanceId = null; 
      aws.instanceState = null; 
      if ( Ext.MessageBox.isVisible() )
         Ext.MessageBox.updateText( 'Instance was terminated.' );
      aws.timeout = setTimeout( function() { loadWelcome(); twaEnable( true, 'Start Instance'); }, 3500 );
   } else if ( aws.instanceId ) {                               // Gone?
      aws.instanceId = null; 
      aws.instanceState = null; 
      showErrorBox( 'Unknown state of instance', 
              'The state of the instance could not be determined.<br><br>' +
              'Please try reloading your credentials or reloading TWA to ' +
              'recheck the instance state. You may also have to use check ' +
              'using the EC2 console.' );
   }
   return;
}

/**
 * Load welcome in the content iframe
 */
function loadWelcome() {
   var c = document.getElementById('content');
   if ( c.attachEvent )         // IE events
      c.attachEvent( 'onload', function() { Ext.MessageBox.hide(); } );
   else
      c.onload = function() { Ext.MessageBox.hide(); clearTimeout( aws.timeout ); };
// Just in case load fails and message box is visible
//setTimeout( function() { Ext.MessageBox.hide() }, 5000 );
   c.src = 'welcome.html';
}

/**
 * Attempt to connect to the EC2 instance and load the Petunia main page
 * in the content iframe 
 */
function loadPetuniaError() {
   var m = "The TPP Web Launcher was unable to " +
           " connect to the newly started EC2 instance. Please try" +
           " reloading the page or resigning back in.";
   showErrorBox( 'Error connecting to TPP',  m );
}

function loadPetuniaResponse() { 
   clearTimeout( aws.timeout );
   console.debug( "loadPetuniaResponse() received" );

   if ( aws.connected )
      return;
   aws.connected=true;

   Ext.MessageBox.updateText( 'Connected to instance ' + aws.instanceId );

   // Load petunia in the iframe.  BTW, we can't really tell if this succeeds
   // succeeds or fails due to XHR as the event handlers usually are disabled
   var c = document.getElementById('content');
   c.onload  = function() { clearTimeout( aws.timeout ); Ext.MessageBox.hide(); };
   if ( c.attachEvent )
      c.attachEvent( 'onload', c.onload );
   c.onerror = function() { clearTimeout( aws.timeout ); loadPetuniaError(); };  

   c.src = aws.instanceUrl;

   // Since most browsers keep silent about loading iframes just hide message
   // and cross fingers
   aws.timeout = setTimeout( function() { clearTimeout( aws.timeout ); Ext.MessageBox.hide(); }, 1000 );
}

// http://stackoverflow.com/questions/4845762/onload-handler-for-script-tag-in-internet-explorer
function loadPetunia( retries ) {
   console.debug( "loadPetunia() retries = " + retries );
   if ( aws.connected ) {
      return;
   } else if ( retries <= 0 ) {
       loadPetuniaError();
       return;
   }

   var head   = document.getElementsByTagName("head")[0];
   var script = document.createElement('script'); 
   script.type = 'text/javascript';
   script.src  = aws.instanceUrl + "?Action=AJAXPing" + 
                 "&Callback=loadPetuniaResponse";
   var done = false;

   // Attach handlers for all browsers
   script.onerror = script.onload = script.onreadystatechange = function( e ) { 
       console.debug( 'loadPetunia() event ', e );
       if ( !done && (!this.readyState || 
               this.readyState === "loaded" || 
               this.readyState === "complete") ) {
          done = true;
          // Handle memory leak in IE
          script.onload = script.onreadystatechange = null;
          if ( head && script.parentNode ) {
              head.removeChild( script );
          }
       if ( !aws.connected )
          /*jsl:ignore*/
          aws.timeout = setTimeout( function() { loadPetunia( --retries); }, 500 );
          /*jsl:end*/
       }
   }; 

   head.insertBefore( script, head.firstChild );
   return;

//   document.getElementsByTagName('head')[0].appendChild(script);
//   Ext.MessageBox.hide();
   //c.src = aws.instanceUrl;

   // Make proxy request to check for Apache server...
/*
   aws.checkPetunia( 7, function( status, msg ) {
       
          // Try to load it in the iframe regardless of status.  BTW, we can't
          // really tell if this succeeds or fails due to XHR security.
          if ( c.attachEvent ) // IE events
             c.attachEvent( 'onload', function() { Ext.MessageBox.hide(); } );
          else
             c.onload = function() { Ext.MessageBox.hide(); clearTimeout(); };
          c.src = aws.instanceUrl + "?cb=mycallback";
*/          
/* FIX: onload() XHR issues with this
          setTimeout( function() { 
              
             if ( status != 200 ) {
                var m = msg + "<br><br>The TPP Web Launcher was unable to " +
                      " connect to the newly started EC2 instance. Please try" +
                      " reloading the page or resigning back in.";
                showErrorBox( 'Error connecting to TPP',  m );
             } else {
                Ext.MessageBox.hide();
             }
             
          }, '3500' );
*/
//   });
}

/**
 * Create EBS action
 */
function createEBSClicked( button ) { 

    if ( aws.ebsVolId ) {
        showErrorBox( 'Error creating EBS volume', 
                      'EBS volume associated with TWA already exists.' );
        return;
    }

    ebsWindow.show(); 
}

/**
 * Delete EBS action
 */
function deleteEBSClicked( button ) { 

    if ( !aws.ebsVolId ) {
        showErrorBox( 'Delete EBS Volume', 
                      'No EBS volume associated with TWA was found.' );
        return;
    }
    else if ( aws.ebsAttach && aws.instanceId ) {
        showErrorBox( 'Delete EBS Volume', 
                      'Cannot delete EBS volume while in may be in use by ' +
                      'a running instance.');
        return;
    }

    Ext.MessageBox.confirm( 'Delete EBS Volume',
        'Are you sure you want to delete the EBS volume? ' +
        'Any data on the volume will be deleted and cannot ' +
        'be recovered.', function( button ) {
            if ( button !== 'yes' )
                return;
            
            aws.deleteEBSVolume( function( status, msg ) {
            if ( status == 200 ) {
                Ext.MessageBox.alert( 'Delete EBS Volume', 
                   'EBS volume has been deleted.' );
                Ext.ComponentMgr.get('ebsAttachId').setDisabled( true );
            } else {
               showErrorBox( 'Delete EBS Volume', 
                             'Error occurred deleting EBS volume: ' + msg );
            } } );
        });
}

/**
 * Show a busy message box with messages
 */
function showBusyBox( title, msg ) {
    Ext.MessageBox.show( {
        id: 'controlDialogId',
        title: title,
        msg: msg,
        icon: 'twa-busy-icon',
        width: 300,
        closable: false
    } );
}

/**
 * Show a error message box with messages
 */
function showErrorBox( title, msg ) {
    Ext.MessageBox.show( { 
        title: title, 
        msg: msg, 
        buttons: Ext.MessageBox.OK, 
        icon: Ext.MessageBox.ERROR 
    } );
}

/**
 * Build the create EBS volume window
 */
function createEBSWindow() {
    var win = Ext.create( 'Ext.window.Window', {
        id: 'createEBSWinId',
        title: 'Create EBS Volume',
        modal:   true,
        closeAction: 'hide',
        layout: 'fit',
        items: [{
            xtype: 'form',
            id: 'createEBSFormId',
            animCollapse: false,
            constrainHeader: true,
            bodyPadding: 15,
            autoScroll: true,
            items: [ {
                xtype: 'combo',
                name: 'zone',
                fieldLabel: 'Availability Zone',
                labelAlign: 'right',
                store: aws.getAvailabilityZones(),
                allowBlank: false,
                validateBlank: true,
                autoScroll: true,
                editable: false
            },{
                xtype: 'numberfield',
                name: 'size',
                fieldLabel: 'Volume Size (GiB)',
                labelAlign: 'right',
                value:    100,
                minValue: 1,
                maxValue: 1000,
                tooltip: 'Enter size of partition (1GiB to 1000GiB) to create'
            } ]
        }],
        buttons: [{
            text: 'Create Volume',
            handler: function() {
                var f = Ext.ComponentMgr.get('createEBSFormId').getForm();
                if ( f.hasInvalidField() ) {
                    showErrorBox( 'Creating EBS Volume', 
                                  'One or more options are invalid and must first be corrected' );
                    return;
              } else {
                    var d = f.getValues();
                    this.up('window').hide();
                    showBusyBox( 'Create EBS Volume', 'Creating EBS volume...' );
                    aws.createEBSVolume( d.zone, d.size, function( status, msg ) {
                        if ( status == 200 ) {
                            Ext.MessageBox.alert( 'Create EBS Volume', 
                                                  'EBS volume has been created.' );
                            Ext.ComponentMgr.get('ebsAttachId').setDisabled( false );
                        } else {
                            showErrorBox( 'Create EBS Volume', 
                            'Error occurred creating EBS volume: ' + msg );
                        } 
                    } );
              }
        } 
    },{
            text: 'Cancel',
            handler: function() {
            this.up('window').hide();
            }
        }]
    } );
    return win;
}
