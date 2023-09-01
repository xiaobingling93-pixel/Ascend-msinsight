import eventBus from '../eventBus';
describe('eventBus test', () => {
    type dataType = {
        rankGroups: Map<string, number[]>;
        currentCommunicator: string;
    };
    let testParam: dataType;
    beforeAll(() => {
        const groups = new Map<string, number[]>();
        groups.set('stage1', [ 0, 1, 2 ]);
        testParam = {
            rankGroups: groups,
            currentCommunicator: 'stage1',
        };
    });

    it('add event test group', () => {
        let result;
        eventBus.on('selectCommunicator', (params) => {
            result = params;
        });
        eventBus.emit('selectCommunicator', testParam);
        expect(result).toBe(testParam);
    });

    it('remove event test group', () => {
        let result;
        const func = (arg: any): void => {
            result = arg;
        };
        eventBus.on('selectCommunicator', func);
        eventBus.off('selectCommunicator', func);
        eventBus.emit('selectCommunicator', testParam);
        expect(result).toBe(undefined);
    });
});
