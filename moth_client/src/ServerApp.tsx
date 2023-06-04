import ShowChartIcon from '@mui/icons-material/ShowChart';
import WysiwygIcon from '@mui/icons-material/Wysiwyg';
import ConstructionIcon from '@mui/icons-material/Construction';
import { createTheme, CssBaseline, Fab, IconButton, ThemeProvider, Tooltip } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import Typography from '@mui/material/Typography';
import { useEffect, useRef, useState } from 'react';
import './App.css';
import ApiCalibrate from './components/ApiCalibrate';
import ApiData from './components/ApiData';
import ApiDelete from './components/ApiDelete';
import ApiEncrypt from './components/ApiEncrypt';
import ApiFile from './components/ApiFile';
import ApiFolder from './components/ApiFolder';
import ApiSimple from './components/ApiSimple';
import ApiUpload from './components/ApiUpload';
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

const ServerApp = () => {

  const boxUrl = `${window.location.origin}/api`; // when running directly from device
  // const boxUrl = `http://192.168.0.178/api`; // when running directly from device

  const urlParams = new URLSearchParams(window.location.search);
  // const boxUrlParamValue = `http://${urlParams.get("boxUrl")}/api`; // when not running directly from device

  const panelParamValue = urlParams.get("panel");
  let messageTimeout: number = -1;

  const rebuildAndSetApiProps = () => {
    setApiProps({
      ...apiProps,
      boxUrl,
      panels: panels.current,
      pstate: status.current,
      handlePanel: handlePanel,
      handleApiCall
    });
  }

  /**
   * change in open panels panel
   * @param panel
   * @returns
   */
  const handlePanel = (panel: string) => (event: React.SyntheticEvent, isExpanded: boolean) => {
    const indexOfPanel = panels.current.indexOf(panel);
    if (indexOfPanel >= 0) {
      panels.current = panels.current.filter(p => p !== panel)
    } else {
      panels.current.push(panel);
    }
    rebuildAndSetApiProps();
  };

  /**
   * handle a specific call to the api
   * @param call
   */
  const handleApiCall = (call: IApiCall) => {

    var iframe = document.getElementById("callframe") as HTMLIFrameElement;
    iframe.contentWindow.postMessage(call, "*");

    clearTimeout(messageTimeout);
    messageTimeout = window.setTimeout(() => {
      iframe.src = iframeSrc;
      status.current = 'disconnected';
      rebuildAndSetApiProps();
    }, 10000);

  }

  const panels = useRef<string[]>([panelParamValue]);
  const status = useRef<EStatus>('disconnected');

  const [apiProps, setApiProps] = useState<IApiProperties>({
    boxUrl,
    panels: panels.current,
    pstate: status.current,
    handlePanel: handlePanel,
    handleApiCall
  });

  useEffect(() => {

    console.debug('✨ building app component', window.location);

    window.setTimeout(() => {
      document.getElementById(panelParamValue)?.scrollIntoView({ behavior: "smooth" });
    }, 1000);

    window.addEventListener('message', ({ data }) => {

      if (data.call) {
        window.clearTimeout(messageTimeout);
        const _apiProps: IApiProperties = {
          ...apiProps,
          boxUrl,
          panels: panels.current,
          pstate: status.current,
          handlePanel: handlePanel,
          handleApiCall
        };
        _apiProps[data.call] = data.data; // set the specific message on the api props
        setApiProps(_apiProps);
      }
      if (data.event && data.event === 'loaded') {
        status.current = 'connected';
        rebuildAndSetApiProps();
      }

    });

  }, []);

  // const iframeSrc = `http://${boxUrlParamValue}/iframe`; //when not running directly from device
  const iframeSrc = 'iframe.html'; //when running directly from device

  return (
    <ThemeProvider theme={darkTheme}>

      <CssBaseline />
      <Tooltip title="moth-latest">
        <Fab href='client.html' variant="circular" size='small' sx={{ position: 'fixed', right: 60, top: 10 }} >
          <WysiwygIcon />
        </Fab>
      </Tooltip>
      <Tooltip title="moth-chart">
        <Fab href='chart.html' variant="circular" size='small' sx={{ position: 'fixed', right: 10, top: 10 }} >
          <ShowChartIcon />
        </Fab>
      </Tooltip>

      <Typography variant="h4" component="h4" sx={{ paddingLeft: '10px' }}>
        <IconButton><ConstructionIcon sx={{ width: '1.5em', height: '1.5em' }} /></IconButton>moth-api <iframe title="callframe" id="callframe" src={iframeSrc} style={{ height: '30px', border: 'none' }} />
      </Typography>
      <Typography variant="body1" sx={{ paddingLeft: '20px' }}>
        {boxUrl ? boxUrl : "no box specified"} ({status.current})
      </Typography>
      <Card sx={{ padding: '0px' }}>

        <CardContent>
          <Typography gutterBottom variant="subtitle1" component="div">
            data
          </Typography>
          <ApiSimple {...{
            ...apiProps,
            apiName: 'latest',
            apiDesc: 'get the latest measurement'
          }} />
          <ApiData {...apiProps} />
        </CardContent>
      </Card>
      <Card sx={{ padding: '0px' }}>
        <CardContent>
          <Typography gutterBottom variant="subtitle1" component="div">
            files, status
          </Typography>
          <ApiSimple {...{
            ...apiProps,
            apiName: 'status',
            apiDesc: 'get details about device status'
          }} />
          <ApiFolder {...apiProps} />
          <ApiFile {...apiProps} />
          <ApiUpload  {...apiProps} />
          <ApiDelete {...apiProps} />
        </CardContent>
      </Card>
      <Card sx={{ padding: '0px' }}>
        <CardContent>
          <Typography gutterBottom variant="subtitle1" component="div">
            connectivity
          </Typography>
          <ApiSimple {...{
            ...apiProps,
            apiName: 'networks',
            apiDesc: 'get a list of networks visible to the device'
          }} />
          <ApiSimple {...{
            ...apiProps,
            apiName: 'disconnect',
            apiDesc: 'disconnect the device'
          }} />
        </CardContent>
      </Card>
      <Card sx={{ padding: '0px' }}>
        <CardContent>
          <Typography gutterBottom variant="subtitle1" component="div">
            admin
          </Typography>
          <ApiEncrypt {...apiProps} />
          <ApiCalibrate {...apiProps} />
          <ApiSimple {...{
            ...apiProps,
            apiName: 'hibernate',
            apiDesc: 'hibernate the device',
            confirm: {
              title: 'do you really want to hibernate?',
              content: 'this call will put the box into hibernation. no more measurements will be taken until the box is re-activated by pressing the reset button.'
            }
          }} />
          <ApiSimple {...{
            ...apiProps,
            apiName: 'co2_reset',
            apiDesc: 'reset the CO₂ sensor to factory',
            confirm: {
              title: 'do you really want to reset?',
              content: 'this call will remove all calibration history from the box\'s CO₂ sensor. only use if the sensor seems to be stuck.'
            }
          }} />
          <ApiSimple {...{
            ...apiProps,
            apiName: 'reset',
            apiDesc: 'resets the device ',
            confirm: {
              title: 'do you really want to reset?',
              content: 'this call will reset the decive, data not written to permanent storage may be lost.'
            }
          }} />
        </CardContent>
        <div style={{ height: '12px' }}></div>
      </Card>


      {/* <Paper sx={{ position: 'fixed', bottom: 0, left: 0, right: 0 }} elevation={3}>
        <BottomNavigation
          showLabels

        // value={value}
        // onChange={(event, newValue) => {
        //   setValue(newValue);
        // }}
        >
          <BottomNavigationAction label="client" icon={<WysiwygIcon />} />
          <BottomNavigationAction label="chart" icon={<ShowChartIcon />} />
        </BottomNavigation>
      </Paper> */}

    </ThemeProvider>
  );

};

export default ServerApp;
