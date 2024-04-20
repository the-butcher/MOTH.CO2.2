export interface ICallProps {
    name: string;
    time: number;
    url: string;
    bodyParams: {};
    returns: 'json' | 'csv';
}