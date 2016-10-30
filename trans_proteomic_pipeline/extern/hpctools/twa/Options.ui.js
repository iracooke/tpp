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


//
// Amazon SDK doesn't provide an API for returning the possible instance
// types to use.  So hardcode these using this as reference: 
//
//    http://aws.amazon.com/amazon-linux-ami/instance-type-matrix/
//
// (these all have to be S3 backed)
var InstanceTypes = [ 
                      // Current generation, General purpose
                      'm3.medium', 'm3.large', 'm3.xlarge', 'm3.2xlarge',
                      // Current generation, Compute optimized
                      'c3.large', 'c3.xlarge', 'c3.2xlarge', 'c3.4xlarge',
                      // Current generation, Memory optimized
                      'r3.large', 'r3.xlarge', 'r3.2xlarge', 'r3.4xlarge', 'r3.8xlarge',
                      // Current generation, Storage optimized
                      'i2.xlarge', 'i2.2xlarge', 'i2.4xlarge', 'i2.8xlarge', 'hs1.8xlarge',

                      // Previous generation, General purpose
                      'm1.small',  'm1.medium',  'm1.large',    'm1.xlarge', 
                      // Previous generation, Compute optimized
                      'c1.medium', 'c1.xlarge',  'cc2.8xlarge',
                      // Previous generation, Memory optimized
                      'm2.xlarge', 'm2.2xlarge', 'm2.4xlarge', 'cr1.8xlarge',
                      // Previous generation, Storage optimized
                      'hi1.4xlarge' 
                    ];

// FIX: Nice to load zones dynamically
var Zones = [ 'No Preference', 
              'us-west-2a',
              'us-west-2b',
              'us-west-2c' ];

Ext.define( 'ami', { extend: 'Ext.data.Model', 
                     fields: [ { name: 'id',   type: 'string'}, 
                               { name: 'desc', type: 'string' } ], 
                     proxy: { type: 'localstorage', id: 'id' }
                   } );
                   
Ext.define( 'keypair', { extend: 'Ext.data.Model', 
                         fields: [ { name: 'id', type: 'string'},
                                   { name: 'name', type: 'string' }
                                 ], 
                     proxy: { type: 'localstorage', id: 'id' }
                   } );

