import { createTheme, CssBaseline, ThemeProvider } from '@mui/material';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import { useEffect, useState } from 'react';
import './App.css';
import { ChartData } from './util/ChartData';
import { JsonLoader } from './util/JsonLoader';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';

const data = [
  {
    name: 'Page A',
    uv: 4000,
    pv: 2400,
    amt: 2400,
  },
  {
    name: 'Page B',
    uv: 3000,
    pv: 1398,
    amt: 2210,
  },
  {
    name: 'Page C',
    uv: 2000,
    pv: 9800,
    amt: 2290,
  },
  {
    name: 'Page D',
    uv: 2780,
    pv: 3908,
    amt: 2000,
  },
  {
    name: 'Page E',
    uv: 1890,
    pv: 4800,
    amt: 2181,
  },
  {
    name: 'Page F',
    uv: 2390,
    pv: 3800,
    amt: 2500,
  },
  {
    name: 'Page G',
    uv: 3490,
    pv: 4300,
    amt: 2100,
  },
];

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

const ChartApp = () => {

  const [chartData, setChartData] = useState<ChartData>(new ChartData());

  useEffect(() => {

    console.debug('✨ building clientapp component');

    new JsonLoader().load('http://192.168.0.179/api/folder').then(_folder => {
      console.log('_folder', _folder);
    }).catch(e => {

    });

  }, []);

  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      <Card sx={{ width: '100%', height: '43vw', padding: '0px', margin: '0px' }}>
        <CardContent style={{ width: 'inherit', height: 'inherit', padding: '0px', margin: '0px' }}>

          <ResponsiveContainer width="100%" height="100%">

            <LineChart
              width={500}
              height={300}
              data={data}
              margin={{
                top: 5,
                right: 30,
                left: 20,
                bottom: 5,
              }}
            >
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="name" />
              <YAxis />
              <Tooltip />
              <Legend />
              <Line type="monotone" dataKey="pv" stroke="#8884d8" activeDot={{ r: 8 }} />
              <Line type="monotone" dataKey="uv" stroke="#82ca9d" />
            </LineChart>
          </ResponsiveContainer>

        </CardContent>
      </Card>
    </ThemeProvider>
  );

};

export default ChartApp;
