import { createTheme, CssBaseline, Fab, FormControl, IconButton, InputLabel, MenuItem, Select, ThemeProvider, Typography } from '@mui/material';
import { Tooltip as TooltipR } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import { DatePicker } from '@mui/x-date-pickers/DatePicker';
import { useEffect, useState, WheelEvent } from 'react';
import { CartesianGrid, Label, Line, LineChart, ResponsiveContainer, Tooltip, XAxis, YAxis } from 'recharts';
import './App.css';
import { JsonLoader } from './util/JsonLoader';

import { AdapterMoment } from '@mui/x-date-pickers/AdapterMoment';
import { LocalizationProvider } from '@mui/x-date-pickers/LocalizationProvider';
import moment from 'moment';
import 'moment/locale/de';
import { CsvToJsonLoader } from './util/CsvToJsonLoader';

import ConstructionIcon from '@mui/icons-material/Construction';
import ShowChartIcon from '@mui/icons-material/ShowChart';
import WysiwygIcon from '@mui/icons-material/Wysiwyg';
import { TimePicker } from '@mui/x-date-pickers';
import { IChartProperty } from './util/IChartProperty';

const darkTheme = createTheme({
  typography: {
    fontFamily: [
      'SimplyMono-Bold',
    ].join(','),
    button: {
      textTransform: 'none'
    }
  },
  components: {
    MuiCard: {
      defaultProps: {
        elevation: 10
      },
      styleOverrides: {
        root: {
          margin: '6px',
          padding: '12px'
        }
      }
    },
  }
});

const propertyLookup: { [K in string]: IChartProperty } = {
  'co2': {
    label: 'CO₂ ppm',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.co2));
      max = Math.ceil(max / 250) * 250;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 800,
      riskHi: 1000
    }
  },
  'temperature': {
    label: 'Temperature °C',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.temperature));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 14,
      warnLo: 19,
      warnHi: 25,
      riskHi: 30
    }
  },
  'humidity': {
    label: 'Humidity %RH',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.humidity));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 25,
      warnLo: 30,
      warnHi: 60,
      riskHi: 65
    }
  },
  'temperature_bme': {
    label: 'Temperature °C (bme)',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.temperature_bme));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 14,
      warnLo: 19,
      warnHi: 25,
      riskHi: 30
    }
  },
  'humidity_bme': {
    label: 'Humidity %RH (bme)',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.humidity_bme));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 25,
      warnLo: 30,
      warnHi: 60,
      riskHi: 65
    }
  },
  'pressure': {
    label: 'Pressure hPa',
    toDomain: (data: any[]) => {
      let min = Math.min(...data.map(data => data.pressure));
      let max = Math.max(...data.map(data => data.pressure));
      min = Math.floor(min / 5) * 5;
      max = Math.ceil(max / 5) * 5;
      return [
        min,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 10000,
      riskHi: 10000
    }
  },
  'pm010': {
    label: 'PM 1.0 µg/m³',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pm010));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'pm025': {
    label: 'PM 2.5 µg/m³',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pm025));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'pm100': {
    label: 'PM 10.0 µg/m³',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pm100));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'pc003': {
    label: 'Particle count 0.3µm',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pc003));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'pc005': {
    label: 'Particle count 0.5µm',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pc005));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'pc010': {
    label: 'Particle count 1.0µm',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pc010));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'pc025': {
    label: 'Particle count 2.5µm',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pc025));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'pc050': {
    label: 'Particle count 5.0µm',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pc050));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'pc100': {
    label: 'Particle count 10.0µm',
    toDomain: (data: any[]) => {
      let max = Math.max(...data.map(data => data.pc100));
      max = Math.ceil(max / 5) * 5;
      return [
        0,
        max
      ];
    },
    thresholds: {
      riskLo: 0,
      warnLo: 0,
      warnHi: 15,
      riskHi: 45
    }
  },
  'percent': {
    label: 'Battery %',
    toDomain: (data: any[]) => {
      return [
        0,
        100
      ];
    },
    thresholds: {
      riskLo: 10,
      warnLo: 20,
      warnHi: 100,
      riskHi: 100
    }
  }
}


