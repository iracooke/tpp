/**
*/
Ext.define('AmztppSim.view.ResultsPanel' ,{
    extend: 'Ext.form.Panel',
    alias: 'widget.resultspanel',
    title: 'Simulation Results',
    collapsible: true,
    
    layout: { type: 'hbox', align: 'stretch', padding: 2 },

    defaultMargins: '5',                // doesn't seem to work
    
    items: [{
        flex: 1,
        xtype: 'fieldset',
        title: 'Summary',
        margin: '5',
        items: [{
            xtype: 'displayfield',
            fieldLabel: '# EC2 Instances',
            name: 'ec2Count',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Run Time (min)',
            name: 'runTime',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Data Uploaded (MB)',
            name: 'upBytes',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Data Downloaded (MB)',
            name: 'downBytes',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Total Est. Cost',
            name: 'totalCost',
            value: ''
        }]
    },{
        flex: 1,
        xtype: 'fieldset',
        title: 'EC2 Cost',
        margin: '5',
        items: [{
            xtype: 'displayfield',
            fieldLabel: 'Instances',
            name: 'instancesCost',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'PublicIP-In',
            name: 'publicIPIn',
            value: '<i>free</i>'
        },{
            xtype: 'displayfield',
            fieldLabel: 'PublicIP-Out',
            name: 'publicIPOut',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Regional-In',
            name: 'regionalIn',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Instances-In',
            name: 'instancesIn',
            value: '<i>free</i>'
        },{
            xtype: 'displayfield',
            fieldLabel: 'Instances-Out',
            name: 'instancesOut',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'EC2 Subtotal',
            name: 'ec2Total',
            value: ''
        }]
    },{
        flex: 1,
        xtype: 'fieldset',
        title: 'S3 Cost',
        margin: '5',
        items: [{
            xtype: 'displayfield',
            fieldLabel: 'PUT, COPY, POST, or LIST',
            name: 's3MiscOperCost',
            value: '$ 0.0'
        },{
            xtype: 'displayfield',
            fieldLabel: 'GET, other',
            name: 's3GetOperCost',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Data In (MB)',
            name: 's3DataInCost',
            value: '<i>free</i>'
        },{
            xtype: 'displayfield',
            fieldLabel: 'Data Out (MB)',
            name: 's3DataOutCost',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Storage (GB/month)',
            name: 's3StorageCost',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'S3 Subtotal',
            name: 's3Total',
            value: ''
        }]
    },{
        flex: 1,
        xtype: 'fieldset',
        title: 'SQS Cost',
        margin: '5',
        items: [{
            xtype: 'displayfield',
            fieldLabel: 'Requests',
            name: 'sqsRequestsCost',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'Data In (MB)',
            name: 'sqsDataInCost',
            value: '<i>free</i>'
        },{
            xtype: 'displayfield',
            fieldLabel: 'Data Out (MB)',
            name: 'sqsDataOutCost',
            value: ''
        },{
            xtype: 'displayfield',
            fieldLabel: 'SQS Subtotal',
            name: 'sqsTotal',
            value: ''
        }]
    }],
    
    setEC2Values : function( c ) {
       this.getForm().setValues( { 
           instancesCost: mfmt( c.instancesCost ),
//         publicIPIn:    mfmt( c.publicIPIn ),
           publicIPOut:   mfmt( c.publicIPOut ),
           regionalIn:    mfmt( c.regionalIn ),
//         instancesIn:   mfmt( c.instancesIn ),
           instancesOut:  mfmt( c.instancesOut ),
           ec2Total:      mfmt( c.total )
       } );
    },
                                 
    
    setS3Values : function( c ) {
       this.getForm().setValues( { 
           s3MiscOperCost: mfmt( c.miscOperCost ),
           s3GetOperCost: mfmt( c.getOperCost ),
           s3DataOutCost: mfmt( c.dataOutCost ),
           s3StorageCost: mfmt( c.storageCost ),
           s3Total:       mfmt( c.total )
       } );
    },

    setSQSValues : function( c ) {
       this.getForm().setValues( { 
           sqsRequestsCost: mfmt( c.requestsCost ),
           sqsDataInCost:   mfmt( c.dataIn ),
           sqsDataOutCost:  mfmt( c.dataOut ),
           sqsTotal:        mfmt( c.total )
       } );
    },

});

// format a floating point into a money string
function mfmt ( v ) {
   v = '$' + v.toFixed(2);
   return v;
}
