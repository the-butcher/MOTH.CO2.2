export type TConfigStatus = 'DEFAULT' | 'LOADED' | 'MODIFIED_VALID' | 'MODIFIED_INVALID';

export interface IConfig {
    status: TConfigStatus;
}