import Co2Icon from '@mui/icons-material/Co2';
import DeviceThermostatIcon from '@mui/icons-material/DeviceThermostat';
import SpeedIcon from '@mui/icons-material/Speed';
import WaterDropIcon from '@mui/icons-material/WaterDrop';
import { Stack } from '@mui/material';
import { LineChart } from '@mui/x-charts/LineChart';
import { DateTimePicker, renderDateViewCalendar, renderTimeViewClock } from '@mui/x-date-pickers';
import { AdapterMoment } from '@mui/x-date-pickers/AdapterMoment';
import { LocalizationProvider } from '@mui/x-date-pickers/LocalizationProvider';
import moment from 'moment';
import 'moment/locale/de';
import { useEffect, useState } from 'react';
import { ILatest } from '../types/ILatest';
import { IRecord, TRecordKey } from '../types/IRecord';
import { ISeriesDef } from '../types/ISeriesDef';
import { ITabProperties, TOrientation } from '../types/ITabProperties';
import { ByteLoader } from '../util/ByteLoader';
import { JsonLoader } from '../util/JsonLoader';
import { TimeUtil } from '../util/TimeUtil';
import Value from './Value';

const COLOR_G = '#0ec600';
const COLOR_Y = '#c9b800';
const COLOR_R = '#e20e00';

const seriesDefs: { [K in TRecordKey]: ISeriesDef } = {
  instant: {
    dataKey: 'instant',
    label: 'time',
    valueFormatter: value => value ? value.toString() : 'NA'
  },
  co2Lpf: {
    dataKey: 'co2Lpf',
    label: 'CO₂ ppm (filtered)',
    valueFormatter: value => value ? value.toFixed(0) : 'NA',
    colorMap: {
      type: 'piecewise',
      thresholds: [800, 1000],
      colors: [COLOR_G, COLOR_Y, COLOR_R]
    }
  },
  deg: {
    dataKey: 'deg',
    label: 'Temperature °C',
    valueFormatter: value => value.toFixed(1),
    colorMap: {
      type: 'piecewise',
      thresholds: [14, 19, 25, 30],
      colors: [COLOR_R, COLOR_Y, COLOR_G, COLOR_Y, COLOR_R]
    }
  },
  hum: {
    dataKey: 'hum',
    label: 'Humidity %RH',
    valueFormatter: value => value.toFixed(1),
    colorMap: {
      type: 'piecewise',
      thresholds: [25, 30, 60, 65],
      colors: [COLOR_R, COLOR_Y, COLOR_G, COLOR_Y, COLOR_R]
    }
  },
  co2Raw: {
    dataKey: 'co2Lpf',
    label: 'CO₂ ppm (filtered)',
    valueFormatter: value => value.toFixed(0),
    colorMap: {
      type: 'continuous',
      min: 600,
      max: 1000,
      color: ['green', 'red']
    }
  },
  hpa: {
    dataKey: 'hpa',
    label: 'Pressure hPa',
    valueFormatter: value => value.toFixed(1),
  },
  bat: {
    dataKey: 'bat',
    label: 'Battery %',
    valueFormatter: value => value.toFixed(1),
  }
}

