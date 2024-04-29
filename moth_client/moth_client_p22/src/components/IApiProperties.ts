import { IApiCall } from "./IApiCall";

export type EStatus = 'disconnected' | 'connected';

export interface IApiProperties {
    boxUrl: string;
    panels: string[];
    pstate: EStatus;
    handlePanel: (panel: string, expaneded: boolean) => void;
    handleApiCall: (call: IApiCall) => void;
}