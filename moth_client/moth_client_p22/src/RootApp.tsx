import ShowChartIcon from '@mui/icons-material/ShowChart';
import TocIcon from '@mui/icons-material/Toc';
import TuneIcon from '@mui/icons-material/Tune';
import { CssBaseline, IconButton, Paper, Stack, ThemeProvider } from '@mui/material';
import { useEffect, useRef, useState } from 'react';
import './App.css';
import TabConfig from './components/TabConfig';
import TabServer from './components/TabServer';
import TabValues from './components/TabValues';
import { SERIES_DEFS } from './types/ISeriesDef';
import { ITabValuesProps } from './types/ITabValuesProps';
import { ThemeUtil } from './util/ThemeUtil';

const RootApp = () => {

  // const boxUrl = `${window.location.origin}/api`; // when running directly from device
  const boxUrl = `http://192.168.0.73/api`; // when running directly from device

  /**
   * TODO :: colors for all seriesDefs
   * TODO :: trigger data range change from date time picker
   * TODO :: strategy for not having to re-evaluate (lots of http requests) date range often and for keeping historic data in cache
   * TODO :: create form around device configuration (display, wifi, mqtt) and implement reassembly of those values for re-upload to device
   */

  const [value, setValue] = useState<string>('values');

  const tabValuesUpdate = (updates: Partial<ITabValuesProps>) => {

    console.debug(`📞 handling tab values update`, updates);

    tabValuesPropsRef.current = {
      ...tabValuesPropsRef.current,
      ...updates
    };
    setTabValuesProps(tabValuesPropsRef.current);

  };

  const tabValuesPropsRef = useRef<ITabValuesProps>({
    boxUrl,
    dateRangeData: [new Date(), new Date()],
    dateRangeUser: [new Date(), new Date()],
    latest: {
      time: '',
      co2_lpf: 0,
      deg: 0,
      hum: 0,
      co2_raw: 0,
      hpa: 0,
      bat: 0
    },
    records: [],
    seriesDef: SERIES_DEFS.co2Lpf,
    handleUpdate: tabValuesUpdate
  })
  const [tabValuesProps, setTabValuesProps] = useState<ITabValuesProps>(tabValuesPropsRef.current);

  useEffect(() => {
    console.debug('✨ building root component');
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // const handleButtonPress = () => {

  //   const str = JSON.stringify({
  //     "key1": "value1",
  //     "key2": "value2"
  //   });
  //   const bytes = new TextEncoder().encode(str);
  //   const blob = new Blob([bytes], {
  //     type: "application/json;charset=utf-8"
  //   });

  //   var file = new File([blob], "file_name", { lastModified: Date.now() });

  //   console.log('file', file);

  //   const request = new XMLHttpRequest();
  //   const formData = new FormData();

  //   request.open("POST", boxUrl + '/test', true);
  //   request.onreadystatechange = () => {
  //     if (request.readyState === 4 && request.status === 200) {
  //       console.log(request.responseText);
  //     }
  //   };
  //   formData.append("file", "config/test.json");
  //   formData.append("content", file);
  //   request.send(formData);

  // }

  return (
    <ThemeProvider theme={ThemeUtil.createTheme()}>
      <CssBaseline />
      <Stack direction={'row'} spacing={0} sx={{ height: '100%' }}>
        <Paper elevation={4} sx={{ display: 'flex', flexDirection: 'column', position: 'fixed', marginTop: '5px', backgroundColor: '#FAFAFA', border: '1px solid #DDDDDD' }}>
          <IconButton size='small' aria-label="values" onClick={() => setValue('values')}>
            <ShowChartIcon />
          </IconButton>
          <IconButton size='small' aria-label="config" onClick={() => setValue('config')}>
            <TuneIcon />
          </IconButton>
          <IconButton size='small' aria-label="api" onClick={() => setValue('api')}>
            <TocIcon />
          </IconButton>
        </Paper>
        <div style={{ minWidth: '45px' }}></div>
        <TabValues {...tabValuesProps} style={{ display: value === 'values' ? 'block' : 'none' }} />
        <TabConfig boxUrl={boxUrl} style={{ display: value === 'config' ? 'block' : 'none' }} />
        {
          value === 'api' ? <TabServer boxUrl={boxUrl} /> : null
        }
      </Stack >
    </ThemeProvider >
  );

};

export default RootApp;
