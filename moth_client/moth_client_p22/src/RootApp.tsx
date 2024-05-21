import ShowChartIcon from '@mui/icons-material/ShowChart';
import TocIcon from '@mui/icons-material/Toc';
import TuneIcon from '@mui/icons-material/Tune';
import { Alert, CssBaseline, IconButton, Paper, Snackbar, Stack, ThemeProvider } from '@mui/material';
import { useEffect, useRef, useState } from 'react';
import TabConfig from './components/TabConfig';
import TabServer from './components/TabServer';
import TabValues from './components/TabValues';
import { DISP_CONFIG_DEFAULT } from './types/IDispConfig';
import { MQTT_CONFIG_DEFAULT } from './types/IMqttConfig';
import { IPiecewiseColorConfig, SERIES_DEFS } from './types/ISeriesDef';
import { IStatus } from './types/IStatus';
import { ITabConfigProps } from './types/ITabConfigProps';
import { IMessage } from './types/ITabProps';
import { ITabValuesProps } from './types/ITabValuesProps';
import { WIFI_CONFIG_DEFAULT } from './types/IWifiConfig';
import { JsonLoader } from './util/JsonLoader';
import { ThemeUtil } from './util/ThemeUtil';
import { CERT_CONFIG_DEFAULT } from './types/ICertConfig';

type VIEW_TYPE = 'values' | 'config' | 'server';

const theme = ThemeUtil.createTheme();

