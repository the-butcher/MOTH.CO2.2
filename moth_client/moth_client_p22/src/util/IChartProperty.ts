export interface IChartProperty {
    label: string;
    toDomain: (data: any[]) => [number, number];
    thresholds: {
        riskLo: number;
        warnLo: number;
        warnHi: number;
        riskHi: number;
    }
}