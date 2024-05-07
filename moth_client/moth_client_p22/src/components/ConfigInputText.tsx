import { TextField } from "@mui/material";
import { IValueStringConfig } from "../types/IConfigChoiceProps";

const ConfigInputText = (props: IValueStringConfig) => {

    const { value, help, handleUpdate } = { ...props }

    return (
        <TextField
            value={value}
            onChange={(e) => handleUpdate(e.target.value)}
            helperText={help ? help : undefined}
        />
    );

};

export default ConfigInputText;