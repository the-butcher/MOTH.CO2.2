export interface IApiCall {
    href: string;
    call: string;
    meth: 'POST' | 'GET';
    type: 'json' | 'csv';
    qstr?: {}
}