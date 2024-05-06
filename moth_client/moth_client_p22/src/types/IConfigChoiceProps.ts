export interface IValueToggleConfig {
    type: 'toggle';
    value: boolean;
    handleUpdate: (value: boolean) => void;
}

export interface IValueNumberConfig {
    type: 'number';
    value: number;
    fixed: boolean;
    unit?: string;
    help?: string;
    items?: { [K: string]: number };
    handleUpdate: (value: number) => void;
}

export interface IValueStringConfig {
    type: 'string';
    value: string;
    items?: { [K: string]: string };
    handleUpdate: (value: string) => void;
}

export interface IConfigChoiceProps {
    caption: string;
    value: IValueToggleConfig | IValueNumberConfig | IValueStringConfig;
}