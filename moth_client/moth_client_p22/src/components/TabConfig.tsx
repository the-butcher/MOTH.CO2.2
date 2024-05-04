import { useEffect } from 'react';
import { ITabProperties } from '../types/ITabProperties';

const TabConfig = (props: ITabProperties) => {

  const { boxUrl } = { ...props };



  useEffect(() => {

    console.debug('✨ building tab config component');


    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  return (
    <div>


    </div>
  );

};

export default TabConfig;
