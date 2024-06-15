import { TRequestMethod, TResponseType } from "./IResponseProps";

export interface IApiCall {
    href: string;
    call: string;
    meth: TRequestMethod;
    type: TResponseType;
    qstr?: {}
}