//
// Returns map for creating a window containing the options form
//
function optionsWindow() {

   // Add a vtype to check s3 url
   Ext.apply( Ext.form.field.VTypes, {
      s3url : function( val, field ) {
         var bpat = new RegExp( '^[a-z0-9][a-z0-9.-]{2,}$' );
         if ( !bpat.test( val.split('/',1)[0] ) )
            return false;

         return Ext.form.VTypes.url( 'http://mytest.com/' + val );
      },
      s3urlText : 'The S3 bucket name can only only contain numbers, lowercase ' +
                  'characters, periods(.), and dashes(-).  It must also start '  +
                  'with a letter or character and be more than 3 characters.'    +
                  'Additional folder paths using the (/) delimiter may also be ' +
                  'included.'
   });
   
   var generalOptionsGroup = {
       xtype: 'fieldset',
       id: 'generalOptionsGroupId',
       title: 'General Options',
       layout: 'anchor',
       defaults : {
           anchor: '100%',
           labelAlign: 'right',
           hideEmptyLabel: true
           }, 
       items: [{
               xtype: 'textfield',
               fieldLabel: 'Guest password',
               allowBlank: false,
               editable: true,
               name: 'guestPassword',
               id: 'guestPassword',
               inputType: 'password',
               value: aws.guestPassword,
               listeners: { 'change' : function(f,v) { aws.guestPassword = v; } }
               },
               {
               xtype: 'checkbox',
               boxLabel: 'Install demo/tutorial data on instance startup',
               checked: aws.installDemo,
               listeners: { 'change' : function(f,v) { aws.installDemo = v; } }
               }
           ]
   };
   
   var ami = {
      xtype: 'combo',
      id: 'amiComboId',
      fieldLabel: 'AMI',
      labelAlign: 'right',
      allowBlank: false,
      forceSelection: true,
      disableKeyFilter: true,
      editable: false,
      autoSelect: true,
      queryMode: 'local',
      displayField: 'desc',
      valueField: 'id',
      store:  new Ext.data.Store( { model: 'ami' } ),
      listeners : { 'select': function(r) { aws.imageId = r.getValue(); } }
   };
   
   var instanceType = {
      xtype: 'combo',
      fieldLabel: 'Instance Type',
      labelAlign: 'right',
      store: InstanceTypes,
      value: aws.instanceType,
      allowBlank: false,
      autoScroll: true,
      editable: false,
      size : 30, // not working right now, unless width set on form
      triggerAction: 'all',
      listeners : { 'blur': function(r) { aws.instanceType = r.getValue(); } }
   };

   var zone = {
      xtype: 'combo',
      id: 'EC2ZoneId',
      fieldLabel: 'Availability Zone',
      store: Zones,
      value: 'No Preference',
      allowBlank: false,
      autoScroll: true,
      editable: false,
      validateValue : function (v) {
          console.debug( "validating " + v);
         var errs = this.getErrors(v);
         
         if ( (v || v !== "") && v !== 'No Preference' && aws.ebsAttach &&
              aws.ebsZone !== v) {
            errs.push( 'EC2 instance must be in the same availability ' +
                       'zone (' + aws.ebsZone + ') of the EBS volume' );
         }
         
         var error = errs[0];
         if ( error == undefined ) {
             this.clearInvalid();
             return true;
         } else {
             this.markInvalid( error );
             return false;
         }
      },
      listeners : { 'change': function(f,v) { 
      aws.zone = v;
          if ( aws.zone && aws.zone == 'No Preference' )
             aws.zone = null;
          }
      }
   };
   
   var keyPair = {
      xtype: 'combo',
      id: 'keyPairComboId',
      fieldLabel: 'Key Pair',
      labelAlign: 'right',
      allowBlank: true,
      disableKeyFilter: true,
      store:  new Ext.data.Store( { model: 'keypair' } ),
      queryMode: 'local',
      displayField: 'name',
      valueField: 'id',
      autoScroll: true,
      editable: true,
      listeners : { 'blur': function(r) { 
      aws.keyName = r.getValue();
      if ( aws.keyName === 0 ) // allow to be blank (id=0) 
         aws.keyName = null;
      } }
   };
   
   var timeout = {
      xtype: 'numberfield',
      fieldLabel: 'Auto shutdown (hrs)',
      labelAlign: 'right',
      allowDecimals: false,
      allowNegative: false,
      allowBlank: false,
      maxLengthText: 3,
      value: aws.shutdown,
      valid : function(r) { aws.shutdown = r.getValue(); },
      minValue: 1,
      maxValue: 72
   };
   
   var ec2OptionsGroup = {
      xtype: 'fieldset',
      id: 'ec2OptionsGroupId',
      title: 'EC2 Options',
      layout: 'anchor',
      defaults : {
      anchor: '100%',
      labelAlign: 'right',
      hideEmptyLabel: true
      }, items: [ instanceType, ami, zone, keyPair, timeout ]
   };
   
   var s3OptionsGroup = {
      xtype: 'fieldset',
      id: 's3OptionsGroupId',
      title: 'S3 Options',
      defaultType: 'checkbox',
      defaults : {
         hideEmptyLabel: true
      },
      items: [{
         xtype: 'textfield',
         fieldLabel: 'S3 Bucket/Folder path',
         labelAlign: 'top',
         allowBlank: true,
         editable: true,
         name: 's3Url',
         id: 's3UrlId',
         size: 30, // not working right now, unless width set on form
         width: 275,
         vtype: 's3url',
         value: aws.s3Url,
         listeners: { 
             'change': function(f,v) { 
                 if ( f.isValid() )
                     aws.s3Url = v; 
                 else
                     aws.s3Url = null; 
                 } 
             } 
      },{
         boxLabel: 'Sync with S3 contents on instance startup',
         checked: aws.s3SyncOnStart,
         listeners: { 'change' : function(f,v) { aws.s3SyncOnStart = v; } }
      },{
         boxLabel: 'Sync with S3 contents on instance shutdown',
         checked: aws.s3SyncOnStop,
         listeners: { 'change' : function(f,v) { aws.s3SyncOnStop = v; } }
      }]
   };
   
   var ebsOptionsGroup = {
      xtype: 'fieldset',
      id: 'ebsOptionsGroupId',
      title: 'EBS Options',
      defaults : {
          hideEmptyLabel: true
      },
      items: [{
         xtype: 'checkbox',
         id: 'ebsAttachId',
         boxLabel: ' Attach the EBS storage volume' +
                   ' to the instance at startup.',
         name: 'ebsAttach',
         inputValue: 'false',
         checked: true,
         disabled: true,
         listeners: { 'change' : function(f,v) { 
             aws.ebsAttach = v; 
             if ( aws.ebsAttach && aws.zone !== null && aws.ebsZone !== aws.zone ) {
                 var of = Ext.getCmp('optionsFormId').getForm().findField('EC2ZoneId');
                 of.markInvalid('EC2 instance must be in the same availability ' +
                                'zone (' + aws.ebsZone + ') of the EBS volume ' +
                                'to be able to attach it.' );
              }
         } }
      }]
   };
   
   var win = Ext.create( 'Ext.window.Window', {
       id: 'optionsWin',
       title: 'EC2 Start Options',
       height: 350,
       width: 400,
       modal:   true,
       closeAction: 'hide',
       layout: 'fit',
       items: [{
           xtype: 'form',
           id: 'optionsFormId',
           animCollapse: false,
           constrainHeader: true,
           bodyPadding: 15,
           autoScroll: true,
           items: [ generalOptionsGroup, ec2OptionsGroup, s3OptionsGroup, 
                    ebsOptionsGroup ]
       }],
       buttons: [{
           text: 'OK',
           handler: function() {
               
              if ( Ext.ComponentMgr.get('optionsFormId').getForm().isValid() ) {
                   this.up('window').hide();
               } else {
                   Ext.MessageBox.alert( 'Error Starting Instance', 
                       'One or more options are invalid and must first be corrected' );
                       return;
               }
           }
         }]
//       },{
//           text: 'Help'
//       }]
   } );
   return win;
}
