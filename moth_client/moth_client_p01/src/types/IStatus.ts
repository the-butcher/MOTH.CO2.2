/**
 * definition for the 'api/status' response
 */
export interface IStatus {
    /**
     * boot time of the device (in the format 2024.05.17 19:14:03)
     */
    boot: string;
    /**
     * version number of the device
     */
    vnum: string;
}