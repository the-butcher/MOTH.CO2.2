import ShowChartIcon from '@mui/icons-material/ShowChart';
import TocIcon from '@mui/icons-material/Toc';
import TuneIcon from '@mui/icons-material/Tune';
import { Alert, CssBaseline, IconButton, Paper, Snackbar, Stack, ThemeProvider } from '@mui/material';
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
import { IMessage } from './types/ITabProps';

type VIEW_TYPE = 'values' | 'config' | 'server';

const RootApp = () => {

  // const boxUrl = `${window.location.origin}/api`; // when running directly from device
  const boxUrl = `http://192.168.0.66/api`; // when running directly from device

  /**
   * TODO :: initial step to establish and validate connection (i.e. do not show an empty chart)
   */

  const [viewType, setViewType] = useState<VIEW_TYPE>('values');
  const [alertMessage, setAlertMessage] = useState<IMessage>({
    message: '',
    severity: 'success',
    active: false
  });

  const handleClose = (event: React.SyntheticEvent | Event, reason?: string) => {
    if (reason === 'clickaway') {
      return;
    }
    setAlertMessage({
      ...alertMessage,
      active: false
    })
  };

  const handleAlertMessage = (_alertMessage: IMessage) => {

    console.debug(`📞 handling alert message`, _alertMessage);

    setAlertMessage(_alertMessage);

  }

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
    if (updates.disp && !compare(updates.disp.co2, tabConfigPropsRef.current.disp.co2, ['wHi', 'rHi'])) {
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
    if (updates.disp && !compare(updates.disp.deg, tabConfigPropsRef.current.disp.deg, ['rLo', 'wLo', 'wHi', 'rHi'])) {
      seriesDefUpdateRequired = true;
      (SERIES_DEFS.deg.colorMap as PiecewiseColorConfig).thresholds = [
        updates.disp.deg.rLo,
        updates.disp.deg.wLo,
        updates.disp.deg.wHi,
        updates.disp.deg.rHi
      ];
    }
    if (updates.disp && !compare(updates.disp.hum, tabConfigPropsRef.current.disp.hum, ['rLo', 'wLo', 'wHi', 'rHi'])) {
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
    handleUpdate: handleValuesUpdate,
    handleAlertMessage
  });
  const [tabValuesProps, setTabValuesProps] = useState<ITabValuesProps>(tabValuesPropsRef.current);

  const tabConfigPropsRef = useRef<ITabConfigProps>({
    boxUrl,
    disp: DISP_CONFIG_DEFAULT,
    wifi: WIFI_CONFIG_DEFAULT,
    mqtt: MQTT_CONFIG_DEFAULT,
    handleUpdate: handleConfigUpdate,
    handleAlertMessage
  });
  const [tabConfigProps, setTabConfigProps] = useState<ITabConfigProps>(tabConfigPropsRef.current);

  useEffect(() => {
    console.debug('✨ building root component');
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  return (
    <ThemeProvider theme={ThemeUtil.createTheme()}>
      <CssBaseline />
      <Snackbar
        open={alertMessage.active}
        // autoHideDuration={5000}
        onClose={handleClose}
        sx={{ left: '72px' }}
      >
        <Alert
          onClose={handleClose}
          severity={alertMessage.severity}
          variant='filled'
          sx={{ width: '100%' }}
        >
          {alertMessage.message}
        </Alert>
      </Snackbar>
      <Stack direction={'row'} spacing={0} sx={{ height: '100%' }}>
        <Paper elevation={4} sx={{ display: 'flex', flexDirection: 'column', position: 'fixed', marginTop: '5px', backgroundColor: '#FAFAFA', border: '1px solid #DDDDDD' }}>
          <IconButton size='small' title='data' onClick={() => setViewType('values')} sx={{ color: viewType === 'values' ? 'black' : 'gray' }}>
            <ShowChartIcon />
          </IconButton>
          <IconButton size='small' title='configuration' onClick={() => setViewType('config')} sx={{ color: viewType === 'config' ? 'black' : 'gray' }}>
            <TuneIcon />
          </IconButton>
          <IconButton size='small' title='server api' onClick={() => setViewType('server')} sx={{ color: viewType === 'server' ? 'black' : 'gray' }}>
            <TocIcon />
          </IconButton>
        </Paper>
        <div style={{ minWidth: '45px' }}></div>
        <TabValues {...tabValuesProps} style={{ display: viewType === 'values' ? 'flex' : 'none' }} />
        <TabConfig {...tabConfigProps} style={{ display: viewType === 'config' ? 'flex' : 'none' }} />
        <TabServer boxUrl={boxUrl} handleAlertMessage={handleAlertMessage} style={{ display: viewType === 'server' ? 'flex' : 'none' }} />
      </Stack >
    </ThemeProvider >
  );

};

export default RootApp;
