import { IRecord } from "./IRecord";

export interface IChartProperty {
    label: string;
    toDomain: (data: IRecord[]) => [number, number];
    thresholds: {
        riskLo: number;
        warnLo: number;
        warnHi: number;
        riskHi: number;
    }
}