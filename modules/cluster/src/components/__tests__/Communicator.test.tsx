import * as Container from '../communicatorContainer/ContainerUtils';

it('Communicator test', () => {
    const data = Container.generateCommunicatorData({ ppSize: 2, dpSize: 8, tpSize: 1 }, 8, 16);
    expect(data.defaultPPSize).toBe(8);
    expect(data.partitionModes.length).toBe(3);
    const allStageIds = Container.getAllPpStageIds(data);
    expect(allStageIds).toStrictEqual([ '(0, 1, 2, 3, 4, 5, 6, 7)', '(8, 9, 10, 11, 12, 13, 14, 15)' ]);
    const stageOptions = Container.getPpContainerData(data, 'pp');
    expect(stageOptions).toStrictEqual([ { label: 'All', value: 'All' },
        { label: '(0, 1, 2, 3, 4, 5, 6, 7)', value: '(0, 1, 2, 3, 4, 5, 6, 7)' },
        { label: '(8, 9, 10, 11, 12, 13, 14, 15)', value: '(8, 9, 10, 11, 12, 13, 14, 15)' } ]);
});
