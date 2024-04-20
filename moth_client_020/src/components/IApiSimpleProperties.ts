import { IApiCall } from "./IApiCall";
import { IApiProperties } from "./IApiProperties";

export interface IApiSimpleProperties extends IApiProperties {
    apiName: string;
    apiDesc: string;
    boxUrl: string;
    panels: string[];
    handlePanel: (panel: string) => (event: React.SyntheticEvent, isExpanded: boolean) => void;
    handleApiCall: (call: IApiCall) => void;
    confirm?: {
        title: string;
        content: string;
    }
}