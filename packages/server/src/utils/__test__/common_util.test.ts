import { getTrackId, getDbPath } from '../common_util';

describe('common_util test', () => {
    it('getTrackId test', () => {
        expect(getTrackId(1, 'test')).toBe(458165912);
    });

    it('getDbPath test', function () {
        expect(getDbPath(['aaa.json'], 'rank0')).toBe('./aaa_rank0.db');
    });
});
