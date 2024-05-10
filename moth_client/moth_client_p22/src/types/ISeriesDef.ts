import { ContinuousColorConfig, PiecewiseColorConfig } from '@mui/x-charts/models/colorMapping';
import { TRecordKey } from './IRecord';

export interface ISeriesDef {
    id: TRecordKey;
    label: string;
    valueFormatter: (value: number) => string;
    colorMap?: ContinuousColorConfig | PiecewiseColorConfig;
    min?: number;
}

export const COLOR_G = '#0ec600';
export const COLOR_Y = '#c9b800';
export const COLOR_R = '#e20e00';

export const SERIES_DEFS: { [K in TRecordKey]: ISeriesDef } = {
    instant: {
        id: 'instant',
        label: 'time (HH:MM)',
        valueFormatter: value => Number.isFinite(value) ? value.toString() : 'NA'
    },
    co2Lpf: {
        id: 'co2Lpf',
        label: 'CO₂ ppm',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(0) : 'NA',
        colorMap: {
            type: 'piecewise',
            thresholds: [800, 1000],
            colors: [COLOR_G, COLOR_Y, COLOR_R]
        },
        min: 0
    },
    deg: {
        id: 'deg',
        label: 'Temperature °C',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
        colorMap: {
            type: 'piecewise',
            thresholds: [14, 19, 25, 30],
            colors: [COLOR_R, COLOR_Y, COLOR_G, COLOR_Y, COLOR_R]
        }
    },
    hum: {
        id: 'hum',
        label: 'Humidity %RH',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
        colorMap: {
            type: 'piecewise',
            thresholds: [25, 30, 60, 65],
            colors: [COLOR_R, COLOR_Y, COLOR_G, COLOR_Y, COLOR_R]
        }
    },
    co2Raw: {
        id: 'co2Lpf',
        label: 'CO₂ ppm (filtered)',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(0) : 'NA',
        colorMap: {
            type: 'piecewise',
            thresholds: [800, 1000],
            colors: [COLOR_G, COLOR_Y, COLOR_R]
        },
        min: 0
    },
    hpa: {
        id: 'hpa',
        label: 'Pressure hPa',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
    },
    bat: {
        id: 'bat',
        label: 'Battery %',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
    }
}