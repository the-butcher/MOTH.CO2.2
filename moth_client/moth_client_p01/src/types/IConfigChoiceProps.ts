export interface IValueToggleConfig {
    type: 'toggle';
    value: boolean;
    handleUpdate: (value: boolean) => void;
}

export interface IValueConfig<T = number | string> {
    value: T;
    items?: { [K: string]: T };
    handleUpdate: (value: T) => void;
}

export interface IValueNumberConfig<T = number> extends IValueConfig<T> {
    type: 'number';
    fixed: boolean;
    unit?: string;
    help?: string;
    step?: T;
    min?: T,
    max?: T,
}

export interface IValueStringConfig<T = string> extends IValueConfig<T> {
    type: 'string';
    help?: string;
    pwd?: boolean,
}

export interface IConfigChoiceProps {
    caption: string;
    value: IValueToggleConfig | IValueNumberConfig | IValueStringConfig;
}