const TabValues = (props: ITabProperties) => {

  const { boxUrl } = { ...props };

  const [orientation, setOrientation] = useState<TOrientation>('landscape');
  const [height, setHeight] = useState<number>(400);
  const [latest, setLatest] = useState<ILatest>({
    time: '',
    co2_lpf: 0,
    deg: 0,
    hum: 0,
    co2_raw: 0,
    hpa: 0,
    bat: 0
  });
  const [dateRangeData, setDateRangeData] = useState<[Date, Date]>([new Date(), new Date()]);
  const [dateRangeUser, setDateRangeUser] = useState<[Date, Date]>([new Date(), new Date()]);
  const [records, setRecords] = useState<IRecord[]>([]);
  const [seriesDef, setSeriesDef] = useState<ISeriesDef>(seriesDefs.co2Lpf);

  let latestTo: number = -1;
  const getLatestValues = () => {

    new JsonLoader().load(`${boxUrl}/latest`).then((_latest: ILatest) => {
      setLatest(_latest);
      const seconds = (Date.now() / 1000) % 60;
      const secwait = 70 - seconds;
      window.clearTimeout(latestTo);
      latestTo = window.setTimeout(() => {
        getLatestValues();
      }, secwait * 1000);
      // TODO :: think about recovery in case of stray error
    }).catch(e => {
      console.log('e', e);
    });

  }

  const getDateRange = () => {

    TimeUtil.collectYears(boxUrl).then(_dateRange => {

      const dateMaxMisc = new Date(_dateRange[1].getTime() + TimeUtil.MILLISECONDS_PER_HOUR * 23 + TimeUtil.MILLISECONDS_PER_MINUTE * 59);
      const dateMinData = _dateRange[0];
      const dateMinUser = _dateRange[1];
      setDateRangeData([dateMinData, dateMaxMisc]);
      setDateRangeUser([dateMinUser, dateMaxMisc]);
    }).catch(e => {
      console.log(e);
    });

  }

  const handleResize = () => {
    setOrientation(window.innerWidth > window.innerHeight ? 'landscape' : 'portrait');
    const offY = 80 + document.getElementById('valuebar').getBoundingClientRect().height;
    setHeight(window.innerHeight - offY);
  }

  useEffect(() => {

    console.debug(`⚙ updating tab values component (dateRangeUser)`, dateRangeUser);

    const minInstant = dateRangeUser[0].getTime(); // + TimeUtil.MILLISECONDS_PER_HOUR * 6;
    const maxInstant = dateRangeUser[1].getTime(); // + TimeUtil.MILLISECONDS_PER_HOUR * 18;

    let urlDate: Date;
    const curDate = new Date();
    let urls: string[] = [];
    for (let instant = minInstant; instant <= maxInstant; instant += TimeUtil.MILLISECONDS_PER__DAY) {
      urlDate = new Date(instant);
      console.log('urlDate', urlDate);
      urls.push(`${boxUrl}/datout?file=${urlDate.getFullYear()}/${String(urlDate.getMonth() + 1).padStart(2, '0')}/${TimeUtil.toUTCDate(urlDate)}.dat`);
      if (TimeUtil.toLocalDate(urlDate.getTime()) === TimeUtil.toLocalDate(curDate.getTime())) {
        urls.push(`${boxUrl}/valout`);
      }
    }

    console.log('urls', urls);

    new ByteLoader().loadAll(urls).then(_records => {
      _records = _records.filter(r => r.instant >= minInstant && r.instant <= maxInstant);
      setRecords(_records);
    });

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [dateRangeUser]);

  useEffect(() => {

    console.debug('✨ building tab values component');

    window.addEventListener('resize', handleResize);

    handleResize();
    getLatestValues();
    getDateRange();

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const customize = {
    legend: { hidden: true },
    margin: { top: 25, right: 25, bottom: 25, left: 55 }
  };

  const handleValueClick = (value: TRecordKey) => {
    setSeriesDef(seriesDefs[value]);
  }

  const handleDateMinChanged = (value: moment.Moment) => {
    const dateMin = new Date(value.year(), value.month(), value.date(), value.hour(), value.minute());
    setDateRangeUser([dateMin, dateRangeUser[1]]);
  };

  const handleDateMaxChanged = (value: moment.Moment) => {
    const dateMax = new Date(value.year(), value.month(), value.date(), value.hour(), value.minute());
    setDateRangeUser([dateRangeUser[0], dateMax]);
  };

  return (
    <Stack spacing={0} sx={{ padding: '0px', flexGrow: 10, display: 'flex' }}>
      <LocalizationProvider dateAdapter={AdapterMoment}>
        <div id="valuebar" style={{ display: 'flex', flexDirection: 'row', flexWrap: 'wrap', margin: '0px', flexGrow: 10 }}>
          <Value
            icon={<Co2Icon />}
            value={latest.co2_lpf.toFixed(0)}
            unit='ppm'
            grow='1'
            handleClick={() => handleValueClick('co2Lpf')}
          ></Value>
          <Value
            icon={<DeviceThermostatIcon />}
            value={latest.deg.toFixed(1)}
            unit='°C'
            grow='1'
            handleClick={() => handleValueClick('deg')}
          ></Value>
          <Value
            icon={<WaterDropIcon />}
            value={latest.hum.toFixed(1)}
            unit='%'
            grow='1'
            handleClick={() => handleValueClick('hum')}
          ></Value>
          <Value
            icon={<SpeedIcon />}
            value={latest.hpa.toFixed(1)}
            unit='hPa'
            grow='1'
            handleClick={() => handleValueClick('hpa')}
          ></Value>
        </div>

        {/* <div style={{ width: '100%', flexGrow: 1000, backgroundColor: 'red', display: 'flex', flexDirection: 'column' }}> */}
        <Stack direction={'column'} spacing={2} sx={{ flexGrow: 1000 }}>
          <LineChart
            height={height}
            xAxis={[{
              dataKey: 'instant',
              valueFormatter: (instant) => TimeUtil.toLocalTime(instant),
              min: dateRangeUser[0],
              max: dateRangeUser[1]
            }]}
            yAxis={[{
              colorMap: seriesDef.colorMap,
              valueFormatter: seriesDef.valueFormatter
            }]}
            series={[{
              dataKey: seriesDef.dataKey,
              label: seriesDef.label,
              showMark: false,
              type: 'line',
              curve: 'linear',
              valueFormatter: seriesDef.valueFormatter
            }]}
            dataset={records}
            {...customize}
          />
          <Stack direction={'row'} spacing={0} sx={{ flexGrow: 10 }}>
            {orientation === 'landscape' ? <div style={{ width: '50px' }}></div> : null}
            <DateTimePicker
              value={moment(dateRangeUser[0])}
              minDateTime={moment(dateRangeData[0])} // lowest possible value
              maxDateTime={moment(dateRangeUser[1])}
              onAccept={(newValue) => handleDateMinChanged(newValue)}
              label="from"
              orientation={orientation}
              desktopModeMediaQuery='(min-width:300px)'
              viewRenderers={{
                hours: renderTimeViewClock,
                minutes: renderTimeViewClock
              }}
              sx={{ maxWidth: '300px', marginRight: '5px' }}
            />
            <div style={{ flexGrow: 15 }}></div>
            <DateTimePicker
              value={moment(dateRangeUser[1])}
              minDateTime={moment(dateRangeUser[0])}
              maxDateTime={moment(dateRangeData[1])} // highest possible value
              onAccept={(newValue) => handleDateMaxChanged(newValue)}
              label="to"
              orientation={orientation}
              desktopModeMediaQuery='(min-width:300px)'
              viewRenderers={{
                day: renderDateViewCalendar,
                hours: renderTimeViewClock,
                minutes: renderTimeViewClock
              }}
              sx={{ maxWidth: '300px', marginLeft: '5px' }}
            />
            {orientation === 'landscape' ? <div style={{ width: '10px' }}></div> : null}
          </Stack>
        </Stack>
        {/* <Stack spacing={0} sx={{ minHeight: '48px', flexDirection: 'row', alignItems: 'center' }}>
        <Typography sx={{ flexGrow: 100, padding: '5px' }}>{boxUrl}</Typography>
        <Value
          icon={<BatteryStdIcon />}
          value={valBat.toFixed(1)}
          unit='%'
          grow='1'
        ></Value>
      </Stack> */}
      </LocalizationProvider>
    </Stack >
  );

};

export default TabValues;
