// The guided tour of the whole project, served at /welcome.
//
// Steps the website can verify check themselves against the same API the rest
// of the site uses: the public game list, the session, the avatar, playtime,
// friends and the games this account published. The steps it cannot see —
// opening Studio, editing in RBLXCode, running the trainer — are ticked off by
// hand and kept in localStorage, so a half-finished tour can be resumed.

const onboardingManualKey = 'rblxOnboardingManual';
const onboardingResults = new Map();
let onboardingSessionPromise = null;

function onboardingManualIds() {
    try {
        const stored = localStorage.getItem(onboardingManualKey);
        const parsed = stored ? JSON.parse(stored) : null;
        return new Set(Array.isArray(parsed) ? parsed.filter((id) => typeof id === 'string') : []);
    } catch {
        return new Set();
    }
}

function writeOnboardingManual(ids) {
    try {
        localStorage.setItem(onboardingManualKey, JSON.stringify([...ids]));
    } catch {
        // A browser that refuses storage still gets the tour; it just forgets
        // the hand-ticked steps at the next reload.
    }
}

// /api/verify is the same call the nav makes. It is cached for the length of one
// pass over the steps so six checks do not send six session requests.
function onboardingSession() {
    if (!getToken()) return Promise.resolve(null);
    if (!onboardingSessionPromise) {
        onboardingSessionPromise = verifySession().catch(() => null);
    }
    return onboardingSessionPromise;
}

function freshOnboardingSession() {
    onboardingSessionPromise = null;
    return onboardingSession();
}

function onboardingFailure(status) {
    return new Error(`The website answered HTTP ${status}. Is WebApp.exe still running?`);
}

// Each check resolves to one of:
//   { done: true, detail }                  the step is visibly complete
//   { done: false, detail }                 not done yet, and the detail says why
//   { done: false, blocked: true, detail }  needs an account first
// A check that throws means the website itself did not answer.
const onboardingChecks = {
    async website() {
        const started = performance.now();
        const response = await fetch('/api/games', { cache: 'no-store' });
        if (!response.ok) throw onboardingFailure(response.status);
        const data = await response.json();
        const games = Array.isArray(data.games) ? data.games : [];
        const elapsed = Math.max(1, Math.round(performance.now() - started));
        return {
            done: true,
            detail: games.length
                ? `WebApp.exe answered in ${elapsed} ms with ${games.length} public game${games.length === 1 ? '' : 's'} ready to join.`
                : `WebApp.exe answered in ${elapsed} ms. No public games yet.`
        };
    },

    async account() {
        const session = await onboardingSession();
        if (!session) {
            return {
                done: false,
                blocked: true,
                detail: 'No session in this browser yet: sign up or log in, then check again.'
            };
        }
        return {
            done: true,
            detail: `Signed in as ${session.username}${session.isAdmin ? ' (site admin)' : ''}.`
        };
    },

    async avatar() {
        const session = await onboardingSession();
        if (!session) {
            return { done: false, blocked: true, detail: 'Sign in first: the avatar belongs to the account.' };
        }

        const { response, data } = await getJson('/api/avatar', getToken());
        if (!response.ok) throw onboardingFailure(response.status);

        const avatar = data.avatar || {};
        const faceId = typeof avatar.faceId === 'string' ? avatar.faceId : '';
        const hatId = typeof avatar.hatId === 'string' ? avatar.hatId : '';
        const customFace = faceId.length > 0 && faceId !== 'classic';

        if (!customFace && !hatId) {
            return {
                done: false,
                detail: 'Still on the classic face with no hat: pick one on the avatar page and this ticks itself.'
            };
        }

        const worn = [];
        if (customFace) worn.push(`the ${faceId} face`);
        if (hatId) worn.push(`the ${hatId} hat`);
        return { done: true, detail: `Wearing ${worn.join(' and ')}.` };
    },

    async play() {
        const session = await onboardingSession();
        if (!session) {
            return { done: false, blocked: true, detail: 'Play needs an account: sign in, then press Play on a game page.' };
        }

        const { response, data } = await getJson('/api/me/social', getToken());
        if (!response.ok) throw onboardingFailure(response.status);

        const seconds = Number((data.profile && data.profile.stats && data.profile.stats.playtimeSeconds) || 0);
        if (!(seconds > 0)) {
            return { done: false, detail: 'No playtime recorded yet: open a game page and press Play.' };
        }

        const played = typeof formatPlaytime === 'function' ? formatPlaytime(seconds) : `${seconds}s`;
        return { done: true, detail: `The site has recorded ${played} of playtime on this account.` };
    },

    async friends() {
        const session = await onboardingSession();
        if (!session) {
            return { done: false, blocked: true, detail: 'Friends belong to accounts: sign in, then search for someone.' };
        }

        const { response, data } = await getJson('/api/me/social', getToken());
        if (!response.ok) throw onboardingFailure(response.status);

        const friends = Array.isArray(data.friends) ? data.friends : [];
        if (friends.length === 0) {
            return {
                done: false,
                detail: 'No friends yet: this one needs a second account, so sign up in a private window and send a request.'
            };
        }

        const online = friends.filter((friend) => friend && friend.online).length;
        return {
            done: true,
            detail: `${friends.length} friend${friends.length === 1 ? '' : 's'}${online ? `, ${online} online right now` : ''}.`
        };
    },

    async publish() {
        const session = await onboardingSession();
        if (!session) {
            return { done: false, blocked: true, detail: 'Publishing needs an account: sign in, then publish a world.' };
        }

        const { response, data } = await getJson('/api/games/mine', getToken());
        if (!response.ok) throw onboardingFailure(response.status);

        const games = (Array.isArray(data.games) ? data.games : []).filter(Boolean);
        if (games.length === 0) {
            return { done: false, detail: 'Nothing published by this account yet: use Studio or the Create page.' };
        }

        const titles = games.slice(0, 3).map((game) => game.title).filter(Boolean);
        const list = titles.length
            ? `: ${titles.join(', ')}${games.length > titles.length ? '…' : ''}`
            : '';
        return {
            done: true,
            detail: `${games.length} game${games.length === 1 ? '' : 's'} published by this account${list}.`
        };
    }
};

