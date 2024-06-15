import { TResponseType } from "./IResponseProps";

/**
 * TODO :: distinction from IApiCall
 */
export interface ICallProps {
    name: string;
    time: number;
    url: string;
    bodyParams: {};
    returns: TResponseType;
}