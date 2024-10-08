import { JsonLoader } from "./JsonLoader";

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

    /**
     * load a list of available days for a given year and month
     * @param boxUrl
     * @param year
     * @param month
     * @param dateRange
     */
    static async collectDays(boxUrl: string, year: number, month: number, dateRange: [Date, Date]): Promise<void> {
        const urlYYYY_MM = `${boxUrl}/dirout?folder=${year}/${String(month).padStart(2, '0')}`;
        const folderYYYY_MM = await new JsonLoader().load(urlYYYY_MM);
        let fileName: string;
        folderYYYY_MM.files.forEach(_file => {
            fileName = _file.file;
            if (fileName.match('[0-9]{8}[.](dap|dar)')) {
                const day = parseInt(fileName.substring(6, 8));
                const date = new Date(year, month - 1, day);
                if (date.getTime() < dateRange[0].getTime()) {
                    dateRange[0] = date;
                }
                if (date.getTime() > dateRange[1].getTime()) {
                    dateRange[1] = date;
                }
            }
        });
    };

    /**
     * load a list of available months and days for a given year
     * @param boxUrl
     * @param year
     * @param dateRange
     */
    static async collectMonths(boxUrl: string, year: number, dateRange: [Date, Date]): Promise<void> {
        const urlYYYY = `${boxUrl}/dirout?folder=${year}`;
        const folderYYYY = await new JsonLoader().load(urlYYYY);
        for (let _subfolder of folderYYYY.folders) {
            if (!isNaN(_subfolder.folder)) {
                await TimeUtil.collectDays(boxUrl, year, parseInt(_subfolder.folder), dateRange);
            }
        }; // done iterating folders
    };

    /**
     * load a list of a available years, months and days
     * @param boxUrl
     * @returns
     */
    static async collectYears(boxUrl: string): Promise<[Date, Date]> {
        // find all days that have data
        const dateRange: [Date, Date] = [new Date('2100-01-01'), new Date('2000-01-01')];
        for (var year = 2024; year <= new Date().getUTCFullYear(); year++) {
            await TimeUtil.collectMonths(boxUrl, year, dateRange);
        }
        return dateRange;
    };

}