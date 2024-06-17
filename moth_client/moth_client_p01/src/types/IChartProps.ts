import { IRecord } from "./IRecord";
import { ISeriesDef } from "./ISeriesDef";

export type TExportTo = '' | 'png';

export interface IChartProps {
    height: number;
    width?: number;
    records: IRecord[];
    seriesDef: ISeriesDef;
    exportTo: TExportTo;
    handleExportComplete: () => void;
}