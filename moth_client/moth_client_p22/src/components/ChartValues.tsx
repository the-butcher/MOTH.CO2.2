import { LineChart } from "@mui/x-charts";
import { axisClasses } from '@mui/x-charts/ChartsAxis';
import { useEffect, useRef, useState } from "react";
import { IChartProps } from "../types/IChartProps";
import { TICK_DEFINITIONS } from "../types/ITickDefinition";
import { TimeUtil } from "../util/TimeUtil";

const ChartValues = (props: IChartProps) => {

    const { width, height, seriesDef, records, exportTo, handleExportComplete } = props;

    const [tickInterval, setTickInterval] = useState<number[]>();
    const chartRef = useRef<SVGElement>();

    /**
     * https://gist.github.com/SunPj/14fe4f10db43be2d84751f5595d48246
     * @param stylesheet
     * @returns
     */
    const stringifyStylesheet = (stylesheet: CSSStyleSheet): string => {
        return stylesheet.cssRules ? Array.from(stylesheet.cssRules).map(rule => rule.cssText || '').join('\n') : '';
    }
    const collectStyles = (): string => {
        return Array.from(document.styleSheets).map(s => stringifyStylesheet(s)).join('\n');
    }
    const collectDefs = (): string => {
        const styles = collectStyles()
        // console.log('styles', styles);
        return `<defs><style type="text/css"><![CDATA[${styles}]]></style></defs>`
    }

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
            console.error('e', e);
            handleExportComplete();
        };
        image.src = svgDataUrl;

    }

    const rebuildTickInterval = () => {

        if (chartRef.current && records.length > 0) {

            const chartWidth = chartRef.current.getBoundingClientRect().width - 85; // 85 measured from
            if (chartWidth > 0) {

                let minInstant = records[0].instant; // - TimeUtil.getTimezoneOffsetSeconds();
                let maxInstant = records[records.length - 1].instant; // - TimeUtil.getTimezoneOffsetSeconds();

                const difInstant = maxInstant - minInstant;
                const maxTickCount = chartWidth / 15;
                // console.log('chartWidth', chartWidth, 'maxTickCount', maxTickCount, 'dpr', window.devicePixelRatio);

                let tickDefinitionIndex = 0;
                for (; tickDefinitionIndex < TICK_DEFINITIONS.length; tickDefinitionIndex++) {
                    const curTickCount = difInstant / TICK_DEFINITIONS[tickDefinitionIndex].step;
                    if (curTickCount < maxTickCount) {
                        // console.log('using definition', TICK_DEFINITIONS[tickDefinitionIndex].step / (1000 * 60 * 60));
                        break;
                    }
                }

                minInstant = getTickInstant(records[0].instant, TICK_DEFINITIONS[tickDefinitionIndex].step);
                maxInstant = getTickInstant(records[records.length - 1].instant, TICK_DEFINITIONS[tickDefinitionIndex].step);

                const _tickInterval: number[] = [];
                for (let instant = minInstant; instant < maxInstant; instant += TICK_DEFINITIONS[tickDefinitionIndex].step) {
                    _tickInterval.push(instant);
                }
                setTickInterval(_tickInterval);

            }

        }

    }

    const handleRefChange = (ref: SVGElement) => {

        console.debug(`⚙ updating chart component (ref)`, ref);
        if (!chartRef.current) {
            chartRef.current = ref;
            rebuildTickInterval();
        }

    };

    useEffect(() => {

        console.debug(`⚙ updating chart component (records)`, records);
        rebuildTickInterval();

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [records]);

    useEffect(() => {

        if (tickInterval && exportTo !== '') {
            console.debug(`⚙ updating chart component (tickInterval, exportTo)`, tickInterval, exportTo);
            if (exportTo === 'png') {
                setTimeout(() => {
                    exportToPng();
                }, 250);

            }
        }

        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [tickInterval]);

    const getTickInstant = (instant: number, step: number) => {
        const offsetInstant = instant - TimeUtil.getTimezoneOffsetSeconds() * 1000;
        const moduloInstant = offsetInstant - offsetInstant % step + step
        return moduloInstant + TimeUtil.getTimezoneOffsetSeconds() * 1000;
    }

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
                    return TimeUtil.toLocalTime(instant)
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
                min: seriesDef.min,
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
            dataset={records}
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