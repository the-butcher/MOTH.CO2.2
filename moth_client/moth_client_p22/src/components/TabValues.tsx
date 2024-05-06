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
import { createRef, useEffect, useRef, useState } from 'react';
import { ILatest } from '../types/ILatest';
import { TRecordKey } from '../types/IRecord';
import { TOrientation } from '../types/ITabProps';
import { ByteLoader } from '../util/ByteLoader';
import { JsonLoader } from '../util/JsonLoader';
import { TimeUtil } from '../util/TimeUtil';

import { ITabValuesProps } from '../types/ITabValuesProps';
import ValueChoice from './ValueChoice';
import { SERIES_DEFS } from '../types/ISeriesDef';

// const DIV_ID_VALUES = 'valuediv';


const TabValues = (props: ITabValuesProps) => {

  const { boxUrl, latest, dateRangeData, dateRangeUser, records, seriesDef, handleUpdate } = { ...props };

  const [orientation, setOrientation] = useState<TOrientation>('landscape');
  const [height, setHeight] = useState<number>(400);
  const [resizeCount, setResizeCount] = useState<number>();

  const latestToRef = useRef<number>(-1);

  const chartRef = createRef<SVGElement>();
  const valueRef = createRef<HTMLDivElement>();

  const getLatestValues = () => {
    new JsonLoader().load(`${boxUrl}/latest`).then((latest: ILatest) => {
      handleUpdate({
        latest
      });
    }).catch(e => {
      console.log('e', e);
    });
  }

  const loadDateRange = () => {

    TimeUtil.collectYears(boxUrl).then(_dateRange => {
      const dateMaxMisc = new Date();
      const dateMinData = _dateRange[0];
      const dateMinUser = _dateRange[1];
      handleUpdate({
        dateRangeData: [dateMinData, dateMaxMisc],
        dateRangeUser: [dateMinUser, dateMaxMisc]
      });
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

    new ByteLoader().loadAll(Array.from(urlset)).then(records => {
      records = records.filter(r => r.instant >= minInstant && r.instant <= maxInstant);
      handleUpdate({
        records
      });
    }).catch(e => {
      console.log('e', e);
    });

  }

  const handleResize = () => {
    setResizeCount(Math.random());
  }


  /**
   * https://gist.github.com/SunPj/14fe4f10db43be2d84751f5595d48246
   * @param stylesheet
   * @returns
   */
  const stringifyStylesheet = (stylesheet: CSSStyleSheet): string => {
    return stylesheet.cssRules ? Array.from(stylesheet.cssRules).map(rule => rule.cssText || '').join('\n') : '';
  }
  const collectStyles = (): string => {
    return Array.from(document.styleSheets).map(s => stringifyStylesheet(s)).join("\n");
  }
  const collectDefs = (): string => {
    const styles = collectStyles()
    console.log('styles', styles);
    return `<defs><style type="text/css"><![CDATA[${styles}]]></style></defs>`
  }

  const exportToPng = () => {

    const chartSvg = chartRef.current;
    const { width, height } = chartSvg.getBoundingClientRect();

    const chartSvgClone: SVGElement = chartSvg.cloneNode(true) as SVGElement;

    const defs = collectDefs()
    chartSvgClone.insertAdjacentHTML("afterbegin", defs);

    const chartOuterHTML = (new XMLSerializer()).serializeToString(chartSvgClone);
    const chartBlob = new Blob([chartOuterHTML], {
      type: 'image/svg+xml;charset=utf-8'
    });
    const chartBlobUrl = URL.createObjectURL(chartBlob);

    const image = new Image();
    image.onload = () => {

      const canvas = document.createElement('canvas');
      canvas.width = width;
      canvas.height = height;

      const context = canvas.getContext('2d');
      context.fillStyle = 'white';
      context.fillRect(0, 0, width, height);
      context.drawImage(image, 0, 0, width, height);

      const pngDataUrl = canvas.toDataURL();
      const pngDownloadLink = document.createElement('a');
      pngDownloadLink.setAttribute('href', pngDataUrl);
      pngDownloadLink.setAttribute('download', "chart"); // TODO format with dates
      pngDownloadLink.click();

    };
    image.onerror = (e) => {
      console.error('e', e);
    };
    image.src = chartBlobUrl;



  }

  useEffect(() => {

    console.debug(`⚙ updating tab values component (resizeCount)`, resizeCount);

    if (valueRef.current) {
      setOrientation(window.innerWidth > window.innerHeight ? 'landscape' : 'portrait');
      const offY = 80 + valueRef.current.getBoundingClientRect().height;
      setHeight(window.innerHeight - offY);
    }

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [resizeCount]);

  useEffect(() => {

    console.debug(`⚙ updating tab values component (latest)`, latest);

    const updates: Partial<ITabValuesProps> = {
      dateRangeData: [dateRangeData[0], new Date()]
    };
    // if the user range is close to "now" adjust user range to include the newest records
    if (Math.abs(dateRangeUser[1].getTime() - new Date().getTime()) <= TimeUtil.MILLISECONDS_PER_MINUTE * 5) {
      updates.dateRangeUser = [dateRangeUser[0], new Date()];
    }
    handleUpdate(updates);

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

    loadDateRange();

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const handleValueClick = (value: TRecordKey) => {
    handleUpdate({
      seriesDef: SERIES_DEFS[value]
    });
  }

  const handleDateMinChanged = (value: moment.Moment) => {

    console.debug(`📞 handling date min change`, value);

    const dateMin = new Date(value.year(), value.month(), value.date(), value.hour(), value.minute());
    handleUpdate({
      dateRangeUser: [dateMin, dateRangeUser[1]]
    })

  };

  const handleDateMaxChanged = (value: moment.Moment) => {

    console.debug(`📞 handling date max change`, value);

    const dateMax = new Date(value.year(), value.month(), value.date(), value.hour(), value.minute());
    handleUpdate({
      dateRangeUser: [dateRangeUser[0], dateMax]
    })

  };

  return (
    <>
      <Stack spacing={1} sx={{ flexDirection: 'column', position: 'fixed', left: '14px', top: '170px', ...props.style }}>
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
          onClick={exportToPng}
        >
          <ImageIcon sx={{ fontSize: '1.0em' }} />
        </IconButton>
      </Stack>
      <Stack spacing={0} sx={{ padding: '0px', flexGrow: 10, display: 'flex', ...props.style }}>
        <LocalizationProvider dateAdapter={AdapterMoment}>
          <div ref={valueRef} style={{ display: 'flex', flexDirection: 'row', flexWrap: 'wrap', margin: '0px', flexGrow: 10 }}>
            <div style={{ display: 'flex', flexDirection: 'row', flexWrap: 'wrap', flexGrow: 10 }}>
              <ValueChoice
                icon={<Co2Icon sx={{ fontSize: '0.8em' }} />}
                value={latest.co2_lpf.toFixed(0)}
                unit='ppm'
                grow='5'
                active={seriesDef.id === 'co2Lpf'}
                handleClick={() => handleValueClick('co2Lpf')}
              ></ValueChoice>
              <ValueChoice
                icon={<DeviceThermostatIcon sx={{ fontSize: '0.8em' }} />}
                value={latest.deg.toFixed(1)}
                unit='°C'
                grow='5'
                active={seriesDef.id === 'deg'}
                handleClick={() => handleValueClick('deg')}
              ></ValueChoice>
            </div>
            <div style={{ display: 'flex', flexDirection: 'row', flexWrap: 'wrap', flexGrow: 10 }}>
              <ValueChoice
                icon={<WaterDropIcon sx={{ fontSize: '0.8em' }} />}
                value={latest.hum.toFixed(1)}
                unit='%'
                grow='5'
                active={seriesDef.id === 'hum'}
                handleClick={() => handleValueClick('hum')}
              ></ValueChoice>
              <ValueChoice
                icon={<SpeedIcon sx={{ fontSize: '0.8em' }} />}
                value={latest.hpa.toFixed(1)}
                unit='hPa'
                grow='5'
                active={seriesDef.id === 'hpa'}
                handleClick={() => handleValueClick('hpa')}
              ></ValueChoice>
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
            <div>
              <LineChart
                ref={chartRef}
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
            </div>
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
        </LocalizationProvider >
      </Stack >
    </>
  );

};

export default TabValues;
