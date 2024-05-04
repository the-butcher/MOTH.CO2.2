export type TRecordKey = 'instant' | 'co2Lpf' | 'deg' | 'hum' | 'co2Raw' | 'hpa' | 'bat';
export type IRecord = { [K in TRecordKey]: number }; // extends DatasetElementType<number> {
export interface IDataset {
    records: IRecord[];
}