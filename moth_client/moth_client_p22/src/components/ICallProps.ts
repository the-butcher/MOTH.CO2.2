import { TResponseType } from "./IResponseProps";

export interface ICallProps {
    name: string;
    time: number;
    url: string;
    bodyParams: {};
    returns: TResponseType;
}