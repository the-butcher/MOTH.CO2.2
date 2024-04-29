import DataArrayIcon from '@mui/icons-material/DataArray';
import DataObjectIcon from '@mui/icons-material/DataObject';
import HttpIcon from '@mui/icons-material/Http';
import QueryBuilderIcon from '@mui/icons-material/QueryBuilder';
import SmartScreenIcon from '@mui/icons-material/SmartScreen';
import Typography from '@mui/material/Typography';
import { useEffect } from 'react';
import { IResponseProps } from './IResponseProps';

const ApiResponse = (props: IResponseProps) => {

    const { time, href, http, type, data } = props;

    const content = type === 'json' ? JSON.stringify(JSON.parse(data), null, 2) : data;

    useEffect(() => {
        console.debug('⚙ updating ApiResponse component', time, href);
    }, [time, href, http, data]);

    return (
        <div>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center" }}>
                <QueryBuilderIcon sx={{ fontSize: 'medium' }} />
                &nbsp;{new Date(time).toLocaleTimeString()}
            </Typography>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center" }}>
                <SmartScreenIcon sx={{ fontSize: 'medium' }} />
                &nbsp;{href}
            </Typography>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center" }}>
                <HttpIcon sx={{ fontSize: 'medium' }} />
                &nbsp;{http}
            </Typography>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center" }}>
                {
                    type === 'json' ? <DataObjectIcon sx={{ fontSize: 'medium' }} /> : <DataArrayIcon sx={{ fontSize: 'medium' }} />
                }
                &nbsp;{type}
            </Typography>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center", height: '200px', background: 'black', color: 'white', marginTop: '4px' }}>
                <pre style={{ height: 'inherit', width: '100%', overflow: 'auto', padding: '6px' }}>{content}</pre>
            </Typography>
        </div>

    );
}

export default ApiResponse;