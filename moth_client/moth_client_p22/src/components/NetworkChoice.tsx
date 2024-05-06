import AddBoxIcon from '@mui/icons-material/AddBox';
import DeleteIcon from '@mui/icons-material/Delete';
import { FormControl, IconButton, InputAdornment, Stack, TextField, Typography } from "@mui/material";
import { INetworkChoiceProperties } from "../types/INetworkChoiceProperties";
import { useEffect, useState } from 'react';
import VisibilityIcon from '@mui/icons-material/Visibility';
import VisibilityOffIcon from '@mui/icons-material/VisibilityOff';

const NetworkChoice = (props: INetworkChoiceProperties) => {

    const { idx, lbl, pwd, handleNetworkUpdate } = { ...props };

    const isCreate = lbl === '' && pwd === '';

    const [lblCreate, setLblCreate] = useState<string>(lbl);
    const [pwdCreate, setPwdCreate] = useState<string>(pwd);
    const [pwdType, setPwdType] = useState<'password' | 'text'>('password');

    useEffect(() => {

        console.debug(`⚙ updating tab config component (lbl, pwd)`, lbl, pwd);
        setLblCreate(lbl);
        setPwdCreate(pwd);

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [idx, lbl, pwd]);

    const togglePwdType = () => {
        if (pwdType === 'password') {
            setPwdType('text');
        } else {
            setPwdType('password');
        }
    }

    const handleLblEdit = (value: string) => {
        if (isCreate) {
            setLblCreate(value);
        } else {
            handleNetworkUpdate(idx, value, pwd);
        }
    }

    const handlePwdEdit = (value: string) => {
        if (isCreate) {
            setPwdCreate(value);
        } else {
            handleNetworkUpdate(idx, lbl, value);
        }
    }

    const handleNetworkCreate = () => {
        handleNetworkUpdate(idx, lblCreate, pwdCreate);
    }

    const handleNetworkDelete = () => {
        handleNetworkUpdate(idx, '', '');
    }

    return (
        <Stack direction={'row'} sx={{ alignItems: 'center', padding: '0px' }}>
            {
                idx === 0 ? <Typography>network connections</Typography> : null
            }

            <div style={{ flexGrow: 5 }}></div>
            <Stack spacing={0} direction={'row'} sx={{ alignItems: 'center', minWidth: 120, width: '450px' }}>
                <FormControl sx={{ m: 1, flexGrow: 5, margin: '0px 5px 0px 0px' }}>
                    <TextField
                        value={lblCreate}
                        onChange={(e) => handleLblEdit(e.target.value)}
                    />
                </FormControl>
                <FormControl sx={{ m: 1, flexGrow: 5, margin: '0px 0px 0px 5px' }}>
                    <TextField
                        type={pwdType}
                        value={pwdCreate}
                        onChange={(e) => handlePwdEdit(e.target.value)}
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
                </FormControl>
                {
                    isCreate ? <IconButton size='small' aria-label="values" onClick={handleNetworkCreate}>
                        <AddBoxIcon />
                    </IconButton> : <IconButton size='small' aria-label="values" onClick={handleNetworkDelete}>
                        <DeleteIcon />
                    </IconButton>
                }
            </Stack>
        </Stack>
    );

};

export default NetworkChoice;