function onboardingSteps() {
    return [...document.querySelectorAll('[data-onboarding] [data-step]')];
}

function setOnboardingBadge(step, label, state) {
    step.dataset.state = state;
    const badge = step.querySelector('[data-step-state]');
    if (!badge) return;
    badge.textContent = label;
    badge.classList.remove('checking', 'todo', 'done', 'blocked', 'error');
    badge.classList.add(state);
}

function syncManualButton(step, isDone) {
    const button = step.querySelector('[data-manual-toggle]');
    if (!button) return;
    button.textContent = isDone ? 'Undo' : 'Mark as done';
    button.setAttribute('aria-pressed', isDone ? 'true' : 'false');
}

function renderOnboardingStep(id) {
    const step = document.querySelector(`[data-onboarding] [data-step="${id}"]`);
    if (!step) return;

    const hasCheck = step.hasAttribute('data-check');
    const check = step.querySelector('[data-step-check]');
    const manual = onboardingManualIds().has(id);

    if (manual) {
        setOnboardingBadge(step, 'Done', 'done');
        syncManualButton(step, true);
        if (check && hasCheck && check.dataset.detail) check.textContent = check.dataset.detail;
        return;
    }

    syncManualButton(step, false);

    // The steps the website cannot see are ticked by hand, so they wait at
    // "To do" rather than looking like a check that is still running.
    if (!hasCheck) {
        setOnboardingBadge(step, 'To do', 'todo');
        return;
    }

    const result = onboardingResults.get(id);
    if (!result) {
        setOnboardingBadge(step, 'Checking', 'checking');
        return;
    }

    if (result.error) {
        setOnboardingBadge(step, 'Error', 'error');
        if (check) check.textContent = result.error;
        return;
    }

    if (check) {
        check.textContent = result.detail;
        check.dataset.detail = result.detail;
    }

    if (result.done) {
        setOnboardingBadge(step, 'Done', 'done');
    } else if (result.blocked) {
        setOnboardingBadge(step, 'Sign in', 'blocked');
    } else {
        setOnboardingBadge(step, 'To do', 'todo');
    }
}

async function runOnboardingCheck(id) {
    const step = document.querySelector(`[data-onboarding] [data-step="${id}"]`);
    const check = onboardingChecks[id];
    if (!step || !check) return;

    if (!onboardingManualIds().has(id)) {
        setOnboardingBadge(step, 'Checking', 'checking');
        const line = step.querySelector('[data-step-check]');
        if (line) line.textContent = 'Contacting the website…';
    }

    try {
        onboardingResults.set(id, await check());
    } catch (error) {
        onboardingResults.set(id, { error: error && error.message ? error.message : 'The website did not answer.' });
    }

    renderOnboardingStep(id);
    updateOnboardingProgress();
}

async function runAllOnboardingChecks() {
    const steps = onboardingSteps();
    await Promise.all(steps.map((step) => {
        const id = step.dataset.step;
        return step.hasAttribute('data-check') ? runOnboardingCheck(id) : Promise.resolve(renderOnboardingStep(id));
    }));
    updateOnboardingProgress();
}

