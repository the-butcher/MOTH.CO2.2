import { TimeUtil } from "../util/TimeUtil";

export interface ITickDefinition {
    step: number;
}

export const TICK_DEFINITIONS: ITickDefinition[] = [
    {
        step: TimeUtil.MILLISECONDS_PER_MINUTE * 10
    },
    {
        step: TimeUtil.MILLISECONDS_PER_MINUTE * 15
    },
    {
        step: TimeUtil.MILLISECONDS_PER_MINUTE * 30
    },
    {
        step: TimeUtil.MILLISECONDS_PER_HOUR
    },
    {
        step: TimeUtil.MILLISECONDS_PER_HOUR * 3
    },
    {
        step: TimeUtil.MILLISECONDS_PER_HOUR * 6
    },
    {
        step: TimeUtil.MILLISECONDS_PER_HOUR * 12
    },
    {
        step: TimeUtil.MILLISECONDS_PER__DAY
    }
];