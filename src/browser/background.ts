import { NativeConnection } from './native'
import { newWorkspace, WindowData } from './windows'

let native = new NativeConnection("io.github.berrylium0078.plasma_desktop_fix");

async function main() {
    var workspace = await newWorkspace(native, 10);

    browser.windows.getAll().then((windows) => windows.forEach((window) => {
        const wid = window.id!;
        new WindowData(workspace, { kind: "present", wid: wid });
    }))

    browser.tabs.onCreated.addListener(async (tab) => {
        const currentDesktop = workspace.desktop.get();
        const currentActivity = workspace.activity.get();
        const tid = tab.id;
        const wid = tab.windowId;
        if (tid === undefined || wid === undefined) return;
        //console.log(`new tab ${tid} in ${wid}`);

        const window = workspace.windowByID.get(wid) || new WindowData(workspace, { kind: "present", wid: wid });

        if (await window.is_restored.wait()) return;
        // TODO Can we assume that new windows are always on the current desktop & activity?
        await window.desktops.wait();
        await window.activities.wait();

        const target = workspace.focusList.find((winData) => {
            let desktops = winData.desktops.get();
            let activities = winData.activities.get();
            return (desktops?.size == 0 || desktops?.has(currentDesktop)) &&
                (activities?.size == 0 || activities?.has(currentActivity));
        });

        if (target === undefined) {
            //console.log(`creating window`);
            const window = new WindowData(workspace, {
                kind: "future",
                wid: browser.windows.create({ tabId: tid }).then((window) => window.id!),
                activities: [currentActivity],
                desktops: [currentDesktop],
            });
            await window.move([currentActivity], [currentDesktop]);
            native.call("switchToActivityDesktop", currentActivity, currentDesktop);
        } else {
            const widnew = await target.wid.wait();
            //console.log(`moving to ${widnew}`)
            browser.tabs.move(tid, {
                windowId: widnew,
                index: -1,
            });
            browser.windows.update(widnew, { focused: true });
        }
        // TODO Should we add timeout before switching?
    })
}

main();
