/**
 * possible configuration stati
 */
export type TConfigStatus = 'DEFAULT' | 'LOADED_VALID' | 'LOADED_INVALID' | 'MODIFIED_VALID' | 'MODIFIED_INVALID';

/**
 * definition for types having a configuration status
 */
export interface IConfig {
    status: TConfigStatus;
}