function onboardingDoneIds() {
    const done = new Set(onboardingManualIds());
    for (const [id, result] of onboardingResults) {
        if (result && result.done) done.add(id);
    }
    return done;
}

function updateOnboardingProgress() {
    const steps = onboardingSteps();
    if (steps.length === 0) return;

    const done = onboardingDoneIds();
    const finished = steps.filter((step) => done.has(step.dataset.step)).length;
    const ratio = finished / steps.length;

    const bar = document.querySelector('[data-onboarding-bar]');
    if (bar) bar.style.width = `${Math.round(ratio * 100)}%`;

    const progress = document.querySelector('[data-onboarding-progress]');
    if (progress) {
        progress.setAttribute('aria-valuenow', String(finished));
        progress.setAttribute('aria-valuemax', String(steps.length));
        progress.setAttribute('aria-valuetext', `${finished} of ${steps.length} steps done`);
    }

    const summary = document.querySelector('[data-onboarding-summary]');
    if (summary) {
        if (finished === steps.length) {
            summary.textContent = `All ${steps.length} steps done. That is the whole build.`;
        } else if (finished === 0) {
            summary.textContent = `Nothing done yet: ${steps.length} steps to go.`;
        } else {
            summary.textContent = `${finished} of ${steps.length} steps done.`;
        }
    }

    const complete = document.querySelector('[data-onboarding-complete]');
    if (complete) complete.hidden = finished < steps.length;
}

// The first step still open is where a new visitor should start, so on the first
// pass the tour opens that one and collapses the rest.
function openFirstUnfinishedStep() {
    const steps = onboardingSteps();
    const done = onboardingDoneIds();
    const target = steps.find((step) => !done.has(step.dataset.step));
    for (const step of steps) {
        const details = step.querySelector('details');
        if (details) details.open = step === target;
    }
}

function flashButtonLabel(button, message) {
    if (!button) return;
    const original = button.dataset.originalLabel || button.textContent;
    button.dataset.originalLabel = original;
    button.textContent = message;
    window.setTimeout(() => {
        button.textContent = original;
        delete button.dataset.originalLabel;
    }, 1600);
}

async function copyOnboardingCommand(button) {
    const text = button.getAttribute('data-copy') || '';
    if (!text) return;

    try {
        await navigator.clipboard.writeText(text);
        flashButtonLabel(button, 'Copied');
    } catch {
        // Copying needs a secure context in some browsers, so fall back to the
        // selection trick for a plain http:// address on the local network.
        const helper = document.createElement('textarea');
        helper.value = text;
        helper.setAttribute('readonly', '');
        helper.style.position = 'fixed';
        helper.style.opacity = '0';
        document.body.appendChild(helper);
        helper.select();
        try {
            document.execCommand('copy');
            flashButtonLabel(button, 'Copied');
        } catch {
            flashButtonLabel(button, 'Copy failed');
        } finally {
            helper.remove();
        }
    }
}

function initOnboarding() {
    const root = document.querySelector('[data-onboarding]');
    if (!root) return;

    for (const button of root.querySelectorAll('[data-recheck]')) {
        button.addEventListener('click', () => {
            freshOnboardingSession();
            runOnboardingCheck(button.getAttribute('data-recheck'));
        });
    }

    for (const button of root.querySelectorAll('[data-manual-toggle]')) {
        button.addEventListener('click', () => {
            const step = button.closest('[data-step]');
            if (!step) return;
            const id = step.dataset.step;
            const manual = onboardingManualIds();
            if (manual.has(id)) {
                manual.delete(id);
            } else {
                manual.add(id);
            }
            writeOnboardingManual(manual);
            renderOnboardingStep(id);
            updateOnboardingProgress();
        });
    }

    for (const button of root.querySelectorAll('[data-copy]')) {
        button.addEventListener('click', () => copyOnboardingCommand(button));
    }

    for (const button of root.querySelectorAll('[data-onboarding-reset]')) {
        button.addEventListener('click', () => {
            writeOnboardingManual(new Set());
            onboardingResults.clear();
            freshOnboardingSession();
            runAllOnboardingChecks();
        });
    }

    // Coming back to this tab is when a step is most likely to have become true
    // (signed in, played a game, published a world), so re-check on focus.
    document.addEventListener('visibilitychange', () => {
        if (document.hidden) return;
        freshOnboardingSession();
        runAllOnboardingChecks();
    });

    runAllOnboardingChecks().then(openFirstUnfinishedStep);
}

document.addEventListener('DOMContentLoaded', initOnboarding);
