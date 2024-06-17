import { Avatar, Paper } from '@mui/material';
import { IValueChoiceProps } from '../types/IValueChoiceProps';
import { useState } from 'react';

/**
 * component, renders a value choice button in the TabValues component, i.e CO2, deg, hum, ...
 * @param props
 * @returns
 */
const ValueChoice = (props: IValueChoiceProps) => {

    const { style } = { ...props };

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
            elevation={props.active ? elevation : elevation + 2}
            sx={{ ...style, cursor: 'pointer', display: 'flex', flexDirection: 'row', flexWrap: 'wrap', margin: '5px', padding: '4px 6px 2px 5px', height: '36px', alignItems: 'center', backgroundColor: '#FAFAFA', border: '1px solid #DDDDDD' }}
        >
            <Avatar sx={{ verticalAlign: 'middle', margin: '0px 10px 4px 0px', width: '26px', height: '26px' }} >
                {props.icon}
            </Avatar>
            <div style={{ display: 'flex', fontSize: '0.8em', alignItems: 'baseline' }}>
                <span style={{ fontSize: '1.3em' }}>{props.value}</span>
                <span>&nbsp;{props.unit}</span>
            </div>
        </Paper>
    );

};

export default ValueChoice;
