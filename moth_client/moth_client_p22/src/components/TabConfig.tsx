import SouthIcon from '@mui/icons-material/South';
import VideoLabelIcon from '@mui/icons-material/VideoLabel';
import { Badge, IconButton, Stack } from '@mui/material';
import { useEffect, useState } from 'react';
import { DISP_CONFIG_DEFAULT, IDispConfig } from '../types/IDispConfig';
import { IMqttConfig, MQTT_CONFIG_DEFAULT } from '../types/IMqttConfig';
import { ITabProps } from '../types/ITabProps';
import { IWifiConfig, WIFI_CONFIG_DEFAULT } from '../types/IWifiConfig';
import ConfigChoice from './ConfigChoice';
import LabelledDivider from './LabelledDivider';
import NetworkChoice from './NetworkChoice';
import { JsonLoader } from '../util/JsonLoader';

type DeepPartial<T> = T extends object ? {
  [P in keyof T]?: DeepPartial<T[P]>;
} : T;

type FLAG_CONFIG = 'DEFAULT' | 'LOADED' | 'MODIFIED';

interface IFlagConfig {
  disp: FLAG_CONFIG;
  wifi: FLAG_CONFIG;
  mqtt: FLAG_CONFIG;
}

const TabConfig = (props: ITabProps) => {

  const { boxUrl } = { ...props };

  const [dispConfig, setDispConfig] = useState<IDispConfig>(DISP_CONFIG_DEFAULT);
  const [wifiConfig, setWifiConfig] = useState<IWifiConfig>(WIFI_CONFIG_DEFAULT);
  const [mqttConfig, setMqttConfig] = useState<IMqttConfig>(MQTT_CONFIG_DEFAULT);
  const [flagConfig, setFlagConfig] = useState<IFlagConfig>({
    disp: 'DEFAULT',
    wifi: 'DEFAULT',
    mqtt: 'DEFAULT',
  })

  const loadDispConfig = async (): Promise<IDispConfig> => {
    return await new JsonLoader().load(`${boxUrl}/datout?file=config/disp.json`);
  };

  const loadWifiConfig = async (): Promise<IWifiConfig> => {
    return await new JsonLoader().load(`${boxUrl}/datout?file=config/wifi.json`);
  }

  const loadMqttConfig = async (): Promise<IMqttConfig> => {
    return await new JsonLoader().load(`${boxUrl}/datout?file=config/mqtt.json`);
  }

  useEffect(() => {

    console.debug(`⚙ updating tab config component (configDisp)`, dispConfig);

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [dispConfig]);

  useEffect(() => {

    console.debug(`⚙ updating tab config component (wifiConfig)`, wifiConfig);

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [wifiConfig]);

  useEffect(() => {

    console.debug(`⚙ updating tab config component (mqttConfig)`, mqttConfig);

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [mqttConfig]);

  useEffect(() => {

    console.debug('✨ building tab config component');

    loadDispConfig().then(_dispConfig => {
      setDispConfig(_dispConfig);
      setFlagConfig({
        ...flagConfig,
        disp: 'LOADED'
      });
      loadWifiConfig().then(_wifiConfig => {
        setWifiConfig(_wifiConfig);
        setFlagConfig({
          ...flagConfig,
          wifi: 'LOADED'
        });
        loadMqttConfig().then(_mqttConfig => {
          setMqttConfig(_mqttConfig);
          setFlagConfig({
            ...flagConfig,
            mqtt: 'LOADED'
          });
        }).catch(e => {
          console.error('e', e);
        });
      }).catch(e => {
        console.error('e', e);
      });
    }).catch(e => {
      console.error('e', e);
    });

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const handleDispUpdate = (update: DeepPartial<IDispConfig>) => {
    setDispConfig({
      ...dispConfig,
      ...update,
      co2: {
        ...dispConfig.co2,
        ...update.co2
      },
      deg: {
        ...dispConfig.deg,
        ...update.deg
      },
      hum: {
        ...dispConfig.hum,
        ...update.hum
      },
      bme: {
        ...dispConfig.bme,
        ...update.bme
      },
    });
    setFlagConfig({
      ...flagConfig,
      disp: 'MODIFIED'
    });
  };

  const handleWifiUpdate = (update: Partial<IWifiConfig>) => {
    setWifiConfig({
      ...wifiConfig,
      ...update
    });
    setFlagConfig({
      ...flagConfig,
      wifi: 'MODIFIED'
    });
  };

  const handleNetworkUpdate = (idx: number, key: string, pwd: string) => {
    if (idx >= 0) { // update
      if (key !== '' && pwd !== '') { // update
        setWifiConfig({
          ...wifiConfig,
          ntw: wifiConfig.ntw.map((value, index) => {
            return index === idx ? {
              key,
              pwd
            } : value;
          })
        });
      } else { // delete
        setWifiConfig({
          ...wifiConfig,
          ntw: wifiConfig.ntw.filter((value, index) => index !== idx)
        });
      }
    } else if (key !== '' && pwd !== '') { // create
      setWifiConfig({
        ...wifiConfig,
        ntw: [
          ...wifiConfig.ntw,
          {
            key,
            pwd
          }
        ]
      });
    } else {
      // invalid
    }
    setFlagConfig({
      ...flagConfig,
      wifi: 'MODIFIED'
    });
  }

  const handleMqttUpdate = (update: Partial<IMqttConfig>) => {
    setMqttConfig({
      ...mqttConfig,
      ...update
    });
    setFlagConfig({
      ...flagConfig,
      mqtt: 'MODIFIED'
    });
  };

  const getModifiedFlag = (): number => {
    let modifiedFlag = 0;
    if (flagConfig.disp === 'MODIFIED') {
      modifiedFlag++;
    }
    if (flagConfig.wifi === 'MODIFIED') {
      modifiedFlag++;
    }
    if (flagConfig.mqtt === 'MODIFIED') {
      modifiedFlag++;
    }
    return modifiedFlag;
  }

  return (
    <>
      <Stack spacing={1} sx={{ flexDirection: 'column', position: 'fixed', left: '14px', top: '170px', ...props.style }}>
        <IconButton
          sx={{ boxShadow: '0px 2px 4px -1px rgba(0,0,0,0.2),0px 4px 5px 0px rgba(0,0,0,0.14),0px 1px 10px 0px rgba(0,0,0,0.12)', width: '28px' }}
          size='small'
          title='synchronize to device'
        >
          <Badge
            color='error'
            // variant='dot'
            overlap='rectangular'
            badgeContent={getModifiedFlag()}
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
              value: dispConfig.min,
              unit: 'minutes',
              min: 1,
              handleUpdate: value => handleDispUpdate({ min: value })
            }}
          />
          <ConfigChoice
            caption='update on significant change'
            value={{
              type: 'toggle',
              value: dispConfig.ssc,
              handleUpdate: value => handleDispUpdate({ ssc: value })
            }}
          />
          <ConfigChoice
            caption='timezone'
            value={{
              type: 'string',
              value: dispConfig.tzn,
              items: {
                'London (GMT/BST)': 'GMT0BST,M3.5.0/1,M10.5.0',
                'Berlin, Vienna (CET/CEST)': 'CET-1CEST,M3.5.0,M10.5.0/3'
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
              value: dispConfig.co2.wHi,
              unit: 'ppm',
              step: 100,
              min: 400,
              max: dispConfig.co2.rHi,
              handleUpdate: value => handleDispUpdate({ co2: { wHi: value } })
            }}
          />
          <ConfigChoice
            caption='CO₂ risk level'
            value={{
              type: 'number',
              fixed: true,
              value: dispConfig.co2.rHi,
              unit: 'ppm',
              step: 100,
              min: dispConfig.co2.wHi,
              handleUpdate: value => handleDispUpdate({ co2: { rHi: value } })
            }}
          />
          <ConfigChoice
            caption='CO₂ stale reference'
            value={{
              type: 'number',
              fixed: true,
              value: dispConfig.co2.ref,
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
              value: dispConfig.co2.cal,
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
              value: dispConfig.co2.lpa,
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
              value: dispConfig.deg.rLo,
              unit: '°C',
              max: dispConfig.deg.wLo,
              handleUpdate: value => handleDispUpdate({ deg: { rLo: value } })
            }}
          />
          <ConfigChoice
            caption='temperature warn level (low)'
            value={{
              type: 'number',
              fixed: true,
              value: dispConfig.deg.wLo,
              unit: '°C',
              min: dispConfig.deg.rLo,
              max: dispConfig.deg.wHi,
              handleUpdate: value => handleDispUpdate({ deg: { wLo: value } })
            }}
          />
          <ConfigChoice
            caption='temperature warn level (high)'
            value={{
              type: 'number',
              fixed: true,
              value: dispConfig.deg.wHi,
              min: dispConfig.deg.wLo,
              max: dispConfig.deg.rHi,
              unit: '°C',
              handleUpdate: value => handleDispUpdate({ deg: { wHi: value } })
            }}
          />
          <ConfigChoice
            caption='temperature risk level (high)'
            value={{
              type: 'number',
              fixed: true,
              value: dispConfig.deg.rHi,
              min: dispConfig.deg.wHi,
              unit: '°C',
              handleUpdate: value => handleDispUpdate({ deg: { rHi: value } })
            }}
          />
          <ConfigChoice
            caption='temperature offset'
            value={{
              type: 'number',
              fixed: false,
              value: dispConfig.deg.off,
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
              value: dispConfig.deg.c2f,
              handleUpdate: value => handleDispUpdate({ deg: { c2f: value } })
            }}
          />
          <LabelledDivider label='humidity thresholds' />
          <ConfigChoice
            caption='humidity risk level (low)'
            value={{
              type: 'number',
              fixed: true,
              value: dispConfig.hum.rLo,
              max: dispConfig.hum.wLo,
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
              value: dispConfig.hum.wLo,
              min: dispConfig.hum.rLo,
              max: dispConfig.hum.wHi,
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
              value: dispConfig.hum.wHi,
              min: dispConfig.hum.wLo,
              max: dispConfig.hum.rHi,
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
              value: dispConfig.hum.rHi,
              min: dispConfig.hum.wHi,
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
              value: dispConfig.bme.alt,
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
              value: dispConfig.bme.lpa,
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
              value: wifiConfig.min,
              unit: 'minutes',
              min: 1,
              handleUpdate: value => handleWifiUpdate({ min: value })
            }}
          />
          {
            wifiConfig.ntw.length > 0 ? <LabelledDivider label='configured connections' style={{ minWidth: '200px', maxWidth: '450px' }} /> : null
          }
          {
            wifiConfig.ntw.map((ntw, index) =>
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
            caption='broker address'
            value={{
              type: 'string',
              value: mqttConfig.srv,
              help: 'i.e. 192.186.1.1 or mybroker.mynetwork.com',
              handleUpdate: value => handleMqttUpdate({ srv: value })
            }}
          />
          <ConfigChoice
            caption='broker port'
            value={{
              type: 'number',
              fixed: true,
              value: mqttConfig.prt,
              step: 1000,
              min: 1,
              max: 65535,
              handleUpdate: value => handleMqttUpdate({ prt: value })
            }}
          />
          <ConfigChoice
            caption='certificate path'
            value={{
              type: 'string',
              value: mqttConfig.crt,
              help: 'certificate must be uploaded to this path',
              handleUpdate: value => handleMqttUpdate({ crt: value })
            }}
          />
          <ConfigChoice
            caption='username'
            value={{
              type: 'string',
              value: mqttConfig.usr,
              handleUpdate: value => handleMqttUpdate({ usr: value })
            }}
          />
          <ConfigChoice
            caption='password'
            value={{
              type: 'string',
              value: mqttConfig.pwd,
              pwd: true,
              handleUpdate: value => handleMqttUpdate({ pwd: value })
            }}
          />
          <ConfigChoice
            caption='client'
            value={{
              type: 'string',
              value: mqttConfig.cli,
              help: 'mqtt message client id',
              handleUpdate: value => handleMqttUpdate({ cli: value })
            }}
          />
          <ConfigChoice
            caption='publish interval'
            value={{
              type: 'number',
              fixed: true,
              value: mqttConfig.min,
              unit: 'minutes',
              min: 1,
              handleUpdate: value => handleMqttUpdate({ min: value })
            }}
          />
        </Stack>

      </Stack>
    </>
  );

};

export default TabConfig;
