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
import { KeyboardEvent, useEffect, useState } from 'react';
import { IApiProperties } from './IApiProperties';

import ApiResponse from './ApiResponse';
import { IResponseProps } from './IResponseProps';

const ApiCo2Cal = (props: IApiProperties) => {

  const apiName = 'co2cal';
  const apiDesc = 'calibrate the CO₂ sensor to a given reference value';
  const apiType = 'json';

  const { boxUrl, panels, handlePanel, handleApiCall } = props;

  const [ref, setRef] = useState<number>(0);
  const [responseProps, setResponseProps] = useState<IResponseProps>();
  const [calNodes, setCalNodes] = useState<JSX.Element[]>([]);
  const [devDivPos, setDevDivPos] = useState<number>(0);
  const [devHeight, setDevHeight] = useState<number>(1);
  const [avgValue, setAvgValue] = useState<number>(0);
  const [devValue, setDevValue] = useState<number>(0);

  const handleKeyUp = (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter') {
      handleClickOpen();
    }
  }

  const issueApiCall = () => {
    handleApiCall({
      href: boxUrl,
      call: apiName,
      meth: 'GET',
      type: apiType,
      qstr: {
        ref
      }
    });
  }

  const handleRefChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setRef(parseInt(event.target.value));
  };

  const co2calHeight = 300;
  const toBarHeight = (value: number, min: number, max: number): number => {
    return (value - min) * co2calHeight / (max - min);
  }
  const toValHeight = (value: number, min: number, max: number): number => {
    return value * co2calHeight / (max - min);
  }

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {

      setResponseProps({
        time: Date.now(),
        href: `${boxUrl}/${apiName}?ref=${ref}`,
        type: apiType,
        http: 'GET',
        data: props[apiName]
      });

      const _calNodes: JSX.Element[] = [];

      if (props[apiName]) {
        const co2cal = JSON.parse(props[apiName]);
        const min = co2cal.minValue - 20;
        const max = co2cal.maxValue + 20;
        const _avgHeight = toBarHeight(co2cal.avgValue, min, max);
        const _devHeight = toValHeight(co2cal.devValue, min, max);
        setDevDivPos(co2calHeight - _avgHeight - _devHeight);
        setDevHeight(_devHeight * 2);
        setAvgValue(co2cal.avgValue);
        setDevValue(co2cal.devValue);
        let keyIndex = 0;
        co2cal.values.forEach((value: number) => {
          const barHeight = toBarHeight(value, min, max);
          _calNodes.push(<div key={`idx_${String(keyIndex++).padStart(2, '0')}`} style={{ height: `${co2calHeight}px`, flexGrow: '3', display: 'flex', flexDirection: 'column', margin: '0px 5px' }}>
            <div style={{ flexGrow: '10' }}></div>
            <div style={{ minHeight: `${barHeight}px`, backgroundColor: '#999999', opacity: '0.95' }}></div>
          </div>);
        });
        setCalNodes(_calNodes);
      }
      // { "code": 200, "minValue": 622, "maxValue": 777, "avgValue": 682, "devValue": 44, "refValue": 0, "values": [684, 700, 641, 641, 654, 622, 674, 700, 727, 777] }
    }

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [props[apiName]]);

  const [open, setOpen] = useState(false);
  const handleClickOpen = () => {
    if (ref === 0) {
      issueApiCall();
    } else {
      setOpen(true);
    }
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
      <AccordionSummary>
        <div>
          <div id={apiName}>/{apiName}</div>
          <div style={{ fontSize: '0.75em' }}>{apiDesc}</div>
        </div>
      </AccordionSummary>
      <AccordionDetails>
        <Card sx={{ position: 'relative', height: `${co2calHeight + 20}px`, padding: '10px 0px' }}>
          <div style={{ position: 'absolute', width: 'calc(100% - 10px)', left: '5px', top: `${devDivPos}px`, height: `${devHeight}px`, display: 'flex', flexDirection: 'row', padding: '0px 5px', backgroundColor: 'gray', opacity: '0.5' }}>

          </div>
          <div style={{ position: 'absolute', width: '100%', display: 'flex', flexDirection: 'row', padding: '0px 5px' }}>
            <div style={{ height: `${co2calHeight}px`, flexGrow: '5', margin: '0px 5px' }}>

            </div>
            {calNodes}
          </div>
          <div style={{ position: 'absolute', width: 'calc(100% - 10px)', left: '5px', top: `${devDivPos}px`, height: `${devHeight}px`, display: 'flex', flexDirection: 'row', padding: '0px 5px' }}>
            <div style={{ position: 'absolute', top: '50%', width: 'calc(100% - 10px)', display: 'flex', flexDirection: 'column', transform: 'translateY(-50%)', opacity: '1' }}>
              <div>{avgValue}</div>
              <div style={{ height: '1px', flexGrow: '5', backgroundColor: 'black' }}></div>
              <div>{devValue}</div>
            </div>
          </div>
        </Card>
        <Card>
          <Stack>
            <TextField
              label="ref"
              value={ref}
              id="outlined-start-adornment"
              size='small'
              onChange={handleRefChange}
              required
              onKeyUp={handleKeyUp}
            />
            <Button variant="contained" endIcon={<WarningAmberIcon />} onClick={handleClickOpen}>
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
      </AccordionDetails >
    </Accordion >
  );
}

export default ApiCo2Cal;