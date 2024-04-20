export interface IResponseProps {
    time: number;
    href: string;
    data: string;
    http: 'POST' | 'GET';
    type: 'json' | 'csv';
}