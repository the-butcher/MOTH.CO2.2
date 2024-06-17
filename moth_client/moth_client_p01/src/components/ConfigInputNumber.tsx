

import { InputAdornment, TextField } from "@mui/material";
import { useEffect, useRef, useState } from 'react';
import { IValueNumberConfig } from '../types/IConfigChoiceProps';

const ConfigInputNumber = (props: IValueNumberConfig) => {

    const { value, min, max, fixed, help, step, unit, handleUpdate } = { ...props };

    const handleFieldNumberToRef = useRef<number>(-1);

    const [fieldValue, setFieldValue] = useState<string | number>(value);
    const handleFieldValue = (_fieldValue: string | number) => {
        setFieldValue(_fieldValue);
        window.clearTimeout(handleFieldNumberToRef.current);
        handleFieldNumberToRef.current = window.setTimeout(() => {
            parseFieldValue(_fieldValue);
        }, 200);
    }

    const parseFieldValue = (_fieldValue: string | number) => {
        let numberValue = typeof _fieldValue === 'string' ? parseFloat(_fieldValue) : _fieldValue;
        if (Number.isFinite(min)) {
            numberValue = Math.max(min, numberValue);
        }
        if (Number.isFinite(max)) {
            numberValue = Math.min(max, numberValue);
        }
        numberValue = fixed ? Math.round(numberValue) : numberValue;
        if (!Number.isFinite(numberValue)) {
            numberValue = value;
        }
        if (numberValue !== value) {
            handleUpdate(numberValue);
        } else {
            setFieldValue(numberValue); // directly reset the value without calling back
        }
    }

    useEffect(() => {

        console.debug('⚙ updating config choice number component (value)', value);
        setFieldValue(value);

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [value]);

    return (
        <TextField
            type='number'
            value={fieldValue}
            onChange={(e) => handleFieldValue(e.target.value)}
            InputProps={{
                endAdornment: unit ? <InputAdornment position="end">{unit}</InputAdornment> : null,
            }}
            inputProps={{
                step: step ? step : 1
            }}
            helperText={help ? help : undefined}
        />
    );

};

export default ConfigInputNumber;
