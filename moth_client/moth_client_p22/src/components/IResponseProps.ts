export type TResponseType = 'json' | 'csv' | 'dat';

export interface IResponseProps {
    time: number;
    href: string;
    data: string;
    http: 'POST' | 'GET';
    type: TResponseType;
}