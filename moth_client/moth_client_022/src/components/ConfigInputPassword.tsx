import VisibilityIcon from '@mui/icons-material/Visibility';
import VisibilityOffIcon from '@mui/icons-material/VisibilityOff';
import { IconButton, InputAdornment, TextField } from "@mui/material";
import { useState } from 'react';
import { IValueStringConfig } from '../types/IConfigChoiceProps';

const ConfigInputPassword = (props: IValueStringConfig) => {

    const { value, handleUpdate } = { ...props };

    const [pwdType, setPwdType] = useState<'password' | 'text'>('password');

    const togglePwdType = () => {
        if (pwdType === 'password') {
            setPwdType('text');
        } else {
            setPwdType('password');
        }
    }

    return (
        <TextField
            type={pwdType}
            value={value}
            onChange={(e) => handleUpdate(e.target.value)}
            InputProps={{
                endAdornment: <InputAdornment position="end">
                    <IconButton sx={{ padding: '0px' }} onClick={togglePwdType}>
                        {
                            pwdType === 'password' ? <VisibilityIcon /> : <VisibilityOffIcon />
                        }
                    </IconButton>
                </InputAdornment>,
            }}
        />
    );

};

export default ConfigInputPassword;
