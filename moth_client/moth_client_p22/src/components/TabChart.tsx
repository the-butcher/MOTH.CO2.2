import { FormControl, InputLabel, MenuItem, Select } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import { DatePicker } from '@mui/x-date-pickers/DatePicker';
import { WheelEvent, useEffect, useState } from 'react';
import { CartesianGrid, Label, Line, LineChart, ResponsiveContainer, Tooltip, XAxis, YAxis } from 'recharts';

import { AdapterMoment } from '@mui/x-date-pickers/AdapterMoment';
import { LocalizationProvider } from '@mui/x-date-pickers/LocalizationProvider';
import moment from 'moment';
import 'moment/locale/de';

import { TimePicker } from '@mui/x-date-pickers';
import { ByteLoader } from '../util/ByteLoader';
import { IChartProperty } from '../types/IChartProperty';
import { IRecord } from '../types/IRecord';
import { TimeUtil } from '../util/TimeUtil';
import { ITabProperties } from '../types/ITabProperties';

const propertyLookup: { [K in string]: IChartProperty } = {
  'co2Lpf': {
    label: 'CO₂ ppm (filtered)',
    toDomain: (data: IRecord[]) => {
      let max = Math.max(...data.map(data => data.co2Raw)); // co2Raw on purpose to get same value range
      max = Math.ceil(max / 250) * 250;
      return [
        0,
        max
      ];
    },
    thresholds: {
      rLo: 0,
      wLo: 0,
      wHi: 800,
      rHi: 1000
    }
  },
  'co2Raw': {
    label: 'CO₂ ppm (raw)',
    toDomain: (data: IRecord[]) => {
      let max = Math.max(...data.map(data => data.co2Raw));
      max = Math.ceil(max / 250) * 250;
      return [
        0,
        max
      ];
    },
    thresholds: {
      rLo: 0,
      wLo: 0,
      wHi: 800,
      rHi: 1000
    }
  },
  'deg': {
    label: 'Temperature °C',
    toDomain: (data: IRecord[]) => {
      let max = Math.max(...data.map(data => data.deg));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      rLo: 14,
      wLo: 19,
      wHi: 25,
      rHi: 30
    }
  },
  'hum': {
    label: 'Humidity %RH',
    toDomain: (data: IRecord[]) => {
      let max = Math.max(...data.map(data => data.hum));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      rLo: 25,
      wLo: 30,
      wHi: 60,
      rHi: 65
    }
  },
  'hpa': {
    label: 'Pressure hPa',
    toDomain: (data: IRecord[]) => {
      let min = Math.min(...data.map(data => data.hpa));
      let max = Math.max(...data.map(data => data.hpa));
      min = Math.floor(min / 5) * 5;
      max = Math.ceil(max / 5) * 5;
      return [
        min,
        max
      ];
    },
    thresholds: {
      rLo: 0,
      wLo: 0,
      wHi: 10000,
      rHi: 10000
    }
  },
  'bat': {
    label: 'Battery %',
    toDomain: (data: IRecord[]) => {
      return [
        0,
        100
      ];
    },
    thresholds: {
      rLo: 10,
      wLo: 20,
      wHi: 100,
      rHi: 100
    }
  }
}


