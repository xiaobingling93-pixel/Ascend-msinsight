export default (() => {
    const send = (args: Record<string, unknown>, reject?: Function): void => {
        const iframe = document.querySelector('iframe');
        if (!iframe?.contentWindow?.postMessage) {
            const errMsg = '[connection]: missed postMessage function, please check your iframe element';
            console.error(errMsg);
            reject && reject(new Error(errMsg));
            return;
        }
        iframe.contentWindow.postMessage(JSON.stringify(args), '*');
    };

    const _msgSequence: Map<MsgSequence, Function> = new Map();
    const listeners: Map<Handler, ListenerCb> = new Map();
    window.onmessage = (e: MessageEvent) => {
        const res = { ...e };
        res.data = JSON.parse(e.data);
        const _resolve = _msgSequence.get(res.data.id);
        if (_resolve) {
            _resolve(res.data);
            _msgSequence.delete(res.data.id);
        }
        listeners.forEach(cb => cb(res));
    };

    type MsgSequence = number;
    let id: MsgSequence = 0;
    const fetch = async (args: Record<string, unknown>) => {
        return new Promise((resolve, reject) => {
            args.id = id;
            send(args, reject);
            _msgSequence.set(id++, resolve);
        })
    };

    type Handler = number;
    type ListenerCb = (res: MessageEvent) => void;
    let _handler = 0;
    const addListener = (cb: ListenerCb): Handler => {
        listeners.set(_handler, cb);
        return _handler++;
    };

    const removeListener = (handler: Handler): void => {
        listeners.delete(handler);
    };

    return {
        fetch,
        send,
        addListener,
        removeListener,
    };
})();
