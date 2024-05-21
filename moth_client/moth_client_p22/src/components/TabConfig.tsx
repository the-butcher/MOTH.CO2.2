import SouthIcon from '@mui/icons-material/South';
import VideoLabelIcon from '@mui/icons-material/VideoLabel';
import { Badge, IconButton, Stack } from '@mui/material';
import { useEffect, useRef } from 'react';
import { ICertConfig } from '../types/ICertConfig';
import { TConfigStatus } from '../types/IConfig';
import { IDispConfig } from '../types/IDispConfig';
import { IMqttConfig } from '../types/IMqttConfig';
import { ITabConfigProps } from '../types/ITabConfigProps';
import { IWifiConfig } from '../types/IWifiConfig';
import { InstLoader } from '../util/InstLoader';
import { JsonLoader } from '../util/JsonLoader';
import { TextLoader } from '../util/TextLoader';
import CertificateChoice from './CertificateChoice';
import ConfigChoice from './ConfigChoice';
import LabelledDivider from './LabelledDivider';
import NetworkChoice from './NetworkChoice';
// var forge = require('node-forge');

type DeepPartial<T> = T extends object ? {
  [P in keyof T]?: DeepPartial<T[P]>;
} : T;

/**
 * component, renders the configuration form in the client-ui
 * @param props
 * @returns
 */
