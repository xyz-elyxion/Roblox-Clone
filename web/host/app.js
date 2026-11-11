'use strict';

// The RBLXHosts control panel. Everything it knows comes from /api/status, which
// the supervisor refreshes on its own thread; the buttons only queue a command,
// so a click never reaches into the pool directly. The token is not in this file:
// it was exchanged for an HttpOnly cookie when the URL printed in the console was
// opened, and the browser sends it on its own.

const POLL_MS = 2000;
const LOG_POLL_MS = 3000;

const elements = {
    summary: document.querySelector('[data-summary]'),
    notice: document.querySelector('[data-notice]'),
    instances: document.querySelector('[data-instances]'),
    config: document.querySelector('[data-config]'),
    logPanel: document.querySelector('[data-log-panel]'),
    logTitle: document.querySelector('[data-log-title]'),
    log: document.querySelector('[data-log]'),
};

const view = {
    status: null,
    logIndex: null,
    logOpener: null,
    logTimer: null,
    busy: false,
};

function showNotice(message) {
    elements.notice.textContent = message;
    elements.notice.hidden = !message;
}

function formatUptime(seconds) {
    if (!seconds || seconds < 0) return '-';
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const rest = seconds % 60;
    return String(hours).padStart(2, '0') + ':' + String(minutes).padStart(2, '0') + ':' +
           String(rest).padStart(2, '0');
}

function make(tag, className, text) {
    const node = document.createElement(tag);
    if (className) node.className = className;
    if (text !== undefined && text !== null) node.textContent = text;
    return node;
}

async function command(path) {
    if (view.busy) return;
    view.busy = true;
    try {
        const response = await fetch(path, { method: 'POST' });
        if (!response.ok) {
            const body = await response.json().catch(() => ({}));
            showNotice(body.error || ('The command was refused (' + response.status + ').'));
        } else {
            showNotice('');
        }
    } catch (error) {
        showNotice('Cannot reach RBLXHosts: ' + error.message);
    } finally {
        view.busy = false;
        setTimeout(loadStatus, 300);
    }
}

function instanceCard(instance) {
    const card = make('article', 'card');
    card.dataset.index = instance.index;
    // Four identical-looking cards of Start/Stop buttons are told apart by the
    // port they belong to.
    card.setAttribute('role', 'group');
    card.setAttribute('aria-label', 'Server on port ' + instance.port);

    const head = make('div', 'card-head');
    head.appendChild(make('span', 'card-port', ':' + instance.port));
    head.appendChild(make('span', 'pill ' + instance.state, instance.state));
    card.appendChild(head);

    const facts = make('dl', 'facts');
    const addFact = (term, value) => {
        facts.appendChild(make('dt', null, term));
        facts.appendChild(make('dd', null, value));
    };
    addFact('slot', instance.index);
    addFact('pid', instance.pid ? instance.pid : '-');
    addFact('uptime', formatUptime(instance.uptimeSeconds));
    addFact('restarts', instance.restarts);
    addFact('log', instance.log);
    card.appendChild(facts);

    if (instance.lastError) {
        card.appendChild(make('p', 'card-error', instance.lastError));
    }

    const actions = make('div', 'card-actions');
    const running = instance.state === 'running';
    const start = make('button', 'button', 'Start');
    start.type = 'button';
    start.dataset.action = 'start';
    start.setAttribute('aria-label', 'Start server on port ' + instance.port);
    start.disabled = running;
    start.addEventListener('click', () => command('/api/instances/' + instance.index + '/start'));

    const stop = make('button', 'button danger', 'Stop');
    stop.type = 'button';
    stop.dataset.action = 'stop';
    stop.setAttribute('aria-label', 'Stop server on port ' + instance.port);
    stop.disabled = instance.state === 'stopped';
    stop.addEventListener('click', () => command('/api/instances/' + instance.index + '/stop'));

    const restart = make('button', 'button', 'Restart');
    restart.type = 'button';
    restart.dataset.action = 'restart';
    restart.setAttribute('aria-label', 'Restart server on port ' + instance.port);
    restart.addEventListener('click',
                            () => command('/api/instances/' + instance.index + '/restart'));

    const log = make('button', 'button quiet', 'Open log');
    log.type = 'button';
    log.dataset.action = 'log';
    log.setAttribute('aria-label', 'Open the log for the server on port ' + instance.port);
    log.addEventListener('click', () => openLog(instance));

    actions.appendChild(start);
    actions.appendChild(stop);
    actions.appendChild(restart);
    actions.appendChild(log);
    card.appendChild(actions);
    return card;
}

