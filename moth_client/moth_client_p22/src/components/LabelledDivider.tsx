import { Divider, Stack, Typography } from "@mui/material";
import { ILabelledDividerProps } from "../types/ILabelledDividerProps";

const LabelledDivider = (props: ILabelledDividerProps) => {

    const { label, style } = props;

    return (
        <Stack direction={'row'} sx={{ alignItems: 'center', paddingTop: style?.paddingTop ? style.paddingTop : 'unset' }}>
            {
                style?.width ? <div style={{ flexGrow: 5 }}></div> : null
            }
            <Stack direction={'row'} sx={{ alignItems: 'center', width: style?.width ? style.width : '100%' }}>
                <Divider sx={{ flexGrow: 5 }} />
                <Typography variant='caption' sx={{ whiteSpace: 'nowrap' }}>{label}</Typography>
                <Divider sx={{ minWidth: '20px' }} />
            </Stack>
        </Stack>
    );
}

export default LabelledDivider;