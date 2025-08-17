import { NativeConnection } from './native'
import { Stable, Future } from './utility'

type StorageFormat = {
    A: string[],
    D: string[]
};

export class Workspace {
    native: NativeConnection;
    // current Desktop & Activity
    desktop: Stable<string>;
    activity: Stable<string>;
    // TODO: select a better name
    readonly Storage = "pos";
    id_count = 0;

    focusList = [] as WindowData[];
    windowByID = new Map() as Map<number, WindowData>;
    windowByUUID = new Map() as Map<string, WindowData>;

    async updateMapWID(win: WindowData) {
        this.windowByID.set(await win.wid.wait(), win);
    }
    async updateMapUUID(win: WindowData) {
        this.windowByUUID.set(await win.uuid.wait(), win);
    }
    delete(win: WindowData) {
        let wid = win.wid.get();
        let uuid = win.uuid.get();
        if (wid !== undefined) win.context.windowByID.delete(wid);
        if (uuid !== undefined) win.context.windowByUUID.delete(uuid);
        const index = this.focusList.indexOf(win);
        if (index >= 0) {
            this.focusList.splice(index, 1);
        }
    }

    constructor(native: NativeConnection, [activity, desktop]: [string, string], timeout: number) {
        this.native = native;
        native.registerListener("desktopChanged", (desktop: string) => this.desktop.set(desktop));
        native.registerListener("activityChanged", (activity: string) => this.activity.set(activity));
        native.registerListener("windowDesktopsChanged", (window_uuid: string, desktops: string[]) => {
            const window = this.windowByUUID.get(window_uuid);
            if (window === undefined) return;
            window.desktops.set(new Set(desktops));
            window.saveStorage();
        });
        native.registerListener("windowActivitiesChanged", (window_uuid: string, activities: string[]) => {
            const window = this.windowByUUID.get(window_uuid);
            if (window === undefined) return;
            window.activities.set(new Set(activities));
            window.saveStorage();
        });
        this.desktop = new Stable(desktop, timeout);
        this.activity = new Stable(activity, timeout);

        browser.windows.onFocusChanged.addListener((wid) => {
            const win = this.windowByID.get(wid);
            if (win === undefined) return;
            const index = this.focusList.indexOf(win);
            if (index >= 0) this.focusList.splice(index, 1);
            this.focusList.unshift(win);
        });
        browser.windows.onRemoved.addListener((wid) => {
            const win = this.windowByID.get(wid);
            if (win !== undefined) this.delete(win);
        });
    }
}

type WindowInitializeData =
    { kind: "present"; wid: number; } |
    { kind: "future"; wid: Promise<number>; desktops: string[]; activities: string[] };

export class WindowData {
    context: Workspace;

    wid: Future<number>;
    uuid: Future<string>;
    desktops: Future<Set<string>>;
    activities: Future<Set<string>>;

    async getUUID() {
        let uniquePreface = crypto.randomUUID();
        const wid = await this.wid.wait();
        await browser.windows.update(wid, { titlePreface: uniquePreface });
        let uuid = await this.context.native.call("claimWindow", uniquePreface);
        while (uuid === "00000000-0000-0000-0000-000000000000") {
            uniquePreface = crypto.randomUUID();
            await browser.windows.update(wid, { titlePreface: uniquePreface });
            // TODO add timeout
            // Should we throw an exception after too many trials?
            uuid = await this.context.native.call("claimWindow", uniquePreface);
        }
        browser.windows.update(wid, { titlePreface: "" });
        return uuid;
    }
    saveStorage() {
        let wid = this.wid.get();
        let desktops_s = this.desktops.get();
        let activities_s = this.activities.get();
        if (wid !== undefined && desktops_s !== undefined && activities_s !== undefined) {
            let desktops = Array.from(desktops_s);
            let activities = Array.from(activities_s);
            console.log(`saving ${wid} ${activities} ${desktops}`);
            browser.sessions.setWindowValue(wid, this.context.Storage,
                { A: activities, D: desktops } as StorageFormat);
        }
    }
    async getStorage() {
        const wid = await this.wid.wait();
        const data = await browser.sessions.getWindowValue(wid, this.context.Storage);
        // TODO safety checks
        return data as { A: string[], D: string[] } | undefined;
    }
    async getActivities(p: Promise<StorageFormat | undefined>) {
        const activities = (await p)?.A;
        if (activities) return new Set(activities);
        const uuid = await this.uuid.wait();
        return new Set(await this.context.native.call("getWindowActivities", uuid));
    }
    async getDesktops(p: Promise<StorageFormat | undefined>) {
        const desktops = (await p)?.D;
        if (desktops) return new Set(desktops);
        const uuid = await this.uuid.wait();
        return new Set(await this.context.native.call("getWindowDesktops", uuid));
    }
    async move(activities: string[], desktops: string[]) {
        const uuid = await this.uuid.wait();
        return Promise.all([
            this.context.native.call("setWindowActivities", uuid, activities),
            this.context.native.call("setWindowDesktops", uuid, desktops),
        ]);
    }
    constructor(context: Workspace, data: WindowInitializeData) {
        this.context = context;
        context.focusList.unshift(this);
        this.wid = new Future(data.wid);
        this.uuid = new Future(this.getUUID());
        context.updateMapUUID(this);
        if (data.kind == "future") {
            context.updateMapWID(this);
            this.desktops = new Future(new Set(data.desktops));
            this.activities = new Future(new Set(data.activities));
        } else {
            context.windowByID.set(data.wid, this);
            const storage_p = this.getStorage();
            this.desktops = new Future(this.getDesktops(storage_p));
            this.activities = new Future(this.getActivities(storage_p));
            storage_p.then((pos) => {
                console.log(`restoring window ${data.wid}: ${pos?.A} ${pos?.D}`)
                if (pos !== undefined)
                    this.move(pos.A, pos.D);
            });
        }
        Promise.all([this.desktops.wait(), this.activities.wait()]).then(() => this.saveStorage());
    }
};

export async function newWorkspace(native: NativeConnection, timeout: number) {
    const pos = await Promise.all([
        native.call("getCurrentActivity"),
        native.call("getCurrentDesktop"),
    ]);
    return new Workspace(native, pos, timeout);
}
