import PlayCircleOutlineIcon from '@mui/icons-material/PlayCircleOutline';
import { Button } from '@mui/material';
import Accordion from '@mui/material/Accordion';
import AccordionDetails from '@mui/material/AccordionDetails';
import AccordionSummary from '@mui/material/AccordionSummary';
import Card from '@mui/material/Card';
import TextField from '@mui/material/TextField';
import { Stack } from '@mui/system';
import { KeyboardEvent, useEffect, useState } from 'react';
import ApiResponse from './ApiResponse';
import { IApiProperties } from './IApiProperties';
import { IResponseProps } from './IResponseProps';

interface IApiDeleteProps extends IApiProperties {
  apiProp: 'file' | 'folder';
  apiName: string;
}

const ApiDelete = (props: IApiDeleteProps) => {

  const apiName = props.apiName;
  const apiProp = props.apiProp;
  const apiDesc = `delete ${apiProp}s from the device`;
  const apiType = 'json';

  const { boxUrl, panels, pstate: status, handlePanel, handleApiCall } = props;

  const [param, setParam] = useState<string>();
  const [responseProps, setResponseProps] = useState<IResponseProps>();

  const handleKeyUp = (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter') {
      issueApiCall();
    }
  }

  const issueApiCall = () => {
    let qstr = {};
    qstr[apiProp] = param;
    handleApiCall({
      href: boxUrl,
      call: apiName,
      meth: 'GET',
      type: apiType,
      qstr
    });
  }

  const handleParamChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setParam(event.target.value);
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {
      let href = `${boxUrl}/${apiName}?${apiProp}=${param}`;
      setResponseProps({
        time: Date.now(),
        href,
        type: apiType,
        http: 'GET',
        data: props[apiName]
      });
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [status, props[apiName]]);

  return (
    <Accordion expanded={panels.indexOf(apiName) >= 0} onChange={(event, expanded) => handlePanel(apiName, expanded)}>
      <AccordionSummary>
        <div>
          <div id={apiName}>/{apiName}</div>
          <div style={{ fontSize: '0.75em' }}>{apiDesc}</div>
        </div>
      </AccordionSummary>
      <AccordionDetails>
        <Card>
          <Stack>
            <TextField
              disabled={status === 'disconnected'}
              label={apiProp}
              id="outlined-start-adornment"
              size='small'
              onChange={handleParamChange}
              required
              onKeyUp={handleKeyUp}
            />
            <Button disabled={status === 'disconnected'} variant="contained" endIcon={<PlayCircleOutlineIcon />} onClick={issueApiCall}>
              click to execute
            </Button>
            {
              (responseProps) ? <ApiResponse {...responseProps} /> : null
            }
          </Stack>
        </Card>
      </AccordionDetails>
    </Accordion>
  );
}

export default ApiDelete;