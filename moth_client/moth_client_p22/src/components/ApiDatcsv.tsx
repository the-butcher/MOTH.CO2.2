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


const ApiDatcsv = (props: IApiProperties) => {

  const apiName = 'datcsv';
  const apiDesc = 'get historic measurements as csv data';
  const apiType = 'csv';

  const { boxUrl, panels, pstate: status, handlePanel, handleApiCall } = props;

  const [file, setFile] = useState<string>();
  const [responseProps, setResponseProps] = useState<IResponseProps>();

  const handleKeyUp = (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter') {
      issueApiCall();
    }
  }

  const issueApiCall = () => {
    handleApiCall({
      href: boxUrl,
      call: apiName,
      meth: 'GET',
      type: apiType,
      qstr: file && file !== "" ? { file } : null
    });
  };

  useEffect(() => {

    let href = `${boxUrl}/${apiName}`;
    if (file && file !== "") {
      href += `?file=${file}`
    }

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {
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

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setFile(event.target.value);
  };


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
              required
              disabled={status === 'disconnected'}
              label="file"
              id="outlined-start-adornment"
              size='small'
              onChange={handleFileChange}
              onKeyUp={handleKeyUp}
              helperText="dat file name. specifiy to get the content of a specific dat file."
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

export default ApiDatcsv;