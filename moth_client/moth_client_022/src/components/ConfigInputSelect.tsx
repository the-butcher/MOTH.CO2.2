import { MenuItem, Select } from "@mui/material";
import { IValueConfig } from "../types/IConfigChoiceProps";

const ConfigInputSelect = (props: IValueConfig) => {

    const { value, items, handleUpdate } = { ...props }

    return (
        <Select
            value={value}
            displayEmpty
            onChange={(e) => handleUpdate(e.target.value)}
        >
            {
                Object.keys(items).map(key => <MenuItem key={key} value={items[key]}>{key}</MenuItem>)
            }
        </ Select>
    );

};

export default ConfigInputSelect;