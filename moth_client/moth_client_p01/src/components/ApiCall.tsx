import HttpIcon from '@mui/icons-material/Http';
import QueryBuilderIcon from '@mui/icons-material/QueryBuilder';
import SmartScreenIcon from '@mui/icons-material/SmartScreen';
import Typography from '@mui/material/Typography';
import { createRef, useEffect } from 'react';
import { ICallProps } from '../types/ICallProps';
import DataObjectIcon from '@mui/icons-material/DataObject';
import DataArrayIcon from '@mui/icons-material/DataArray';

const ApiCall = (props: ICallProps) => {

    const { name, time, url, bodyParams, returns } = props;
    const frameRef = createRef<HTMLIFrameElement>();
    const formRef = createRef<HTMLFormElement>();

    useEffect(() => {

        console.debug('⚙ updating ApiCall component', time, url);

        if (Object.keys(bodyParams).length > 0) {
            // frameRef.current.style.display = 'none';
            formRef.current.action = url;
            formRef.current.submit();
        } else {
            frameRef.current.src = url;
        }

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [time, url]);

    const inputs: JSX.Element[] = [];
    const bodyParamKeys = Object.keys(bodyParams);
    bodyParamKeys.forEach(bodyParamKey => {
        inputs.push(<input type="hidden" name={bodyParamKey} value={bodyParams[bodyParamKey]}></input>)
    });

    return (
        <div>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center" }}>
                <QueryBuilderIcon sx={{ fontSize: 'medium' }} />
                &nbsp;{new Date(time).toLocaleTimeString()}
            </Typography>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center" }}>
                <SmartScreenIcon sx={{ fontSize: 'medium' }} />
                &nbsp;{url}
            </Typography>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center" }}>
                <HttpIcon sx={{ fontSize: 'medium' }} />
                &nbsp;{inputs.length > 1 ? "POST" : "GET"}
            </Typography>
            <Typography variant="caption" sx={{ display: "flex", alignItems: "center" }}>
                {
                    returns === 'json' ? <DataObjectIcon sx={{ fontSize: 'medium' }} /> : <DataArrayIcon sx={{ fontSize: 'medium' }} />
                }
                &nbsp;{returns}
            </Typography>
            <form ref={formRef} target={name} method="POST">
                {inputs}
            </form>
            <iframe title={'apicallframe'} ref={frameRef} name={name} style={{ width: '100%', border: '1px solid #cccccc', background: 'white' }}></iframe>
        </div>

    );
}

export default ApiCall;