import { IConfig } from "./IConfig";

export interface IMqttConfig extends IConfig {
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
    // srv: '',
    // prt: 0,
    // crt: '',
    // usr: '',
    // pwd: '',
    // cli: '',
    // min: 0
};