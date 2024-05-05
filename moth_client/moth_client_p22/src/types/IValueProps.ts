export interface IValueProps {
    value: string;
    unit: string;
    grow: string;
    icon: JSX.Element;
    active: boolean;
    handleClick: () => void;
}