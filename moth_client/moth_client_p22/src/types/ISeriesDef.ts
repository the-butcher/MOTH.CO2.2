import { ContinuousColorConfig, PiecewiseColorConfig } from "@mui/x-charts/models/colorMapping";
import { TRecordKey } from "./IRecord";

export interface ISeriesDef {
    id: TRecordKey;
    label: string;
    valueFormatter: (value: number) => string;
    colorMap?: ContinuousColorConfig | PiecewiseColorConfig;
    min?: number;
}