import { Stack } from '@mui/material';
import { useEffect, useRef, useState } from 'react';
import { IApiCall } from '../types/IApiCall';
import { IApiProperties } from '../types/IApiProperties';
import { ITabProps } from '../types/ITabProps';
import ApiCo2Cal from './ApiCo2Cal';
import ApiDatOut from './ApiDatOut';
import ApiDatcsv from './ApiDatcsv';
import ApiDelete from './ApiDelete';
import ApiDirOut from './ApiDirOut';
import ApiDspSet from './ApiDspSet';
import ApiSimple from './ApiSimple';
import ApiUpdate from './ApiUpdate';
import ApiUpload from './ApiUpload';
import LabelledDivider from './LabelledDivider';

const TabServer = (props: ITabProps) => {

  const { boxUrl } = { ...props };

  let messageTimeout: number = -1;

  const rebuildAndSetApiProps = () => {
    setApiProps({
      ...apiProps,
      boxUrl,
      panels: panels.current,
      handlePanel,
      handleApiCall
    });
  }

  /**
   * change in open panels
   * @param panel
   * @returns
   */
  const handlePanel = (panel: string, expanded: boolean) => {

    console.debug('handlePanel', panel, expanded);

    const contained = panels.current.indexOf(panel) >= 0;
    if (contained !== expanded) {
      panels.current = panels.current.filter(p => p !== panel)
      if (expanded) {
        panels.current.push(panel);
      }
      console.debug('handlePanel', panel, panels)
      rebuildAndSetApiProps();
    }
  };

  /**
   * handle a specific call to the api
   * @param call
   */
  const handleApiCall = (call: IApiCall) => {

    var iframe = document.getElementById("callframe") as HTMLIFrameElement;
    iframe.contentWindow.postMessage(call, "*");

    clearTimeout(messageTimeout);
    messageTimeout = window.setTimeout(() => {
      iframe.src = iframeSrc;
      rebuildAndSetApiProps();
    }, 10000);

  }

  const panels = useRef<string[]>([]);

  const [apiProps, setApiProps] = useState<IApiProperties>({
    boxUrl,
    panels: panels.current,
    handlePanel,
    handleApiCall
  });

  useEffect(() => {

    console.debug('✨ building tab server component', window.location);

    window.addEventListener('message', ({ data }) => {

      if (data.call) {
        window.clearTimeout(messageTimeout);
        const _apiProps: IApiProperties = {
          ...apiProps,
          boxUrl,
          panels: panels.current,
          handlePanel,
          handleApiCall
        };
        _apiProps[data.call] = data.data; // set the specific message on the api props
        setApiProps(_apiProps);
      }
      if (data.event && data.event === 'loaded') {
        rebuildAndSetApiProps();
      }

    });
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const iframeSrc = 'iframe.html'; //when running directly from device

  return (
    <Stack direction={'column'} spacing={0} sx={{ height: '100%', margin: '5px', ...props.style }}>

      <LabelledDivider label='data' />
      <ApiSimple {...{
        ...apiProps,
        apiName: 'latest',
        apiDesc: 'get the latest measurement as json',
        apiType: 'json'
      }} />
      <ApiSimple {...{
        ...apiProps,
        apiName: 'valcsv',
        apiDesc: 'get the last hour of data as csv data',
        apiType: 'csv'
      }} />
      <ApiDatcsv {...apiProps} />
      <ApiSimple {...{
        ...apiProps,
        apiName: 'valout',
        apiDesc: 'get the last hour of data as binary data',
        apiType: 'dat'
      }} />
      <ApiDatOut {...apiProps} />

      <LabelledDivider label='files, folders' style={{ paddingTop: '12px' }} />
      <ApiDirOut {...apiProps} />
      <ApiUpload {...apiProps} />
      <ApiDelete {...apiProps} apiName='datdel' apiProp='file' />
      <ApiDelete {...apiProps} apiName='dirdel' apiProp='folder' />

      <LabelledDivider label='display, status' style={{ paddingTop: '12px' }} />
      <ApiDspSet {...apiProps} />
      <ApiSimple {...{
        ...apiProps,
        apiName: 'status',
        apiDesc: 'get details about device status',
        apiType: 'json'
      }} />

      <LabelledDivider label='connectivity' style={{ paddingTop: '12px' }} />
      <ApiSimple {...{
        ...apiProps,
        apiName: 'netout',
        apiDesc: 'get a list of networks visible to the device',
        apiType: 'json'
      }} />
      <ApiSimple {...{
        ...apiProps,
        apiName: 'netoff',
        apiDesc: 'disconnect the device',
        apiType: 'json'
      }} />

      <LabelledDivider label='admin' style={{ paddingTop: '12px' }} />
      <ApiCo2Cal {...apiProps} />
      <ApiSimple {...{
        ...apiProps,
        apiName: 'co2rst',
        apiDesc: 'reset the CO₂ sensor to factory',
        apiType: 'json',
        confirm: {
          title: 'do you really want to reset?',
          content: 'this call will remove all calibration history from the box\'s CO₂ sensor. only use if the sensor appears to be stuck.'
        }
      }} />
      <ApiSimple {...{
        ...apiProps,
        apiName: 'esprst',
        apiDesc: 'resets the device ',
        apiType: 'json',
        confirm: {
          title: 'do you really want to reset?',
          content: 'this call will reset the decive, data not written to permanent storage may be lost.'
        }
      }} />
      <ApiUpdate  {...apiProps} />

      <iframe title="callframe" id="callframe" src={iframeSrc} style={{ height: '12px', border: 'none' }} />

    </Stack>
  );

};

export default TabServer;