const TabConfig = (props: ITabConfigProps) => {

  const { boxUrl, disp, wifi, mqtt, cert, handleUpdate, handleAlertMessage } = { ...props };

  const handleConfigUploadToRef = useRef<number>(-1);

  const loadDispConfig = async (): Promise<IDispConfig> => {
    return await new JsonLoader().load(`${boxUrl}/datout?file=config/disp.json`);
  };

  const loadWifiConfig = async (): Promise<IWifiConfig> => {
    return await new JsonLoader().load(`${boxUrl}/datout?file=config/wifi.json`);
  }

  const loadMqttConfig = async (): Promise<IMqttConfig> => {
    return await new JsonLoader().load(`${boxUrl}/datout?file=config/mqtt.json`);
  }

  const loadCertificateFile = () => {

    new TextLoader().load(`${boxUrl}/datout?file=config/ca.crt`).then((response: string) => {

      if (response.indexOf('"code":404') >= 0) { // no certificate found on the device
        handleUpdate({
          cert: {
            status: 'LOADED_INVALID',
            crt: ''
          }
        });
      } else {
        const certLoaded: ICertConfig = {
          status: 'LOADED_VALID',
          crt: response
        };
        handleUpdate({
          cert: {
            ...certLoaded,
            status: isCertConfigValid(certLoaded) ? 'LOADED_VALID' : 'LOADED_INVALID'
          }
        });
      }

    }).catch((e: Error) => {
      console.error('e', e);
      handleAlertMessage({
        message: e.message ? e.message : 'failed to check certificate presence',
        severity: 'error',
        active: true
      });
    });

  }

  /**
   * component init hook, load disp, wifi and mqtt configuration from the device and creates approriate state that will subsequently be rendered in the form
   */
  useEffect(() => {

    console.debug('✨ building tab config component');

    loadDispConfig().then(_disp => {
      loadWifiConfig().then(_wifi => {
        loadMqttConfig().then(_mqttConfig => {
          handleUpdate({
            disp: {
              ..._disp,
              status: 'LOADED_VALID'
            },
            wifi: {
              ..._wifi,
              status: 'LOADED_VALID'
            },
            mqtt: {
              ..._mqttConfig,
              cli: _mqttConfig.cli ? _mqttConfig.cli : `moth${boxUrl.substring(boxUrl.lastIndexOf('.')).replace('.', '_').replace('/api', '').padStart(4, '_')}`,
              min: _mqttConfig.min ? _mqttConfig.min : 5,
              status: 'LOADED_VALID'
            }
          });
        }).catch(e => {
          console.error('failed to load mqtt config', e);
          handleAlertMessage({
            message: e.message ? e.message : 'failed to load mqtt config',
            severity: 'error',
            active: true
          });
        });
      }).catch(e => {
        console.error('failed to load wifi config', e);
        handleAlertMessage({
          message: e.message ? e.message : 'failed to load wifi config',
          severity: 'error',
          active: true
        });
      });
    }).catch(e => {
      console.error('failed to load disp config', e);
      handleAlertMessage({
        message: e.message ? e.message : 'failed to load disp config',
        severity: 'error',
        active: true
      });
    });
    loadCertificateFile();

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  /**
   * handles an update to the disp config
   * @param update
   */
  const handleDispUpdate = (update: DeepPartial<IDispConfig>) => {
    handleUpdate({
      disp: {
        ...disp,
        ...update,
        status: 'MODIFIED_VALID',
        co2: {
          ...disp.co2,
          ...update.co2
        },
        deg: {
          ...disp.deg,
          ...update.deg
        },
        hum: {
          ...disp.hum,
          ...update.hum
        },
        bme: {
          ...disp.bme,
          ...update.bme
        },
      }
    });
  };

  /**
   * handles an update to the wifi config
   * @param update
   */
  const handleWifiUpdate = (update: Partial<IWifiConfig>) => {
    handleUpdate({
      wifi: {
        ...wifi,
        ...update,
        status: 'MODIFIED_VALID',
      }
    });
  };

  /**
   * handles an update to the wifi.network config (change, add, remove)
   * @param update
   */
  const handleNetworkUpdate = (idx: number, key: string, pwd: string) => {
    if (idx >= 0) { // update
      if (key !== '' && pwd !== '') { // update
        handleUpdate({
          wifi: {
            ...wifi,
            status: 'MODIFIED_VALID',
            ntw: wifi.ntw.map((value, index) => {
              return index === idx ? {
                key,
                pwd
              } : value;
            })
          }
        });
      } else { // delete
        handleUpdate({
          wifi: {
            ...wifi,
            status: 'MODIFIED_VALID',
            ntw: wifi.ntw.filter((value, index) => index !== idx)
          }
        });
      }
    } else if (key !== '' && pwd !== '') { // create
      handleUpdate({
        wifi: {
          ...wifi,
          status: 'MODIFIED_VALID',
          ntw: [
            ...wifi.ntw,
            {
              key,
              pwd
            }
          ]
        }
      });
    } else {
      // invalid
    }
  }

  /**
   * evaluates the status of the mqtt config, depending on the properties currently configured
   * @param update
   */
  const getMqttConfigStatus = (mqtt: IMqttConfig): TConfigStatus => {
    if (!mqtt.srv || mqtt.srv === '') {
      return 'MODIFIED_INVALID';
    }
    if (!mqtt.prt) {
      return 'MODIFIED_INVALID';
    }
    if (!mqtt.cli || mqtt.cli === '') {
      return 'MODIFIED_INVALID';
    }
    if (!mqtt.min) {
      return 'MODIFIED_INVALID';
    }
    return 'MODIFIED_VALID';
  }

  /**
   * handles an update to the mqtt config
   * @param update
   */
  const handleMqttUpdate = (update: Partial<IMqttConfig>) => {
    const mqttUpdate: IMqttConfig = {
      ...mqtt,
      ...update
    };
    const mqttConfigStatus: TConfigStatus = getMqttConfigStatus(mqttUpdate);
    handleUpdate({
      mqtt: {
        ...mqttUpdate,
        status: mqttConfigStatus
      }
    });
  };

  /**
 * evaluates the status of the cert config, depending on the properties currently configured
 * @param update
 */
  const isCertConfigValid = (cert: ICertConfig): boolean => {
    const crt = cert.crt.trim();
    return (crt === '' || (crt.startsWith('-----BEGIN CERTIFICATE-----') && crt.endsWith('-----END CERTIFICATE-----')));
  }

  const handleCertUpdate = (update: Partial<ICertConfig>) => {
    const certUpdate: ICertConfig = {
      ...cert,
      ...update,
    };
    handleUpdate({
      cert: {
        ...certUpdate,
        status: isCertConfigValid(certUpdate) ? 'MODIFIED_VALID' : 'MODIFIED_INVALID'
      }
    });
  };

  /**
   * uploads modified configuration to the device and resets the status flag of the associated config to 'LOADED'
   */
  const handleConfigUpload = async (): Promise<void> => {

    const updates: Partial<ITabConfigProps> = {};
    if (disp.status === 'MODIFIED_VALID') {
      await new InstLoader().load(disp, 'config/disp.json', 'application/json;charset=utf-8', `${boxUrl}/upload`, inst => JSON.stringify(inst, null, 2));
      updates.disp = {
        ...disp,
        status: 'LOADED_VALID'
      };
    } else if (wifi.status === 'MODIFIED_VALID') {
      await new InstLoader().load(wifi, 'config/wifi.json', 'application/json;charset=utf-8', `${boxUrl}/upload`, inst => JSON.stringify(inst, null, 2));
      updates.wifi = {
        ...wifi,
        status: 'LOADED_VALID'
      };
    } else if (mqtt.status === 'MODIFIED_VALID') {
      await new InstLoader().load(mqtt, 'config/mqtt.json', 'application/json;charset=utf-8', `${boxUrl}/upload`, inst => JSON.stringify(inst, null, 2));
      updates.mqtt = {
        ...mqtt,
        status: 'LOADED_VALID'
      };
    } else if (cert.status === 'MODIFIED_VALID') {
      if (cert.crt !== '') {
        await new InstLoader().load(cert, 'config/ca.crt', 'application/x-x509-ca-cert', `${boxUrl}/upload`, inst => inst.crt);

      } else {
        await new JsonLoader().load(`${boxUrl}/datdel?file=config/ca.crt`);
      }
      updates.cert = {
        ...cert,
        status: 'LOADED_VALID'
      };
    }

    handleUpdate(updates);

    if (getModifiedCount() > 0) {
      window.clearTimeout(handleConfigUploadToRef.current);
      handleConfigUploadToRef.current = window.setTimeout(() => {
        handleConfigUpload();
      }, 5000);
    }

  }

  /**
   * evaluates the count of modified configurations
   * @returns
   */
  const getModifiedCount = (): number => {
    let modifiedCount = 0;
    if (disp.status === 'MODIFIED_VALID') {
      modifiedCount++;
    }
    if (wifi.status === 'MODIFIED_VALID') {
      modifiedCount++;
    }
    if (mqtt.status === 'MODIFIED_VALID') {
      modifiedCount++;
    }
    if (cert.status === 'MODIFIED_VALID') {
      modifiedCount++;
    }
    return modifiedCount;
  }

  return (
    <>
      <Stack spacing={1} sx={{ flexDirection: 'column', position: 'fixed', left: '14px', top: '170px', ...props.style }}>
        <IconButton
          sx={{ boxShadow: '0px 2px 4px -1px rgba(0,0,0,0.2),0px 4px 5px 0px rgba(0,0,0,0.14),0px 1px 10px 0px rgba(0,0,0,0.12)', width: '28px' }}
          size='small'
          title='synchronize to device'
          onClick={() => handleConfigUpload()}
        >
          <Badge
            color='error'
            // variant='dot'
            overlap='rectangular'
            badgeContent={getModifiedCount()}
            sx={{ color: getModifiedCount() > 0 ? 'black' : 'gray' }}
          >
            <VideoLabelIcon sx={{ fontSize: '0.8em', position: 'relative', left: '6px' }} />
            <SouthIcon sx={{ fontSize: '0.7em', position: 'relative', left: '-7px', top: '-4px' }} />
          </Badge>
        </IconButton>
      </Stack>
      <Stack direction={'column'} sx={{ margin: '5px', width: '800px', ...props.style }}>

        <Stack direction={'column'} sx={{
          padding: '10px',
          '&>:not(style)+:not(style)': {
            marginTop: '8px'
          }
        }}>
          <LabelledDivider label='display settings' />
          <ConfigChoice
            caption='display update interval'
            value={{
              type: 'number',
              fixed: true,
              value: disp.min,
              unit: 'minutes',
              min: 1,
              handleUpdate: value => handleDispUpdate({ min: value })
            }}
          />
          <ConfigChoice
            caption='update on significant change'
            value={{
              type: 'toggle',
              value: disp.ssc,
              handleUpdate: value => handleDispUpdate({ ssc: value })
            }}
          />
          <ConfigChoice
            caption='timezone'
            value={{
              type: 'string',
              value: disp.tzn,
              // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
              items: {
                'Los Angeles (PST8PDT)': 'PST8PDT,M3.2.0,M11.1.0',
                'Denver (MST7MDT)': 'MST7MDT,M3.2.0,M11.1.0',
                'Chicago (CST6CDT)': 'CST6CDT,M3.2.0,M11.1.0',
                'New York (EST5EDT)': 'EST5EDT,M3.2.0,M11.1.0',
                'Atlantic (AST4ADT)': 'AST4ADT,M3.2.0,M11.1.0',
                'Lisbon (WET0WEST)': ' WET0WEST,M3.5.0/1,M10.5.0',
                'London (GMT0BST)': 'GMT0BST,M3.5.0/1,M10.5.0',
                'Vienna (CET-1CEST)': 'CET-1CEST,M3.5.0,M10.5.0/3',
                'Athens (EET-2EEST)': 'EET-2EEST,M3.5.0/3,M10.5.0/4',
                'Sydney (AEST-10AEDT)': 'AEST-10AEDT,M10.1.0,M4.1.0/3',
                'Auckland (NZST-12NZDT)': 'NZST-12NZDT,M9.5.0,M4.1.0/3',
              },
              handleUpdate: value => handleDispUpdate({ tzn: value })
            }}
          />
          <LabelledDivider label='CO₂ thresholds and settings' />
          <ConfigChoice
            caption='CO₂ warn level'
            value={{
              type: 'number',
              fixed: true,
              value: disp.co2.wHi,
              unit: 'ppm',
              step: 100,
              min: 400,
              max: disp.co2.rHi,
              handleUpdate: value => handleDispUpdate({ co2: { wHi: value } })
            }}
          />
          <ConfigChoice
            caption='CO₂ risk level'
            value={{
              type: 'number',
              fixed: true,
              value: disp.co2.rHi,
              unit: 'ppm',
              step: 100,
              min: disp.co2.wHi,
              handleUpdate: value => handleDispUpdate({ co2: { rHi: value } })
            }}
          />
          <ConfigChoice
            caption='CO₂ stale reference'
            value={{
              type: 'number',
              fixed: true,
              value: disp.co2.ref,
              unit: 'ppm',
              step: 5,
              min: 400,
              max: 500,
              handleUpdate: value => handleDispUpdate({ co2: { ref: value } })
            }}
          />
          <ConfigChoice
            caption='CO₂ calibration default'
            value={{
              type: 'number',
              fixed: true,
              value: disp.co2.cal,
              unit: 'ppm',
              step: 5,
              min: 400,
              handleUpdate: value => handleDispUpdate({ co2: { cal: value } })
            }}
          />
          <ConfigChoice
            caption='CO₂ low pass filter'
            value={{
              type: 'number',
              fixed: false,
              value: disp.co2.lpa,
              items: {
                'none (1.00)': 1.00,
                'light (0.75)': 0.75,
                'default (0.50)': 0.50,
                'strong (0.25)': 0.25
              },
              help: 'a low value yields stronger filtering',
              handleUpdate: value => handleDispUpdate({ co2: { lpa: value } })
            }}
          />
          <LabelledDivider label='temperature thresholds and settings' />
          <ConfigChoice
            caption='temperature risk level (low)'
            value={{
              type: 'number',
              fixed: true,
              value: disp.deg.rLo,
              unit: '°C',
              max: disp.deg.wLo,
              handleUpdate: value => handleDispUpdate({ deg: { rLo: value } })
            }}
          />
          <ConfigChoice
            caption='temperature warn level (low)'
            value={{
              type: 'number',
              fixed: true,
              value: disp.deg.wLo,
              unit: '°C',
              min: disp.deg.rLo,
              max: disp.deg.wHi,
              handleUpdate: value => handleDispUpdate({ deg: { wLo: value } })
            }}
          />
          <ConfigChoice
            caption='temperature warn level (high)'
            value={{
              type: 'number',
              fixed: true,
              value: disp.deg.wHi,
              min: disp.deg.wLo,
              max: disp.deg.rHi,
              unit: '°C',
              handleUpdate: value => handleDispUpdate({ deg: { wHi: value } })
            }}
          />
          <ConfigChoice
            caption='temperature risk level (high)'
            value={{
              type: 'number',
              fixed: true,
              value: disp.deg.rHi,
              min: disp.deg.wHi,
              unit: '°C',
              handleUpdate: value => handleDispUpdate({ deg: { rHi: value } })
            }}
          />
          <ConfigChoice
            caption='temperature offset'
            value={{
              type: 'number',
              fixed: false,
              value: disp.deg.off,
              step: 0.1,
              unit: '°C',
              help: 'a higher value yields lower measurements',
              handleUpdate: value => handleDispUpdate({ deg: { off: value } })
            }}
          />
          <ConfigChoice
            caption='display as fahrenheit'
            value={{
              type: 'toggle',
              value: disp.deg.c2f,
              handleUpdate: value => handleDispUpdate({ deg: { c2f: value } })
            }}
          />
          <LabelledDivider label='humidity thresholds' />
          <ConfigChoice
            caption='humidity risk level (low)'
            value={{
              type: 'number',
              fixed: true,
              value: disp.hum.rLo,
              max: disp.hum.wLo,
              unit: '%',
              step: 5,
              handleUpdate: value => handleDispUpdate({ hum: { rLo: value } })
            }}
          />
          <ConfigChoice
            caption='humidity warn level (low)'
            value={{
              type: 'number',
              fixed: true,
              value: disp.hum.wLo,
              min: disp.hum.rLo,
              max: disp.hum.wHi,
              unit: '%',
              step: 5,
              handleUpdate: value => handleDispUpdate({ hum: { wLo: value } })
            }}
          />
          <ConfigChoice
            caption='humidity warn level (high)'
            value={{
              type: 'number',
              fixed: true,
              value: disp.hum.wHi,
              min: disp.hum.wLo,
              max: disp.hum.rHi,
              unit: '%',
              step: 5,
              handleUpdate: value => handleDispUpdate({ hum: { wHi: value } })
            }}
          />
          <ConfigChoice
            caption='humidity risk level (high)'
            value={{
              type: 'number',
              fixed: true,
              value: disp.hum.rHi,
              min: disp.hum.wHi,
              unit: '%',
              step: 5,
              handleUpdate: value => handleDispUpdate({ hum: { rHi: value } })
            }}
          />
          <LabelledDivider label='pressure settings' />
          <ConfigChoice
            caption='reference altitude'
            value={{
              type: 'number',
              fixed: true,
              value: disp.bme.alt,
              min: 0,
              max: 10000,
              unit: 'm',
              step: 10,
              handleUpdate: value => handleDispUpdate({ bme: { alt: value } })
            }}
          />
          <ConfigChoice
            caption='pressure low pass filter'
            value={{
              type: 'number',
              fixed: false,
              value: disp.bme.lpa,
              items: {
                'none (1.00)': 1.00,
                'light (0.75)': 0.75,
                'default (0.50)': 0.50,
                'strong (0.25)': 0.25
              },
              help: 'a low value yields stronger filtering',
              handleUpdate: value => handleDispUpdate({ bme: { lpa: value } })
            }}
          />
          <LabelledDivider label='wifi settings' style={{ paddingTop: '32px' }} />
          <ConfigChoice
            caption='network timeout'
            value={{
              type: 'number',
              fixed: true,
              value: wifi.min,
              unit: 'minutes',
              min: 1,
              handleUpdate: value => handleWifiUpdate({ min: value })
            }}
          />
          {
            wifi.ntw.length > 0 ? <LabelledDivider label='configured connections' style={{ minWidth: '200px', maxWidth: '450px' }} /> : null
          }
          {
            wifi.ntw.map((ntw, index) =>
              <NetworkChoice
                key={index}
                idx={index}
                lbl={ntw.key}
                pwd={ntw.pwd}
                handleNetworkUpdate={handleNetworkUpdate}
              />
            )
          }
          <LabelledDivider label='new connection' style={{ minWidth: '200px', maxWidth: '450px' }} />
          <NetworkChoice
            idx={-Math.random()}
            lbl=''
            pwd=''
            handleNetworkUpdate={handleNetworkUpdate}
          />
          <LabelledDivider label='mqtt settings' style={{ paddingTop: '32px' }} />
          <ConfigChoice
            caption='use mqtt'
            value={{
              type: 'toggle',
              value: mqtt.use,
              handleUpdate: value => handleMqttUpdate({ use: value })
            }}
          />
          {
            mqtt.use ? <>
              <ConfigChoice
                caption='broker address'
                value={{
                  type: 'string',
                  value: mqtt.srv,
                  help: 'i.e. 192.186.1.1 or mybroker.mynetwork.com',
                  handleUpdate: value => handleMqttUpdate({ srv: value })
                }}
              />
              <ConfigChoice
                caption='broker port'
                value={{
                  type: 'number',
                  fixed: true,
                  value: mqtt.prt,
                  step: 1000,
                  min: 1,
                  max: 65535,
                  handleUpdate: value => handleMqttUpdate({ prt: value })
                }}
              />
              <CertificateChoice
                cert={cert}
                handleUpdate={value => handleCertUpdate({ crt: value })}
              />
              <ConfigChoice
                caption='username'
                value={{
                  type: 'string',
                  value: mqtt.usr,
                  handleUpdate: value => handleMqttUpdate({ usr: value })
                }}
              />
              <ConfigChoice
                caption='password'
                value={{
                  type: 'string',
                  value: mqtt.pwd,
                  pwd: true,
                  handleUpdate: value => handleMqttUpdate({ pwd: value })
                }}
              />
              <ConfigChoice
                caption='client'
                value={{
                  type: 'string',
                  value: mqtt.cli,
                  help: 'mqtt message client id',
                  handleUpdate: value => handleMqttUpdate({ cli: value })
                }}
              />
              <ConfigChoice
                caption='publish interval'
                value={{
                  type: 'number',
                  fixed: true,
                  value: mqtt.min,
                  unit: 'minutes',
                  min: 1,
                  handleUpdate: value => handleMqttUpdate({ min: value })
                }}
              />
            </> : null
          }
        </Stack>
      </Stack>
    </>
  );

};

export default TabConfig;
