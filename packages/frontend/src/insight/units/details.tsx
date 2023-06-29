import { detail, DetailDescriptor } from '../../entity/insight';
import { isEmpty } from 'lodash';
import { AscendMultiSliceList, ThreadMetaData } from '../../entity/data';
import { Session } from '../../entity/session';

export const slicesListDetail = detail({
    name: 'Slices List',
    columns: [
        [ 'Name', data => `${isEmpty(data.title) ? 'null' : data.title}`, 100 ],
        [ 'Wall Duration', data => `${data.wallDuration ?? 0}`, 180 ],
        [ 'Self Time', data => `${data.selfTime ?? 0}`, 100 ],
        [ 'Average Wall Duration', data => `${data.avgWallDuration ?? 0}`, 180 ],
        [ 'Occurrences', data => `${data.occurrences ?? 0}`, 'auto' ],
    ],
    actions: [
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => a.title?.localeCompare(b?.title ?? '') ?? 0 },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => a.wallDuration ?? 0 - (b.wallDuration ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => a.selfTime ?? 0 - (b.selfTime ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => a.avgWallDuration ?? 0 - (b.avgWallDuration ?? 0) },
        { sorter: (a: AscendMultiSliceList, b: AscendMultiSliceList) => a.occurrences ?? 0 - (b.occurrences ?? 0) },
    ],
    fetchData: async (session: Session, metadata: ThreadMetaData) => {
        const params = {
            rankId: metadata.cardId,
            tid: metadata.threadId,
            pid: metadata.processId,
            startTime: session.selectedRange?.[0],
            endTime: session.selectedRange?.[1],
        };
        const raw = await window.request('unit/threads', params);
        return raw.data;
    },
}) as DetailDescriptor<unknown>;
