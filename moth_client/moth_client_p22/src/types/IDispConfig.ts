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