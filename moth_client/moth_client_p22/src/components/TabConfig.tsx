import { useEffect, useState } from 'react';
import { ITabProps } from '../types/ITabProps';
import { FormControl, FormHelperText, InputLabel, MenuItem, Paper, Select, Stack, TextField, Typography } from '@mui/material';
import { IDisplayConfig } from '../types/IDisplayConfig';
import ConfigChoice from './ConfigChoice';

const TabConfig = (props: ITabProps) => {

  const { boxUrl } = { ...props };

  const [configDisp, setConfigDisp] = useState<IDisplayConfig>({
    min: 3,
    ssc: true,
    tzn: 'CET-1CEST,M3.5.0,M10.5.0/3',
    co2: {
      wHi: 800,
      rHi: 1000,
      ref: 425,
      cal: 400,
      lpa: 0.5
    },
    deg: {
      rLo: 14,
      wLo: 19,
      wHi: 25,
      rHi: 30,
      off: 0.7,
      c2f: false
    },
    hum: {
      rLo: 25,
      wLo: 30,
      wHi: 60,
      rHi: 65
    },
    bme: {
      alt: 153,
      lpa: 0.25
    }
  });

  const getConfigDisp = () => {

    setConfigDisp({
      ...configDisp
    });

  }

  useEffect(() => {

    console.debug(`⚙ updating tab config component (dateRangeUser)`, configDisp);



    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [configDisp]);

  useEffect(() => {

    console.debug('✨ building tab config component');

    getConfigDisp();

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  return (
    <Stack direction={'column'} sx={{ margin: '5px', width: '800px', ...props.style }}>
      <Paper sx={{}} elevation={3}>
        <Stack direction={'column'} sx={{ padding: '10px' }}>
          <Typography sx={{ whiteSpace: 'nowrap' }}>group caption</Typography>
          <ConfigChoice
            caption='display update interval'
          />
          <ConfigChoice
            caption='update on significant change'
          />
          {/* <Stack direction={'row'} sx={{ alignItems: 'center' }}>
            <Typography>caption</Typography>
            <div style={{ flexGrow: 5 }}></div>
            <FormControl sx={{ m: 1, minWidth: 120, width: '400px' }}>
              <Select
                value={10}
                // onChange={handleChange}
                displayEmpty
                inputProps={{ 'aria-label': 'Without label' }}
              >
                <MenuItem value="">
                  <em>None</em>
                </MenuItem>
                <MenuItem value={10}>Ten</MenuItem>
                <MenuItem value={20}>Twenty</MenuItem>
                <MenuItem value={30}>Thirty</MenuItem>
              </Select>
            </FormControl>
          </Stack>
          <Stack direction={'row'} sx={{ alignItems: 'center' }}>
            <Typography>caption</Typography>
            <div style={{ flexGrow: 5 }}></div>
            <FormControl sx={{ m: 1, minWidth: 120, width: '600px' }}>
              <TextField
                type="number"
                value={'800'}
              />
            </FormControl>
          </Stack> */}
        </Stack>
      </Paper>
    </Stack>
  );

};

export default TabConfig;
