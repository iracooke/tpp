/**
 */
Ext.define('AmztppSim.view.VariablesForm', {
    extend: 'Ext.form.Panel',
    alias : 'widget.variableform',

    title: 'Variables',
    collapsible: true,
    bodyPadding: 10,
    defaultType: 'textfield',
    
    items: [ {
            fieldLabel: '# MzFile(s)',
            name: 'mzFileCount',
            xtype: 'numberfield',
            minValue: 1,
            maxValue: 999999,
            value: 1,
            allowDecimals: false
        }, {
            fieldLabel: 'Avg MzFile Size (MB)',
            name: 'mzFileSize',
            xtype: 'numberfield',
            value: 100,
            allowDecimals: true
        }, {
            fieldLabel: 'Avg Output Size (MB)',
            name: 'pepFileSize',
            xtype: 'numberfield',
            value: 100,
            allowDecimals: true
        }, {
            fieldLabel: '# Parallel Transfers',
            name: 'parallel',
            xtype: 'numberfield',
            value: 1,
            allowDecimals: false,
            minValue: 1,
            maxValue: 99
        }, {
            fieldLabel: 'Local Avg Upload (MB/s)',
            name: 'localUploadSpeed',
            xtype: 'numberfield',
            value: 5.0,
            allowDecimals: true
        }, {
            fieldLabel: 'Local Avg Download (MB/s)',
            name: 'localDownloadSpeed',
            xtype: 'numberfield',
            value: 5.0,
            allowDecimals: true
        }, {
            fieldLabel: 'Amazon Avg Upload (MB/s)',
            name: 'amzUploadSpeed',
            xtype: 'numberfield',
            value: 25.0,
            allowDecimals: true
        }, {
            fieldLabel: 'Amazon Avg Download (MB/s)',
            name: 'amzDownloadSpeed',
            xtype: 'numberfield',
            value: 10.0,
            allowDecimals: true
        }, {
            fieldLabel: 'Avg Search Time (min)',
            name: 'avgSearchTime',
            xtype: 'numberfield',
            value: 15,
            allowDecimals: true
        }, {
            fieldLabel: '# Instances',
            name: 'ec2Max',
            xtype: 'numberfield',
            value: 1,
            allowDecimals: false,
            minValue: 1,
            maxValue: 999999,
        }, {
            fieldLabel: 'Instance Cost',
            name: 'ec2Cost',
            xtype: 'numberfield',
            allowDecimals: true,
            value: 0.64
        }, {
            fieldLabel: 'Random distribution',
            name: 'randomize',
            xtype: 'checkbox',
            checked: true
        }, {
            xtype: 'button',
            text:  'Start Simulation',
            margin: 10,
            action: 'run',
            formBind: true,
            style: { float: 'right' }
        }
    ]/*,

    buttons: [
        {
            text: 'Run Simulation',
            action: 'run',
            formBind: true
        }
    ]
    */
});
