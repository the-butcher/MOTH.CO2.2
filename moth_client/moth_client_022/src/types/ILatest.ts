/**
 * definition for the 'api/latest' response
 */
export interface ILatest {
    time: string;
    co2_lpf?: number;
    co2_raw?: number;
    deg: number;
    hum: number;
    hpa: number;
    bat?: number;
}