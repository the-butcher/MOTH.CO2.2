import ShowChartIcon from '@mui/icons-material/ShowChart';
import TocIcon from '@mui/icons-material/Toc';
import TuneIcon from '@mui/icons-material/Tune';
import { CssBaseline, IconButton, Paper, Stack, ThemeProvider } from '@mui/material';
import { useEffect, useState } from 'react';
import './App.css';
import TabConfig from './components/TabConfig';
import TabServer from './components/TabServer';
import TabValues from './components/TabValues';
import { ITabProperties } from './types/ITabProperties';
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

  const [tabProps] = useState<ITabProperties>({
    boxUrl
  });
  const [value, setValue] = useState('values');


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
        {
          value === 'values' ? <TabValues {...tabProps} /> : null
        }
        {
          value === 'config' ? <TabConfig {...tabProps} /> : null
        }
        {
          value === 'api' ? <TabServer {...tabProps} /> : null
        }
      </Stack >
    </ThemeProvider >
  );

};

export default RootApp;
