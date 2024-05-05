import Co2Icon from '@mui/icons-material/Co2';
import DeviceThermostatIcon from '@mui/icons-material/DeviceThermostat';
import ImageIcon from '@mui/icons-material/Image';
import SpeedIcon from '@mui/icons-material/Speed';
import TableRowsIcon from '@mui/icons-material/TableRows';
import WaterDropIcon from '@mui/icons-material/WaterDrop';
import { IconButton, Stack } from '@mui/material';
import { axisClasses } from '@mui/x-charts/ChartsAxis';
import { LineChart } from '@mui/x-charts/LineChart';
import { DateTimePicker, renderDateViewCalendar, renderTimeViewClock } from '@mui/x-date-pickers';
import { AdapterMoment } from '@mui/x-date-pickers/AdapterMoment';
import { LocalizationProvider } from '@mui/x-date-pickers/LocalizationProvider';
import moment from 'moment';
import 'moment/locale/de';
import { useEffect, useRef, useState } from 'react';
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
    id: 'instant',
    label: 'time',
    valueFormatter: value => Number.isFinite(value) ? value.toString() : 'NA'
  },
  co2Lpf: {
    id: 'co2Lpf',
    label: 'CO₂ ppm (filtered)',
    valueFormatter: value => Number.isFinite(value) ? value.toFixed(0) : 'NA',
    colorMap: {
      type: 'piecewise',
      thresholds: [800, 1000],
      colors: [COLOR_G, COLOR_Y, COLOR_R]
    },
    min: 0
  },
  deg: {
    id: 'deg',
    label: 'Temperature °C',
    valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
    colorMap: {
      type: 'piecewise',
      thresholds: [14, 19, 25, 30],
      colors: [COLOR_R, COLOR_Y, COLOR_G, COLOR_Y, COLOR_R]
    }
  },
  hum: {
    id: 'hum',
    label: 'Humidity %RH',
    valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
    colorMap: {
      type: 'piecewise',
      thresholds: [25, 30, 60, 65],
      colors: [COLOR_R, COLOR_Y, COLOR_G, COLOR_Y, COLOR_R]
    }
  },
  co2Raw: {
    id: 'co2Lpf',
    label: 'CO₂ ppm (filtered)',
    valueFormatter: value => Number.isFinite(value) ? value.toFixed(0) : 'NA',
    colorMap: {
      type: 'continuous',
      min: 600,
      max: 1000,
      color: ['green', 'red']
    },
    min: 0
  },
  hpa: {
    id: 'hpa',
    label: 'Pressure hPa',
    valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
  },
  bat: {
    id: 'bat',
    label: 'Battery %',
    valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
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

  let latestToRef = useRef<number>(-1);

  const getLatestValues = () => {
    new JsonLoader().load(`${boxUrl}/latest`).then((_latest: ILatest) => {
      setLatest(_latest);
    }).catch(e => {
      console.log('e', e);
    });
  }

  const loadDateRange = () => {

    TimeUtil.collectYears(boxUrl).then(_dateRange => {
      const dateMaxMisc = new Date(); // new Date(_dateRange[1].getTime() + TimeUtil.MILLISECONDS_PER_HOUR * 23 + TimeUtil.MILLISECONDS_PER_MINUTE * 59);
      const dateMinData = _dateRange[0];
      const dateMinUser = _dateRange[1];
      setDateRangeData([dateMinData, dateMaxMisc]);
      setDateRangeUser([dateMinUser, dateMaxMisc]);
    }).catch(e => {
      console.log('e', e);
    });

  }

  const loadRecords = () => {

    const minInstant = dateRangeUser[0].getTime(); // + TimeUtil.MILLISECONDS_PER_HOUR * 6;
    const maxInstant = dateRangeUser[1].getTime(); // + TimeUtil.MILLISECONDS_PER_HOUR * 18;

    const curDate = new Date();
    const urlset = new Set<string>();
    const pushInstant = (instant: number) => {
      const urlDate = new Date(instant);
      const url = `${boxUrl}/datout?file=${urlDate.getFullYear()}/${String(urlDate.getMonth() + 1).padStart(2, '0')}/${TimeUtil.toUTCDate(urlDate)}.dat`;
      // console.log('urlDate', urlDate, url);
      urlset.add(url);
      if (TimeUtil.toLocalDate(urlDate.getTime()) === TimeUtil.toLocalDate(curDate.getTime())) {
        urlset.add(`${boxUrl}/valout`);
      }
    };
    for (let instant = minInstant; instant < maxInstant; instant += TimeUtil.MILLISECONDS_PER__DAY) {
      pushInstant(instant);
    }
    pushInstant(maxInstant);

    // console.log('urlset', urlset);

    new ByteLoader().loadAll(Array.from(urlset)).then(_records => {
      _records = _records.filter(r => r.instant >= minInstant && r.instant <= maxInstant);
      setRecords(_records);
    }).catch(e => {
      console.log('e', e);
    });

  }

  const handleResize = () => {
    setOrientation(window.innerWidth > window.innerHeight ? 'landscape' : 'portrait');
    const offY = 80 + document.getElementById('valuebar').getBoundingClientRect().height;
    setHeight(window.innerHeight - offY);
  }

  useEffect(() => {

    console.debug(`⚙ updating tab values component (latest)`, latest);

    setDateRangeData([dateRangeData[0], new Date()]);
    // if the user range is close to "now" adjust user range to include the newest records
    if (Math.abs(dateRangeUser[1].getTime() - new Date().getTime()) <= TimeUtil.MILLISECONDS_PER_MINUTE * 5) {
      setDateRangeUser([dateRangeUser[0], new Date()]);
    }

    const seconds = (Date.now() / 1000) % 60;
    const secwait = 70 - seconds;
    window.clearTimeout(latestToRef.current);
    latestToRef.current = window.setTimeout(() => getLatestValues(), secwait * 1000);

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [latest]);

  useEffect(() => {

    console.debug(`⚙ updating tab values component (dateRangeUser)`, dateRangeUser);

    if (dateRangeUser) {

      if (latest.time === '') {
        getLatestValues();
      }
      loadRecords();

    }


    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [dateRangeUser]);

  useEffect(() => {

    console.debug('✨ building tab values component');

    window.addEventListener('resize', handleResize);
    handleResize();

    // getLatestValues();
    loadDateRange();

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);



  const handleValueClick = (value: TRecordKey) => {
    setSeriesDef(seriesDefs[value]);
  }

  const handleDateMinChanged = (value: moment.Moment) => {

    console.debug(`📞 handling date min change`, value);

    const dateMin = new Date(value.year(), value.month(), value.date(), value.hour(), value.minute());
    setDateRangeUser([dateMin, dateRangeUser[1]]);

  };

  const handleDateMaxChanged = (value: moment.Moment) => {

    console.debug(`📞 handling date max change`, value);

    const dateMax = new Date(value.year(), value.month(), value.date(), value.hour(), value.minute());
    setDateRangeUser([dateRangeUser[0], dateMax]);

  };

  const customize = {
    legend: { hidden: true },
  };

  return (
    <>
      <Stack spacing={1} sx={{ display: 'flex', flexDirection: 'column', position: 'fixed', left: '14px', top: '170px' }}>
        <IconButton
          sx={{ boxShadow: '0px 2px 4px -1px rgba(0,0,0,0.2),0px 4px 5px 0px rgba(0,0,0,0.14),0px 1px 10px 0px rgba(0,0,0,0.12)' }}
          aria-label="export csv"
          size='small'
        >
          <TableRowsIcon sx={{ fontSize: '1.0em' }} />
        </IconButton>
        <IconButton
          sx={{ boxShadow: '0px 2px 4px -1px rgba(0,0,0,0.2),0px 4px 5px 0px rgba(0,0,0,0.14),0px 1px 10px 0px rgba(0,0,0,0.12)' }}
          aria-label="export png"
          size='small'
        >
          <ImageIcon sx={{ fontSize: '1.0em' }} />
        </IconButton>
      </Stack>
      <Stack spacing={0} sx={{ padding: '0px', flexGrow: 10, display: 'flex' }}>

        <LocalizationProvider dateAdapter={AdapterMoment}>
          <div id="valuebar" style={{ display: 'flex', flexDirection: 'row', flexWrap: 'wrap', margin: '0px', flexGrow: 10 }}>
            <div style={{ display: 'flex', flexDirection: 'row', flexWrap: 'wrap', flexGrow: 10 }}>
              <Value
                icon={<Co2Icon sx={{ fontSize: '0.8em' }} />}
                value={latest.co2_lpf.toFixed(0)}
                unit='ppm'
                grow='5'
                active={seriesDef.id === 'co2Lpf'}
                handleClick={() => handleValueClick('co2Lpf')}
              ></Value>
              <Value
                icon={<DeviceThermostatIcon sx={{ fontSize: '0.8em' }} />}
                value={latest.deg.toFixed(1)}
                unit='°C'
                grow='5'
                active={seriesDef.id === 'deg'}
                handleClick={() => handleValueClick('deg')}
              ></Value>
            </div>
            <div style={{ display: 'flex', flexDirection: 'row', flexWrap: 'wrap', flexGrow: 10 }}>
              <Value
                icon={<WaterDropIcon sx={{ fontSize: '0.8em' }} />}
                value={latest.hum.toFixed(1)}
                unit='%'
                grow='5'
                active={seriesDef.id === 'hum'}
                handleClick={() => handleValueClick('hum')}
              ></Value>
              <Value
                icon={<SpeedIcon sx={{ fontSize: '0.8em' }} />}
                value={latest.hpa.toFixed(1)}
                unit='hPa'
                grow='5'
                active={seriesDef.id === 'hpa'}
                handleClick={() => handleValueClick('hpa')}
              ></Value>
              {/* <Value
              icon={<BatteryStdIcon />}
              value={latest.bat.toFixed(1)}
              unit='%'
              grow='5'
              handleClick={() => handleValueClick('bat')}
            ></Value> */}
            </div>
          </div>
          <Stack direction={'column'} spacing={2} sx={{ flexGrow: 1000 }}>
            <LineChart
              height={height}
              xAxis={[{
                dataKey: 'instant',
                valueFormatter: (instant) => TimeUtil.toLocalTime(instant),
                min: records.length > 0 ? (records[0].instant - TimeUtil.MILLISECONDS_PER_MINUTE * 5) : undefined,
                max: records.length > 0 ? (records[records.length - 1].instant + TimeUtil.MILLISECONDS_PER_MINUTE * 5) : undefined,
                label: 'time'
              }]}
              yAxis={[{
                colorMap: seriesDef.colorMap,
                valueFormatter: seriesDef.valueFormatter,
                min: seriesDef.min,
                label: `${seriesDef.label}`
              }]}
              series={[{
                dataKey: seriesDef.id,
                label: seriesDef.label,
                showMark: false,
                type: 'line',
                curve: 'linear',
                valueFormatter: seriesDef.valueFormatter
              }]}
              dataset={records}
              grid={{ vertical: true, horizontal: true }}
              margin={{ top: 15, right: 25, bottom: 40, left: 60 }}
              sx={{
                [`& .${axisClasses.left} .${axisClasses.label}`]: {
                  transform: 'translateX(-20px)',
                },
              }}
              {...{
                legend: { hidden: true }
              }}
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
    </>
  );

};

export default TabValues;
