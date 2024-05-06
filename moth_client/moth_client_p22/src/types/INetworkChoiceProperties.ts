
export interface INetworkChoiceProperties {
    idx: number;
    lbl: string;
    pwd: string;
    handleNetworkUpdate: (idx: number, lbl: string, pwd: string) => void;
}