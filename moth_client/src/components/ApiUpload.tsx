import FileUploadOutlined from '@mui/icons-material/FileUploadOutlined';
import WarningAmberIcon from '@mui/icons-material/WarningAmber';
import { Button, IconButton } from '@mui/material';
import Accordion from '@mui/material/Accordion';
import AccordionDetails from '@mui/material/AccordionDetails';
import AccordionSummary from '@mui/material/AccordionSummary';
import Card from '@mui/material/Card';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogContentText from '@mui/material/DialogContentText';
import DialogTitle from '@mui/material/DialogTitle';
import TextField from '@mui/material/TextField';
import { Stack } from '@mui/system';
import * as React from 'react';
import { useEffect, useState } from 'react';
import ApiResponse from './ApiResponse';
import { IApiProperties } from './IApiProperties';
import { IResponseProps } from './IResponseProps';

const ApiUpload = (props: IApiProperties) => {

  const apiName = 'upload';
  const apiDesc = 'upload files to the device';
  const apiType = 'json';

  const { boxUrl, panels, pstate: status, handlePanel: handleChange, handleApiCall } = props;
  const [file, setFile] = useState<string>();
  const [data, setData] = useState<string>();
  const apiHref = `${boxUrl}/${apiName}`;

  const [responseProps, setResponseProps] = useState<IResponseProps>();
  const issueApiCall = () => {
    (document.getElementById('uploadform') as HTMLFormElement).submit();
  }

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {

      setResponseProps({
        time: Date.now(),
        href: apiHref,
        type: apiType,
        http: 'POST',
        data: props[apiName]
      });
    }

  }, [status, props[apiName]]);

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setFile(event.target.value);
  };
  const handleUploadFilePicked = (event: React.ChangeEvent<HTMLInputElement>) => {
    setData(event.target.value);
  };

  const [open, setOpen] = useState(false);
  const handleClickOpen = () => {
    setOpen(true);
  };
  const handleCancel = () => {
    setOpen(false);
  };
  const handleProceed = () => {
    setOpen(false);
    issueApiCall();
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
        <form id="uploadform" action={apiHref} method="POST" encType="multipart/form-data" target="uploadframe">
          <Stack>
            <iframe title="callframe" name="uploadframe" style={{ height: '60px', width: '100%', border: '1px solid gray' }} />
            <Card>
              <Stack>
                <TextField
                  required
                  label="file name"
                  name="file"
                  id="outlined-start-adornment"
                  size='small'
                  onChange={handleFileChange}
                  disabled={status === 'disconnected'}
                  value={file}
                />
                <TextField
                  type="text"
                  id="outlined-start-adornment"
                  size='small'
                  label={data && data !== "" ? data : "file"}
                  disabled={status === 'disconnected'}
                  InputProps={{
                    endAdornment: (
                      <IconButton component="label">
                        <FileUploadOutlined />
                        <input
                          type="file"
                          hidden
                          onChange={handleUploadFilePicked}
                          name="content"
                        />
                      </IconButton>
                    ),
                  }}
                />
                <Button disabled={status === 'disconnected'} variant="contained" endIcon={<WarningAmberIcon />} onClick={handleClickOpen}>
                  click to execute
                </Button>
                <Dialog
                  open={open}
                  onClose={handleCancel}
                  aria-labelledby="alert-dialog-title"
                  aria-describedby="alert-dialog-description"
                >
                  <DialogTitle id="alert-dialog-title">
                    do you really want to upload?
                  </DialogTitle>
                  <DialogContent>
                    <DialogContentText id="alert-dialog-description">
                      this call can be used to alter the iframe.html and login.html documents, continue at your own risk.
                    </DialogContentText>
                  </DialogContent>
                  <DialogActions>
                    <Button onClick={handleCancel} autoFocus>cancel</Button>
                    <Button onClick={handleProceed}>{apiName}</Button>
                  </DialogActions>
                </Dialog>
                {
                  (responseProps) ? <ApiResponse {...responseProps} /> : null
                }
              </Stack>
            </Card>
          </Stack>
        </form>
      </AccordionDetails>
    </Accordion>
  );
}

export default ApiUpload;