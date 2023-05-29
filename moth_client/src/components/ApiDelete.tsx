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


const ApiDelete = (props: IApiProperties) => {

  const apiName = 'delete';
  const apiDesc = 'delete files or folders from the device';
  const apiType = 'json';

  const { boxUrl, panels, pstate: status, handlePanel: handleChange, handleApiCall } = props;

  const [file, setFile] = useState<string>();
  const [folder, setFolder] = useState<string>();
  const [responseProps, setResponseProps] = useState<IResponseProps>();

  const issueApiCall = () => {
    handleApiCall({
      href: boxUrl,
      call: apiName,
      meth: 'GET',
      type: apiType,
      qstr: {
        file,
        folder
      }
    });
  }

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setFile(event.target.value);
  };

  const handleFolderChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setFolder(event.target.value);
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {

      let href = `${boxUrl}/${apiName}`;
      if (file !== '') {
        href += `?file=${file}`;
      } else if (folder !== '') {
        href += `?folder=${folder}`;
      }

      setResponseProps({
        time: Date.now(),
        href,
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
              label="file"
              id="outlined-start-adornment"
              size='small'
              onChange={handleFileChange}
              required
            />
            <TextField
              disabled={status === 'disconnected'}
              label="folder"
              id="outlined-start-adornment"
              size='small'
              onChange={handleFolderChange}
              required
            />
            <Button disabled={status == 'disconnected'} variant="contained" endIcon={<PlayCircleOutlineIcon />} onClick={issueApiCall}>
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