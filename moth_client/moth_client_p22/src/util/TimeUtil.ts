import { JsonLoader } from "./JsonLoader";

export class TimeUtil {

    static readonly MILLISECONDS_PER_MINUTE = 1000 * 60;
    static readonly MILLISECONDS_PER_HOUR = TimeUtil.MILLISECONDS_PER_MINUTE * 60;
    static readonly MILLISECONDS_PER__DAY = TimeUtil.MILLISECONDS_PER_HOUR * 24;

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
     * format the given date to 'yyyyMMdd'
     * TODO :: find a better name and remove the 'UTC' from signature
     * @param date
     */
    static toUTCDate(date: Date) {
        return `${date.getFullYear()}${TimeUtil.formatValue(date.getMonth() + 1, 0, 2, '0')}${TimeUtil.formatValue(date.getDate(), 0, 2, '0')}`;
    }

    static toExportDateTime(date: Date) {
        return `${date.getFullYear()}${TimeUtil.formatValue(date.getMonth() + 1, 0, 2, '0')}${TimeUtil.formatValue(date.getDate(), 0, 2, '0')}${TimeUtil.formatValue(date.getHours(), 0, 2, '0')}${TimeUtil.formatValue(date.getMinutes(), 0, 2, '0')}`;
    };

    static toLocalDate(instant: number) {
        return new Date(instant).toLocaleDateString(window.navigator.language, { // you can use undefined as first argument
            year: "numeric",
            month: "2-digit",
            day: "2-digit",
        });
    };

    static toLocalTime(instant: number) {
        return new Date(instant).toLocaleTimeString(window.navigator.language, { // you can use undefined as first argument
            hour: "2-digit",
            minute: "2-digit"
        });
    };

    static async collectDays(boxUrl: string, year: number, month: number, dateRange: [Date, Date]): Promise<void> {
        const urlYYYY_MM = `${boxUrl}/dirout?folder=${year}/${String(month).padStart(2, '0')}`;
        const folderYYYY_MM = await new JsonLoader().load(urlYYYY_MM);
        folderYYYY_MM.files.forEach(_file => {
            const day = _file.file.substring(6, 8);
            const date = new Date(year, month - 1, day);
            if (date.getTime() < dateRange[0].getTime()) {
                dateRange[0] = date;
            }
            if (date.getTime() > dateRange[1].getTime()) {
                dateRange[1] = date;
            }
        });
    };

    static async collectMonths(boxUrl: string, year: number, dateRange: [Date, Date]): Promise<void> {
        const urlYYYY = `${boxUrl}/dirout?folder=${year}`;
        const folderYYYY = await new JsonLoader().load(urlYYYY);
        for (let _subfolder of folderYYYY.folders) {
            if (!isNaN(_subfolder.folder)) {
                await TimeUtil.collectDays(boxUrl, year, parseInt(_subfolder.folder), dateRange);
            }
        }; // done iterating folders
    };

    static async collectYears(boxUrl: string): Promise<[Date, Date]> {
        // find all days that have data
        const dateRange: [Date, Date] = [new Date('2100-01-01'), new Date('2000-01-01')];
        for (var year = 2024; year <= new Date().getUTCFullYear(); year++) {
            await TimeUtil.collectMonths(boxUrl, year, dateRange);
        }
        return dateRange;
    };

}