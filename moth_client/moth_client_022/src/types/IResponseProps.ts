export type TResponseType = 'json' | 'csv' | 'dat';
export type TRequestMethod = 'POST' | 'GET';

export interface IResponseProps {
    time: number;
    href: string;
    data: string;
    http: TRequestMethod;
    type: TResponseType;
}