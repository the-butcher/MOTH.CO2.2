import PlayCircleOutlineIcon from '@mui/icons-material/PlayCircleOutline';
import { Button, CardContent } from '@mui/material';
import Accordion from '@mui/material/Accordion';
import AccordionDetails from '@mui/material/AccordionDetails';
import AccordionSummary from '@mui/material/AccordionSummary';
import Card from '@mui/material/Card';
import { Stack } from '@mui/system';
import { DatePicker } from '@mui/x-date-pickers';
import { AdapterMoment } from '@mui/x-date-pickers/AdapterMoment';
import { LocalizationProvider } from '@mui/x-date-pickers/LocalizationProvider';
import moment from 'moment';
import 'moment/locale/de';
import { KeyboardEvent, useEffect, useState } from 'react';
import { ByteLoader } from '../util/ByteLoader';
import { IRecord } from '../util/IRecord';
import { JsonLoader } from '../util/JsonLoader';
import ApiResponse from './ApiResponse';
import { IApiProperties } from './IApiProperties';
import { IResponseProps } from './IResponseProps';


const ApiCalcsv = (props: IApiProperties) => {

  const apiName = 'calcsv';
  const apiDesc = 'get historic measurements as csv data';
  const apiType = 'csv';
  const secondsFrom1970To2000 = 946684800;
  const valueScaleCo2Lpf = 8;
  const recordLength = 20;


  const { boxUrl, panels, pstate, handlePanel, handleApiCall } = props;

  // const [file, setFile] = useState<string>('2024/04/20240424.dat');
  const [responseProps, setResponseProps] = useState<IResponseProps>();
  const [dateRangeData, setDateRangeData] = useState<[Date, Date]>(null);
  const [dateRangeUser, setDateRangeUser] = useState<[Date, Date]>(null);


  const toFloatDeg = (shortValue: number): number => {
    return shortValue / 640.0 - 50.0;
  }

  const toFloatHum = (shortValue: number): number => {
    return shortValue / 640.0;
  }

  const toFloatPercent = (shortValue: number): number => {
    return shortValue / 640.0;
  }

  const handleKeyUp = (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter') {
      issueApiCall();
    }
  }

  const loadUrl = async (url: string): Promise<IRecord[]> => {
    const byteLoader = new ByteLoader();
    const data = await byteLoader.load(url);
    let secondstime: number;
    const records: IRecord[] = [];
    for (let i = 0; i < data.byteLength; i += recordLength) {
      secondstime = data.getUint32(i, true);
      records.push({
        instant: new Date((secondsFrom1970To2000 + secondstime) * 1000).getTime(),
        co2Lpf: data.getUint16(i + 4, true) / valueScaleCo2Lpf,
        deg: toFloatDeg(data.getUint16(i + 6, true)),
        hum: toFloatHum(data.getUint16(i + 8, true)),
        co2Raw: data.getUint16(i + 10, true),
        hpa: data.getFloat32(i + 12, true),
        nrg: toFloatPercent(data.getUint16(i + 16, true))
      });
    }
    return records;
  }

  const loadAll = async (urls: string[]): Promise<IRecord[]> => {
    const records: IRecord[] = [];
    let _records: IRecord[];
    for (let url of urls) {
      _records = await loadUrl(url);
      records.push(..._records);
    }
    return records;
  }

  const formatValue = (value: number, precision: number, length: number, pad: string): string => {
    return value.toFixed(precision).replace('.', ',').padStart(length, pad);
  }

  const issueApiCall = () => {

    const millisecondsPerHour = 1000 * 60 * 60;
    const millisecondsPerDay = 1000 * 60 * 60 * 24;

    const minInstant = dateRangeUser[0].getTime() + millisecondsPerHour * 6;
    const maxInstant = dateRangeUser[1].getTime() + millisecondsPerHour * 18;

    let fetchDate: Date;
    let urls: string[] = [];
    for (let instant = minInstant; instant <= maxInstant; instant += millisecondsPerDay) {
      fetchDate = new Date(instant);
      urls.push(`${boxUrl}/datout?file=${fetchDate.getFullYear()}/${String(fetchDate.getMonth() + 1).padStart(2, '0')}/${fetchDate.getFullYear()}${String(fetchDate.getMonth() + 1).padStart(2, '0')}${String(fetchDate.getDate()).padStart(2, '0')}.dat`);
    }

    loadAll(urls).then(records => {
      let date: Date;
      let csvLines: string[] = [
        'time;co2_lpf;co2_raw;deg;hum;hpa;bat'
      ];
      for (let record of records) {
        date = new Date(record.instant);
        csvLines.push(`${date.getUTCFullYear()}-${formatValue(date.getMonth() + 1, 0, 2, '0')}-${formatValue(date.getUTCDate(), 0, 2, '0')} ${formatValue(date.getUTCHours(), 0, 2, '0')}:${formatValue(date.getMinutes(), 0, 2, '0')}:${formatValue(date.getSeconds(), 0, 2, '0')};${formatValue(record.co2Lpf, 0, 4, ' ')};${formatValue(record.deg, 1, 5, ' ')};${formatValue(record.hum, 1, 4, ' ')};${formatValue(record.co2Lpf, 0, 4, ' ')};${formatValue(record.co2Lpf, 0, 4, ' ')};${formatValue(record.nrg, 1, 5, ' ')}`);
      }
      let csvFile: string = csvLines.join('\r\n');
      const blob = new Blob([csvFile], { type: 'text/csv;charset=utf-8,' });
      const objUrl = URL.createObjectURL(blob);
      const link = document.createElement('a');
      link.setAttribute('href', objUrl);
      link.setAttribute('download', 'data.csv'); // TODO format more unique
      link.click();
    });

  };

  // handleApiCall({
  //   href: boxUrl,
  //   call: apiName,
  //   meth: 'GET',
  //   type: apiType,
  //   qstr: file && file !== "" ? { file } : null
  // });

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component (pstate, props[apiName])`, pstate, props[apiName]);

    // let href = `${boxUrl}/${apiName}`;
    // if (file && file !== "") {
    //   href += `?file=${file}`
    // }

    // if (props[apiName]) {
    //   setResponseProps({
    //     time: Date.now(),
    //     href,
    //     type: apiType,
    //     http: 'GET',
    //     data: props[apiName]
    //   });

    // }

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [pstate, props[apiName]]);

  const collectDays = async (year: number, month: number, _dateRange: [Date, Date]) => {
    const urlYYYY_MM = `${boxUrl}/dirout?folder=${year}/${String(month).padStart(2, '0')}`;
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
    const urlYYYY = `${boxUrl}/dirout?folder=${year}`;
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
    for (var year = 2024; year <= new Date().getFullYear(); year++) {
      _dateRange = await collectMonths(year, _dateRange);
    }
    return _dateRange;
  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component (panels)`, panels);

    if (panels.indexOf(apiName) >= 0 && dateRangeData == null) {

      const _dateRange: [Date, Date] = [new Date('2100-01-01'), new Date('2000-01-01')];
      collectYears(_dateRange).then(() => {

        console.log('done collecting dates', _dateRange);
        setDateRangeData(_dateRange);
        setDateRangeUser(_dateRange);

        // would be good to know initial time and latest time (or is it good enough with daily granularity?)

      }).catch(e => {
        console.error(e);
      });

      console.log('need to load dates');
    }

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [panels]);


  const handleDateMinChanged = (value: moment.Moment) => {

    const dateMin = new Date(value.year(), value.month(), value.date());
    setDateRangeUser([dateMin, dateRangeUser[1]]);

  };

  const handleDateMaxChanged = (value: moment.Moment) => {

    const dateMax = new Date(value.year(), value.month(), value.date());
    setDateRangeUser([dateRangeUser[0], dateMax]);

  };


  return (
    <Accordion expanded={panels.indexOf(apiName) >= 0} onChange={(event, expanded) => handlePanel(apiName, expanded)}>

      <AccordionSummary>
        <div>
          <div id={apiName}>/{apiName}</div>
          <div style={{ fontSize: '0.75em' }}>{apiDesc}</div>
        </div>
      </AccordionSummary>
      <AccordionDetails>
        <Card>
          <CardContent>


            <Stack>
              <div style={{ display: 'flex', flexDirection: 'row' }}>
                <LocalizationProvider dateAdapter={AdapterMoment}>
                  {
                    dateRangeData && dateRangeUser ? <DatePicker sx={{ margin: '6px', width: '250px' }}
                      label="from"
                      value={moment(dateRangeUser[0])}
                      minDate={moment(dateRangeData[0])} // lowest possible value
                      maxDate={moment(dateRangeUser[1])}
                      onChange={(newValue) => handleDateMinChanged(newValue)}
                    /> : null
                  }
                  {
                    dateRangeData && dateRangeUser ? <DatePicker sx={{ margin: '6px', width: '250px' }}
                      label="to"
                      value={moment(dateRangeUser[1])}
                      minDate={moment(dateRangeUser[0])}
                      maxDate={moment(dateRangeData[1])} // highest possible value
                      onChange={(newValue) => handleDateMaxChanged(newValue)}
                    /> : null
                  }
                </LocalizationProvider>
              </div>

              <Button disabled={pstate === 'disconnected'} variant="contained" endIcon={<PlayCircleOutlineIcon />} onClick={issueApiCall}>
                click to execute
              </Button>
              {
                (responseProps) ? <ApiResponse {...responseProps} /> : null
              }
            </Stack>

          </CardContent>
        </Card>

      </AccordionDetails>

    </Accordion >
  );
}

export default ApiCalcsv;