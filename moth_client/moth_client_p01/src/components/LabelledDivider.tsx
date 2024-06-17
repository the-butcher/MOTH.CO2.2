import { Divider, Stack, Typography } from '@mui/material';
import { ILabelledDividerProps } from '../types/ILabelledDividerProps';

const LabelledDivider = (props: ILabelledDividerProps) => {

    const { label, style } = props;

    return (
        <Stack direction={'row'} sx={{ alignItems: 'center', paddingTop: style?.paddingTop ? style.paddingTop : 'unset' }}>
            {
                style?.minWidth ? <div style={{ flexGrow: 1 }}></div> : null
            }
            <Stack direction={'row'} sx={{ alignItems: 'center', minWidth: style?.minWidth ? style.minWidth : '100%', maxWidth: style?.maxWidth ? style.maxWidth : '100%', flexGrow: 5 }}>
                <Divider sx={{ flexGrow: 5 }} />
                <Typography variant='caption' sx={{ whiteSpace: 'nowrap' }}>{label}</Typography>
                <Divider sx={{ minWidth: '32px' }} />
            </Stack>
        </Stack>
    );
}

export default LabelledDivider;