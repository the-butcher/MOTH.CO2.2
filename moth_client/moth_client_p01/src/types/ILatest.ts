/**
 * definition for the 'api/latest' response
 */
export interface ILatest {
    time: string;
    pm010: number;
    pm025: number;
    pm100: number;
    deg: number;
    hum: number;
    hpa: number;
}