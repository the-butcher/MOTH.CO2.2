import { IDispConfig } from "./IDispConfig";
import { IMqttConfig } from "./IMqttConfig";
import { ITabProps } from "./ITabProps";
import { IWifiConfig } from "./IWifiConfig";

/**
 * properties needed to render the TabConfig component
 */
export interface ITabConfigProps extends ITabProps {
    disp: IDispConfig,
    wifi: IWifiConfig,
    mqtt: IMqttConfig,
    handleUpdate: (tabProps: Partial<ITabConfigProps>) => void;
}