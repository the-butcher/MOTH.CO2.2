export class Thresholds {

    private readonly riskLo: number;
    private readonly warnLo: number;
    private readonly warnHi: number;
    private readonly riskHi: number;

    constructor(riskLo: number, warnLo: number, warnHi: number, riskHi: number) {
        this.riskLo = riskLo;
        this.warnLo = warnLo;
        this.warnHi = warnHi;
        this.riskHi = riskHi;
    }

    isRisk(value: number): boolean {
        return value < this.riskLo || value > this.riskHi;
    }

    isWarn(value: number): boolean {
        return value < this.warnLo || value > this.warnHi;
    }


    getFillColor(value: number): string {
        if (this.isRisk(value)) {
            return 'black';
        } else if (this.isWarn(value)) {
            return 'lightgray';
        } else {
            return 'white';
        }
    }

    getTextColor(value: number): string {
        if (this.isRisk(value)) {
            return 'white';
        } else {
            return 'black';
        }
    }


}