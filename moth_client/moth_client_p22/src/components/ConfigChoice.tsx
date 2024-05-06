import { FormControl, MenuItem, Select, Stack, Typography } from "@mui/material";
import { IConfigChoiceProps } from "../types/IConfigChoiceProps";


const ConfigChoice = (props: IConfigChoiceProps) => {

    return (
        <Stack direction={'row'} sx={{ alignItems: 'center' }}>
            <Typography>{props.caption}</Typography>
            <div style={{ flexGrow: 5 }}></div>
            <FormControl sx={{ m: 1, minWidth: 120, width: '450px' }}>
                <Select
                    value={10}
                    // onChange={handleChange}
                    displayEmpty
                    inputProps={{ 'aria-label': 'Without label' }}
                >
                    <MenuItem value="">
                        <em>None</em>
                    </MenuItem>
                    <MenuItem value={10}>Ten</MenuItem>
                    <MenuItem value={20}>Twenty</MenuItem>
                    <MenuItem value={30}>Thirty</MenuItem>
                </Select>
            </FormControl>
        </Stack>
    );

};

export default ConfigChoice;