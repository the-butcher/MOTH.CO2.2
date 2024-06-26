import { IConfig } from "../types/IConfig";

/**
 * helper type that that loads json-content
 */
export class InstLoader {

    /**
     * load from the given url and return a promise
     * @param url
     */
    async load<T extends IConfig>(inst: T, path: string, type: string, url: string, trns: (config: T) => string): Promise<any> {

        delete inst['status']; // remove the status flag (not needed in uploaded file)

        const instTrns = trns(inst);
        const instData = new TextEncoder().encode(instTrns);
        const instBlob = new Blob([instData], {
            type
        });
        var instFile = new File([instBlob], "file_name", { lastModified: Date.now() });

        return new Promise(function (resolve, reject) {

            var xhr = new XMLHttpRequest();
            var frm = new FormData();
            xhr.open('POST', url, true);

            xhr.onload = function () {
                if (this.status >= 200 && this.status < 300) {
                    resolve(JSON.parse(xhr.responseText));
                } else {
                    reject(new Error(this.statusText));
                }
            };
            xhr.onerror = (e) => {
                reject(e);
            };
            frm.append("file", path);
            frm.append("content", instFile);
            xhr.send(frm);

        });
    }

}