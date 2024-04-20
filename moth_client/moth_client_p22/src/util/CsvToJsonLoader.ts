/**
 * helper type that that loads json-content
 */
export class CsvToJsonLoader {

    /**
     * load from the given url and return a promise resolving to an instance of LfvEvent
     * @param url
     */
    async load(url: string): Promise<any> {

        // console.debug('☁ loading', url);
        return new Promise(function (resolve, reject) {
            const xhr = new XMLHttpRequest();
            xhr.open('GET', url);
            xhr.onload = function () {
                if (this.status >= 200 && this.status < 300) {
                    const reponseText = xhr.responseText;
                    const responseLines = reponseText.toString().split(/\r(\n)?/).filter(l => l !== undefined && l !== '\n');
                    const responseHeaders = responseLines[0].split(";");
                    const responseData = [];
                    let responseRecord: {};
                    // console.log(responseLines[0]);
                    for (var line = 1; line < responseLines.length - 1; line += 1) {
                        const responseValues = responseLines[line].split(";");
                        responseRecord = {};
                        for (var index = 0; index < responseHeaders.length; index++) {
                            if (responseHeaders[index] === 'time') {
                                responseRecord['instant'] = new Date(responseValues[index]?.trim()).getTime();
                            } else {
                                responseRecord[responseHeaders[index].trim()] = parseFloat(responseValues[index]?.trim().replace(',', '.'));
                            }
                        }
                        responseData.push(responseRecord);
                    }
                    resolve(responseData);
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