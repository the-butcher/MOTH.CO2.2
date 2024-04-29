import PlayCircleOutlineIcon from '@mui/icons-material/PlayCircleOutline';
import WarningAmberIcon from '@mui/icons-material/WarningAmber';
import { Button } from '@mui/material';
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
import ApiResponse from './ApiResponse';
import { IApiSimpleProperties } from './IApiSimpleProperties';
import { IResponseProps } from './IResponseProps';

const ApiSimple = (props: IApiSimpleProperties) => {

  const { apiName, apiDesc, boxUrl, panels, pstate: status, handlePanel, handleApiCall } = props;
  const [responseProps, setResponseProps] = useState<IResponseProps>();
  const apiType = 'json';

  const confirmOrCall = () => {
    console.log('confirmOrCall');
    if (props.confirm) {
      handleClickOpen();
    } else {
      issueApiCall();
    }
  };

  const issueApiCall = () => {
    handleApiCall({
      href: boxUrl,
      call: apiName,
      meth: 'GET',
      type: apiType
    });
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, status, props[apiName]);
    if (props[apiName]) {
      setResponseProps({
        time: Date.now(),
        href: `${boxUrl}/${apiName}`,
        type: apiType,
        http: 'GET',
        data: props[apiName]
      });
    }

    // eslint-disable-next-line react-hooks/exhaustive-deps
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
    <Accordion expanded={panels.indexOf(apiName) >= 0} onChange={(event, expanded) => handlePanel(apiName, expanded)}>
      <AccordionSummary >
        <div>
          <div style={{ display: "flex", alignItems: "center" }} id={apiName}>/{apiName}</div>
          <div style={{ fontSize: '0.75em' }}>{apiDesc}</div>
        </div>
      </AccordionSummary>
      <AccordionDetails>
        <Card>
          <Stack>
            <Button disabled={status === 'disconnected'} variant="contained" endIcon={props.confirm ? <WarningAmberIcon /> : <PlayCircleOutlineIcon />} onClick={confirmOrCall}>
              click to execute
            </Button>
            {
              props.confirm ? <Dialog
                open={open}
                onClose={handleCancel}
                aria-labelledby="alert-dialog-title"
                aria-describedby="alert-dialog-description"
              >
                <DialogTitle id="alert-dialog-title">
                  {props.confirm.title}
                </DialogTitle>
                <DialogContent>
                  <DialogContentText id="alert-dialog-description">
                    {props.confirm.content}
                  </DialogContentText>
                </DialogContent>
                <DialogActions>
                  <Button onClick={handleCancel} autoFocus>cancel</Button>
                  <Button onClick={handleProceed}>{apiName}</Button>
                </DialogActions>
              </Dialog> : null
            }
            {
              (responseProps) ? <ApiResponse {...responseProps} /> : null
            }
          </Stack>
        </Card>
      </AccordionDetails>
    </Accordion>
  );
}

export default ApiSimple;