import { IValueStringConfig } from "../types/IConfigChoiceProps";
import ConfigInputPassword from "./ConfigInputPassword";
import ConfigInputSelect from "./ConfigInputSelect";
import ConfigInputText from "./ConfigInputText";

const ConfigChoiceString = (props: IValueStringConfig) => {
    const { items, pwd } = { ...props }
    return items ? <ConfigInputSelect {...props} /> : pwd ? <ConfigInputPassword {...props} /> : <ConfigInputText {...props} />
};

export default ConfigChoiceString;