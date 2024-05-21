/**
 * helper type that that loads json-content
 */
export class TextLoader {

    /**
     * load from the given url and return a promise resolving to an instance of LfvEvent
     * @param url
     */
    async load(url: string): Promise<any> {
        return new Promise(function (resolve, reject) {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', url);
            xhr.onload = function () {
                if (this.status >= 200 && this.status < 300) {
                    resolve(xhr.responseText);
                } else {
                    reject(new Error(this.statusText));
                }
            };
            xhr.onerror = (e) => {
                reject(e);
            };
            xhr.send();
        });
    }

}