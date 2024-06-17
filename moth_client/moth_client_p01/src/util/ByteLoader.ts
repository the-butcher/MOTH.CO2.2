import { IRecord } from "../types/IRecord";
import { TimeUtil } from "./TimeUtil";

/**
 * helper type that that loads json-content
 */
export class ByteLoader {

    static readonly secondsFrom1970To2000 = 946684800;
    static readonly recordLength = 28;

    async loadDat(url: string): Promise<IRecord[]> {
        return new Promise((resolve, reject) => {
            this.load(url).then(data => {
                let secondstime: number;
                const timezoneOffsetSeconds = TimeUtil.getTimezoneOffsetSeconds();
                const records: IRecord[] = [];
                for (let i = 0; i < data.byteLength; i += ByteLoader.recordLength) {
                    secondstime = data.getUint32(i, true);
                    records.push({
                        instant: new Date((ByteLoader.secondsFrom1970To2000 + secondstime + timezoneOffsetSeconds) * 1000).getTime(),
                        pm010: data.getUint16(i + 4, true),
                        pm025: data.getUint16(i + 6, true),
                        pm100: data.getUint16(i + 8, true),
                        deg: data.getFloat32(i + 12, true), // appears to have 2 bytes padding after pm values
                        hum: data.getFloat32(i + 16, true),
                        hpa: data.getFloat32(i + 20, true)
                    });
                }
                resolve(records);
            }).catch((e: Error) => {
                reject(e);
            });
        });
    }

    async loadAll(urls: string[]): Promise<IRecord[]> {
        const records: IRecord[] = [];
        let _records: IRecord[];
        for (let url of urls) {
            _records = await this.loadDat(url);
            if (records.length > 0) {
                _records = _records.filter(r => r.instant > records[records.length - 1].instant);
            }
            records.push(..._records);
        }
        return records;
    }

    /**
     * load from the given url and return a promise
     * @param url
     */
    async load(url: string): Promise<DataView> {
        return new Promise(function (resolve, reject) {
            var xhr = new XMLHttpRequest();
            xhr.responseType = 'arraybuffer';
            xhr.open('GET', url);
            xhr.onload = function () {
                if (this.status >= 200 && this.status < 300) {
                    resolve(new DataView(xhr.response));
                } else {
                    reject(new Error(this.statusText));
                }
            };
            xhr.onerror = e => {
                reject(e);
            };
            xhr.send();
        });
    }

}