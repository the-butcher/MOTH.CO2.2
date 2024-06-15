import { CSSProperties } from "react";

/**
 * definition for the properties of a ValueChoice component
 */
export interface IValueChoiceProps {
    value: string;
    unit: string;
    icon: JSX.Element;
    active: boolean;
    handleClick: () => void;
    style?: CSSProperties;
}