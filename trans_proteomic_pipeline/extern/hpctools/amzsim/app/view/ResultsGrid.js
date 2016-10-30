/**
*/
Ext.define('AmztppSim.view.ResultsGrid' ,{
    extend: 'Ext.grid.Panel',
    alias: 'widget.resultsgrid',
    title: 'Simulated Operations',
    collapsible: true,
    store: 'Searches',

    initComponent: function() {
         
        this.columns = [
            { header: 'Num',    dataIndex: 'num',  flex: 1 },
            { header: 'Input (MB)',  dataIndex: 'mzSize', flex: 1 },
            { header: 'Output (MB)', dataIndex: 'pepSize', flex: 1 },
            { header: 'Local Up', 
                columns: [{
                    header: 'Beg', dataIndex: 'localUploadBeg', flex: 1
                },{
                    header: 'End', dataIndex: 'localUploadEnd', flex: 1
                }],
            },
            { header: 'AWS Down',
                columns: [{
                    header: 'Beg', dataIndex: 'amzDownloadBeg', flex: 1
                },{
                    header: 'End', dataIndex: 'amzDownloadEnd', flex: 1
                }],
            },
            { header: 'AWS Up',
                columns: [{
                    header: 'Beg', dataIndex: 'amzUploadBeg', flex: 1
                },{
                    header: 'End', dataIndex: 'amzUploadEnd', flex: 1
                }],
            },
            { header: 'Local Down',
                columns: [{
                    header: 'Beg', dataIndex: 'localDownloadBeg', flex: 1
                },{
                    header: 'End', dataIndex: 'localDownloadEnd', flex: 1
                }],
            }
        ];

        this.callParent(arguments);
    }
});
