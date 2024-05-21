import { FormControl, FormHelperText, IconButton, Stack, TextField, Typography } from '@mui/material';
import { IValueStringConfig } from '../types/IConfigChoiceProps';
import DeleteIcon from '@mui/icons-material/Delete';
import GppGoodIcon from '@mui/icons-material/GppGood';
import GppBadIcon from '@mui/icons-material/GppBad';
import SafetyCheckIcon from '@mui/icons-material/SafetyCheck';
import { FileUploadOutlined, SafetyCheck } from '@mui/icons-material';
import { useEffect, useState } from 'react';
import { JsonLoader } from '../util/JsonLoader';
import { IMessage } from '../types/ITabProps';
import { ByteLoader } from '../util/ByteLoader';
import { TextLoader } from '../util/TextLoader';

export interface IValueCertificateConfig extends IValueStringConfig {
    caption: string;
    boxUrl: string;
    handleAlertMessage: (alertMessage: IMessage) => void;
}



const CertificateChoice = (props: IValueCertificateConfig) => {

    const { caption, value, help, boxUrl, handleUpdate, handleAlertMessage } = { ...props }

    const [data, setData] = useState<string>();

    const handleUploadFilePicked = (event: React.ChangeEvent<HTMLInputElement>) => {
        setData(event.target.value);
    };

    const checkCertificateFile = () => {

        new TextLoader().load(`${boxUrl}/datout?file=${value}`).then((response) => {
            console.log('response', response);
        }).catch((e: Error) => {
            console.error('e', e);
            handleAlertMessage({
                message: e.message ? e.message : 'failed to check certificate presence',
                severity: 'error',
                active: true
            });
        });

    }

    /**
     * component init hook
     */
    useEffect(() => {

        console.debug('✨ building certificate choice component');

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, []);

    useEffect(() => {

        console.log(`⚙ updating certificate choice component (value)`, value);
        checkCertificateFile();

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [value]);


    return (
        <Stack direction={'row'}>
            <Typography className='fieldlabel'>{caption}</Typography>
            <div style={{ flexGrow: 1 }}></div>
            <Stack direction={'column'} sx={{ minWidth: '200px', maxWidth: '450px', flexGrow: 5 }}>
                <Stack spacing={0} direction={'row'} sx={{ alignItems: 'center' }}>
                    <FormControl sx={{ m: 1, width: '40%', margin: '0px 5px 0px 0px' }}>
                        <TextField
                            value={value}
                            onChange={(e) => handleUpdate(e.target.value as string)}
                        />
                    </FormControl>
                    <FormControl sx={{ m: 1, width: '60%', margin: '0px 0px 0px 5px' }}>
                        <TextField
                            type="text"
                            size='small'
                            label={data && data !== "" ? data : "file"}
                            InputProps={{
                                endAdornment: (
                                    <IconButton component="label">
                                        <FileUploadOutlined />
                                        <input
                                            type="file"
                                            hidden
                                            onChange={handleUploadFilePicked}
                                            name="content"
                                            accept='.crt'
                                        />
                                    </IconButton>
                                ),
                            }}
                        />
                    </FormControl>
                    <SafetyCheck sx={{ margin: '0px 5px' }} />
                </Stack>
                <FormHelperText sx={{ margin: '3px 14px 0px 14px !important' }}>{help}</FormHelperText>
            </Stack>
        </Stack>
    );

};

export default CertificateChoice;
