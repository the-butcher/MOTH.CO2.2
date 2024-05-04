import { IRecord } from "./IRecord";
import { IThresholds } from "./IThresholds";

export interface IChartProperty {
    label: string;
    toDomain: (data: IRecord[]) => [number, number];
    thresholds: IThresholds;
}