const TabChart = (props: ITabProperties) => {

  const { boxUrl } = { ...props };

  const [chartData, setChartData] = useState<IRecord[]>([]);
  const [chartMinX, setChartMinX] = useState<number>();
  const [chartMaxX, setChartMaxX] = useState<number>();

  const [dispData, setDispData] = useState<IRecord[]>([]);
  const [dispMinX, setDispMinX] = useState<number>();
  const [dispMaxX, setDispMaxX] = useState<number>();
  const [domain, setDomain] = useState<[number, number]>();
  const [stops, setStops] = useState<string[]>(['0%', '0%', '0%', '100%', '100%', '100%']);

  const [dateRange, setDateRange] = useState<[Date, Date]>([new Date('2100-01-01'), new Date('2000-01-01')]);
  const [chartTicks, setChartTicks] = useState<number[]>([]);

  const [chartProp, setChartProp] = useState<string>('co2Lpf');
  const [chartRefX, setChartRefX] = useState<number>();

  const handleDateChanged = (value: moment.Moment) => {

    const _datePick = new Date(value.year(), value.month(), value.date());
    setChartRefX(_datePick.getTime());

    const file = `${value.year()}/${String(value.month() + 1).padStart(2, '0')}/${value.year()}${String(value.month() + 1).padStart(2, '0')}${String(value.date()).padStart(2, '0')}.dat`;
    let urls: string[] = [
      `${boxUrl}/datout?file=${file}`
    ];
    if (TimeUtil.toLocalDate(Date.now()) === TimeUtil.toLocalDate(_datePick.getTime())) {
      urls.push(`${boxUrl}/valout`);
    }

    new ByteLoader().loadAll(urls).then(records => {
      updateDataset(records);
    });

  };

  const updateDataset = (dataset: IRecord[]) => {
    setChartData(dataset);
    setChartMinX(dataset[0].instant);
    setChartMaxX(dataset[dataset.length - 1].instant);
    setDispMinX(dataset[0].instant);
    setDispMaxX(dataset[dataset.length - 1].instant);
  };

  useEffect(() => {

    console.debug('✨ building chartapp component');

    TimeUtil.collectYears(boxUrl).then(_dateRange => {

      setDateRange(_dateRange);
      handleDateChanged(moment(_dateRange[1]));

      document.addEventListener('wheel', (e) => {

        const chartGrid = document.getElementsByClassName('recharts-cartesian-grid').item(0) as SVGGElement;
        const chartRect = chartGrid.getClientRects().item(0);
        if (e.clientY >= chartRect.y && e.clientY <= chartRect.y + chartRect.height) {
          if (e.clientX >= chartRect.x && e.clientX <= chartRect.x + chartRect.width) {
            e.preventDefault();
          }
        }

      }, { passive: false });

    }).catch(e => {
      console.error(e);
    });

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const menuItems: JSX.Element[] = [];
  for (var property in propertyLookup) {
    menuItems.push(<MenuItem key={property} value={property}>{propertyLookup[property].label}</MenuItem>);
  }

  const formatDate = (tickItem: Date) => {
    return moment(tickItem).format('HH:mm')
  }

  const formatValue = (tickItem: number) => {
    return tickItem !== 0 ? tickItem.toLocaleString() : '';
  }

  const handleChartValueChanged = (_chartProp: string) => {
    setChartProp(_chartProp);
  }

  const handleMouseWheel = (e: WheelEvent) => {

    const chartGrid = document.getElementsByClassName('recharts-cartesian-grid').item(0) as SVGGElement;
    const chartRect = chartGrid.getClientRects().item(0);
    if (e.clientY >= chartRect.y && e.clientY <= chartRect.y + chartRect.height) {
      if (e.clientX >= chartRect.x && e.clientX <= chartRect.x + chartRect.width) {

        // the fraction and the instant where the scroll event occured
        const fracX = (e.clientX - chartRect.x) / chartRect.width;
        const instX = dispMinX + (dispMaxX - dispMinX) * fracX;

        // given the direction of the mouse wheel chartMinX and chartMaxX can now be adjusted
        const mult = e.deltaY > 0 ? 0.98 : 1 / 0.98;
        const offMinX = instX - dispMinX;
        const offMaxY = dispMaxX - instX;
        let _chartMinX = instX - offMinX * mult;
        if (_chartMinX < chartData[0].instant) {
          _chartMinX = chartData[0].instant;
        }
        let _chartMaxX = instX + offMaxY * mult;
        if (_chartMaxX > chartData[chartData.length - 1].instant) {
          _chartMaxX = chartData[chartData.length - 1].instant
        }

        setDispMinX(_chartMinX);
        setDispMaxX(_chartMaxX);

      }
    }

  }

  const handleDispMinChange = (value: moment.Moment) => {
    let _chartMinX = chartRefX + value.hours() * 60 * 60 * 1000 + value.minutes() * 60 * 1000;
    if (_chartMinX < chartData[0].instant) {
      _chartMinX = chartData[0].instant;
    }
    if (_chartMinX < dispMaxX) {
      setDispMinX(_chartMinX);
    }
  }

  const handleDispMaxChange = (value: moment.Moment) => {
    let _chartMaxX = chartRefX + value.hours() * 60 * 60 * 1000 + value.minutes() * 60 * 1000;
    if (_chartMaxX > chartData[chartData.length - 1].instant) {
      _chartMaxX = chartData[chartData.length - 1].instant
    }
    if (_chartMaxX > dispMinX) {
      setDispMaxX(_chartMaxX);
    }
  }

  useEffect(() => {

    let tickSpacing = 60;
    const chartGrid = document.getElementsByClassName('recharts-cartesian-grid').item(0) as SVGGElement;
    if (chartGrid) {
      const chartRect = chartGrid.getClientRects().item(0);
      if (chartRect) {
        const pxPerHour = chartRect.width * 1000 * 60 * 60 / (dispMaxX - dispMinX);
        if (pxPerHour > 750) {
          tickSpacing = 5;
        } else if (pxPerHour > 250) {
          tickSpacing = 15;
        } else if (pxPerHour > 125) {
          tickSpacing = 30;
        }
      }
    }

    const _dispData = chartData.filter((data: any) => data.instant >= dispMinX && data.instant <= dispMaxX);
    const _chartTicks = _dispData.filter((data: any) => new Date(data.instant).getMinutes() % tickSpacing === 0).map((data: any) => data.instant);
    setChartTicks(_chartTicks);
    setDispData(_dispData);

  }, [chartData, chartProp, dispMinX, dispMaxX]);

  useEffect(() => {

    setDomain(propertyLookup[chartProp].toDomain(dispData));

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [dispData]);

  useEffect(() => {

    if (domain) {

      const fracRiskHi = toDomainFraction(propertyLookup[chartProp].thresholds.rHi, domain);
      const fracWarnHi = toDomainFraction(propertyLookup[chartProp].thresholds.wHi, domain);
      const fracWarnLo = toDomainFraction(propertyLookup[chartProp].thresholds.wLo, domain);
      const fracRiskLo = toDomainFraction(propertyLookup[chartProp].thresholds.rLo, domain);
      const _stops = [
        '0%',
        `${100 - fracRiskHi}%`,
        `${100 - fracWarnHi}%`,
        `${100 - fracWarnLo}%`,
        `${100 - fracRiskLo}%`,
        '100%'
      ];
      setStops(_stops);

    }

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [domain]);

  // const toLocalDate = (instant: number) => {

  //   return new Date(instant).toLocaleDateString(window.navigator.language, { // you can use undefined as first argument
  //     year: "numeric",
  //     month: "2-digit",
  //     day: "2-digit",
  //   })

  // };

  const toDomainFraction = (val: number, domain: [number, number]) => {
    if (val > domain[1]) {
      return 100;
    } else if (val < domain[0]) {
      return 0;
    } else {
      return Math.round((val - domain[0]) * 100 / (domain[1] - domain[0]));
    }
  }

  return (
    <div style={{ height: '100%' }}>

      <iframe title="mockframe" id="mockframe" src='' style={{ height: '10px', border: 'none' }} />
      <Card sx={{ padding: '0px' }}>

        <CardContent>
          <LocalizationProvider dateAdapter={AdapterMoment}>

            <DatePicker sx={{ margin: '6px', width: '250px' }}
              label="Date"
              value={moment(dateRange[1])}
              minDate={moment(dateRange[0])}
              maxDate={moment(dateRange[1])}
              onChange={(newValue) => handleDateChanged(newValue)}
            />

            <FormControl variant="outlined">
              <InputLabel id="prop-label" sx={{ margin: '6px' }}>Prop</InputLabel>
              <Select sx={{ margin: '6px', width: '250px' }}
                labelId="prop-label"
                id="demo-simple-select"
                value={chartProp}
                label="Prop"
                onChange={event => handleChartValueChanged(event.target.value)}
              >
                {menuItems}
              </Select>
            </FormControl>

            <TimePicker sx={{ margin: '6px', width: '250px' }}
              label="From"
              value={moment(dispMinX)}
              onChange={(min) => handleDispMinChange(min)}
              minTime={moment(chartMinX)}
              maxTime={moment(dispMaxX)}
            />
            <TimePicker sx={{ margin: '6px', width: '250px' }}
              label="To"
              value={moment(dispMaxX)}
              onChange={(max) => handleDispMaxChange(max)}
              minTime={moment(dispMinX)}
              maxTime={moment(chartMaxX)}
            />


          </LocalizationProvider>
        </CardContent>
      </Card>
      <Card sx={{ height: '100%', padding: '0px' }} onWheel={handleMouseWheel}>
        <CardContent style={{ width: 'inherit', height: 'inherit', padding: '0px', margin: '0px' }}>

          <ResponsiveContainer >
            <LineChart
              data={dispData}
              margin={{
                top: 20,
                right: 30,
                left: 30,
                bottom: 35,
              }}
            >
              <defs>
                <linearGradient id="colorUv" x1="0" y1="0" x2="0" y2="1">
                  <stop offset={stops[0]} stopColor="#FF0000" />
                  <stop offset={stops[1]} stopColor="#CCCC00" />
                  <stop offset={stops[2]} stopColor="#00AA00" />
                  <stop offset={stops[3]} stopColor="#00AA00" />
                  <stop offset={stops[4]} stopColor="#CCCC00" />
                  <stop offset={stops[5]} stopColor="#FF0000" />
                </linearGradient>
              </defs>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="instant" tickFormatter={formatDate} ticks={chartTicks} >
                <Label value={TimeUtil.toLocalDate(dispMinX)} dy={20} />
              </XAxis>
              <YAxis tickFormatter={formatValue} domain={domain} >
                <Label value={propertyLookup[chartProp].label} angle={270} dx={-40} />
              </YAxis>
              <Tooltip labelFormatter={value => formatDate(new Date(value as number))} />
              <Line type="linear" animationDuration={100} dataKey={chartProp} stroke="url(#colorUv)" strokeWidth={3} dot={{ r: 0 }} activeDot={{ r: 4 }} />
            </LineChart>
          </ResponsiveContainer>

        </CardContent>
      </Card>
      <div style={{ height: '12px' }}></div>

    </div>
  );

};

export default TabChart;
