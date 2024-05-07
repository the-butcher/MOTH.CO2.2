import AddBoxIcon from '@mui/icons-material/AddBox';
import DeleteIcon from '@mui/icons-material/Delete';
import { FormControl, IconButton, Stack, TextField, Typography } from '@mui/material';
import { useEffect, useState } from 'react';
import { IValueStringConfig } from '../types/IConfigChoiceProps';
import { INetworkChoiceProperties } from '../types/INetworkChoiceProperties';
import ConfigInputPassword from './ConfigInputPassword';

const NetworkChoice = (props: INetworkChoiceProperties) => {

    const { idx, lbl, pwd, handleNetworkUpdate } = { ...props };

    const isCreate = lbl === '' && pwd === '';

    const [lblCreate, setLblCreate] = useState<string>(lbl);
    const [pwdCreate, setPwdCreate] = useState<string>(pwd);
    const [pwdProps, setPwdProps] = useState<IValueStringConfig>();

    useEffect(() => {

        console.debug(`⚙ updating tab config component (lbl, pwd)`, lbl, pwd);
        setLblCreate(lbl);
        setPwdCreate(pwd);

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [idx, lbl, pwd]);

    useEffect(() => {

        console.debug(`⚙ updating tab config component (pwdCreate)`, pwdCreate);
        setPwdProps({
            type: 'string',
            value: pwdCreate,
            handleUpdate: handlePwdEdit
        });

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [pwdCreate]);

    const handleLblEdit = (value: string) => {
        if (isCreate) {
            setLblCreate(value);
        } else {
            handleNetworkUpdate(idx, value, pwd);
        }
    }

    const handlePwdEdit = (value: string) => {
        console.log('handlePwdEdit', value, isCreate);
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
                idx === 0 ? <Typography className='fieldlabel'>network connections</Typography> : null
            }

            <div style={{ flexGrow: 5 }}></div>
            <Stack spacing={0} direction={'row'} sx={{ alignItems: 'center', minWidth: '200px', maxWidth: '450px' }}>
                <FormControl sx={{ m: 1, flexGrow: 5, margin: '0px 10px 0px 0px' }}>
                    <TextField
                        value={lblCreate}
                        onChange={(e) => handleLblEdit(e.target.value)}
                    />
                </FormControl>
                <ConfigInputPassword {...pwdProps} />
                {
                    isCreate ? <IconButton size='small' aria-label='values' onClick={handleNetworkCreate}>
                        <AddBoxIcon />
                    </IconButton> : <IconButton size='small' aria-label='values' onClick={handleNetworkDelete}>
                        <DeleteIcon />
                    </IconButton>
                }
            </Stack>
        </Stack>
    );

};

export default NetworkChoice;
