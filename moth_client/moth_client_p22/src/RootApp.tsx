import { createTheme, CssBaseline, Stack, Tab, Tabs, ThemeProvider } from '@mui/material';
import './App.css';
import { useState } from 'react';
import TabServer from './components/TabServer';
import TabChart from './components/TabChart';
import ConstructionIcon from '@mui/icons-material/Construction';
import ShowChartIcon from '@mui/icons-material/ShowChart';


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

const RootApp = () => {

  const boxUrl = `${window.location.origin}/api`; // when running directly from device
  // const boxUrl = `http://192.168.0.73/api`; // when running directly from device

  const [value, setValue] = useState('server');
  const handleValue = (event: React.SyntheticEvent, newValue: string) => {
    setValue(newValue);
  };

  return (
    <ThemeProvider theme={darkTheme}>

      <CssBaseline />

      <Stack direction={'row'}>
        <Tabs value={value} onChange={handleValue}>
          <Tab icon={<ConstructionIcon />} iconPosition="start" value="server" label="moth-api" sx={{ fontSize: '16px' }} />
          <Tab icon={<ShowChartIcon />} iconPosition="start" value="chart" label="moth-chart" sx={{ fontSize: '16px' }} />
          <Tab disabled label={boxUrl} sx={{ fontSize: '16px' }} />
        </Tabs>
        {/* {boxUrl} */}
      </Stack>

      <Stack sx={{ height: '100%' }}>
        {
          value === 'server' ? <TabServer boxUrl={boxUrl} /> : null
        }
        {
          value === 'chart' ? <TabChart boxUrl={boxUrl} /> : null
        }
      </Stack>
    </ThemeProvider>
  );

};

export default RootApp;
