import { Stack } from '@mui/material';
import { useEffect, useState } from 'react';
import { IDispConfig } from '../types/IDispConfig';
import { ITabProps } from '../types/ITabProps';
import ConfigChoice from './ConfigChoice';
import LabelledDivider from './LabelledDivider';
import { IWifiConfig } from '../types/IWifiConfig';
import NetworkChoice from './NetworkChoice';
import { IMqttConfig } from '../types/IMqttConfig';

type DeepPartial<T> = T extends object ? {
  [P in keyof T]?: DeepPartial<T[P]>;
} : T;

const TabConfig = (props: ITabProps) => {

  const { boxUrl } = { ...props };

  const [dispConfig, setDispConfig] = useState<IDispConfig>({
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

  const [wifiConfig, setWifiConfig] = useState<IWifiConfig>({
    min: 5,
    ntw: [
      {
        key: 'testkey1',
        pwd: 'testpwd1'
      },
      {
        key: 'testkey2',
        pwd: 'testpwd2'
      }
    ]
  });

  const [mqttConfig, setMqttConfig] = useState<IMqttConfig>({
    srv: '192.168.0.115',
    prt: 8883,
    crt: '/config/ca.crt',
    usr: 'hannes',
    pwd: 'fleischer',
    cli: 'moth__66',
    min: 3
  });

  const getDispConfig = () => {

    setDispConfig({
      ...dispConfig
    });

  };

  const getWifiConfig = () => {

    setWifiConfig({
      ...wifiConfig
    });

  }

  const getMqttConfig = () => {

    setWifiConfig({
      ...wifiConfig
    });

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

    console.debug('✨ building tab config component');

    getDispConfig();

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const handleDispUpdate = (update: DeepPartial<IDispConfig>) => {
    console.log('disp update', update);
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
  };

  const handleWifiUpdate = (update: Partial<IWifiConfig>) => {
    // TODO :: convention for deleting a network
    console.log('wifi update', update);
    setWifiConfig({
      ...wifiConfig,
      ...update
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
  }

  const handleMqttUpdate = (update: Partial<IMqttConfig>) => {
    // TODO :: convention for deleting a network
    console.log('mqtt update', update);
    setMqttConfig({
      ...mqttConfig,
      ...update
    });
  };

  return (
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
            handleUpdate: value => handleDispUpdate({ deg: { wLo: value } })
          }}
        />
        <ConfigChoice
          caption='temperature warn level (high)'
          value={{
            type: 'number',
            fixed: true,
            value: dispConfig.deg.wHi,
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
            unit: '%',
            handleUpdate: value => handleDispUpdate({ hum: { rLo: value } })
          }}
        />
        <ConfigChoice
          caption='humidity warn level (low)'
          value={{
            type: 'number',
            fixed: true,
            value: dispConfig.hum.wLo,
            unit: '%',
            handleUpdate: value => handleDispUpdate({ hum: { wLo: value } })
          }}
        />
        <ConfigChoice
          caption='humidity warn level (high)'
          value={{
            type: 'number',
            fixed: true,
            value: dispConfig.hum.wHi,
            unit: '%',
            handleUpdate: value => handleDispUpdate({ hum: { wHi: value } })
          }}
        />
        <ConfigChoice
          caption='humidity risk level (high)'
          value={{
            type: 'number',
            fixed: true,
            value: dispConfig.hum.rHi,
            unit: '%',
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
            unit: 'm',
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
            handleUpdate: value => handleWifiUpdate({ min: value })
          }}
        />
        {
          wifiConfig.ntw.length > 0 ? <LabelledDivider label='configured connections' style={{ width: '450px' }} /> : null
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
        <LabelledDivider label='new connection' style={{ width: '450px' }} />
        <NetworkChoice
          idx={-Math.random()}
          lbl=''
          pwd=''
          handleNetworkUpdate={handleNetworkUpdate}
        />
        <LabelledDivider label='mqtt settings' style={{ paddingTop: '32px' }} />
      </Stack>

    </Stack>
  );

};

export default TabConfig;
