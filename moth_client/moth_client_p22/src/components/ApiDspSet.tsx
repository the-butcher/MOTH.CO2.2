import PlayCircleOutlineIcon from '@mui/icons-material/PlayCircleOutline';
import { Button, FormControl, InputLabel, MenuItem, Select } from '@mui/material';
import Accordion from '@mui/material/Accordion';
import AccordionDetails from '@mui/material/AccordionDetails';
import AccordionSummary from '@mui/material/AccordionSummary';
import Card from '@mui/material/Card';
import { Stack } from '@mui/system';
import { useEffect, useState } from 'react';
import { IDisplayProperties, IDisplayProperty } from '../util/IDisplayProperties';
import ApiResponse from './ApiResponse';
import { IApiProperties } from './IApiProperties';
import { IResponseProps } from './IResponseProps';

/**
 * - state (table|chart)
 * - theme (light|dark)
 * - value (table) > (co2|hpa|alt)
 * - value (chart) > (co2|deg|hum|hpa|alt|nrg)
 * - hours (chart) > (1|3|6|12|24)
 */

const displayProperties: IDisplayProperties[] = [
  {
    label: 'state',
    props: [
      {
        value: 0,
        label: 'table'
      },
      {
        value: 1,
        label: 'chart'
      },
      {
        value: 2,
        label: 'calib'
      }
    ]
  },
  {
    label: 'theme',
    props: [
      {
        value: 0,
        label: 'light'
      },
      {
        value: 1,
        label: 'dark'
      }
    ]
  },
  {
    label: 'value (table)',
    props: [
      {
        value: 0,
        label: 'co2'
      },
      {
        value: 1,
        label: 'pressure'
      },
      {
        value: 2,
        label: 'altitude'
      }
    ]
  },
  {
    label: 'value (chart)',
    props: [
      {
        value: 0,
        label: 'co2'
      },
      {
        value: 1,
        label: 'temperature'
      },
      {
        value: 2,
        label: 'humidity'
      },
      {
        value: 3,
        label: 'pressure'
      },
      {
        value: 4,
        label: 'altitude'
      },
      {
        value: 5,
        label: 'battery'
      }
    ]
  },
  {
    label: 'hours (chart)',
    props: [
      {
        value: 1,
        label: '1 hour'
      },
      {
        value: 3,
        label: '3 hours'
      },
      {
        value: 6,
        label: '6 hours'
      },
      {
        value: 12,
        label: '12 hours'
      },
      {
        value: 24,
        label: '24 hours'
      }
    ]
  }
];

const ApiDspSet = (props: IApiProperties) => {

  const apiName = 'dspset';
  const apiDesc = 'toggles various display aspects';
  const apiType = 'json';

  const { boxUrl, panels, handlePanel, handleApiCall } = props;

  const [propsIndexP, setPropsIndexP] = useState<number>(0);
  const [propsPropsP, setPropsPropsP] = useState<IDisplayProperty[]>(displayProperties[0].props);
  const [propsIndexV, setPropsIndexV] = useState<number>(0);
  const [responseProps, setResponseProps] = useState<IResponseProps>();

  const issueApiCall = () => {
    handleApiCall({
      href: boxUrl,
      call: apiName,
      meth: 'GET',
      type: apiType,
      qstr: {
        p: propsIndexP,
        v: displayProperties[propsIndexP].props[propsIndexV].value
      }
    });
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component`, props[apiName]);
    if (props[apiName]) {

      setResponseProps({
        time: Date.now(),
        href: `${boxUrl}/${apiName}?p=${propsIndexP}&v=${displayProperties[propsIndexP].props[propsIndexV].value}`,
        type: apiType,
        http: 'GET',
        data: props[apiName]
      });

    }

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [props[apiName]]);

  const handlePropsValuePChange = (propsValue: string) => {
    let _propsIndexP = 0;
    for (let i = 0; i < displayProperties.length; i++) {
      if (displayProperties[i].label === propsValue) {
        _propsIndexP = i;
        break;
      }
    }
    setPropsIndexP(_propsIndexP);
    const _propsPropsP = displayProperties[_propsIndexP].props;
    setPropsPropsP(_propsPropsP);
    setPropsIndexV(0);
  };

  const handlePropsValueVChange = (propsValue: string) => {
    let _propsIndexV = 0;
    for (let i = 0; i < displayProperties[propsIndexP].props.length; i++) {
      if (displayProperties[propsIndexP].props[i].label === propsValue) {
        _propsIndexV = i;
        break;
      }
    }
    setPropsIndexV(_propsIndexV);
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
            <FormControl variant="outlined">
              <InputLabel id="prop-label" size='small'>Display aspect</InputLabel>
              <Select
                size='small'
                labelId="props-value"
                id="demo-simple-select"
                value={displayProperties[propsIndexP].label}
                label="Display aspect"
                onChange={event => handlePropsValuePChange(event.target.value)}
              >
                {
                  displayProperties.map(prop => <MenuItem key={prop.label} value={prop.label}>{prop.label}</MenuItem>)
                }
              </Select>
            </FormControl>
            <FormControl variant="outlined">
              <InputLabel id="prop-label" size='small'>Display aspect</InputLabel>
              <Select
                size='small'
                labelId="props-value-value"
                id="demo-simple-select"
                value={displayProperties[propsIndexP].props[propsIndexV].label}
                label="Display aspect value"
                onChange={event => handlePropsValueVChange(event.target.value)}
              >
                {
                  propsPropsP.map(prop => <MenuItem key={prop.label} value={prop.label}>{prop.label}</MenuItem>)
                }
              </Select>
            </FormControl>
            <Button variant="contained" endIcon={<PlayCircleOutlineIcon />} onClick={issueApiCall}>
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

export default ApiDspSet;