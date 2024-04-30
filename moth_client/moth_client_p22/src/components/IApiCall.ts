import { TResponseType } from "./IResponseProps";

export interface IApiCall {
    href: string;
    call: string;
    meth: 'POST' | 'GET';
    type: TResponseType;
    qstr?: {}
}