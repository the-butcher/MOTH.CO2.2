import ShowChartIcon from '@mui/icons-material/ShowChart';
import TocIcon from '@mui/icons-material/Toc';
import TuneIcon from '@mui/icons-material/Tune';
import { CssBaseline, IconButton, Paper, Stack, ThemeProvider } from '@mui/material';
import { useEffect, useRef, useState } from 'react';
import TabConfig from './components/TabConfig';
import TabServer from './components/TabServer';
import TabValues from './components/TabValues';
import { SERIES_DEFS } from './types/ISeriesDef';
import { ITabValuesProps } from './types/ITabValuesProps';
import { ThemeUtil } from './util/ThemeUtil';
import { ITabConfigProps } from './types/ITabConfigProps';
import { DISP_CONFIG_DEFAULT } from './types/IDispConfig';
import { WIFI_CONFIG_DEFAULT } from './types/IWifiConfig';
import { MQTT_CONFIG_DEFAULT } from './types/IMqttConfig';
import { PiecewiseColorConfig } from '@mui/x-charts/models/colorMapping';

type VIEW_TYPE = 'values' | 'config' | 'server';

const RootApp = () => {

  const boxUrl = `${window.location.origin}/api`; // when running directly from device
  // const boxUrl = `http://192.168.0.66/api`; // when running directly from device
  // const boxUrl = `http://192.168.31.194/api`; // HotelSirius

  /**
   * TODO :: strategy for not having to re-evaluate (lots of http requests) date range often and for keeping historic data in cache
   * TODO :: create form around device configuration (display, wifi, mqtt) and implement reassembly of those values for re-upload to device
   */

  const [viewType, setViewType] = useState<VIEW_TYPE>('values');

  const handleValuesUpdate = (updates: Partial<ITabValuesProps>) => {

    console.debug(`📞 handling tab values update`, updates);

    tabValuesPropsRef.current = {
      ...tabValuesPropsRef.current,
      ...updates
    };
    setTabValuesProps(tabValuesPropsRef.current);

  };

  function compare<T, K extends keyof T>(a: T, b: T, keys: K[]): boolean {
    for (let key of keys) {
      if (!Object.is(a[key], b[key])) {
        return false;
      }
    }
    return true;
  }

  const handleConfigUpdate = (updates: Partial<ITabConfigProps>) => {

    console.debug(`📞 handling tab config update`, updates);

    // updates to the display config
    let seriesDefUpdateRequired = false;
    if (!compare(updates.disp.co2, tabConfigPropsRef.current.disp.co2, ['wHi', 'rHi'])) {
      seriesDefUpdateRequired = true;
      (SERIES_DEFS.co2Lpf.colorMap as PiecewiseColorConfig).thresholds = [
        updates.disp.co2.wHi,
        updates.disp.co2.rHi
      ];
      (SERIES_DEFS.co2Raw.colorMap as PiecewiseColorConfig).thresholds = [
        updates.disp.co2.wHi,
        updates.disp.co2.rHi
      ];
    }
    if (!compare(updates.disp.deg, tabConfigPropsRef.current.disp.deg, ['rLo', 'wLo', 'wHi', 'rHi'])) {
      seriesDefUpdateRequired = true;
      (SERIES_DEFS.deg.colorMap as PiecewiseColorConfig).thresholds = [
        updates.disp.deg.rLo,
        updates.disp.deg.wLo,
        updates.disp.deg.wHi,
        updates.disp.deg.rHi
      ];
      console.log('SERIES_DEFS.deg.colorMap', SERIES_DEFS.deg.colorMap);
    }
    if (!compare(updates.disp.hum, tabConfigPropsRef.current.disp.hum, ['rLo', 'wLo', 'wHi', 'rHi'])) {
      seriesDefUpdateRequired = true;
      (SERIES_DEFS.hum.colorMap as PiecewiseColorConfig).thresholds = [
        updates.disp.hum.rLo,
        updates.disp.hum.wLo,
        updates.disp.hum.wHi,
        updates.disp.hum.rHi
      ];
    }

    tabConfigPropsRef.current = {
      ...tabConfigPropsRef.current,
      ...updates
    };
    setTabConfigProps(tabConfigPropsRef.current);

    if (seriesDefUpdateRequired) {
      handleValuesUpdate({
        seriesDef: SERIES_DEFS[tabValuesProps.seriesDef.id]
      });
    }

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
    handleUpdate: handleValuesUpdate
  });
  const [tabValuesProps, setTabValuesProps] = useState<ITabValuesProps>(tabValuesPropsRef.current);

  const tabConfigPropsRef = useRef<ITabConfigProps>({
    boxUrl,
    disp: DISP_CONFIG_DEFAULT,
    wifi: WIFI_CONFIG_DEFAULT,
    mqtt: MQTT_CONFIG_DEFAULT,
    handleUpdate: handleConfigUpdate
  });
  const [tabConfigProps, setTabConfigProps] = useState<ITabConfigProps>(tabConfigPropsRef.current);

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

  //   var file = new File([blob], "file_name", {lastModified: Date.now() });

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
          <IconButton size='small' title='data' onClick={() => setViewType('values')}>
            <ShowChartIcon />
          </IconButton>
          <IconButton size='small' title='configuration' onClick={() => setViewType('config')}>
            <TuneIcon />
          </IconButton>
          <IconButton size='small' title='server api' onClick={() => setViewType('server')}>
            <TocIcon />
          </IconButton>
        </Paper>
        <div style={{ minWidth: '45px' }}></div>
        <TabValues {...tabValuesProps} style={{ display: viewType === 'values' ? 'flex' : 'none' }} />
        <TabConfig {...tabConfigProps} style={{ display: viewType === 'config' ? 'flex' : 'none' }} />
        <TabServer boxUrl={boxUrl} style={{ display: viewType === 'server' ? 'flex' : 'none' }} />
      </Stack >
    </ThemeProvider >
  );

};

export default RootApp;
