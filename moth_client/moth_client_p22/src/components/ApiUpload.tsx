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
import { IApiProperties } from '../types/IApiProperties';
import { IResponseProps } from '../types/IResponseProps';

/**
 * component, renders a file name and an actual file picker
 * targets the 'api/upload' endpoint
 * @param props
 * @returns
 */
const ApiUpload = (props: IApiProperties) => {

  const apiName = 'upload';
  const apiDesc = 'upload files to the device';
  const apiType = 'json';

  const { boxUrl, panels, handlePanel } = props;

  const [file, setFile] = useState<string>();
  const [data, setData] = useState<string>();

  const apiHref = `${boxUrl}/${apiName}`;

  const [responseProps, setResponseProps] = useState<IResponseProps>();
  const issueApiCall = () => {
    (document.getElementById('uploadform') as HTMLFormElement).submit();
  }

  /**
   * react hook (props[apiName])
   */
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

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [props[apiName]]);

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setFile(event.target.value);
  };
  const handleUploadFilePicked = (event: React.ChangeEvent<HTMLInputElement>) => {
    setData(event.target.value);
  };

  const [dialogOpen, setDialogOpen] = useState<boolean>(false);
  const handleDialogOpen = () => {
    setDialogOpen(true);
  };
  const handleDialogCancel = () => {
    setDialogOpen(false);
  };
  const handleDialogCommit = () => {
    setDialogOpen(false);
    issueApiCall();
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
        <form id="uploadform" action={apiHref} method="POST" encType="multipart/form-data" target="uploadframe">
          <Stack>
            <iframe title="callframe" name="uploadframe" style={{ height: '60px', width: '100%', border: '1px solid gray' }} />
            <Card>
              <Stack>
                <TextField
                  required
                  label="file name"
                  name="file"
                  size='small'
                  onChange={handleFileChange}
                  value={file}
                />
                <TextField
                  type="text"
                  size='small'
                  label={data && data !== "" ? data : "file"}
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
                <Button variant="contained" endIcon={<WarningAmberIcon />} onClick={handleDialogOpen}>
                  click to execute
                </Button>
                <Dialog
                  open={dialogOpen}
                  onClose={handleDialogCancel}
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
                    <Button onClick={handleDialogCancel} autoFocus>cancel</Button>
                    <Button onClick={handleDialogCommit}>{apiName}</Button>
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