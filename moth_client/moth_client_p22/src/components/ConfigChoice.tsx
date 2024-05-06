import { FormControl, FormHelperText, InputAdornment, MenuItem, Select, Stack, Switch, TextField, Typography } from "@mui/material";
import { IConfigChoiceProps } from "../types/IConfigChoiceProps";


const ConfigChoice = (props: IConfigChoiceProps) => {

    const { value } = { ...props };

    const parseNumber = (value: string | number, fixed: boolean): number => {
        const numberValue = typeof value === 'string' ? parseFloat(value) : value;
        return fixed ? Math.round(numberValue) : numberValue;
    }

    return (
        <Stack direction={'row'} sx={{ alignItems: 'center' }}>
            <Typography>{props.caption}</Typography>
            <div style={{ flexGrow: 5 }}></div>
            <FormControl sx={{ m: 1, minWidth: 120, width: '450px' }}>
                {
                    value.type === 'toggle' ? <Switch
                        checked={value.value}
                        onChange={(config, checked) => value.handleUpdate(checked)}
                    /> : null
                }
                {
                    value.type === 'number' && !value.items ? <TextField
                        type='number'
                        value={value.value}
                        onChange={(e) => value.handleUpdate(parseNumber(e.target.value, value.fixed))}
                        InputProps={{
                            endAdornment: value.unit ? <InputAdornment position="end">{value.unit}</InputAdornment> : null,
                        }}
                        helperText={value.help ? value.help : undefined}
                    /> : null
                }
                {
                    value.type === 'number' && value.items ? <>
                        <Select
                            value={value.value}
                            // onChange={handleChange}
                            displayEmpty
                            inputProps={{ 'aria-label': 'Without label' }}
                            onChange={(e) => value.handleUpdate(parseNumber(e.target.value, value.fixed))}
                        // endAdornment={
                        //     <InputAdornment position="end">val&nbsp;&nbsp;</InputAdornment>
                        // }
                        >
                            {
                                Object.keys(value.items).map(key => <MenuItem value={value.items[key]}>{key}</MenuItem>)
                            }
                        </Select>
                        {/* <FormHelperText>select info</FormHelperText> */}
                    </> : null
                }
                {
                    value.type === 'string' && !value.items ? <TextField
                        value={value.value}
                        onChange={(e) => value.handleUpdate(e.target.value)}
                    /> : null
                }
                {
                    value.type === 'string' && value.items ? <>
                        <Select
                            value={value.value}
                            // onChange={handleChange}
                            displayEmpty
                            inputProps={{ 'aria-label': 'Without label' }}
                            onChange={(e) => value.handleUpdate(e.target.value)}
                        // endAdornment={
                        //     <InputAdornment position="end">val&nbsp;&nbsp;</InputAdornment>
                        // }
                        >
                            {
                                Object.keys(value.items).map(key => <MenuItem value={value.items[key]}>{key}</MenuItem>)
                            }
                        </Select>
                        {/* <FormHelperText>select info</FormHelperText> */}
                    </> : null
                }

                {/* <Select
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
                </Select> */}
            </FormControl>
        </Stack>
    );

};

export default ConfigChoice;