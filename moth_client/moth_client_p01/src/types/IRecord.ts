/**
 * definition of possible record-keys
 */
export type TRecordKey = 'instant' | 'pm010' | 'pm025' | 'pm100' | 'deg' | 'hum' | 'hpa';

/**
 * definition for a record (i.e. parsed from the 'api/datout' endpoint)
 */
export type IRecord = { [K in TRecordKey]: number }; // extends DatasetElementType<number> {

/**
 * definition for a dataset
 */
export interface IDataset {
    records: IRecord[];
}