function renderConfig(config) {
    elements.config.textContent = '';
    const rows = [
        ['server', config.server],
        ['directory', config.directory],
        ['world', config.world || '(server default)'],
        ['web', config.web || '(server default)'],
        ['log directory', config.logDirectory],
        ['first port', config.firstPort],
        ['instances', config.instances],
        ['max restarts', config.maxRestarts],
        ['restart window', config.restartWindowMs + ' ms'],
        ['restart delay', config.restartDelayMs + ' ms'],
        ['dashboard', config.dashboardHost + ':' + config.dashboardPort],
    ];
    for (const [name, value] of rows) {
        const row = make('div');
        row.appendChild(make('dt', null, name));
        row.appendChild(make('dd', null, String(value)));
        elements.config.appendChild(row);
    }
}

function render() {
    const status = view.status;
    if (!status) return;

    elements.summary.textContent = status.running + ' of ' + status.total +
                                   ' server(s) running.';

    // The poll rebuilds every card, which would drop the caret of whoever is
    // tabbing through them. Remember which control was focused and put it back
    // once the new card exists; a button that became disabled is left alone.
    const focused = document.activeElement;
    const focusedAction = focused && focused.dataset ? focused.dataset.action : null;
    const focusedCard = focused && focused.closest ? focused.closest('[data-index]') : null;
    const focusedIndex = focusedCard ? focusedCard.dataset.index : null;

    elements.instances.textContent = '';
    for (const instance of status.instances) {
        elements.instances.appendChild(instanceCard(instance));
    }
    if (focusedAction && focusedIndex !== null) {
        for (const button of elements.instances.querySelectorAll('[data-action]')) {
            const card = button.closest('[data-index]');
            if (button.dataset.action === focusedAction && card &&
                card.dataset.index === focusedIndex && !button.disabled) {
                button.focus();
                break;
            }
        }
    }
    renderConfig(status.config);

    if (view.logIndex !== null) {
        const instance = status.instances.find((entry) => entry.index === view.logIndex);
        if (instance) elements.logTitle.textContent = 'Log - port ' + instance.port;
    }
}

async function loadStatus() {
    try {
        const response = await fetch('/api/status', { headers: { Accept: 'application/json' } });
        if (response.status === 401) {
            showNotice('Not signed in to the panel. Open the URL RBLXHosts printed at startup.');
            return;
        }
        if (!response.ok) throw new Error('HTTP ' + response.status);
        view.status = await response.json();
        showNotice('');
        render();
    } catch (error) {
        showNotice('Cannot reach RBLXHosts: ' + error.message);
    }
}

async function loadLog() {
    if (view.logIndex === null) return;
    elements.log.textContent = 'loading...';
    try {
        const response = await fetch('/api/logs/' + view.logIndex + '?tail=200');
        if (!response.ok) throw new Error('HTTP ' + response.status);
        const text = await response.text();
        elements.log.textContent = text || '(the log is empty)';
        elements.log.scrollTop = elements.log.scrollHeight;
    } catch (error) {
        elements.log.textContent = 'Cannot read the log: ' + error.message;
    }
}

function openLog(instance) {
    view.logOpener = document.activeElement;
    view.logIndex = instance.index;
    elements.logTitle.textContent = 'Log - port ' + instance.port;
    elements.logPanel.hidden = false;
    loadLog();
    if (view.logTimer) clearInterval(view.logTimer);
    view.logTimer = setInterval(loadLog, LOG_POLL_MS);
    // The log is readable with the keyboard, so focus follows it into view; the
    // smooth scroll is skipped when the reader asked for less motion.
    elements.log.focus();
    const still = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    elements.logPanel.scrollIntoView({ behavior: still ? 'auto' : 'smooth', block: 'nearest' });
}

function closeLog() {
    view.logIndex = null;
    elements.logPanel.hidden = true;
    if (view.logTimer) {
        clearInterval(view.logTimer);
        view.logTimer = null;
    }
    // Closing hides the panel the focus was in; without this the next Tab would
    // start from the top of the page instead of the button that opened it.
    if (view.logOpener && document.contains(view.logOpener)) {
        view.logOpener.focus();
    }
    view.logOpener = null;
}

document.querySelector('[data-pool="start"]')
    .addEventListener('click', () => command('/api/pool/start'));
document.querySelector('[data-pool="stop"]')
    .addEventListener('click', () => command('/api/pool/stop'));
document.querySelector('[data-log-close]').addEventListener('click', closeLog);
document.querySelector('[data-log-refresh]').addEventListener('click', loadLog);

loadStatus();
setInterval(() => {
    if (!document.hidden) loadStatus();
}, POLL_MS);

document.addEventListener('visibilitychange', () => {
    if (!document.hidden) loadStatus();
});
