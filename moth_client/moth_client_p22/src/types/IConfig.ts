type CONFIG_STATUS = 'DEFAULT' | 'LOADED' | 'MODIFIED';

export interface IConfig {
    status: CONFIG_STATUS;
}