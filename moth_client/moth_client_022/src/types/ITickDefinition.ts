import { TimeUtil } from "../util/TimeUtil";

export interface ITickDefinition {
    tick: number;
    step: number;
}

export const TICK_DEFINITIONS: ITickDefinition[] = [
    {
        tick: TimeUtil.MILLISECONDS_PER_MINUTE * 10,
        step: 10
    },
    {
        tick: TimeUtil.MILLISECONDS_PER_MINUTE * 15,
        step: 15
    },
    {
        tick: TimeUtil.MILLISECONDS_PER_MINUTE * 30,
        step: 30
    },
    {
        tick: TimeUtil.MILLISECONDS_PER_HOUR,
        step: 30
    },
    {
        tick: TimeUtil.MILLISECONDS_PER_HOUR * 3,
        step: 36
    },
    {
        tick: TimeUtil.MILLISECONDS_PER_HOUR * 6,
        step: 36
    },
    {
        tick: TimeUtil.MILLISECONDS_PER_HOUR * 12,
        step: 24
    },
    {
        tick: TimeUtil.MILLISECONDS_PER__DAY,
        step: 24
    }
];