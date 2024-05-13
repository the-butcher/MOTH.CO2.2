import { CSSProperties } from "react";

export type TOrientation = 'portrait' | 'landscape';
export type TSeverity = 'success' | 'info' | 'warning' | 'error';

export interface IMessage {
    message: string;
    severity: TSeverity;
    active: boolean;
}

export interface ITabProps {
    boxUrl: string;
    style?: CSSProperties;
    handleAlertMessage: (alertMessage: IMessage) => void;
}