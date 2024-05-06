export interface IMqttConfig {
    srv: string;
    prt: number;
    crt?: string; // TODO :: handle file upload (must be an extra file field, "crt" just manages the path on the device)
    usr?: string;
    pwd?: string;
    cli: string;
    min: number;
}