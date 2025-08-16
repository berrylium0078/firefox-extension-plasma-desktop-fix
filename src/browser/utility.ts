export class Deferred<T> {
    promise: Promise<T>;
    reject!: (reason?: any) => void;
    resolve!: (value: T | PromiseLike<T>) => void;

    constructor() {
        this.promise = new Promise((resolve, reject) => {
            this.resolve = resolve;
            this.reject = reject;
        });
    }
};

export class Stable<T> {
    private value: T;
    private timer?: number;
    private timeout: number;

    constructor(value: T, timeout: number) {
        this.value = value;
        this.timeout = timeout;
    }
    get(): Readonly<T> {
        return this.value;
    }
    set(value: T) {
        if (this.timer !== undefined)
            clearTimeout(this.timer);
        this.timer = setTimeout(() => {
            this.value = value;
        }, this.timeout);
    }
};

export class Future<T> {
    private value?: T;
    private promise: Promise<T>;

    set(value: T) { this.value = value; }
    get(): T | undefined { return this.value; }
    async wait(): Promise<T> {
        if (this.value === undefined)
            this.value = await this.promise;
        return this.value;
    }
    constructor(value: T | Promise<T>) {
        if (value instanceof Promise) {
            this.promise = value.then((v) => { return this.value = v; });
        } else {
            this.value = value;
            this.promise = new Promise(() => { });
        }
    }
};
