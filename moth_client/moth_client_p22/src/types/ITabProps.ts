import { CSSProperties } from "react";

export type TOrientation = 'portrait' | 'landscape';

export interface ITabProps {
    boxUrl: string;
    style?: CSSProperties;
}