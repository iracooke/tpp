Ext.application({
    requires: ['Ext.container.Viewport'],
    name: 'AmztppSim',
    version: '1.0',

    appFolder: 'app',

    controllers: [
        'Variables'
    ],

    views: [
        'ResultsPanel'
    ],

    launch: function() {
        console.log('launching');

        Ext.create('Ext.container.Viewport', {
            layout: 'border',
            items: [
                {
                    region: 'north',
                    xtype: 'toolbar',
                    padding: 5, 
                    items: [ 
                        { xtype: 'tbtext', text : 'AMZTPP Simulator',
                          style: { 'font-size': 'larger',  'font-weight' : 'bold' } }, 
                        { xtype: 'tbspacer', width: 5 },
                        { xtype: 'tbtext', text : 'v' + this.version }
                    ]
                },
                {
                    region: 'west',
                    title: 'Variables',
                    xtype: 'variableform'
                },
                {
                    region: 'center',
                    xtype: 'panel',
                    layout: { type: 'vbox', align: 'stretch' },
                    items: [ // { xtype: 'summarypanel', flex: 0 }, 
                             { xtype: 'resultspanel', flex:0 }, 
                             { xtype: 'timelinepanel', flex:1 }, 
                             // { xtype: 'timechart', title: 'EC2', flex:1 }, 
                             { xtype: 'resultsgrid', flex: 1 } ]

                    //xtype: 'timechart'
                    //xtype: 'resultspanel'
// items: [ {xtype: 'summarypanel' }, { xtype: 'timechart' }, { xtype: 'resultsgrid' } ]
                },
            ]
        });
    }
});
