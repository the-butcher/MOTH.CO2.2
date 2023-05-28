import { createTheme, CssBaseline, ThemeProvider, Typography } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import { useEffect, useState } from 'react';
import './App.css';
import { JsonLoader } from './util/JsonLoader';
import { Thresholds } from './util/Thresholds';

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

  const boxUrl = `${window.location.origin}/api`; // when running directly from device

  const [latest, setLatest] = useState<any>({
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
      setLatest(_latest);
    }).catch(e => {

    });

  }, []);

  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      <CssBaseline />
      <Typography variant="h4" component="h4" sx={{ paddingLeft: '10px', paddingBottom: '20px' }}>
        moth-latest
      </Typography>
      <Card sx={{ width: '100%', height: '43vw', padding: '0px', margin: '0px' }}>
        <CardContent style={{ width: 'inherit', height: 'inherit', padding: '0px', margin: '0px' }}>
          <div style={{ position: 'relative', left: '0%', top: '0%', border: '0.5vw solid lightgray', width: '100%', height: '17%' }}>

          </div>
          <div style={{ position: 'relative', left: '0%', top: '0%', width: '100%', height: '66%' }}>
            <div style={{ position: 'relative', left: '0%', top: '0%', border: '0.5vw solid lightgray', width: '66%', height: '100%', color: thresholdsCo2.getTextColor(latest.co2), backgroundColor: thresholdsCo2.getFillColor(latest.co2) }}>
              <div style={{ position: 'relative', left: '0%', top: '-0%', width: '100%', height: '100%', fontSize: '23vw', textAlign: 'right', paddingRight: '2vw' }}>
                {latest.co2}
              </div>
              <div style={{ position: 'relative', left: '-62%', top: '-100%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'right', paddingRight: '2vw' }}>
                CO₂ ppm
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
            <div style={{ position: 'relative', left: '0%', top: '-5%', width: '100%', height: '100%', fontSize: '5vw', textAlign: 'right', paddingRight: '2vw' }}>
              {latest.percent.toFixed(1)}%
            </div>
          </div>
          {/* <Typography variant="body1" component="div" sx={{ fontSize: '5em', margin: '0px', padding: '0px' }}>
            {latest.co2}<Typography gutterBottom variant="body1" component="span">&nbsp;CO₂ ppm</Typography>
          </Typography> */}
        </CardContent>
      </Card>
      {/* <Card sx={{ padding: '0px' }}>
        <CardContent>
          <Typography gutterBottom variant="h4" component="div">
            {latest.temperature}<Typography gutterBottom variant="subtitle1" component="span">
              &nbsp;°C
            </Typography>
          </Typography>
          <Typography gutterBottom variant="subtitle1" component="div">
            %RH: {latest.humidity}
          </Typography>
          <Typography gutterBottom variant="subtitle1" component="div">
            hPa: {latest.pressure / 100}
          </Typography>
        </CardContent>
      </Card> */}
    </ThemeProvider>
  );

};

export default ClientApp;
