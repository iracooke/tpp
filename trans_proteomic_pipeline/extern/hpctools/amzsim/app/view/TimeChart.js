/**
*/
Ext.define('Ext.chart.theme.MyChartTheme' ,{
    extend: 'Ext.chart.theme.Base',

    constructor: function(config) {
        this.callParent([Ext.apply({
            axis: {
                font: '9px Arial'
            },
            axisTitleLeft: {
                font: '9px Arial'
            },
            axisTitleBottom: {
                font: '9px Arial'
            }
        }, config) ]);
    }
});

Ext.define('AmztppSim.view.TimeChart' ,{
    extend: 'Ext.chart.Chart',
    alias: 'widget.timechart',
    title: 'EC2 Provisioning Characteristics',

    store: 'TimeSlices',

    axes: [ {
        title: 'Counts',
        type: 'Numeric',
        position: 'left',
        fields: ['numPending'],
        minimum: 0,
        maximum: 10
    },{
        title: 'Time (minutes)',
        type: 'Numeric',
        position: 'bottom',
        fields: ['time'],
        minimum: 0
    }],

    series: [ {
        type: 'line',
        xField: 'time',
        yField: 'pendUp',
        title: '# Awaiting Upload',
        showMarkers: false,
        // style: { fill: '#80A080' }
    },{
        type: 'line',
        xField: 'time',
        yField: 'pendSrv',
        title: '# Awaiting Searches',
        showMarkers: false,
        // style: { fill: '#80A080' }
    },{
        type: 'line',
        xField: 'time',
        yField: 'activeSrv',
        title: '# Active Searches',
        showMarkers: false
    },{
        type: 'line',
        xField: 'time',
        yField: 'pendDown',
        title: '# Awaiting Download',
        showMarkers: false
    }],

    legend: {
        position: 'right',
        labelFont: '9px Arial'
    },

    theme: 'MyChartTheme'
});
