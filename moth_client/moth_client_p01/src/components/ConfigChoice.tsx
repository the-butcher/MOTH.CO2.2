import { FormControl, Stack, Switch, Typography } from '@mui/material';
import { IConfigChoiceProps } from '../types/IConfigChoiceProps';
import ConfigChoiceNumber from './ConfigChoiceNumber';
import ConfigChoiceString from './ConfigChoiceString';


const ConfigChoice = (props: IConfigChoiceProps) => {

    const { value } = { ...props };

    return (
        <Stack direction={'row'}>
            <Typography className='fieldlabel'>{props.caption}</Typography>
            <div style={{ flexGrow: 1 }}></div>
            <FormControl sx={{ minWidth: '200px', maxWidth: '450px', flexGrow: 10 }}>
                {
                    value.type === 'toggle' ? <Switch
                        checked={value.value}
                        onChange={(config, checked) => value.handleUpdate(checked)}
                    /> : null
                }
                {
                    value.type === 'number' ? <ConfigChoiceNumber {...value} /> : null
                }
                {
                    value.type === 'string' ? <ConfigChoiceString {...value} /> : null
                }
            </FormControl>
        </Stack>
    );

};

export default ConfigChoice;