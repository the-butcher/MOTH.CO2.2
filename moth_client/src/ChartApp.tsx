import { createTheme, CssBaseline, MenuItem, Select, ThemeProvider } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import { DatePicker } from '@mui/x-date-pickers/DatePicker';
import { useEffect, useState } from 'react';
import { CartesianGrid, Label, Line, LineChart, ResponsiveContainer, Tooltip, XAxis, YAxis } from 'recharts';
import './App.css';
import { JsonLoader } from './util/JsonLoader';

import { AdapterMoment } from '@mui/x-date-pickers/AdapterMoment';
import { LocalizationProvider } from '@mui/x-date-pickers/LocalizationProvider';
import moment from 'moment';
import 'moment/locale/de';
import { CsvToJsonLoader } from './util/CsvToJsonLoader';

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

const propertyLookup = {
  'co2': 'CO₂ ppm',
  'temperature': 'Temperature °C',
  'humidity': 'Humidity %RH',
  'pressure': 'Pressure hPa',
  'percent': 'Battery %',
  'voltage': 'Battery V',
}


const ChartApp = () => {

  const boxUrl = `${window.location.origin}/api`; // when running directly from device

  const [chartData, setChartData] = useState<any[]>([]);
  const [dateRange, setDateRange] = useState<[Date, Date]>([new Date('2100-01-01'), new Date('2000-01-01')]);
  const [chartTicks, setChartTicks] = useState<number[]>([]);
  const [chartProp, setChartProp] = useState<string>('co2');

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

    const file = `${value.year()}/${String(value.month() + 1).padStart(2, '0')}/${value.year()}${String(value.month() + 1).padStart(2, '0')}${String(value.date()).padStart(2, '0')}.csv`;
    new CsvToJsonLoader().load(`${boxUrl}/file?file=${file}`).then(dataset => {
      const _chartTicks = dataset.filter((data: any) => new Date(data.instant).getMinutes() === 0).map((data: any) => data.instant);
      setChartTicks(_chartTicks);
      setChartData(dataset);
    }).catch(e => {
      console.error(e);
    });

  };

  useEffect(() => {

    console.debug('✨ building chartapp component');

    const _dateRange: [Date, Date] = [new Date('2100-01-01'), new Date('2000-01-01')];
    collectYears(_dateRange).then(() => {
      console.log('done', _dateRange);
      setDateRange(_dateRange);
      handleDateChanged(moment(_dateRange[1]));
    }).catch(e => {

    });

  }, []);

  const menuItems: JSX.Element[] = [];
  for (var property in propertyLookup) {
    menuItems.push(<MenuItem value={property}>{propertyLookup[property]}</MenuItem>);
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

  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      <Card sx={{ width: '100%', padding: '0px', margin: '12px 0px' }}>
        <CardContent>
          <LocalizationProvider dateAdapter={AdapterMoment}>
            <DatePicker
              label="Date"
              value={moment(dateRange[1])}
              minDate={moment(dateRange[0])}
              maxDate={moment(dateRange[1])}
              onChange={(newValue) => handleDateChanged(newValue)}
            /></LocalizationProvider>
          <Select sx={{ marginLeft: '12px' }}
            labelId="demo-simple-select-label"
            id="demo-simple-select"
            value={chartProp}
            label="Prop"
            onChange={event => handleChartValueChanged(event.target.value)}
          >
            {menuItems}
          </Select>

        </CardContent>
      </Card>
      <Card sx={{ width: '100%', height: '43vw', padding: '0px', margin: '0px' }}>
        <CardContent style={{ width: 'inherit', height: 'inherit', padding: '0px', margin: '0px' }}>

          <ResponsiveContainer >
            <LineChart
              data={chartData}
              margin={{
                top: 20,
                right: 30,
                left: 30,
                bottom: 35,
              }}
            >
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="instant" tickFormatter={formatDate} ticks={chartTicks} >
                <Label value="date" dy={20} />
              </XAxis>
              <YAxis tickFormatter={formatValue} >
                <Label value={propertyLookup[chartProp]} angle={270} dx={-40} />
              </YAxis>
              <Tooltip labelFormatter={value => formatDate(new Date(value as number))} />
              <Line type="linear" dataKey={chartProp} stroke="#000000" strokeWidth={2} dot={{ r: 0 }} activeDot={{ r: 4 }} />
            </LineChart>
          </ResponsiveContainer>

        </CardContent>
      </Card>
    </ThemeProvider>
  );

};

export default ChartApp;
