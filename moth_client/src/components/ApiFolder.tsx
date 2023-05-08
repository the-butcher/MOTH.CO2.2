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


const ApiFolder = (props: IApiProperties) => {

  const apiName = 'folder';
  const apiDesc = 'list the contents of a folder on the device';

  const { boxUrl, panels, pstate: status, handlePanel: handleChange, handleApiCall } = props;

  const [folder, setFolder] = useState<string>();
  const [responseProps, setResponseProps] = useState<IResponseProps>();

  const issueApiCall = () => {
    handleApiCall({
      href: `http://${boxUrl}`,
      call: apiName,
      meth: 'GET',
      qstr: folder && folder !== "" ? { folder } : null
    });
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {

      let href = `http://${boxUrl}/${apiName}`;
      if (folder && folder !== "") {
        href += `?folder${folder}`
      }

      setResponseProps({
        time: Date.now(),
        href,
        type: 'json',
        http: 'GET',
        data: props[apiName]
      });

    }

  }, [status, props[apiName]]);

  const handleFolderChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setFolder(event.target.value);
  };

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
              disabled={status == 'disconnected'}
              label="folder"
              id="outlined-start-adornment"
              size='small'
              onChange={handleFolderChange}
              helperText="optional folder name. specify to list the contents of a specific folder. leave empty to get the root folders contents"
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

export default ApiFolder;