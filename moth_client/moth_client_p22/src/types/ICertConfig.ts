import { IConfig } from "./IConfig";

export interface ICertConfig extends IConfig {
    crt: string;
}

export const CERT_CONFIG_DEFAULT: ICertConfig = {
    status: 'DEFAULT',
    crt: ''
};