const ChartApp = () => {

  const boxUrl = `${window.location.origin}/api`; // when running directly from device
  // const boxUrl = `http://192.168.0.172/api`; // when running directly from device

  const [chartData, setChartData] = useState<any[]>([]);
  const [chartMinX, setChartMinX] = useState<number>();
  const [chartMaxX, setChartMaxX] = useState<number>();

  const [dispData, setDispData] = useState<any[]>([]);
  const [dispMinX, setDispMinX] = useState<number>();
  const [dispMaxX, setDispMaxX] = useState<number>();
  const [domain, setDomain] = useState<[number, number]>();
  const [stops, setStops] = useState<string[]>(['0%', '0%', '0%', '100%', '100%', '100%']);

  const [dateRange, setDateRange] = useState<[Date, Date]>([new Date('2100-01-01'), new Date('2000-01-01')]);
  const [chartTicks, setChartTicks] = useState<number[]>([]);

  const [chartProp, setChartProp] = useState<string>('co2');
  const [chartRefX, setChartRefX] = useState<number>();

  const collectDays = async (year: number, month: number, _dateRange: [Date, Date]) => {
    const urlYYYY_MM = `${boxUrl}/folder?folder=${year}/${String(month).padStart(2, '0')}`;
    const folderYYYY_MM = await new JsonLoader().load(urlYYYY_MM);
    folderYYYY_MM.files.forEach(_file => {
      const day = _file.file.substring(6, 8);
      const date = new Date(year, month - 1, day);
      if (date.getTime() < _dateRange[0].getTime()) {
        _dateRange[0] = date;
      }
      if (date.getTime() > _dateRange[1].getTime()) {
        _dateRange[1] = date;
      }
    });
    return _dateRange;
  };

  const collectMonths = async (year: number, _dateRange: [Date, Date]) => {
    const urlYYYY = `${boxUrl}/folder?folder=${year}`;
    const folderYYYY = await new JsonLoader().load(urlYYYY);
    for (let _subfolder of folderYYYY.folders) {
      if (!isNaN(_subfolder.folder)) {
        _dateRange = await collectDays(year, parseInt(_subfolder.folder), _dateRange);
      }
    }; // done iterating folders
    return _dateRange;
  };

  const collectYears = async (_dateRange: [Date, Date]) => {
    // find all days that have data
    for (var year = 2023; year <= new Date().getFullYear(); year++) {
      _dateRange = await collectMonths(year, _dateRange);
    }
    return _dateRange;
  };

  const handleDateChanged = (value: moment.Moment) => {

    const _datePick = new Date(value.year(), value.month(), value.date());
    setChartRefX(_datePick.getTime());

    const file = `${value.year()}/${String(value.month() + 1).padStart(2, '0')}/${value.year()}${String(value.month() + 1).padStart(2, '0')}${String(value.date()).padStart(2, '0')}.csv`;
    new CsvToJsonLoader().load(`${boxUrl}/file?file=${file}`).then(dataset1 => {
      updateDataset(dataset1);
      if (toLocalDate(Date.now()) === toLocalDate(_datePick.getTime())) {
        new CsvToJsonLoader().load(`${boxUrl}/data`).then(dataset2 => {
          const extraData = dataset2.filter(data => data.instant > dataset1[dataset1.length - 1].instant);
          updateDataset([
            ...dataset1,
            ...extraData
          ]);
        }).catch(e => {
          console.error(e);
        });
      }
    }).catch(e => {
      console.error(e);
    });

  };

  const updateDataset = (dataset: any[]) => {
    setChartData(dataset);
    setChartMinX(dataset[0].instant);
    setChartMaxX(dataset[dataset.length - 1].instant);
    setDispMinX(dataset[0].instant);
    setDispMaxX(dataset[dataset.length - 1].instant);
  };

  useEffect(() => {

    console.debug('✨ building chartapp component');

    const _dateRange: [Date, Date] = [new Date('2100-01-01'), new Date('2000-01-01')];
    collectYears(_dateRange).then(() => {

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

  }, [dispData]);

  useEffect(() => {

    if (domain) {

      const fracRiskHi = toDomainFraction(propertyLookup[chartProp].thresholds.riskHi, domain);
      const fracWarnHi = toDomainFraction(propertyLookup[chartProp].thresholds.warnHi, domain);
      const fracWarnLo = toDomainFraction(propertyLookup[chartProp].thresholds.warnLo, domain);
      const fracRiskLo = toDomainFraction(propertyLookup[chartProp].thresholds.riskLo, domain);
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

  }, [domain]);

  const toLocalDate = (instant: number) => {

    return new Date(instant).toLocaleDateString(window.navigator.language, { // you can use undefined as first argument
      year: "numeric",
      month: "2-digit",
      day: "2-digit",
    })

  };

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
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      <TooltipR title="moth-latest">
        <Fab href='client.html' variant="circular" size='small' sx={{ position: 'fixed', right: 60, top: 10 }} >
          <WysiwygIcon />
        </Fab>
      </TooltipR>
      <TooltipR title="moth-api">
        <Fab href='server.html' variant="circular" size='small' sx={{ position: 'fixed', right: 10, top: 10 }} >
          <ConstructionIcon />
        </Fab>
      </TooltipR>

      <Typography variant="h4" component="h4" sx={{ padding: '0px 10px' }}>
        <IconButton><ShowChartIcon sx={{ width: '1.5em', height: '1.5em' }} /></IconButton> moth-chart
      </Typography>
      <Card sx={{ width: '100%', padding: '0px', margin: '12px 0px' }}>

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
      <Card sx={{ width: '100%', height: '100%', padding: '0px', margin: '0px' }} onWheel={handleMouseWheel}>
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
                <Label value={toLocalDate(dispMinX)} dy={20} />
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
    </ThemeProvider>
  );

};

export default ChartApp;
