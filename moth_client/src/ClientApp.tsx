import ConstructionIcon from '@mui/icons-material/Construction';
import ShowChartIcon from '@mui/icons-material/ShowChart';
import { createTheme, CssBaseline, Fab, IconButton, ThemeProvider, Tooltip, Typography } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import { useEffect, useState } from 'react';
import './App.css';
import { JsonLoader } from './util/JsonLoader';
import { Thresholds } from './util/Thresholds';
import WysiwygIcon from '@mui/icons-material/Wysiwyg';

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

const ClientApp = () => {


  const ip = window.location.origin.substring(window.location.protocol.length + 2);
  // const ip = '192.168.0.66'
  const boxUrl = `http://${ip}/api`; // when running directly from device

  const [latest, setLatest] = useState<any>({
    time: '00:00',
    co2: 0,
    temperature: 0,
    humidity: 0,
    pressure: 0,
    percent: 0
  });

  const [thresholdsCo2, setThresholdsCo2] = useState<Thresholds>(new Thresholds(0, 0, 800, 1000));
  const [thresholdsDeg, setThresholdsDeg] = useState<Thresholds>(new Thresholds(14, 19, 25, 30));
  const [thresholdsHum, setThresholdsHum] = useState<Thresholds>(new Thresholds(25, 30, 60, 65));

  useEffect(() => {

    console.debug('✨ building clientapp component');

    new JsonLoader().load(`${boxUrl}/file?file=config/disp.json`).then(_disp => {
      console.log('disp', _disp);
      setThresholdsCo2(new Thresholds(0, 0, _disp.co2.wHi, _disp.co2.rHi));
      setThresholdsDeg(new Thresholds(_disp.deg.rLo, _disp.deg.wLo, _disp.deg.wHi, _disp.deg.rHi))
      setThresholdsHum(new Thresholds(_disp.hum.rLo, _disp.hum.wLo, _disp.hum.wHi, _disp.hum.rHi))
    }).catch(e => {

    });

    new JsonLoader().load(`${boxUrl}/latest`).then(_latest => {
      setLatest({
        ..._latest,
        time: new Date(_latest.time).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
      });
    }).catch(e => {

    });

  }, []);

  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      <Tooltip title="moth-chart">
        <Fab href='chart.html' variant="circular" size='small' sx={{ position: 'fixed', right: 60, top: 10 }} >
          <ShowChartIcon />
        </Fab>
      </Tooltip>
      <Tooltip title="moth-api">
        <Fab href='server.html' variant="circular" size='small' sx={{ position: 'fixed', right: 10, top: 10 }} >
          <ConstructionIcon />
        </Fab>
      </Tooltip>

      <Typography variant="h4" component="h4" sx={{ padding: '0px 10px' }}>
        <IconButton><WysiwygIcon sx={{ width: '1.5em', height: '1.5em' }} /></IconButton> moth-latest
      </Typography>
      <Card sx={{ width: '100%', height: '43vw', padding: '0px', margin: '12px 0px' }}>
        <CardContent style={{ width: 'inherit', height: 'inherit', padding: '0px', margin: '0px' }}>
          <div style={{ position: 'relative', left: '0%', top: '0%', border: '0.5vw solid lightgray', width: '100%', height: '17%' }}>
            <div style={{ position: 'relative', left: '2%', top: '-5%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'left', paddingRight: '2vw' }}>
              {ip}
            </div>
          </div>
          <div style={{ position: 'relative', left: '0%', top: '0%', width: '100%', height: '66%' }}>
            <div style={{ position: 'relative', left: '0%', top: '0%', border: '0.5vw solid lightgray', width: '66%', height: '100%', color: thresholdsCo2.getTextColor(latest.co2), backgroundColor: thresholdsCo2.getFillColor(latest.co2) }}>
              <div style={{ position: 'relative', left: '0%', top: '-0%', width: '100%', height: '100%', fontSize: '23vw', textAlign: 'right', paddingRight: '2vw' }}>
                {latest.co2}
              </div>
              <div style={{ position: 'relative', left: '2%', top: '-100%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'left', paddingRight: '2vw' }}>
                CO₂
              </div>
              <div style={{ position: 'relative', left: '0%', top: '-200%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'right', paddingRight: '2vw' }}>
                ppm
              </div>

            </div>
            <div style={{ position: 'relative', left: '66%', top: '-100%', border: '0.5vw solid lightgray', width: '34%', height: '50%', color: thresholdsDeg.getTextColor(latest.temperature), backgroundColor: thresholdsDeg.getFillColor(latest.temperature) }}>
              <div style={{ position: 'relative', left: '-19%', top: '-28%', width: '100%', height: '100%', fontSize: '14vw', textAlign: 'right', paddingRight: '2vw' }}>
                {Math.floor(Math.round(latest.temperature * 10) / 10)}
              </div>
              <div style={{ position: 'relative', left: '0%', top: '-53%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'right', paddingRight: '2vw' }}>
                .{Math.round(latest.temperature * 10) % 10}
              </div>
              <div style={{ position: 'relative', left: '0%', top: '-200%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'right', paddingRight: '2vw' }}>
                °C
              </div>
            </div>
            <div style={{ position: 'relative', left: '66%', top: '-100%', border: '0.5vw solid lightgray', width: '34%', height: '50%', color: thresholdsHum.getTextColor(latest.humidity), backgroundColor: thresholdsHum.getFillColor(latest.humidity) }}>
              <div style={{ position: 'relative', left: '-19%', top: '-28%', width: '100%', height: '100%', fontSize: '14vw', textAlign: 'right', paddingRight: '2vw' }}>
                {Math.floor(Math.round(latest.humidity * 10) / 10)}
              </div>
              <div style={{ position: 'relative', left: '0%', top: '-53%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'right', paddingRight: '2vw' }}>
                .{Math.round(latest.humidity * 10) % 10}
              </div>
              <div style={{ position: 'relative', left: '0%', top: '-200%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'right', paddingRight: '2vw' }}>
                %
              </div>
            </div>
          </div>
          <div style={{ position: 'relative', left: '0%', top: '0%', border: '0.5vw solid lightgray', width: '100%', height: '17%' }}>
            <div style={{ position: 'relative', left: '2%', top: '-5%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'left', paddingRight: '2vw' }}>
              {latest.time}
            </div>
            <div style={{ position: 'relative', left: '0%', top: '-105%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'right', paddingRight: '2vw' }}>
              {latest.percent.toFixed(0)}%
            </div>
          </div>
        </CardContent>
      </Card>
      <div style={{ height: '12px' }}></div>
    </ThemeProvider>
  );

};

export default ClientApp;
