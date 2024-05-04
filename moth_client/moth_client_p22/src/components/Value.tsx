import { Avatar, Paper } from '@mui/material';
import { IValueProps } from '../types/IValueProps';
import { useState } from 'react';

const Value = (props: IValueProps) => {

    // const { value, unit, grow, icon } = { ...props };

    const [elevation, setElevation] = useState<number>(1);

    const handleMouseOver = () => {
        setElevation(3);
    }

    const handleMouseOut = () => {
        setElevation(1);
    }

    return (
        <Paper
            onClick={() => props.handleClick()}
            onMouseDown={handleMouseOut}
            onMouseUp={handleMouseOver}
            onMouseOver={handleMouseOver}
            onMouseOut={handleMouseOut}
            elevation={elevation}
            sx={{ cursor: 'pointer', flexGrow: props.grow, display: 'flex', flexDirection: 'row', flexWrap: 'wrap', margin: '5px', padding: '5px 10px 2px 5px', height: '48px', alignItems: 'center', backgroundColor: '#FAFAFA', border: '1px solid #DDDDDD' }}
        >
            <Avatar sx={{ verticalAlign: 'middle', margin: '0px 10px 4px 0px', width: '38px', height: '38px' }} >
                {props.icon}
            </Avatar>
            <div style={{ display: 'flex', fontSize: '20px', alignItems: 'baseline' }}>
                <span style={{ fontSize: '1.3em' }}>{props.value}</span>
                <span>&nbsp;{props.unit}</span>
            </div>
        </Paper>
    );

};

export default Value;
