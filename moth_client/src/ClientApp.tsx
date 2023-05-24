import { createTheme, CssBaseline, ThemeProvider } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import Typography from '@mui/material/Typography';
import { useEffect, useState } from 'react';
import './App.css';
import { JsonLoader } from './util/JsonLoader';

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
        elevation: 3
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

  const [latest, setLatest] = useState<any>({
    co2: 0,
    temperature: 0,
    humidity: 0,
    pressure: 0
  });

  useEffect(() => {

    console.debug('✨ building clientapp component');

    new JsonLoader().load('../api/latest').then(_latest => {
      setLatest(_latest);
    }).catch(e => {

    });

  }, []);

  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      <Card sx={{ padding: '0px' }}>
        <CardContent>
          <Typography gutterBottom variant="subtitle1" component="div">
            CO₂: {latest.co2}
          </Typography>
          <Typography gutterBottom variant="subtitle1" component="div">
            °C : {latest.temperature}
          </Typography>
          <Typography gutterBottom variant="subtitle1" component="div">
            %RH: {latest.humidity}
          </Typography>
          <Typography gutterBottom variant="subtitle1" component="div">
            hPa: {latest.pressure / 100}
          </Typography>
        </CardContent>
      </Card>
    </ThemeProvider>
  );

};

export default ClientApp;
