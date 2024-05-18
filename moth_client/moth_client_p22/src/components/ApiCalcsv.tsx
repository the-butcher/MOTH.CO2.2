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
import { useEffect, useState } from 'react';
import { IApiProperties } from '../types/IApiProperties';
import { IResponseProps } from '../types/IResponseProps';
import { ByteLoader } from '../util/ByteLoader';
import { TimeUtil } from '../util/TimeUtil';
import ApiResponse from './ApiResponse';

/**
 * @deprecated
 */
const ApiCalcsv = (props: IApiProperties) => {

  const apiName = 'calcsv';
  const apiDesc = 'get historic measurements as csv data';


  const { boxUrl, panels, handlePanel } = props;

  // const [file, setFile] = useState<string>('2024/04/20240424.dat');
  const [responseProps] = useState<IResponseProps>();
  const [dateRangeData, setDateRangeData] = useState<[Date, Date]>(null);
  const [dateRangeUser, setDateRangeUser] = useState<[Date, Date]>(null);

  const issueApiCall = () => {

    const minInstant = dateRangeUser[0].getTime() + TimeUtil.MILLISECONDS_PER_HOUR * 6;
    const maxInstant = dateRangeUser[1].getTime() + TimeUtil.MILLISECONDS_PER_HOUR * 18;

    let urlDate: Date;
    const curDate = new Date();
    let urls: string[] = [];
    for (let urlInstant = minInstant; urlInstant <= maxInstant; urlInstant += TimeUtil.MILLISECONDS_PER__DAY) {
      urlDate = new Date(urlInstant);
      urls.push(`${boxUrl}/datout?file=${urlDate.getUTCFullYear()}/${String(urlDate.getUTCMonth() + 1).padStart(2, '0')}/${TimeUtil.toExportDate(urlInstant)}.dat`);
      if (TimeUtil.toExportDate(urlDate.getTime()) === TimeUtil.toExportDate(curDate.getTime())) {
        urls.push(`${boxUrl}/valout`);
      }
    }

    new ByteLoader().loadAll(urls).then(records => {
      let date: Date;
      let csvLines: string[] = [
        'time;co2_lpf;co2_raw;deg;hum;hpa;bat'
      ];
      for (let record of records) {
        date = new Date(record.instant);
        csvLines.push(`${TimeUtil.toCsvDate(date)};${TimeUtil.formatValue(record.co2Lpf, 0, 4, ' ')};${TimeUtil.formatValue(record.co2Raw, 0, 4, ' ')};${TimeUtil.formatValue(record.deg, 1, 5, ' ')};${TimeUtil.formatValue(record.hum, 1, 4, ' ')};${TimeUtil.formatValue(record.hpa, 2, 7, ' ')};${TimeUtil.formatValue(record.bat, 1, 5, ' ')}`);
      }
      let csvFile: string = csvLines.join('\r\n');
      const blob = new Blob([csvFile], { type: 'text/csv;charset=utf-8,' });
      const downloadName = `mothdat_${TimeUtil.toExportDate(minInstant)}_${TimeUtil.toExportDate(maxInstant)}.dat`
      const objUrl = URL.createObjectURL(blob);
      const link = document.createElement('a');
      link.setAttribute('href', objUrl);
      link.setAttribute('download', downloadName); // TODO format more unique
      link.click();
    });

  };

  useEffect(() => {

    console.debug(`⚙ updating ${apiName} component (panels)`, panels);

    if (panels.indexOf(apiName) >= 0 && dateRangeData == null) {

      TimeUtil.collectYears(boxUrl).then(_dateRange => {
        setDateRangeData(_dateRange);
        setDateRangeUser(_dateRange);
      }).catch(e => {
        console.error(e);
      });

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
          <div id={apiName}>{apiName}()</div>
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
              <Button variant="contained" endIcon={<PlayCircleOutlineIcon />} onClick={issueApiCall}>
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