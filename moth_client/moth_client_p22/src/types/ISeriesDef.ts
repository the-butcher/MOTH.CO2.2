import { TRecordKey } from './IRecord';

export interface IContinuousColorConfig<Value = number | Date> {
    type: 'continuous';
    /**
     * The minimal value of the color scale.
     * @default 0
     */
    min?: Value;
    /**
     * The maximal value of the color scale.
     * @default 100
     */
    max?: Value;
    /**
     * The colors to render. It can be an array with the extremum colors, or an interpolation function.
     */
    color: [string, string] | ((t: number) => string);
}

export interface IPiecewiseColorConfig<Value = number | Date> {
    type: 'piecewise';
    /**
     * The thresholds where color should change from one category to another.
     */
    thresholds: Value[];
    /**
     * The colors used for each band defined by `thresholds`.
     * Should contain N+1 colors, where N is the number of thresholds.
     */
    colors: string[];
}

/**
 * definition for a charts series
 */
export interface ISeriesDef {
    /**
     * id of the data property
     */
    id: TRecordKey;
    /**
     * a name that can i.e. be labelled on the y-axis of the chart
     */
    label: string;
    /**
     * a formatter that provides human-readable representation of the data
     */
    valueFormatter: (value: number) => string;
    /**
     * optional color map for the series
     */
    colorMap?: IContinuousColorConfig | IPiecewiseColorConfig;
    /**
     * a function that can provide a min value for the y-axis
     * @param min
     * @returns a speacific min value or undefined (for chart default behaviour)
     */
    min: (min: number) => number;
    /**
     * a function that can provide a max value for the y-axis
     * @param max
     * @returns a speacific max value or undefined (for chart default behaviour)
     */

    max: (max: number) => number;
}

export const COLOR_G = '#0ec600';
export const COLOR_Y = '#c9b800';
export const COLOR_R = '#e20e00';

/**
 * a set of predefined series defs for all data-key sprecified in TRecordKey
 */
export const SERIES_DEFS: { [K in TRecordKey]: ISeriesDef } = {
    instant: {
        id: 'instant',
        label: 'time (HH:MM)',
        valueFormatter: value => Number.isFinite(value) ? value.toString() : 'NA',
        min: () => undefined,
        max: () => undefined,
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
        min: () => 0,
        max: () => undefined,
    },
    deg: {
        id: 'deg',
        label: 'Temperature °C',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
        colorMap: {
            type: 'piecewise',
            thresholds: [14, 19, 25, 30],
            colors: [COLOR_R, COLOR_Y, COLOR_G, COLOR_Y, COLOR_R]
        },
        min: () => undefined,
        max: () => undefined,
    },
    hum: {
        id: 'hum',
        label: 'Humidity %RH',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
        colorMap: {
            type: 'piecewise',
            thresholds: [25, 30, 60, 65],
            colors: [COLOR_R, COLOR_Y, COLOR_G, COLOR_Y, COLOR_R]
        },
        min: () => undefined,
        max: () => undefined,
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
        min: () => 0,
        max: () => undefined,
    },
    hpa: {
        id: 'hpa',
        label: 'Pressure hPa',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
        min: () => undefined,
        max: () => undefined,
    },
    bat: {
        id: 'bat',
        label: 'Battery %',
        valueFormatter: value => Number.isFinite(value) ? value.toFixed(1) : 'NA',
        min: min => Math.floor(min),
        max: max => Math.ceil(max),
    }
}