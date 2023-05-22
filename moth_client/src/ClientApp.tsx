import { createTheme, CssBaseline, ThemeProvider } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import Typography from '@mui/material/Typography';
import { useEffect, useRef, useState } from 'react';
import './App.css';
import { IApiCall } from './components/IApiCall';
import { EStatus, IApiProperties } from './components/IApiProperties';

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
    MuiAccordion: {
      styleOverrides: {
        root: {
          backgroundColor: '#eeeeee',
        }
      }
    },
    MuiAccordionSummary: {
      styleOverrides: {
        content: {
          '&.Mui-expanded': {
            margin: '6px 0px',
          },
          margin: '6px 0px',
        }
      }
    },
    MuiAccordionDetails: {
      styleOverrides: {
        root: {
          paddingLeft: '40px',
        }
      }
    },
    MuiStack: {
      defaultProps: {
        spacing: 2
      }
    },
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

  useEffect(() => {

    console.debug('✨ building clientapp component', window.location);

  }, []);

  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      <Typography variant="h4" component="h4" sx={{ paddingLeft: '10px' }}>
        moth-api
      </Typography>
      <Typography variant="body1" component="h4" sx={{ paddingLeft: '10px' }}>
        subtitle
      </Typography>
      <Card sx={{ padding: '0px' }}>
        <CardContent>
          <Typography gutterBottom variant="subtitle1" component="div">
            client
          </Typography>
        </CardContent>
      </Card>
    </ThemeProvider>
  );

};

export default ClientApp;