const RootApp = () => {

  const boxUrl = `${window.location.origin}/api`; // when running directly from device
  // const boxUrl = `http://192.168.0.178/api`; // when running directly from device

  /**
   * steps needed to deploy a new version
   * -- build (will create gzipped versions of root.html and root.js)
   * -- delete non-zipped versions and rename zipped
   * -- replace in the SD/server directory
   * -- upload to device from SD/server directory
   */

  const [viewType, setViewType] = useState<VIEW_TYPE>('values');
  const [alertMessage, setAlertMessage] = useState<IMessage>({
    message: '',
    severity: 'success',
    active: false
  });

  const loadStatusToRef = useRef<number>(-1);
  const [status, setStatus] = useState<IStatus>();

  /**
   * load device status
   */
  const loadStatus = () => {

    new JsonLoader().load(`${boxUrl}/status`).then((status: IStatus) => {
      setStatus(status);
      handleAlertMessage({
        ...alertMessage,
        active: false
      })
    }).catch((e: Error) => {
      console.error('failed to connect to device', e);
      handleAlertMessage({
        message: e.message ? e.message : 'failed to connect to device',
        severity: 'error',
        active: true
      });
      window.clearTimeout(loadStatusToRef.current);
      loadStatusToRef.current = window.setTimeout(() => {
        loadStatus();
      }, 60000);
    });

  }

  /**
   * handle closing of the snackbar message
   * @param event
   * @param reason
   * @returns
   */
  const handleSnackbarClose = (event: React.SyntheticEvent | Event, reason?: string) => {
    console.debug(`📞 handling snackbar close (event)`, event);
    if (reason === 'clickaway') {
      return;
    }
    setAlertMessage({
      ...alertMessage,
      active: false
    })
  };

  /**
   * handle an incoming alert message
   * set state, which causes the snackbar and alert message to be shown
   * @param _alertMessage
   */
  const handleAlertMessage = (_alertMessage: IMessage) => {
    console.debug(`📞 handling alert message`, _alertMessage);
    setAlertMessage(_alertMessage);
  }

  /**
   * handle an update to the values tab (i.e. date range or latest values)
   * @param updates
   */
  const handleValuesUpdate = (updates: Partial<ITabValuesProps>): void => {
    console.debug(`📞 handling tab values update`, updates);
    tabValuesPropsRef.current = {
      ...tabValuesPropsRef.current,
      ...updates
    };
    setTabValuesProps(tabValuesPropsRef.current);
  };

  /**
   * compare a specific set of keys of 2 instances of a specific type
   * @param a 1st instance of comparison
   * @param b 2nd instance of comparison
   * @param keys the key that shall be compared
   * @returns
   */
  function areObjectKeysEqual<T, K extends keyof T>(a: T, b: T, keys: K[]): boolean {
    for (let key of keys) {
      if (!Object.is(a[key], b[key])) {
        return false;
      }
    }
    return true;
  }

  /**
   * handle an update to the config tab (i.e. date range or latest values)
   * @param updates
   */
  const handleConfigUpdate = (updates: Partial<ITabConfigProps>) => {

    console.debug(`📞 handling tab config update`, updates);

    // updates to the display config
    let seriesDefUpdateRequired = false;
    if (updates.disp && !areObjectKeysEqual(updates.disp.co2, tabConfigPropsRef.current.disp.co2, ['wHi', 'rHi'])) {
      seriesDefUpdateRequired = true;
      (SERIES_DEFS.co2Lpf.colorMap as IPiecewiseColorConfig).thresholds = [
        updates.disp.co2.wHi,
        updates.disp.co2.rHi
      ];
      (SERIES_DEFS.co2Raw.colorMap as IPiecewiseColorConfig).thresholds = [
        updates.disp.co2.wHi,
        updates.disp.co2.rHi
      ];
    }
    if (updates.disp && !areObjectKeysEqual(updates.disp.deg, tabConfigPropsRef.current.disp.deg, ['rLo', 'wLo', 'wHi', 'rHi'])) {
      seriesDefUpdateRequired = true;
      (SERIES_DEFS.deg.colorMap as IPiecewiseColorConfig).thresholds = [
        updates.disp.deg.rLo,
        updates.disp.deg.wLo,
        updates.disp.deg.wHi,
        updates.disp.deg.rHi
      ];
    }
    if (updates.disp && !areObjectKeysEqual(updates.disp.hum, tabConfigPropsRef.current.disp.hum, ['rLo', 'wLo', 'wHi', 'rHi'])) {
      seriesDefUpdateRequired = true;
      (SERIES_DEFS.hum.colorMap as IPiecewiseColorConfig).thresholds = [
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

  /**
   * references to the current tab values properties
   */
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

  /**
   * references to the current tab config properties
   */
  const tabConfigPropsRef = useRef<ITabConfigProps>({
    boxUrl,
    disp: DISP_CONFIG_DEFAULT,
    wifi: WIFI_CONFIG_DEFAULT,
    mqtt: MQTT_CONFIG_DEFAULT,
    cert: CERT_CONFIG_DEFAULT,
    handleUpdate: handleConfigUpdate,
    handleAlertMessage
  });
  const [tabConfigProps, setTabConfigProps] = useState<ITabConfigProps>(tabConfigPropsRef.current);

  /**
   * component init hook
   */
  useEffect(() => {

    console.debug('✨ building root component');

    handleAlertMessage({
      severity: 'info',
      message: 'connecting to device',
      active: true
    });
    loadStatus();

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Snackbar
        open={alertMessage.active}
        autoHideDuration={30000}
        onClose={handleSnackbarClose}
        sx={{ left: '72px' }}
      >
        <Alert
          onClose={handleSnackbarClose}
          severity={alertMessage.severity}
          variant='filled'
          sx={{ width: '100%' }}
        >
          {alertMessage.message}
        </Alert>
      </Snackbar>
      <Stack direction={'row'} spacing={0} sx={{ height: '100%' }}>
        <Paper elevation={4} sx={{ display: 'flex', flexDirection: 'column', position: 'fixed', marginTop: '5px', backgroundColor: '#FAFAFA', border: '1px solid #DDDDDD' }}>
          <IconButton disabled={!status} size='small' title='data' onClick={() => setViewType('values')} sx={{ color: viewType === 'values' ? 'black' : 'gray' }}>
            <ShowChartIcon />
          </IconButton>
          <IconButton disabled={!status} size='small' title='configuration' onClick={() => setViewType('config')} sx={{ color: viewType === 'config' ? 'black' : 'gray' }}>
            <TuneIcon />
          </IconButton>
          <IconButton disabled={!status} size='small' title='server api' onClick={() => setViewType('server')} sx={{ color: viewType === 'server' ? 'black' : 'gray' }}>
            <TocIcon />
          </IconButton>
        </Paper>
        <div style={{ minWidth: '45px' }}></div>
        {
          status ? <>
            <TabValues {...tabValuesProps} style={{ display: viewType === 'values' ? 'flex' : 'none' }} />
            <TabConfig {...tabConfigProps} style={{ display: viewType === 'config' ? 'flex' : 'none' }} />
            <TabServer boxUrl={boxUrl} handleAlertMessage={handleAlertMessage} style={{ display: viewType === 'server' ? 'flex' : 'none' }} />
          </> : null
        }
      </Stack >
    </ThemeProvider >
  );

};

export default RootApp;
