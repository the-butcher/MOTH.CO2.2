import { LineChart } from "@mui/x-charts";
import { axisClasses } from '@mui/x-charts/ChartsAxis';
import { useEffect, useRef, useState } from "react";
import { IChartProps } from "../types/IChartProps";
import { IRecord } from "../types/IRecord";
import { TICK_DEFINITIONS } from "../types/ITickDefinition";
import { TimeUtil } from "../util/TimeUtil";

/**
 * component, renders a chart, using the mui/x-charts component
 * @param props
 * @returns
 */
const ChartValues = (props: IChartProps) => {

    const { width, height, seriesDef, records, exportTo, handleExportComplete } = props;

    const [tickDefIndex, setTickDefIndex] = useState<number>(0);
    const [tickInterval, setTickInterval] = useState<number[]>();
    const [stepRecords, setStepRecords] = useState<IRecord[]>([]);
    const [minmax, setMinMax] = useState<[number, number]>([0, 0]);
    const chartRef = useRef<SVGElement>();

    /**
     * https://gist.github.com/SunPj/14fe4f10db43be2d84751f5595d48246
     * @param stylesheet
     * @returns
     */
    const stringifyStylesheet = (stylesheet: CSSStyleSheet): string => {
        return stylesheet.cssRules ? Array.from(stylesheet.cssRules).map(rule => rule.cssText || '').join('\n') : '';
    }
    /**
     * iterates all stylesheets in the document and collects and concatenates all rules from those stylesheets
     * @returns
     */
    const collectStyles = (): string => {
        return Array.from(document.styleSheets).map(s => stringifyStylesheet(s)).join('\n');
    }
    /**
     * collects all styles in the document and creates a <def/> node from it
     * needed for exporting when all current styles need to be attached to the <svg/> clone
     * @returns
     */
    const collectDefs = (): string => {
        const styles = collectStyles()
        return `<defs><style type="text/css"><![CDATA[${styles}]]></style></defs>`
    }

    /**
     * exports this chart to a png image
     */
    const exportToPng = () => {

        const chartSvg = chartRef.current;
        const { width, height } = chartSvg.getBoundingClientRect();

        const chartSvgClone: SVGElement = chartSvg.cloneNode(true) as SVGElement;

        const defs = collectDefs()
        chartSvgClone.insertAdjacentHTML('afterbegin', defs);

        const svgContent = (new XMLSerializer()).serializeToString(chartSvgClone);
        const svgBlob = new Blob([svgContent], {
            type: 'image/svg+xml;charset=utf-8'
        });
        const svgDataUrl = URL.createObjectURL(svgBlob);

        const image = new Image();
        image.onload = () => {

            const pngPadding = 10;

            const canvas = document.createElement('canvas');
            canvas.width = width + pngPadding * 2;
            canvas.height = height + pngPadding * 2;

            const context = canvas.getContext('2d');
            context.fillStyle = 'white';
            context.fillRect(0, 0, canvas.width, canvas.height);
            context.drawImage(image, pngPadding, pngPadding, width, height);

            context.font = '14px smb';
            context.fillStyle = 'black';

            const textY = height + pngPadding - 3;
            context.fillText(TimeUtil.toCsvDate(new Date(records[0].instant)), 70, textY);
            context.fillText(TimeUtil.toCsvDate(new Date(records[records.length - 1].instant)), width + pngPadding - 166, textY);

            const pngDataUrl = canvas.toDataURL();
            const pngDownloadLink = document.createElement('a');
            pngDownloadLink.setAttribute('href', pngDataUrl);
            pngDownloadLink.setAttribute('download', TimeUtil.getExportName('png', records[0].instant, records[records.length - 1].instant)); // TODO format with dates
            pngDownloadLink.click();

            handleExportComplete();

        };
        image.onerror = (e) => {
            console.error('failed to complete export', e);
            handleExportComplete();
        };
        image.src = svgDataUrl;

    }

    /**
     * recalculates the tick interval of the chart, given the current width and date-range
     */
    const rebuildTickInterval = () => {

        if (chartRef.current && records.length > 0) {

            const chartWidth = chartRef.current.getBoundingClientRect().width - 85; // 85 measured from
            if (chartWidth > 0) {

                let minInstant = records[0].instant; // - TimeUtil.getTimezoneOffsetSeconds();
                let maxInstant = records[records.length - 1].instant; // - TimeUtil.getTimezoneOffsetSeconds();

                const difInstant = maxInstant - minInstant;
                const maxTickCount = chartWidth / 15;

                let _tickDefIndex = 0;
                for (; _tickDefIndex < TICK_DEFINITIONS.length; _tickDefIndex++) {
                    const curTickCount = difInstant / TICK_DEFINITIONS[_tickDefIndex].tick;
                    if (curTickCount < maxTickCount) {
                        break;
                    }
                }

                minInstant = getTickInstant(records[0].instant, TICK_DEFINITIONS[_tickDefIndex].tick);
                maxInstant = getTickInstant(records[records.length - 1].instant, TICK_DEFINITIONS[_tickDefIndex].tick);

                const _tickInterval: number[] = [];
                for (let instant = minInstant; instant < maxInstant; instant += TICK_DEFINITIONS[_tickDefIndex].tick) {
                    _tickInterval.push(instant);
                }
                setTickDefIndex(_tickDefIndex);
                setTickInterval(_tickInterval);

            }

        }

    }

    /**
     * calculates a tick instant snapped to the time-step specified
     * @param instant
     * @param step
     * @returns
     */
    const getTickInstant = (instant: number, step: number) => {
        const offsetInstant = instant - TimeUtil.getTimezoneOffsetSeconds() * 1000;
        const moduloInstant = offsetInstant - offsetInstant % step + step
        return moduloInstant + TimeUtil.getTimezoneOffsetSeconds() * 1000;
    }

    /**
     * rebuilds a reduced list of records, depending on the current tick definition
     */
    const rebuildStepRecords = () => {

        if (records?.length > 0) {

            const step = TICK_DEFINITIONS[tickDefIndex].tick / TICK_DEFINITIONS[tickDefIndex].step;

            let minInstant = getTickInstant(records[0].instant - step, step);
            let maxInstant = getTickInstant(records[records.length - 1].instant, step);

            const _stepRecords: IRecord[] = [];
            let record: IRecord;
            let recordIndex = 0;
            let offInstant: number;
            for (let instant = minInstant; instant < maxInstant; instant += step) {
                for (; recordIndex < records.length; recordIndex++) {
                    record = records[recordIndex];
                    offInstant = record.instant - instant; // positive value while records are older than instant
                    if (Math.abs(offInstant) <= TimeUtil.MILLISECONDS_PER_MINUTE / 2) {
                        _stepRecords.push(record);
                        // break; // will continue with the next searchable instant
                    } else if (offInstant > 0) {
                        break;
                    }

                }
            }
            setStepRecords(_stepRecords);

        }

    }

    /**
     * callback, triggered when the chartRef becomes valid
     * @param ref
     */
    const handleRefChange = (ref: SVGElement) => {

        console.debug(`⚙ updating chart component (ref)`, ref);
        if (!chartRef.current) {
            chartRef.current = ref;
            rebuildTickInterval();
        }

    };

    /**
     * recalculates the y-axis min and max values
     */
    const recalculateMinMax = () => {
        const values = records.map(r => r[seriesDef.id]);
        const _min = Math.min(...values);
        const _max = Math.max(...values);
        setMinMax([_min, _max]);
    };

    /**
     * react hook (records)
     */
    useEffect(() => {

        console.debug(`⚙ updating chart component (records)`, records);
        rebuildTickInterval();
        recalculateMinMax();

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [records]);

    /**
     * react hook (seriesDef)
     * when the series changes, min and max need to calculates too
     */
    useEffect(() => {

        console.debug(`⚙ updating chart component (seriesDef)`, seriesDef);
        recalculateMinMax();

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [seriesDef]);

    /**
     * react hook (tickInterval)
     * once the tick-interval is built, the reduced set of records can be evaluated
     */
    useEffect(() => {

        console.debug(`⚙ updating chart component (tickInterval)`, tickInterval);
        if (tickInterval) {
            rebuildStepRecords();
        }

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [tickInterval]);

    /**
     * react hook (stepRecords)
     * when the step-records are complete, the chart can be treated as rendered and, if required, the chart can be exported to png
     */
    useEffect(() => {

        if (stepRecords?.length > 0) {
            console.debug(`⚙ updating chart component (stepRecords, exportTo)`, stepRecords, exportTo);
            if (exportTo === 'png') {
                setTimeout(() => {
                    exportToPng();
                }, 250);
            }

        }

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [stepRecords]);


    /**
     * component init hook
     */
    useEffect(() => {

        console.debug('✨ building chart component', chartRef);

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, []);

    const getXAxisMin = () => {
        if (records.length > 0) {
            return records[0].instant;
        } else {
            return undefined;
        }
    }

    const getXAxisMax = () => {
        if (records.length > 0) {
            return records[records.length - 1].instant;
        } else {
            return undefined;
        }
    }

    return (
        <LineChart
            skipAnimation
            ref={handleRefChange}
            height={height}
            width={width}
            xAxis={[{
                dataKey: 'instant',
                valueFormatter: (instant, context) => {
                    if (context.location === 'tooltip') {
                        return TimeUtil.toLocalDateTime(instant);
                    } else {
                        return TimeUtil.toLocalTime(instant);
                    }

                },
                min: getXAxisMin(),
                max: getXAxisMax(),
                label: 'time (HH:MM)',
                tickInterval,
                tickLabelStyle: {
                    angle: -90,
                    translate: -4,
                    textAnchor: 'end',
                    fontSize: 12,
                },
            }]}
            yAxis={[{
                colorMap: seriesDef.colorMap,
                valueFormatter: seriesDef.valueFormatter,
                min: seriesDef.min(minmax[0]),
                max: seriesDef.max(minmax[1]),
                label: `${seriesDef.label}`
            }]}
            series={[{
                dataKey: seriesDef.id,
                label: seriesDef.label,
                showMark: false,
                type: 'line',
                curve: 'linear',
                valueFormatter: seriesDef.valueFormatter
            }]}
            dataset={stepRecords}
            grid={{ vertical: true, horizontal: true }}
            margin={{ top: 15, right: 10, bottom: 65, left: 60 }}
            sx={{
                [`& .${axisClasses.left} .${axisClasses.label}`]: {
                    transform: 'translateX(-20px)',
                },
                [`& .${axisClasses.bottom} .${axisClasses.label}`]: {
                    transform: 'translateY(25px)',
                },
            }}
            {...{
                legend: { hidden: true }
            }}
        >
        </LineChart>

    );
}

export default ChartValues;