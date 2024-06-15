import { IConfig } from "./IConfig";

export interface INetwork {
    key: string,
    pwd: string
}

export interface IWifiConfig extends IConfig {
    /**
     * wifi timeout minutes
     */
    min: number;
    /**
     * configured networks
     */
    ntw: INetwork[];
}

export const WIFI_CONFIG_DEFAULT: IWifiConfig = {
    status: 'DEFAULT',
    min: 5,
    ntw: []
};