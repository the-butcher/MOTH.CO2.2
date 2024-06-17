
/**
 * utility class for time related functionality
 */
export class TimeUtil {

    static readonly MILLISECONDS_PER_MINUTE = 1000 * 60;
    static readonly MILLISECONDS_PER_HOUR = TimeUtil.MILLISECONDS_PER_MINUTE * 60;
    static readonly MILLISECONDS_PER__DAY = TimeUtil.MILLISECONDS_PER_HOUR * 24;

    /**
     * formats the given instants to yyyyMMddHHmm_yyyyMMddHHmm
     * @param type
     * @param instantA
     * @param instantB
     * @returns
     */
    static getExportName(type: string, instantA: number, instantB: number) {
        return `mothdat_${TimeUtil.toExportDateTime(new Date(instantA))}_${TimeUtil.toExportDateTime(new Date(instantB))}.${type}`;
    }

    /**
     * formats a single value to specific precision and length, with optional padding
     * @param value
     * @param precision
     * @param length
     * @param pad
     * @returns
     */
    static formatValue(value: number, precision: number, length: number, pad: string): string {
        return value.toFixed(precision).replace('.', ',').padStart(length, pad);
    }

    static getTimezoneOffsetSeconds() {
        return new Date().getTimezoneOffset() * 60;
    }
    /**
     * format the given date to 'yyyy-MM-dd HH:mm:ss'
     * @param date
     * @returns
     */
    static toCsvDate(date: Date) {
        return `${date.getFullYear()}-${TimeUtil.formatValue(date.getMonth() + 1, 0, 2, '0')}-${TimeUtil.formatValue(date.getDate(), 0, 2, '0')} ${TimeUtil.formatValue(date.getHours(), 0, 2, '0')}:${TimeUtil.formatValue(date.getMinutes(), 0, 2, '0')}:${TimeUtil.formatValue(date.getSeconds(), 0, 2, '0')}`;
    };

    /**
     * format the given date to 'yyyyMMddHHmm'
     * @param date
     */
    static toExportDateTime(date: Date) {
        return `${date.getFullYear()}${TimeUtil.formatValue(date.getMonth() + 1, 0, 2, '0')}${TimeUtil.formatValue(date.getDate(), 0, 2, '0')}${TimeUtil.formatValue(date.getHours(), 0, 2, '0')}${TimeUtil.formatValue(date.getMinutes(), 0, 2, '0')}`;
    };

    /**
     * format the given instant to 'yyyyMMdd'
     * @param date
     */
    static toExportDate(instant: number) {
        const date = new Date(instant);
        return `${date.getFullYear()}${TimeUtil.formatValue(date.getMonth() + 1, 0, 2, '0')}${TimeUtil.formatValue(date.getDate(), 0, 2, '0')}`;
    };

    /**
     * format the given instant to 'dd.MM.yyyy HH:mm'
     * @param date
     */
    static toLocalDateTime(instant: number) {
        const date = new Date(instant);
        return `${TimeUtil.formatValue(date.getDate(), 0, 2, '0')}.${TimeUtil.formatValue(date.getMonth() + 1, 0, 2, '0')}.${date.getFullYear()} ${TimeUtil.formatValue(date.getHours(), 0, 2, '0')}:${TimeUtil.formatValue(date.getMinutes(), 0, 2, '0')}`;
    };

    /**
     * format the given instant to 'HH:mm'
     * @param date
     */
    static toLocalTime(instant: number) {
        const date = new Date(instant);
        return `${TimeUtil.formatValue(date.getHours(), 0, 2, '0')}:${TimeUtil.formatValue(date.getMinutes(), 0, 2, '0')}`;
    };


}