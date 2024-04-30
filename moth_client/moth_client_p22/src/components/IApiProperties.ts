import { IApiCall } from "./IApiCall";

export interface IApiProperties {
    boxUrl: string;
    panels: string[];
    handlePanel: (panel: string, expaneded: boolean) => void;
    handleApiCall: (call: IApiCall) => void;
}