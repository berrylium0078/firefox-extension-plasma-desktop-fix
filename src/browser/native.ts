import { Deferred } from "./utility"

type Signals = {
    windowDesktopsChanged: (window_uuid: string, desktop_ids: string[]) => void,
    windowActivitiesChanged: (window_uuid: string, activity_uuids: string[]) => void,
    activityChanged: (activity_uuid: string) => void,
    desktopChanged: (desktop_id: string) => void,
};

type Methods = {
    getCurrentActivity: () => string,
    getCurrentDesktop: () => string,
    getWindowActivities: (window_uuid: string) => string[],
    getWindowDesktops: (window_uuid: string) => string[],
    switchToActivityDesktop: (activity_uuid: string, desktop_id: string) => void,
    setWindowDesktops: (window_uuid: string, desktop_ids: string[]) => void,
    setWindowActivities: (window_uuid: string, activity_uuids: string[]) => void,
    claimWindow: (window_caption_prefix: string) => string,
};

type SignalHandler = (...params: Parameters<Signals[keyof Signals]>) => void;
type MethodParams = { [method in keyof Methods]: Parameters<Methods[method]>; };
type MethodResults = { [method in keyof Methods]: ReturnType<Methods[method]>; };

type MessageSignalEmitted =
    { [m in keyof Signals]: { signal: m, params: Parameters<Signals[m]> }; }[keyof Signals];
type MessageMethodSuccess =
    { [m in keyof Methods]: { result: MethodResults[m], id: number }; }[keyof Methods];
type MessageMethodFailure =
    { error: any, id: number };
type MessageMethodReturned = MessageMethodSuccess | MessageMethodFailure;

/** Establish a connection to the native messaging host */
export class NativeConnection {
    private port: browser.runtime.Port;
    private transmissionId = 1;
    private handlers = new Map() as Map<number, (msg: MessageMethodReturned) => void>;
    private eventListeners = new Map() as Map<keyof Signals, SignalHandler[]>;

    constructor(nativeName: string) {
        this.port = browser.runtime.connectNative(nativeName);
        this.port.onMessage.addListener((msg: any) => this.callback(msg));
    }

    private callback(msg: MessageSignalEmitted | MessageMethodReturned) {
        // TODO safety checks?
        console.log("received", msg);
        if ("debug" in msg) {
            return;
        }
        if ("signal" in msg) {
            const listeners = this.eventListeners.get(msg.signal);
            return listeners?.forEach((f) => f(...msg.params));
        }
        let id = msg.id;
        let func = this.handlers.get(id);
        if (func !== undefined) {
            func(msg);
            this.handlers.delete(id);
        }
    }

    public registerListener<S extends keyof Signals>(signal: S, listener: Signals[S]) {
        let listeners = this.eventListeners.get(signal);
        if (listeners)
            listeners.push(listener as any);
        else
            this.eventListeners.set(signal, [listener as any]);
    }

    public call<M extends keyof Methods>(method: M, ...params: MethodParams[M]): Promise<MethodResults[M]> {
        let id = this.transmissionId++;
        let message = { method: method, params: params, id: id };
        console.log("sent", message);

        this.port.postMessage(message);

        const deferred = new Deferred<MethodResults[M]>();
        this.handlers.set(id, (msg: MessageMethodReturned) => {
            if ("error" in msg) {
                deferred.reject(msg.error);
            } else if (msg.id === id) {
                // TODO safety checks?
                deferred.resolve(msg.result as MethodResults[M]);
            }
        });
        return deferred.promise;
    }
}
