import WarningAmberIcon from '@mui/icons-material/WarningAmber';
import { Button, TextField } from '@mui/material';
import Accordion from '@mui/material/Accordion';
import AccordionDetails from '@mui/material/AccordionDetails';
import AccordionSummary from '@mui/material/AccordionSummary';
import Card from '@mui/material/Card';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogContentText from '@mui/material/DialogContentText';
import DialogTitle from '@mui/material/DialogTitle';
import { Stack } from '@mui/system';
import { useEffect, useState } from 'react';
import { IApiProperties } from './IApiProperties';

import ApiResponse from './ApiResponse';
import { IResponseProps } from './IResponseProps';

const ApiCalibrate = (props: IApiProperties) => {

  const apiName = 'calibrate';
  const apiDesc = 'calibrate the CO₂ sensor to a given reference value';

  const { boxUrl, panels, pstate: status, handlePanel: handleChange, handleApiCall } = props;

  const [ref, setRef] = useState<number>(420);
  const [responseProps, setResponseProps] = useState<IResponseProps>();



  const issueApiCall = () => {
    handleApiCall({
      href: `http://${boxUrl}`,
      call: apiName,
      meth: 'GET',
      qstr: {
        ref
      }
    });
  }

  const handleRefChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setRef(parseInt(event.target.value));
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {

      setResponseProps({
        time: Date.now(),
        href: `http://${boxUrl}/${apiName}?ref=${ref}`,
        type: 'json',
        http: 'GET',
        data: props[apiName]
      });
    }

  }, [status, props[apiName]]);

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
        <Card>
          <Stack>
            <TextField
              disabled={status == 'disconnected'}
              label="ref"
              value={ref}
              id="outlined-start-adornment"
              size='small'
              onChange={handleRefChange}
              required
            />
            <Button disabled={status == 'disconnected'} variant="contained" endIcon={<WarningAmberIcon />} onClick={handleClickOpen}>
              click to execute
            </Button>
            <Dialog
              open={open}
              onClose={handleCancel}
              aria-labelledby="alert-dialog-title"
              aria-describedby="alert-dialog-description"
            >
              <DialogTitle id="alert-dialog-title">do you really want to calibrate?</DialogTitle>
              <DialogContent>
                <DialogContentText id="alert-dialog-description">this call will perform calibration of the box\'s CO₂ sensor. please expose the sensor to an environment having the co2 concentration that is being used as reference. move away from the sensor during calibration to not let exhaled air alter CO₂ values.</DialogContentText>
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
      </AccordionDetails>
    </Accordion>
  );
}

export default ApiCalibrate;