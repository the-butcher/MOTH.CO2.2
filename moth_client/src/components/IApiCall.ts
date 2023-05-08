export interface IApiCall {
    href: string;
    call: string;
    meth: 'GET' | 'POST';
    qstr?: {}
}