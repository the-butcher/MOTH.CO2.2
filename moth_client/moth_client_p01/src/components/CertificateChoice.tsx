import GppBadIcon from '@mui/icons-material/GppBad';
import GppGoodIcon from '@mui/icons-material/GppGood';
import { FormControl, Stack, TextField, Typography } from '@mui/material';
import { useEffect } from 'react';
import { ICertConfig } from '../types/ICertConfig';

export interface IValueCertificateConfig {
    cert: ICertConfig;
    handleUpdate: (value: string) => void;
}

const CertificateChoice = (props: IValueCertificateConfig) => {

    const { cert, handleUpdate } = { ...props }

    /**
     * component init hook
     */
    useEffect(() => {
        console.debug('✨ building certificate choice component');
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, []);

    /**
     * react hook (props[apiName])
     */
    useEffect(() => {
        console.log(`⚙ updating certificate choice component (cert)`, cert);
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [cert]);

    return (
        <Stack direction={'row'}>
            <Typography className='fieldlabel'>mqtt certificate</Typography>
            <div style={{ flexGrow: 1 }}></div>
            <Stack spacing={0} direction={'row'} sx={{ minWidth: '200px', maxWidth: '450px', flexGrow: 10 }}>
                <FormControl sx={{ flexGrow: 5 }}>
                    <TextField
                        value={cert.crt}
                        onChange={(e) => handleUpdate(e.target.value)}
                        multiline
                        rows={10}
                    />
                </FormControl>
                {
                    cert.status === 'MODIFIED_INVALID' ? <GppBadIcon sx={{ margin: '10px 5px' }} /> : <GppGoodIcon sx={{ margin: '10px 5px' }} />
                }
            </Stack>
        </Stack>
    );

};

export default CertificateChoice;
