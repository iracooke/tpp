Ext.define('AmztppSim.model.TimeSlice', {
    extend: 'Ext.data.Model',
    fields: [ 
        { name: 'time',  type: 'int' },
        { name: 'pendUp',  type: 'int' },
        { name: 'pendSrv',  type: 'int' },
        { name: 'activeSrv', type: 'int' },
        { name: 'pendDown',  type: 'int' }
    ]
});
