/**
*/
Ext.define('AmztppSim.view.TimelinePanel' ,{
    extend: 'Ext.form.Panel',
    alias: 'widget.timelinepanel',
    title: 'Simulation Timeline',
    collapsible: true,

    layout: { type: 'hbox', align: 'stretch', padding: 2 },

    items: [{
       xtype: 'timechart',
       flex: 1
    }]
});
