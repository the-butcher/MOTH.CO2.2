import { IConfig } from "./IConfig";
import { IThresholds } from "./IThresholds";

export interface IDispConfigPms {
    wHi: number;
    rHi: number;
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
     * low-pass alpha value for pressure, lower values yield stronger filtering, higher values yield weaker filter, 1 = no filter = raw values
     */
    lpa: number;
}

export interface IDispConfig extends IConfig {
    /**
     * timezone
     * example: "CET-1CEST,M3.5.0,M10.5.0/3"
     */
    tzn: string;
    pms: IDispConfigPms;
    deg: IDispConfigDeg;
    hum: IThresholds;
    bme: IDispConfigBme;
}

export const DISP_CONFIG_DEFAULT: IDispConfig = {
    status: 'DEFAULT',
    tzn: 'CET-1CEST,M3.5.0,M10.5.0/3',
    pms: {
        wHi: 5,
        rHi: 15,
        lpa: 0.5
    },
    deg: {
        rLo: 14,
        wLo: 19,
        wHi: 25,
        rHi: 30,
        off: 9.0,
        c2f: false
    },
    hum: {
        rLo: 25,
        wLo: 30,
        wHi: 60,
        rHi: 65
    },
    bme: {
        lpa: 0.25
    }
};