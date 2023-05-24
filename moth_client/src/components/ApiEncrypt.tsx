import PlayCircleOutlineIcon from '@mui/icons-material/PlayCircleOutline';
import { Button } from '@mui/material';
import Accordion from '@mui/material/Accordion';
import AccordionDetails from '@mui/material/AccordionDetails';
import AccordionSummary from '@mui/material/AccordionSummary';
import Card from '@mui/material/Card';
import TextField from '@mui/material/TextField';
import { Stack } from '@mui/system';
import { useEffect, useState } from 'react';
import ApiResponse from './ApiResponse';
import { IApiProperties } from './IApiProperties';
import { IResponseProps } from './IResponseProps';


const ApiEncrypt = (props: IApiProperties) => {

  const apiName = 'encrypt';
  const apiDesc = 'encrypt a string value';
  const apiType = 'json';

  const { boxUrl, panels, pstate: status, handlePanel: handleChange, handleApiCall } = props;

  const [value, setValue] = useState<string>();
  const [responseProps, setResponseProps] = useState<IResponseProps>();

  const issueApiCall = () => {
    handleApiCall({
      href: boxUrl,
      call: apiName,
      meth: 'GET',
      type: apiType,
      qstr: {
        value: value
      }
    });
  }

  const handleValueChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setValue(event.target.value);
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {

      setResponseProps({
        time: Date.now(),
        href: `${boxUrl}/${apiName}?value=${value}`,
        type: apiType,
        http: 'GET',
        data: props[apiName]
      });
    }

  }, [status, props[apiName]]);

  return (
    <Accordion expanded={panels.indexOf(apiName) >= 0} onChange={handleChange(apiName)}>
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
              label="value"
              id="outlined-start-adornment"
              size='small'
              onChange={handleValueChange}
              required
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

export default ApiEncrypt;