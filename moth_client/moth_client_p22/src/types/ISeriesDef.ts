import { ContinuousColorConfig, PiecewiseColorConfig } from "@mui/x-charts/models/colorMapping";

export interface ISeriesDef {
    dataKey: string;
    label: string;
    valueFormatter: (value: number) => string;
    colorMap?: ContinuousColorConfig | PiecewiseColorConfig;
}