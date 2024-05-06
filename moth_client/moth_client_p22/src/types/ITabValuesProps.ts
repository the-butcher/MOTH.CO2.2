import { ILatest } from "./ILatest";
import { IRecord } from "./IRecord";
import { ISeriesDef } from "./ISeriesDef";
import { ITabProps } from "./ITabProps";

export interface ITabValuesProps extends ITabProps {
    dateRangeData: [Date, Date];
    dateRangeUser: [Date, Date];
    latest: ILatest;
    records: IRecord[];
    seriesDef: ISeriesDef;
    handleUpdate: (tabProps: Partial<ITabValuesProps>) => void;
}