import { IThresholds } from "./IThresholds";

export interface IDispConfigCo2 {
    wHi: number;
    rHi: number;
    /**
     * reference value for the co2 stale calculation
     */
    ref: number;
    /**
     * reference values for button calibration
     * note: calibrating the device to 400 in fresh air yields better overlap with i.e. aranet values in higher ranges
     */
    cal: number;
    /**
     * low-pass alpha value for co2, lower values yield stronger filtering, higher values yield weaker filter, 1 = no filter = raw values
     */
    lpa: number;
}

export interface IDispConfigDeg extends IThresholds {
    /**
     * temperature offset for the co2 temperature sensor
     */
    off: number;
    /**
     * fahrenheit display, true = fahrenheit, false = celsius
     */
    c2f: boolean;
}

export interface IDispConfigBme {
    /**
     * base altitude of the sensor
     */
    alt: number;
    /**
     * low-pass alpha value for pressure, lower values yield stronger filtering, higher values yield weaker filter, 1 = no filter = raw values
     */
    lpa: number;
}

export interface IDispConfig {
    /**
     * display update minutes
     */
    min: number;
    /**
     * show significant change
     */
    ssc: boolean;
    /**
     * timezone
     * example: "CET-1CEST,M3.5.0,M10.5.0/3"
     */
    tzn: string;
    co2: IDispConfigCo2;
    deg: IDispConfigDeg;
    hum: IThresholds;
    bme: IDispConfigBme;
}

export const DISP_CONFIG_DEFAULT: IDispConfig = {
    min: 3,
    ssc: true,
    tzn: 'CET-1CEST,M3.5.0,M10.5.0/3',
    co2: {
        wHi: 1000,
        rHi: 1500,
        ref: 425,
        cal: 400,
        lpa: 0.5
    },
    deg: {
        rLo: 14,
        wLo: 19,
        wHi: 25,
        rHi: 30,
        off: 0.7,
        c2f: false
    },
    hum: {
        rLo: 25,
        wLo: 30,
        wHi: 60,
        rHi: 65
    },
    bme: {
        alt: 153,
        lpa: 0.25
    }
};