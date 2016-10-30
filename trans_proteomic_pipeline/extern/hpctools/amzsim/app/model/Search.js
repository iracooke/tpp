Ext.define('AmztppSim.model.Search', {
    extend: 'Ext.data.Model',
    fields: [ 
        { name: 'num',  type: 'int' },
        { name: 'mzSize', type: 'int' },
        { name: 'pepSize', type: 'int' },
        { name: 'localUploadBeg', type: 'int' },
        { name: 'localUploadEnd', type: 'int' },
        { name: 'amzDownloadBeg', type: 'int' },
        { name: 'amzDownloadEnd', type: 'int' },
        { name: 'amzUploadBeg', type: 'int' },
        { name: 'amzUploadEnd', type: 'int' },
        { name: 'localDownloadBeg', type: 'int' },
        { name: 'localDownloadEnd', type: 'int' }
    ]
});
