/**
 * helper type that that loads json-content
 */
export class ByteLoader {

    /**
     * load from the given url and return a promise resolving to an instance of LfvEvent
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
                    reject({
                        status: this.status,
                        statusText: xhr.statusText
                    });
                }
            };
            xhr.onerror = function () {
                reject({
                    status: this.status,
                    statusText: xhr.statusText
                });
            };
            xhr.send();
        });
    }

}