import { IValueNumberConfig } from "../types/IConfigChoiceProps";
import ConfigInputNumber from "./ConfigInputNumber";
import ConfigInputSelect from "./ConfigInputSelect";

const ConfigChoiceNumber = (props: IValueNumberConfig) => {
    const { items } = { ...props }
    return items ? <ConfigInputSelect {...props} /> : <ConfigInputNumber {...props} />
};

export default ConfigChoiceNumber;