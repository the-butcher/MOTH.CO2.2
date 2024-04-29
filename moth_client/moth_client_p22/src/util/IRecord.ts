export interface IRecord {
    instant: number;
    co2Lpf: number;
    deg: number;
    hum: number;
    co2Raw: number;
    hpa: number;
    nrg: number;
}

export interface IDataset {
    records: IRecord[];
}