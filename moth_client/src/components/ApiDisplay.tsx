import PlayCircleOutlineIcon from '@mui/icons-material/PlayCircleOutline';
import { Button, FormControl, InputLabel, MenuItem, Select } from '@mui/material';
import Accordion from '@mui/material/Accordion';
import AccordionDetails from '@mui/material/AccordionDetails';
import AccordionSummary from '@mui/material/AccordionSummary';
import Card from '@mui/material/Card';
import TextField from '@mui/material/TextField';
import { Stack } from '@mui/system';
import { ChangeEvent, KeyboardEvent, useEffect, useState } from 'react';
import ApiResponse from './ApiResponse';
import { IApiProperties } from './IApiProperties';
import { IResponseProps } from './IResponseProps';


const ApiDisplay = (props: IApiProperties) => {

  const apiName = 'display';
  const apiDesc = 'toggles various display aspects';
  const apiType = 'json';

  const { boxUrl, panels, pstate: status, handlePanel: handleChange, handleApiCall } = props;

  const [display, setDisplay] = useState<string>("");
  const [responseProps, setResponseProps] = useState<IResponseProps>();

  const issueApiCall = () => {
    handleApiCall({
      href: boxUrl,
      call: apiName,
      meth: 'GET',
      type: apiType,
      qstr: {
        display: display
      }
    });
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {

      setResponseProps({
        time: Date.now(),
        href: `${boxUrl}/${apiName}?display=${display}`,
        type: apiType,
        http: 'GET',
        data: props[apiName]
      });

    }

  }, [status, props[apiName]]);

  const handleDisplayChange = (display: string) => {
    setDisplay(display);
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
            <FormControl variant="outlined">
              <InputLabel id="prop-label" size='small'>Display aspect</InputLabel>
              <Select
                disabled={status === 'disconnected'}
                size='small'
                labelId="prop-label"
                id="demo-simple-select"
                value={display}
                label="Display aspect"
                onChange={event => handleDisplayChange(event.target.value)}
              >
                <MenuItem key="state_table" value="state_table">State: Table</MenuItem>
                <MenuItem key="state_chart" value="state_chart">State: Chart</MenuItem>
                <MenuItem key="theme_light" value="theme_light">Theme: Light</MenuItem>
                <MenuItem key="theme_dark" value="theme_dark">Theme: Dark</MenuItem>
                <MenuItem key="value_co2" value="value_co2">Value: CO₂ (when state = table)</MenuItem>
                <MenuItem key="value_deg" value="value_deg">Value: Temperature (when state = table)</MenuItem>
                <MenuItem key="value_hum" value="value_hum">Value: Humidity (when state = table)</MenuItem>
                <MenuItem key="value_hpa" value="value_hpa">Value: Pressure (when state = table)</MenuItem>

              </Select>
            </FormControl>
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

export default ApiDisplay;