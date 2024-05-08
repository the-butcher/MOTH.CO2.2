export interface INetwork {
    key: string,
    pwd: string
}

export interface IWifiConfig {
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
    min: 5,
    ntw: []
};