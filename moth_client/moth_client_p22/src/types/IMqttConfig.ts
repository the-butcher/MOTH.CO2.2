import { IConfig } from "./IConfig";

export interface IMqttConfig extends IConfig {
    use: boolean;
    srv?: string;
    prt?: number;
    crt?: string; // TODO :: handle file upload (must be an extra file field, "crt" just manages the path on the device)
    usr?: string;
    pwd?: string;
    cli?: string;
    min?: number;
}

export const MQTT_CONFIG_DEFAULT: IMqttConfig = {
    status: 'DEFAULT',
    use: false,
    srv: '',
    prt: 1883,
    crt: '',
    usr: '',
    pwd: '',
    cli: '',
    min: 5
};