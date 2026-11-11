const tokenKey = 'authToken';
const faceCatalog = [
    { id: 'classic', label: 'Classic', src: '/faces/classic.png' },
    { id: 'happy', label: 'Happy', src: '/faces/happy.png' },
    { id: 'surprised', label: 'Surprised', src: '/faces/surprised.png' },
    { id: 'smirk', label: 'Smirk', src: '/faces/smirk.png' },
    { id: 'wink', label: 'Wink', src: '/faces/wink.png' }
];
const defaultFaceId = 'classic';

function faceSrc(faceId) {
    return faceCatalog.find((face) => face.id === faceId)?.src || faceCatalog[0].src;
}

function normalizeAvatar(avatar) {
    const faceId = faceCatalog.some((face) => face.id === avatar?.faceId) ? avatar.faceId : defaultFaceId;
    const hatId = typeof avatar?.hatId === 'string' ? avatar.hatId : '';
    return { ...defaultAvatar, ...(avatar || {}), faceId, hatId };
}

function getToken() {
    return localStorage.getItem(tokenKey);
}

function setStatus(element, message, type) {
    if (!element) return;
    // The role is set before the text so the change is what a live region reports:
    // an error interrupts, a confirmation waits for a pause in speech.
    element.setAttribute('role', type === 'error' ? 'alert' : 'status');
    element.setAttribute('aria-live', type === 'error' ? 'assertive' : 'polite');
    element.textContent = message || '';
    element.className = `status ${type || ''}`.trim();
}

async function postJson(url, body, token) {
    const headers = { 'Content-Type': 'application/json' };
    if (token) headers.Authorization = `Bearer ${token}`;

    const response = await fetch(url, {
        method: 'POST',
        headers,
        body: JSON.stringify(body || {})
    });
    const data = await response.json();
    return { response, data };
}

async function getJson(url, token) {
    const headers = {};
    if (token) headers.Authorization = `Bearer ${token}`;

    const response = await fetch(url, { headers });
    const data = await response.json();
    return { response, data };
}

function saveSession(data) {
    localStorage.setItem(tokenKey, data.token);
    localStorage.setItem('username', data.username);
    localStorage.setItem('userId', data.userId);
}

function clearSession() {
    localStorage.removeItem(tokenKey);
    localStorage.removeItem('username');
    localStorage.removeItem('userId');
}

async function verifySession() {
    const token = getToken();
    if (!token) return null;

    const { response, data } = await postJson('/api/verify', {}, token);
    if (!response.ok || !data.success) {
        clearSession();
        return null;
    }
    localStorage.setItem('username', data.username);
    localStorage.setItem('userId', data.userId);
    return data;
}

async function requireSession() {
    const session = await verifySession();
    if (!session) {
        const next = `${window.location.pathname}${window.location.search}${window.location.hash}`;
        window.location.href = `/login?next=${encodeURIComponent(next)}`;
        return null;
    }
    return session;
}

function colorToCss(color) {
    return `rgb(${color.map((value) => Math.round(value * 255)).join(' ')})`;
}

function formatPlaytime(seconds) {
    const total = Math.max(0, Number(seconds || 0));
    if (total < 60) return total > 0 ? `${total}s` : '0m';

    const minutes = Math.floor(total / 60);
    if (minutes < 60) return `${minutes}m`;

    const hours = Math.floor(minutes / 60);
    const remainingMinutes = minutes % 60;
    return remainingMinutes ? `${hours}h ${remainingMinutes}m` : `${hours}h`;
}

function formatJoined(value) {
    const date = value ? new Date(value) : null;
    if (!date || Number.isNaN(date.getTime())) return 'Joined recently';
    return `Joined ${date.toLocaleDateString(undefined, { month: 'short', day: 'numeric', year: 'numeric' })}`;
}

function formatDate(value) {
    const date = value ? new Date(value) : null;
    if (!date || Number.isNaN(date.getTime())) return 'Unknown';
    return date.toLocaleDateString(undefined, { month: 'short', day: 'numeric', year: 'numeric' });
}

function formatDateTime(value) {
    const date = value ? new Date(value) : null;
    if (!date || Number.isNaN(date.getTime())) return 'an unknown date';
    return date.toLocaleString(undefined, {
        month: 'short',
        day: 'numeric',
        hour: '2-digit',
        minute: '2-digit'
    });
}

function profileHref(user) {
    return `/profile/${user.id}`;
}

function createMiniAvatar(avatar) {
    const safeAvatar = normalizeAvatar(avatar);
    const figure = document.createElement('div');
    figure.className = 'mini-avatar';
    figure.setAttribute('aria-hidden', 'true');

    const parts = [
        ['mini-head', safeAvatar.headColor],
        ['mini-torso', safeAvatar.torsoColor],
        ['mini-left-arm', safeAvatar.leftArmColor],
        ['mini-right-arm', safeAvatar.rightArmColor],
        ['mini-left-leg', safeAvatar.leftLegColor],
        ['mini-right-leg', safeAvatar.rightLegColor]
    ];

    for (const [className, color] of parts) {
        const part = document.createElement('span');
        part.className = className;
        part.style.background = colorToCss(color);
        if (className === 'mini-head') {
            const face = document.createElement('img');
            face.src = faceSrc(safeAvatar.faceId);
            face.alt = '';
            face.decoding = 'async';
            part.appendChild(face);
        }
        figure.appendChild(part);
    }

    return figure;
}

function emptyMessage(message) {
    const paragraph = document.createElement('p');
    paragraph.className = 'empty-state';
    paragraph.textContent = message;
    return paragraph;
}

function relationshipCopy(relationship) {
    if (relationship === 'friends') return 'Friends';
    if (relationship === 'outgoing') return 'Request sent';
    if (relationship === 'incoming') return 'Request received';
    if (relationship === 'self') return 'Your profile';
    return 'Not friends';
}

function makeButton(label, className, onClick) {
    const button = document.createElement('button');
    button.type = 'button';
    button.className = className || 'button';
    button.textContent = label;
    button.addEventListener('click', (event) => {
        Promise.resolve(onClick(event)).catch((error) => {
            window.alert(error.message || 'That action could not be completed.');
        });
    });
    return button;
}

async function runFriendAction(buttons, action) {
    buttons.forEach((button) => {
        button.disabled = true;
        button.dataset.originalText = button.textContent;
        button.textContent = 'Working';
    });

    try {
        await action();
    } finally {
        buttons.forEach((button) => {
            button.disabled = false;
            button.textContent = button.dataset.originalText || button.textContent;
            delete button.dataset.originalText;
        });
    }
}

function createBlockButton(user, onChanged) {
    const blocked = Boolean(user.blocked);
    return makeButton(blocked ? 'Unblock' : 'Block', 'button secondary small', async () => {
        const { response, data } = await postJson('/api/blocks', {
            userId: user.id,
            blocked: !blocked
        }, getToken());
        if (!response.ok || !data.success) throw new Error(data.error || 'Could not update the block.');
        // Unfriending happens on the server with the block, so the relationship
        // shown here drops back to none either way.
        onChanged?.({ ...user, blocked: Boolean(data.blocked), relationship: 'none' });
    });
}

function createReportButton(user) {
    return makeButton('Report', 'button secondary small', async () => {
        const reason = window.prompt(`Why are you reporting ${user.username}?`);
        if (!reason) return;
        const { response, data } = await postJson('/api/reports', {
            targetType: 'user',
            targetId: user.id,
            reason
        }, getToken());
        if (!response.ok || !data.success) throw new Error(data.error || 'Could not send the report.');
        window.alert('Report sent to the site admin.');
    });
}

function createRelationshipActions(user, onChanged) {
    const token = getToken();
    const actions = document.createElement('div');
    actions.className = 'user-actions';

    if (user.relationship === 'self') {
        const link = document.createElement('a');
        link.className = 'button secondary small';
        link.href = '/avatar';
        link.textContent = 'Edit avatar';
        actions.appendChild(link);
        return actions;
    }

    if (user.relationship === 'none') {
        const add = makeButton('Add friend', 'button small', async () => {
            await runFriendAction([add], async () => {
                const { response, data } = await postJson('/api/friends/request', { userId: user.id }, token);
                if (!response.ok) throw new Error(data.error || 'Friend request failed.');
                onChanged?.(data.profile || user);
            });
        });
        if (!user.blocked) {
            actions.appendChild(add);
        }
        actions.appendChild(createBlockButton(user, onChanged));
        actions.appendChild(createReportButton(user));
        return actions;
    }

    if (user.relationship === 'incoming') {
        const accept = makeButton('Accept', 'button small', async () => {
            await runFriendAction([accept, decline], async () => {
                const { response, data } = await postJson('/api/friends/respond', {
                    userId: user.id,
                    action: 'accept'
                }, token);
                if (!response.ok) throw new Error(data.error || 'Could not accept request.');
                onChanged?.(data.profile || user);
            });
        });
        const decline = makeButton('Decline', 'button secondary small', async () => {
            await runFriendAction([accept, decline], async () => {
                const { response, data } = await postJson('/api/friends/respond', {
                    userId: user.id,
                    action: 'decline'
                }, token);
                if (!response.ok) throw new Error(data.error || 'Could not decline request.');
                onChanged?.(data.profile || { ...user, relationship: 'none' });
            });
        });
        actions.append(accept, decline);
        actions.appendChild(createBlockButton(user, onChanged));
        actions.appendChild(createReportButton(user));
        return actions;
    }

    const removeLabel = user.relationship === 'friends' ? 'Unfriend' : 'Cancel request';
    const removeClass = user.relationship === 'friends' ? 'button danger small' : 'button secondary small';
    const remove = makeButton(removeLabel, removeClass, async () => {
        await runFriendAction([remove], async () => {
            const { response, data } = await postJson('/api/friends/remove', { userId: user.id }, token);
            if (!response.ok) throw new Error(data.error || 'Could not update friend.');
            onChanged?.(data.profile || user);
        });
    });
    actions.appendChild(remove);
    actions.appendChild(createBlockButton(user, onChanged));
    actions.appendChild(createReportButton(user));
    return actions;
}

function createUserCard(user, onChanged) {
    const card = document.createElement('article');
    card.className = 'user-card';

    const avatarLink = document.createElement('a');
    avatarLink.className = 'user-avatar-link';
    avatarLink.href = profileHref(user);
    avatarLink.setAttribute('aria-label', `${user.username} profile`);
    avatarLink.appendChild(createMiniAvatar(normalizeAvatar(user.avatar)));

    const content = document.createElement('div');
    content.className = 'user-card-content';

    const name = document.createElement('a');
    name.className = 'user-name';
    name.href = profileHref(user);
    name.textContent = user.username;
    if (user.online) {
        const dot = document.createElement('span');
        dot.className = 'presence-dot';
        dot.title = 'Online now';
        dot.setAttribute('aria-label', 'Online now');
        name.append(' ', dot);
    }

    const meta = document.createElement('p');
    meta.className = 'user-meta';
    meta.textContent = `ID ${user.id} - ${user.friendCount} friend${user.friendCount === 1 ? '' : 's'} - ${formatPlaytime(user.stats?.playtimeSeconds)} played`;

    const relation = document.createElement('p');
    relation.className = 'relationship-label';
    relation.textContent = relationshipCopy(user.relationship);

    content.append(name, meta, relation, createRelationshipActions(user, onChanged));
    card.append(avatarLink, content);
    return card;
}

function renderUserList(container, users, emptyText, onChanged) {
    if (!container) return;
    container.innerHTML = '';
    if (!users || users.length === 0) {
        container.appendChild(emptyMessage(emptyText));
        return;
    }

    for (const user of users) {
        container.appendChild(createUserCard(user, onChanged));
    }
}

// Pages that are usable while signed out show Login/Sign up; signed-in visitors
// get the same Logout button the dashboard uses instead.
function initNavAuth() {
    const guestLinks = document.querySelectorAll('[data-nav-guest]');
    const userButtons = document.querySelectorAll('[data-nav-user]');
    if (guestLinks.length === 0 && userButtons.length === 0) return;

    const signedIn = Boolean(getToken());
    guestLinks.forEach((link) => { link.hidden = signedIn; });
    userButtons.forEach((button) => { button.hidden = !signedIn; });

    // The Admin entry is part of the same bar on every page, so the dashboard
    // is no longer the only page that reveals it.
    const adminLinks = document.querySelectorAll('[data-admin-only]');
    if (signedIn && adminLinks.length > 0) {
        verifySession()
            .then((session) => {
                if (!session) return;
                adminLinks.forEach((link) => { link.hidden = !session.isAdmin; });
            })
            .catch(() => {});
    }
}

// The page being read is marked in the menu, so the link is announced as the
// current one before it is followed. Links that carry a hash point at a section
// of a page rather than the page itself, and are left out of the comparison.
function initNavCurrent() {
    const here = (window.location.pathname.replace(/\/+$/, '') || '/');
    for (const link of document.querySelectorAll('.nav a[href^="/"]')) {
        if (link.hash) continue;
        const target = new URL(link.href, window.location.href).pathname.replace(/\/+$/, '') || '/';
        if (target === here) link.setAttribute('aria-current', 'page');
    }
}

// The site keeps a restricted account signed in: it has to be able to read its
// own record and appeal it. The restriction is therefore a notice on every page
// rather than a login that refuses, drawn from the same session answer the nav
// already fetches.
function standingLabel(session) {
    if (session.banned) return 'Banned';
    if (session.suspendedUntil) return 'Suspended';
    return 'Warned';
}

function standingSentence(session) {
    if (session.banned) {
        return ' - this account is banned and cannot join games. Its record and an appeal are in Settings.';
    }
    if (session.suspendedUntil) {
        return ` - suspended until ${formatDateTime(session.suspendedUntil)}. Its record and an appeal are in Settings.`;
    }
    const strikes = Number(session.strikes || 0);
    return ` - ${strikes} strike${strikes === 1 ? '' : 's'} recorded. The next confirmed detection suspends this account.`;
}

function renderStandingBanner(session) {
    if (!session || (!session.restricted && !Number(session.strikes || 0))) return;
    if (document.querySelector('[data-standing-banner]')) return;
    const topbar = document.querySelector('.page > .topbar');
    if (!topbar) return;

    const banner = document.createElement('div');
    banner.className = `standing-banner ${session.standing || 'warned'}`;
    banner.dataset.standingBanner = '';
    banner.setAttribute('role', 'status');

    const text = document.createElement('p');
    const label = document.createElement('strong');
    label.textContent = standingLabel(session);
    const detail = document.createElement('span');
    detail.textContent = standingSentence(session);
    text.append(label, detail);

    const link = document.createElement('a');
    link.className = 'button small';
    link.href = '/settings#standing';
    link.textContent = session.openAppeal ? 'Appeal sent' : 'Account standing';

    banner.append(text, link);
    topbar.insertAdjacentElement('afterend', banner);
}

function initStandingNotice() {
    if (!getToken()) return;
    verifySession()
        .then((session) => renderStandingBanner(session))
        .catch(() => {});
}

function initLogoutButtons() {
    document.querySelectorAll('[data-logout]').forEach((button) => {
        button.addEventListener('click', () => {
            clearSession();
            window.location.href = '/login';
        });
    });
}

// The account endpoints take either captcha: a browser solves the image one,
// while scripts and the desktop client solve the machine proof-of-work.
async function loadCaptcha(form) {
    const image = form.querySelector('[data-captcha-image]');
    const idField = form.querySelector('[data-captcha-id]');
    const answer = form.querySelector('input[name="captchaAnswer"]');
    if (!image || !idField) return;

    idField.value = '';
    if (answer) answer.value = '';
    image.removeAttribute('src');
    image.alt = 'Loading captcha';

    try {
        const response = await fetch('/api/captcha?kind=human');
        const data = await response.json();
        if (!response.ok || !data.image) throw new Error(data.error || 'Captcha unavailable.');
        image.src = data.image;
        image.alt = 'Captcha image: type the four digits shown';
        idField.value = data.id;
    } catch (error) {
        image.alt = 'Captcha could not be loaded. Use New image to retry.';
    }
}

// A challenge is spent whether the answer was right or not, so every failed
// attempt comes back with a fresh image.
function initCaptcha(form) {
    const refresh = form.querySelector('[data-captcha-refresh]');
    if (refresh) {
        refresh.addEventListener('click', (event) => {
            event.preventDefault();
            loadCaptcha(form);
        });
    }
    loadCaptcha(form);
    return () => loadCaptcha(form);
}

function initLogin() {
    const form = document.querySelector('[data-login-form]');
    if (!form) return;

    const status = document.querySelector('[data-status]');
    const submit = form.querySelector('button[type="submit"]');
    const params = new URLSearchParams(window.location.search);
    const playerRedirect = safePlayerRedirect(params.get('redirect'));
    const refreshCaptcha = initCaptcha(form);

    if (params.get('client') === 'player') {
        const lede = document.querySelector('[data-login-lede]');
        if (lede) lede.textContent = 'Sign in here, then this page will return your session to the open player.';
    }

    form.addEventListener('submit', async (event) => {
        event.preventDefault();
        setStatus(status, '', '');
        submit.disabled = true;
        submit.textContent = 'Signing in';

        try {
            const body = Object.fromEntries(new FormData(form).entries());
            const { response, data } = await postJson('/api/login', body);
            if (!response.ok) throw new Error(data.error || 'Login failed.');

            saveSession(data);
            if (playerRedirect) {
                playerRedirect.searchParams.set('token', data.token);
                playerRedirect.searchParams.set('username', data.username);
                playerRedirect.searchParams.set('userId', data.userId);
                setStatus(status, 'Signed in. Returning to player.', 'success');
                window.setTimeout(() => { window.location.href = playerRedirect.toString(); }, 350);
            } else {
                setStatus(status, 'Signed in. Opening dashboard.', 'success');
                const next = params.get('next');
                const destination = next && next.startsWith('/') && !next.startsWith('//') ? next : '/dashboard';
                window.setTimeout(() => { window.location.href = destination; }, 350);
            }
        } catch (error) {
            refreshCaptcha();
            setStatus(status, error.message || 'Network error.', 'error');
            submit.disabled = false;
            submit.textContent = 'Sign in';
        }
    });
}

function safePlayerRedirect(value) {
    if (!value) return null;

    try {
        const url = new URL(value);
        const port = Number.parseInt(url.port, 10);
        const isLoopback = url.protocol === 'http:' &&
            (url.hostname === '127.0.0.1' || url.hostname === 'localhost') &&
            url.pathname === '/auth';
        const isPlayerPort = Number.isInteger(port) && port >= 39170 && port < 39190;
        return isLoopback && isPlayerPort ? url : null;
    } catch {
        return null;
    }
}

function initSignup() {
    const form = document.querySelector('[data-signup-form]');
    if (!form) return;

    const status = document.querySelector('[data-status]');
    const submit = form.querySelector('button[type="submit"]');
    const refreshCaptcha = initCaptcha(form);

    form.addEventListener('submit', async (event) => {
        event.preventDefault();
        setStatus(status, '', '');

        const body = Object.fromEntries(new FormData(form).entries());
        if (body.password !== body.confirmPassword) {
            setStatus(status, 'Passwords do not match.', 'error');
            return;
        }
        delete body.confirmPassword;

        submit.disabled = true;
        submit.textContent = 'Creating account';

        try {
            const { response, data } = await postJson('/api/signup', body);
            if (!response.ok) throw new Error(data.error || 'Signup failed.');

            saveSession(data);

            const recoveryPanel = document.querySelector('[data-recovery-panel]');
            const recoveryCode = document.querySelector('[data-signup-recovery-code]');
            if (recoveryPanel && data.recoveryCode) {
                // The code is shown once and only ever stored as a hash, so the
                // form gives way to it instead of navigating away immediately.
                if (recoveryCode) recoveryCode.textContent = data.recoveryCode;
                form.hidden = true;
                recoveryPanel.hidden = false;
                setStatus(status, 'Account created. Save your recovery code.', 'success');
                return;
            }

            setStatus(status, 'Account created. Opening dashboard.', 'success');
            window.setTimeout(() => { window.location.href = '/dashboard'; }, 350);
        } catch (error) {
            refreshCaptcha();
            setStatus(status, error.message || 'Network error.', 'error');
            submit.disabled = false;
            submit.textContent = 'Create account';
        }
    });
}

function initDashboard() {
    const dashboard = document.querySelector('[data-dashboard]');
    if (!dashboard) return;

    const welcomeHeading = document.querySelector('[data-welcome-heading]');
    const profileSummary = document.querySelector('[data-profile-summary]');
    const friendsList = document.querySelector('[data-friends-list]');
    const incomingList = document.querySelector('[data-incoming-requests]');
    const outgoingList = document.querySelector('[data-outgoing-requests]');
    const status = document.querySelector('[data-dashboard-status]');
    const tabButtons = [...document.querySelectorAll('[data-dashboard-tab]')];
    const tabPanels = [...document.querySelectorAll('[data-dashboard-panel]')];
    const friendsCount = document.querySelector('[data-friends-count]');
    const requestsCount = document.querySelector('[data-requests-count]');
    const searchForm = document.querySelector('[data-dashboard-search-form]');
    const searchInput = document.querySelector('[data-dashboard-search-input]');
    const searchResults = document.querySelector('[data-dashboard-search-results]');
    const searchStatus = document.querySelector('[data-dashboard-search-status]');
    let socialReload = null;
    let lastSearchQuery = '';

    function setTab(tabName, updateHash = true) {
        const nextTab = tabPanels.some((panel) => panel.dataset.dashboardPanel === tabName) ? tabName : 'friends';

        for (const button of tabButtons) {
            const active = button.dataset.dashboardTab === nextTab;
            button.classList.toggle('active', active);
            button.setAttribute('aria-selected', active ? 'true' : 'false');
            button.setAttribute('tabindex', active ? '0' : '-1');
        }

        for (const panel of tabPanels) {
            const active = panel.dataset.dashboardPanel === nextTab;
            panel.classList.toggle('active', active);
            panel.hidden = !active;
        }

        if (updateHash) {
            const nextUrl = nextTab === 'friends'
                ? `${window.location.pathname}${window.location.search}`
                : `${window.location.pathname}${window.location.search}#${nextTab}`;
            window.history.replaceState(null, '', nextUrl);
        }

        if (nextTab === 'search' && searchResults && !searchResults.dataset.loaded) {
            loadDashboardSearch('').catch((error) => {
                setStatus(searchStatus, error.message || 'Search failed.', 'error');
            });
        }
    }

    // The tab strip is one tab stop: arrows, Home and End move between the tabs and
    // select them, which is the keyboard half of the pattern the markup declares.
    if (tabButtons.length > 0) {
        tabButtons[0].parentElement.addEventListener('keydown', (event) => {
            const activeIndex = tabButtons.findIndex(
                (button) => button.getAttribute('aria-selected') === 'true');
            let next = -1;
            if (event.key === 'Home') {
                next = 0;
            } else if (event.key === 'End') {
                next = tabButtons.length - 1;
            } else if (event.key === 'ArrowRight' || event.key === 'ArrowDown') {
                next = (activeIndex + 1) % tabButtons.length;
            } else if (event.key === 'ArrowLeft' || event.key === 'ArrowUp') {
                next = (activeIndex - 1 + tabButtons.length) % tabButtons.length;
            }
            if (next < 0) return;
            event.preventDefault();
            setTab(tabButtons[next].dataset.dashboardTab);
            tabButtons[next].focus();
        });
    }

    async function loadDashboardSearch(query) {
        if (!searchResults) return;
        lastSearchQuery = query || '';
        searchResults.dataset.loaded = 'true';
        setStatus(searchStatus, '', '');
        searchResults.innerHTML = '';
        searchResults.appendChild(emptyMessage('Searching people'));

        const { response, data } = await getJson(`/api/users/search?q=${encodeURIComponent(lastSearchQuery)}`, getToken());
        if (!response.ok || !data.success) throw new Error(data.error || 'Search failed.');

        renderUserList(
            searchResults,
            data.users,
            lastSearchQuery ? `No people found for "${lastSearchQuery}".` : 'No other people have signed up yet.',
            async () => {
                if (socialReload) await socialReload();
                await loadDashboardSearch(lastSearchQuery);
            }
        );
    }

    tabButtons.forEach((button) => {
        button.addEventListener('click', () => {
            setTab(button.dataset.dashboardTab);
        });
    });

    window.addEventListener('hashchange', () => {
        setTab(window.location.hash.replace('#', ''), false);
    });

    searchForm?.addEventListener('submit', (event) => {
        event.preventDefault();
        setTab('search');
        loadDashboardSearch(searchInput.value.trim()).catch((error) => {
            setStatus(searchStatus, error.message || 'Search failed.', 'error');
        });
    });

    verifySession()
        .then(async (session) => {
            if (!session) {
                window.location.href = '/login';
                return;
            }
            if (welcomeHeading) welcomeHeading.textContent = `Welcome, ${session.username}`;

            for (const link of document.querySelectorAll('[data-admin-only]')) {
                link.hidden = !session.isAdmin;
            }

            async function loadSocial() {
                setStatus(status, '', '');
                const { response, data } = await getJson('/api/me/social', getToken());
                if (!response.ok || !data.success) throw new Error(data.error || 'Could not load friends.');

                if (friendsCount) friendsCount.textContent = String(data.friends.length);
                if (requestsCount) requestsCount.textContent = String(data.incomingRequests.length + data.outgoingRequests.length);

                if (profileSummary) {
                    profileSummary.innerHTML = '';
                    const profile = document.createElement('div');
                    profile.className = 'profile-summary';
                    profile.appendChild(createMiniAvatar(normalizeAvatar(data.profile.avatar)));

                    const text = document.createElement('div');
                    const name = document.createElement('h2');
                    name.textContent = data.profile.username;
                    const meta = document.createElement('p');
                    meta.textContent = `${data.profile.friendCount} friend${data.profile.friendCount === 1 ? '' : 's'} - ${formatPlaytime(data.profile.stats.playtimeSeconds)} played`;
                    const link = document.createElement('a');
                    link.className = 'button small';
                    link.href = profileHref(data.profile);
                    link.textContent = 'View profile';
                    text.append(name, meta, link);
                    profile.appendChild(text);
                    profileSummary.appendChild(profile);
                }

                renderUserList(friendsList, data.friends, 'No friends yet. Open Search and add someone.', loadSocial);
                renderUserList(incomingList, data.incomingRequests, 'No friend requests right now.', loadSocial);
                renderUserList(outgoingList, data.outgoingRequests, 'No sent requests.', loadSocial);
            }

            socialReload = loadSocial;
            await loadSocial();
            setTab(window.location.hash.replace('#', ''), false);
        })
        .catch((error) => {
            setStatus(status, error.message || 'Could not load dashboard.', 'error');
        });
}

function initSearch() {
    const root = document.querySelector('[data-search-page]');
    if (!root) return;

    const form = document.querySelector('[data-search-form]');
    const input = document.querySelector('[data-search-input]');
    const results = document.querySelector('[data-search-results]');
    const status = document.querySelector('[data-search-status]');

    async function loadResults(query) {
        setStatus(status, '', '');
        results.innerHTML = '';
        results.appendChild(emptyMessage('Searching people'));

        const url = `/api/users/search?q=${encodeURIComponent(query || '')}`;
        const { response, data } = await getJson(url, getToken());
        if (!response.ok || !data.success) throw new Error(data.error || 'Search failed.');

        renderUserList(
            results,
            data.users,
            query ? `No people found for "${query}".` : 'No other people have signed up yet.',
            () => loadResults(input.value.trim())
        );
    }

    requireSession()
        .then((session) => {
            if (!session) return;
            const params = new URLSearchParams(window.location.search);
            const initialQuery = params.get('q') || '';
            input.value = initialQuery;

            form.addEventListener('submit', (event) => {
                event.preventDefault();
                const query = input.value.trim();
                const nextUrl = query ? `/search?q=${encodeURIComponent(query)}` : '/search';
                window.history.replaceState(null, '', nextUrl);
                loadResults(query).catch((error) => {
                    setStatus(status, error.message || 'Search failed.', 'error');
                });
            });

            return loadResults(initialQuery);
        })
        .catch((error) => {
            setStatus(status, error.message || 'Search failed.', 'error');
        });
}

function initProfilePage() {
    const root = document.querySelector('[data-profile-page]');
    if (!root) return;

    const title = document.querySelector('[data-profile-title]');
    const label = document.querySelector('[data-profile-label]');
    const stats = document.querySelector('[data-profile-stats]');
    const actions = document.querySelector('[data-profile-actions]');
    const avatar = document.querySelector('[data-profile-avatar]');
    const friends = document.querySelector('[data-profile-friends]');
    const status = document.querySelector('[data-profile-status]');
    let previewPromise = null;

    function selectedProfileId() {
        const match = window.location.pathname.match(/^\/profile\/(\d+)$/);
        return match ? match[1] : localStorage.getItem('userId');
    }

    async function renderProfile(profileData) {
        const { profile, friends: profileFriends } = profileData;
        document.title = `${profile.username} - RBLX Clone`;
        if (title) title.textContent = profile.username;
        if (label) label.textContent = profile.relationship === 'self' ? 'Your profile' : `User ID ${profile.id}`;

        if (stats) {
            stats.innerHTML = '';
            const rows = [
                ['Friends', String(profile.friendCount)],
                ['Playtime', formatPlaytime(profile.stats.playtimeSeconds)],
                ['Coins', String(profile.stats.coins ?? 0)],
                ['Last played', profile.stats.lastPlayedAt ? new Date(profile.stats.lastPlayedAt).toLocaleString() : 'Not yet'],
                ['Joined', formatJoined(profile.createdAt).replace('Joined ', '')]
            ];

            for (const [name, value] of rows) {
                const row = document.createElement('div');
                row.className = 'stat-tile';
                const statName = document.createElement('span');
                statName.textContent = name;
                const statValue = document.createElement('strong');
                statValue.textContent = value;
                row.append(statName, statValue);
                stats.appendChild(row);
            }
        }

        if (actions) {
            actions.innerHTML = '';
            actions.appendChild(createRelationshipActions(profile, loadProfile));
        }

        if (avatar) {
            if (!previewPromise) {
                previewPromise = createAvatarPreview(avatar, () => {});
            }
            const preview = await previewPromise;
            preview.update(normalizeAvatar(profile.avatar), '');
        }

        renderUserList(
            friends,
            profileFriends,
            profile.relationship === 'self' ? 'You have not added friends yet.' : `${profile.username} has no friends showing yet.`,
            loadProfile
        );
    }

    async function loadProfile() {
        setStatus(status, '', '');
        const profileId = selectedProfileId();
        if (!profileId) {
            window.location.href = '/dashboard';
            return;
        }

        const { response, data } = await getJson(`/api/users/${profileId}`, getToken());
        if (response.status === 404) {
            setStatus(status, 'User not found.', 'error');
            return;
        }
        if (!response.ok || !data.success) throw new Error(data.error || 'Profile could not load.');

        if (window.location.pathname === '/profile') {
            window.history.replaceState(null, '', profileHref(data.profile));
        }
        await renderProfile(data);
    }

    requireSession()
        .then((session) => {
            if (!session) return null;
            return loadProfile();
        })
        .catch((error) => {
            setStatus(status, error.message || 'Profile could not load.', 'error');
        });
}

const avatarParts = [
    ['head', 'Head', 'headColor'],
    ['torso', 'Torso', 'torsoColor'],
    ['leftArm', 'Left arm', 'leftArmColor'],
    ['rightArm', 'Right arm', 'rightArmColor'],
    ['leftLeg', 'Left leg', 'leftLegColor'],
    ['rightLeg', 'Right leg', 'rightLegColor']
];

const defaultAvatar = {
    headColor: [0.8, 0.6, 0.4],
    torsoColor: [0.2, 0.4, 0.8],
    leftArmColor: [0.8, 0.6, 0.4],
    rightArmColor: [0.8, 0.6, 0.4],
    leftLegColor: [0.2, 0.6, 0.2],
    rightLegColor: [0.2, 0.6, 0.2],
    faceId: defaultFaceId,
    hatId: ''
};

// Filled from GET /api/avatar. The shop and the preview read the same catalog,
// so a hat has one size and colour rather than a copy per renderer.
let avatarItemCatalog = [];

function avatarItem(id) {
    return avatarItemCatalog.find((item) => item.id === id) || null;
}

const avatarBlocks = [
    { id: 'head', label: 'Head', key: 'headColor', size: [1.2, 1.2, 1.0], position: [0, 4.8, 0] },
    { id: 'torso', label: 'Torso', key: 'torsoColor', size: [2.0, 2.4, 1.0], position: [0, 3.0, 0] },
    { id: 'leftArm', label: 'Left arm', key: 'leftArmColor', size: [0.85, 2.4, 1.0], position: [-1.425, 3.0, 0] },
    { id: 'rightArm', label: 'Right arm', key: 'rightArmColor', size: [0.85, 2.4, 1.0], position: [1.425, 3.0, 0] },
    { id: 'leftLeg', label: 'Left leg', key: 'leftLegColor', size: [1.0, 1.8, 1.0], position: [-0.5, 0.9, 0] },
    { id: 'rightLeg', label: 'Right leg', key: 'rightLegColor', size: [1.0, 1.8, 1.0], position: [0.5, 0.9, 0] }
];

function threeColor(color) {
    return color.map((value) => Math.round(value * 255) / 255);
}

function colorToHex(color) {
    return `#${color.map((value) => Math.round(value * 255).toString(16).padStart(2, '0')).join('')}`;
}

function hexToColor(hex) {
    const value = hex.replace('#', '');
    return [
        parseInt(value.slice(0, 2), 16) / 255,
        parseInt(value.slice(2, 4), 16) / 255,
        parseInt(value.slice(4, 6), 16) / 255
    ];
}

async function createAvatarPreview(container, onSelect) {
    const THREE = await import('/vendor/three/three.module.js');
    const scene = new THREE.Scene();
    const camera = new THREE.OrthographicCamera();
    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: false, preserveDrawingBuffer: true });
    const root = new THREE.Group();
    const raycaster = new THREE.Raycaster();
    const pointer = new THREE.Vector2();
    const meshes = new Map();
    const loader = new THREE.TextureLoader();
    const faceTextures = new Map();
    await Promise.all(faceCatalog.map(async (face) => {
        const texture = await loader.loadAsync(face.src).catch(() => null);
        if (texture) {
            texture.colorSpace = THREE.SRGBColorSpace;
            faceTextures.set(face.id, texture);
        }
    }));
    let faceMesh = null;
    let hatMesh = null;

    renderer.setClearColor(0xffffff, 1);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
    renderer.shadowMap.enabled = false;
    renderer.domElement.className = 'avatar-canvas';
    // The figure around the canvas carries the description; the canvas itself is
    // the drawing of it, and repeating it would make a screen reader say it twice.
    renderer.domElement.setAttribute('aria-hidden', 'true');
    renderer.domElement.dataset.faceTexture = faceTextures.size > 0 ? 'loaded' : 'missing';

    scene.background = new THREE.Color(0xffffff);
    scene.add(root);
    root.rotation.set(-0.08, -0.35, 0);

    for (const block of avatarBlocks) {
        const geometry = new THREE.BoxGeometry(...block.size);
        const material = new THREE.MeshBasicMaterial({ color: 0xffffff });
        const mesh = new THREE.Mesh(geometry, material);
        const edge = new THREE.LineSegments(
            new THREE.EdgesGeometry(geometry),
            new THREE.LineBasicMaterial({ color: 0x050505 })
        );

        mesh.position.set(...block.position);
        mesh.userData.partId = block.id;
        mesh.userData.partKey = block.key;
        mesh.userData.edge = edge;
        mesh.add(edge);

        if (block.id === 'head' && faceTextures.size > 0) {
            faceMesh = new THREE.Mesh(
                new THREE.PlaneGeometry(block.size[0] * 0.82, block.size[1] * 0.82),
                new THREE.MeshBasicMaterial({
                    map: faceTextures.get(defaultFaceId),
                    transparent: true,
                    alphaTest: 0.08,
                    depthWrite: false
                })
            );
            faceMesh.position.set(0, 0, block.size[2] * 0.5 + 0.012);
            mesh.add(faceMesh);
        }

        root.add(mesh);
        meshes.set(block.id, mesh);
    }

    container.replaceChildren(renderer.domElement);

    function updateCanvasProbe() {
        const gl = renderer.getContext();
        const width = renderer.domElement.width;
        const height = renderer.domElement.height;
        const readWidth = Math.min(140, width);
        const readHeight = Math.min(140, height);
        const x = Math.floor((width - readWidth) / 2);
        const y = Math.floor((height - readHeight) / 2);
        const pixels = new Uint8Array(readWidth * readHeight * 4);
        let nonWhite = 0;
        let colored = 0;
        let hash = 0;

        gl.readPixels(x, y, readWidth, readHeight, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
        for (let index = 0; index < pixels.length; index += 4) {
            const red = pixels[index];
            const green = pixels[index + 1];
            const blue = pixels[index + 2];
            if (red < 245 || green < 245 || blue < 245) nonWhite += 1;
            if (Math.max(red, green, blue) - Math.min(red, green, blue) > 25) colored += 1;
            hash = (hash + ((index + 1) * (red + 3 * green + 7 * blue + 11 * pixels[index + 3]))) % 1000000007;
        }

        renderer.domElement.dataset.nonWhitePixels = String(nonWhite);
        renderer.domElement.dataset.coloredPixels = String(colored);
        renderer.domElement.dataset.renderHash = String(hash);
    }

    function render() {
        renderer.render(scene, camera);
        renderer.domElement.dataset.rotationX = root.rotation.x.toFixed(4);
        renderer.domElement.dataset.rotationY = root.rotation.y.toFixed(4);
        updateCanvasProbe();
    }

    function resize() {
        const width = Math.max(240, container.clientWidth);
        const height = Math.max(300, container.clientHeight);
        const aspect = width / height;
        const viewHeight = 6.4;

        camera.left = -viewHeight * aspect * 0.5;
        camera.right = viewHeight * aspect * 0.5;
        camera.top = viewHeight * 0.5;
        camera.bottom = -viewHeight * 0.5;
        camera.near = -20;
        camera.far = 20;
        camera.position.set(0, 2.65, 8);
        camera.lookAt(0, 2.65, 0);
        camera.updateProjectionMatrix();

        renderer.setSize(width, height, false);
        render();
    }

    function update(avatar, selected) {
        const safeAvatar = normalizeAvatar(avatar);
        for (const block of avatarBlocks) {
            const mesh = meshes.get(block.id);
            mesh.material.color.setRGB(...threeColor(safeAvatar[block.key]));
        }
        if (faceMesh) {
            faceMesh.material.map = faceTextures.get(safeAvatar.faceId) || faceTextures.get(defaultFaceId);
            faceMesh.material.needsUpdate = true;
            renderer.domElement.dataset.faceId = safeAvatar.faceId;
        }
        const hat = avatarItem(safeAvatar.hatId);
        if (hat) {
            if (!hatMesh) {
                const geometry = new THREE.BoxGeometry(1, 1, 1);
                hatMesh = new THREE.Mesh(geometry, new THREE.MeshBasicMaterial({ color: 0xffffff }));
                hatMesh.add(new THREE.LineSegments(
                    new THREE.EdgesGeometry(geometry),
                    new THREE.LineBasicMaterial({ color: 0x050505 })
                ));
                root.add(hatMesh);
            }
            hatMesh.visible = true;
            hatMesh.scale.set(...hat.size);
            hatMesh.position.set(0, 4.8 + hat.offsetY, hat.offsetZ || 0);
            hatMesh.material.color.setRGB(...threeColor(hat.color));
            renderer.domElement.dataset.hatId = hat.id;
        } else if (hatMesh) {
            hatMesh.visible = false;
            renderer.domElement.dataset.hatId = '';
        }
        render();
    }

    const resizeObserver = new ResizeObserver(resize);
    resizeObserver.observe(container);
    resize();

    let dragging = false;
    let moved = false;
    let startX = 0;
    let startY = 0;
    let startRotationX = 0;
    let startRotationY = 0;
    let pointerDragActive = false;

    function setPointer(event) {
        const rect = renderer.domElement.getBoundingClientRect();
        pointer.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
        pointer.y = -(((event.clientY - rect.top) / rect.height) * 2 - 1);
    }

    function hitTest(event) {
        setPointer(event);
        raycaster.setFromCamera(pointer, camera);
        const hits = raycaster.intersectObjects([...meshes.values()], false);
        return hits.length > 0 ? hits[0].object.userData.partId : '';
    }

    function beginDrag(clientX, clientY) {
        dragging = true;
        moved = false;
        startX = clientX;
        startY = clientY;
        startRotationX = root.rotation.x;
        startRotationY = root.rotation.y;
    }

    function moveDrag(clientX, clientY) {
        if (!dragging) return;

        const deltaX = clientX - startX;
        const deltaY = clientY - startY;
        moved = moved || Math.abs(deltaX) > 3 || Math.abs(deltaY) > 3;
        root.rotation.y = startRotationY + deltaX * 0.01;
        root.rotation.x = Math.max(-0.65, Math.min(0.65, startRotationX + deltaY * 0.008));
        render();
    }

    function endDrag(event) {
        dragging = false;
        if (moved) return;

        const partId = hitTest(event);
        if (partId) onSelect(partId);
    }

    renderer.domElement.addEventListener('pointerdown', (event) => {
        pointerDragActive = true;
        beginDrag(event.clientX, event.clientY);
        renderer.domElement.setPointerCapture(event.pointerId);
    });

    renderer.domElement.addEventListener('pointermove', (event) => {
        moveDrag(event.clientX, event.clientY);
    });

    renderer.domElement.addEventListener('pointerup', (event) => {
        if (renderer.domElement.hasPointerCapture(event.pointerId)) {
            renderer.domElement.releasePointerCapture(event.pointerId);
        }
        endDrag(event);
        pointerDragActive = false;
    });

    renderer.domElement.addEventListener('pointercancel', () => {
        dragging = false;
        pointerDragActive = false;
    });

    renderer.domElement.addEventListener('mousedown', (event) => {
        if (pointerDragActive) return;
        event.preventDefault();
        beginDrag(event.clientX, event.clientY);
    });

    window.addEventListener('mousemove', (event) => {
        if (pointerDragActive || !dragging) return;
        moveDrag(event.clientX, event.clientY);
    });

    window.addEventListener('mouseup', (event) => {
        if (pointerDragActive || !dragging) return;
        endDrag(event);
    });

    return { update };
}

function initAvatar() {
    const root = document.querySelector('[data-avatar]');
    if (!root) return;

    const token = getToken();
    if (!token) {
        window.location.href = '/login';
        return;
    }

    let avatar = normalizeAvatar();
    let selected = 'head';
    const figure = document.querySelector('[data-avatar-figure]');
    const tabs = document.querySelector('[data-part-tabs]');
    const colorPicker = document.querySelector('[data-color-picker]');
    const colorCode = document.querySelector('[data-color-code]');
    const faceOptions = document.querySelector('[data-face-options]');
    const hatOptions = document.querySelector('[data-hat-options]');
    const coinsOutput = document.querySelector('[data-coins]');
    const status = document.querySelector('[data-status]');
    let coins = 0;
    let preview = null;

    function selectedKey() {
        return avatarParts.find(([id]) => id === selected)[2];
    }

    function syncColorPicker() {
        const hex = colorToHex(avatar[selectedKey()]);
        colorPicker.value = hex;
        colorCode.value = hex.toUpperCase();
    }

    function syncPreview() {
        if (preview) preview.update(avatar, selected);
    }

    function drawFaceOptions() {
        if (!faceOptions) return;
        faceOptions.innerHTML = '';
        for (const face of faceCatalog) {
            const button = document.createElement('button');
            button.type = 'button';
            button.className = `face-option${avatar.faceId === face.id ? ' active' : ''}`;
            button.setAttribute('aria-pressed', avatar.faceId === face.id ? 'true' : 'false');
            button.addEventListener('click', () => selectFace(face.id));

            const image = document.createElement('img');
            image.src = face.src;
            image.alt = '';
            image.decoding = 'async';

            const label = document.createElement('span');
            label.textContent = face.label;

            button.append(image, label);
            faceOptions.appendChild(button);
        }
    }

    function drawHats() {
        if (!hatOptions) return;
        hatOptions.innerHTML = '';
        for (const item of avatarItemCatalog) {
            const button = document.createElement('button');
            button.type = 'button';
            button.className = `face-option hat-option${item.equipped ? ' active' : ''}`;
            button.setAttribute('aria-pressed', item.equipped ? 'true' : 'false');
            const swatch = document.createElement('span');
            swatch.className = 'hat-swatch';
            swatch.style.background = colorToHex(item.color);
            const label = document.createElement('span');
            label.textContent = item.owned
                ? `${item.name} - ${item.equipped ? 'wearing' : 'wear'}`
                : `${item.name} - ${item.price} coins`;
            button.append(swatch, label);
            button.addEventListener('click', () => {
                (item.owned ? equipHat(item.id) : buyHat(item)).catch((error) => {
                    setStatus(status, error.message || 'Could not update your hats.', 'error');
                });
            });
            hatOptions.appendChild(button);
        }
        if (avatar.hatId) {
            const remove = document.createElement('button');
            remove.type = 'button';
            remove.className = 'face-option hat-option';
            remove.textContent = 'Take hat off';
            remove.addEventListener('click', () => {
                equipHat('').catch((error) => {
                    setStatus(status, error.message || 'Could not remove that hat.', 'error');
                });
            });
            hatOptions.appendChild(remove);
        }
        if (coinsOutput) coinsOutput.value = `${coins} coins`;
    }

    function applyHats(items, nextCoins) {
        if (Array.isArray(items)) avatarItemCatalog = items;
        if (typeof nextCoins === 'number') coins = nextCoins;
        drawHats();
    }

    async function buyHat(item) {
        const { response, data } = await postJson('/api/avatar/buy', { itemId: item.id }, token);
        if (!response.ok || !data.success) throw new Error(data.error || 'Purchase failed.');
        applyHats(data.items, data.coins);
        setStatus(status, `${item.name} bought. Press it again to wear it.`, 'success');
    }

    async function equipHat(itemId) {
        const { response, data } = await postJson('/api/avatar/equip', { itemId }, token);
        if (!response.ok || !data.success) throw new Error(data.error || 'Could not wear that.');
        avatar.hatId = data.avatar?.hatId || '';
        applyHats(data.items, data.coins);
        syncPreview();
        setStatus(status, avatar.hatId ? 'Hat updated.' : 'Hat removed.', 'success');
    }

    function drawTabs() {
        tabs.innerHTML = '';
        for (const [id, label] of avatarParts) {
            const button = document.createElement('button');
            button.type = 'button';
            button.className = `part-tab${id === selected ? ' active' : ''}`;
            button.textContent = label;
            button.addEventListener('click', () => selectPart(id));
            tabs.appendChild(button);
        }
    }

    function selectPart(id) {
        selected = id;
        drawTabs();
        syncColorPicker();
        syncPreview();
    }

    function selectFace(faceId) {
        avatar.faceId = faceCatalog.some((face) => face.id === faceId) ? faceId : defaultFaceId;
        drawFaceOptions();
        syncPreview();
    }

    function updateSelectedColor() {
        avatar[selectedKey()] = hexToColor(colorPicker.value);
        syncColorPicker();
        syncPreview();
    }

    colorPicker.addEventListener('input', updateSelectedColor);

    createAvatarPreview(figure, selectPart)
        .then((createdPreview) => {
            preview = createdPreview;
            drawFaceOptions();
            syncPreview();
        })
        .catch(() => {
            setStatus(status, '3D preview could not load.', 'error');
        });

    document.querySelector('[data-save-avatar]').addEventListener('click', async () => {
        const button = document.querySelector('[data-save-avatar]');
        setStatus(status, '', '');
        button.disabled = true;
        button.textContent = 'Saving';

        try {
            const { response, data } = await postJson('/api/avatar', { avatar }, token);
            if (!response.ok || !data.success) throw new Error(data.error || 'Avatar save failed.');
            setStatus(status, 'Avatar saved.', 'success');
        } catch (error) {
            setStatus(status, error.message || 'Network error.', 'error');
        } finally {
            button.disabled = false;
            button.textContent = 'Save avatar';
        }
    });

    fetch('/api/avatar', { headers: { Authorization: `Bearer ${token}` } })
        .then((response) => response.json())
        .then((data) => {
            if (data.success && data.avatar) avatar = normalizeAvatar(data.avatar);
            applyHats(data.items, data.coins);
            drawFaceOptions();
            selectPart('head');
        })
        .catch(() => {
            drawFaceOptions();
            selectPart('head');
            setStatus(status, 'Using local defaults until the server responds.', 'error');
        });
}

function gameHref(game) {
    return `/games/${game.id}`;
}

function formatGameCount(value, singular, plural) {
    const count = Number(value || 0);
    return `${count} ${count === 1 ? singular : plural}`;
}

function statTile(label, value) {
    const row = document.createElement('div');
    row.className = 'stat-tile';
    const name = document.createElement('span');
    name.textContent = label;
    const strong = document.createElement('strong');
    strong.textContent = value;
    row.append(name, strong);
    return row;
}

function formatLikes(count) {
    const likes = Math.max(0, Number(count || 0));
    return `${likes} like${likes === 1 ? '' : 's'}`;
}

// One account, one like: the server owns the row, so the button asks it rather
// than counting up locally and hoping the page and the file agree.
function createLikeControl(game, status) {
    const wrap = document.createElement('span');
    wrap.className = 'like-control';

    const button = document.createElement('button');
    button.type = 'button';
    button.className = 'button small like-button';

    const count = document.createElement('span');
    count.className = 'like-count';

    const paint = () => {
        const liked = Boolean(game.liked);
        button.textContent = liked ? 'Liked' : 'Like';
        button.setAttribute('aria-pressed', liked ? 'true' : 'false');
        button.classList.toggle('liked', liked);
        count.textContent = formatLikes(game.likes);
    };
    paint();

    button.addEventListener('click', async () => {
        const token = getToken();
        if (!token) {
            const next = `${window.location.pathname}${window.location.search}`;
            window.location.href = `/login?next=${encodeURIComponent(next)}`;
            return;
        }

        button.disabled = true;
        try {
            const { response, data } = await postJson(`/api/games/${game.id}/like`, {}, token);
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not save your like.');
            game.likes = data.likes;
            game.liked = data.liked;
            paint();
        } catch (error) {
            setStatus(status, error.message || 'Could not save your like.', 'error');
        } finally {
            button.disabled = false;
        }
    });

    wrap.append(button, count);
    return wrap;
}

// Most liked first, with the title as the tie-break so two games on the same
// score never swap places between visits.
function sortGames(games, order) {
    const sorted = [...games];
    if (order === 'newest') {
        sorted.sort((first, second) => String(second.createdAt).localeCompare(String(first.createdAt)));
    } else if (order === 'title') {
        sorted.sort((first, second) => first.title.localeCompare(second.title));
    } else {
        sorted.sort((first, second) => (second.likes || 0) - (first.likes || 0)
            || first.title.localeCompare(second.title));
    }
    return sorted;
}

function createGameCard(game, status) {
    const card = document.createElement('article');
    card.className = 'game-card';

    const media = document.createElement('a');
    media.className = 'game-card-media';
    media.href = gameHref(game);
    media.setAttribute('aria-label', `${game.title} details`);

    const banner = document.createElement('img');
    banner.src = game.bannerUrl || '/assets/games/default-game-banner.png';
    banner.alt = '';
    banner.loading = 'lazy';
    media.appendChild(banner);

    const body = document.createElement('div');
    body.className = 'game-card-body';

    const icon = document.createElement('img');
    icon.className = 'game-icon';
    icon.src = game.iconUrl || '/assets/games/default-game-icon.png';
    icon.alt = '';
    icon.loading = 'lazy';

    const text = document.createElement('div');
    text.className = 'game-card-text';

    const title = document.createElement('a');
    title.className = 'game-title-link';
    title.href = gameHref(game);
    title.textContent = game.title;

    const meta = document.createElement('p');
    meta.className = 'game-meta';
    meta.textContent = `By ${game.ownerUsername} - ${formatGameCount(game.stats?.partsCount, 'part', 'parts')} - ${formatDate(game.createdAt)}`;

    const description = document.createElement('p');
    description.className = 'game-description';
    description.textContent = game.description || 'No description yet.';

    const actions = document.createElement('div');
    actions.className = 'game-card-actions';
    const details = document.createElement('a');
    details.className = 'button small';
    details.href = gameHref(game);
    details.textContent = 'Open';
    actions.appendChild(details);
    actions.appendChild(createLikeControl(game, status));

    text.append(title, meta, description, actions);
    body.append(icon, text);
    card.append(media, body);
    return card;
}

function renderGameCards(container, games, emptyText, status) {
    if (!container) return;
    container.innerHTML = '';
    if (!games || games.length === 0) {
        container.appendChild(emptyMessage(emptyText));
        return;
    }
    games.forEach((game) => container.appendChild(createGameCard(game, status)));
}

async function initGamesPage() {
    const root = document.querySelector('[data-games-page]');
    if (!root) return;

    const list = document.querySelector('[data-games-list]');
    const status = document.querySelector('[data-games-status]');
    const sort = document.querySelector('[data-games-sort]');
    let games = [];

    // The page fetches once and orders what it has, so choosing a different sort
    // costs no request and cannot show a different set of games.
    const draw = () => {
        renderGameCards(list, sortGames(games, sort ? sort.value : 'likes'),
            'No public games have been published yet.', status);
    };

    list.appendChild(emptyMessage('Loading games'));

    try {
        const { response, data } = await getJson('/api/games');
        if (!response.ok || !data.success) throw new Error(data.error || 'Could not load games.');
        games = data.games || [];
        draw();
        if (sort) sort.addEventListener('change', draw);
    } catch (error) {
        list.innerHTML = '';
        setStatus(status, error.message || 'Could not load games.', 'error');
    }
}

function createMyGameRow(game) {
    const row = document.createElement('article');
    row.className = 'my-game-row';

    const icon = document.createElement('img');
    icon.className = 'game-icon small-icon';
    icon.src = game.iconUrl || '/assets/games/default-game-icon.png';
    icon.alt = '';

    const text = document.createElement('div');
    const title = document.createElement('a');
    title.className = 'game-title-link';
    title.href = gameHref(game);
    title.textContent = game.title;
    const meta = document.createElement('p');
    meta.className = 'game-meta';
    meta.textContent = `${game.isPublic ? 'Public' : 'Private'} - ${formatGameCount(game.stats?.partsCount, 'part', 'parts')} - Updated ${formatDate(game.updatedAt)}`;
    text.append(title, meta);

    // The script is what makes a game move, so the creator's own list is the
    // shortest way into the website's editor.
    const code = document.createElement('a');
    code.className = 'game-code-link';
    code.href = `/editor?game=${game.id}`;
    code.textContent = 'Code';

    row.append(icon, text, code);
    return row;
}

async function loadMyGames(container, status) {
    if (!container) return;
    container.innerHTML = '';
    container.appendChild(emptyMessage('Loading your games'));

    const { response, data } = await getJson('/api/games/mine', getToken());
    if (!response.ok || !data.success) throw new Error(data.error || 'Could not load your games.');

    container.innerHTML = '';
    if (data.games.length === 0) {
        container.appendChild(emptyMessage('No games published yet.'));
        return;
    }
    data.games.forEach((game) => container.appendChild(createMyGameRow(game)));
    setStatus(status, '', '');
}

function initCreateGamePage() {
    const root = document.querySelector('[data-create-game-page]');
    if (!root) return;

    const form = document.querySelector('[data-create-game-form]');
    const status = document.querySelector('[data-create-game-status]');
    const myGames = document.querySelector('[data-my-games-list]');
    const submit = form.querySelector('button[type="submit"]');
    const worldFile = form.elements.world;
    // The baseplate is the default so a room can be made from the browser; the
    // file picker only takes part when it is the chosen source.
    const syncWorldInput = () => {
        worldFile.disabled = form.elements.worldMode.value !== 'file';
    };
    form.querySelectorAll('input[name="worldMode"]').forEach((radio) => {
        radio.addEventListener('change', syncWorldInput);
    });
    syncWorldInput();

    requireSession()
        .then((session) => {
            if (!session) return;
            return loadMyGames(myGames, status);
        })
        .catch((error) => {
            setStatus(status, error.message || 'Could not load creator data.', 'error');
        });

    form.addEventListener('submit', async (event) => {
        event.preventDefault();
        setStatus(status, '', '');

        const useFile = form.elements.worldMode.value === 'file';
        const file = useFile ? form.elements.world.files[0] : null;
        if (useFile && !file) {
            setStatus(status, 'Choose a .world file, or start from the baseplate.', 'error');
            return;
        }

        submit.disabled = true;
        submit.textContent = 'Publishing';

        try {
            const body = {
                title: form.elements.title.value,
                description: form.elements.description.value,
                isPublic: form.elements.isPublic.checked
            };
            if (useFile) {
                body.worldText = await file.text();
            } else {
                body.starter = true;
            }
            const { response, data } = await postJson('/api/games', body, getToken());
            if (!response.ok || !data.success) throw new Error(data.error || 'Publish failed.');
            setStatus(status, 'Game published.', 'success');
            form.reset();
            form.elements.isPublic.checked = true;
            syncWorldInput();
            await loadMyGames(myGames, status);
            window.history.replaceState(null, '', `/create?published=${data.game.id}`);
        } catch (error) {
            setStatus(status, error.message || 'Publish failed.', 'error');
        } finally {
            submit.disabled = false;
            submit.textContent = 'Publish game';
        }
    });
}

// The website's editor edits the .rbx script a game runs, not the engine's own
// sources: it loads the file the game server is launched with and saves back to
// it, so a fix lands in the next run without opening the desktop Studio.
function initEditorPage() {
    const root = document.querySelector('[data-editor-page]');
    if (!root) return;

    const picker = document.querySelector('[data-editor-game]');
    const text = document.querySelector('[data-editor-text]');
    const gutter = document.querySelector('[data-editor-gutter]');
    const status = document.querySelector('[data-editor-status]');
    const cursor = document.querySelector('[data-editor-cursor]');
    const state = document.querySelector('[data-editor-state]');
    const limit = document.querySelector('[data-editor-limit]');
    const title = document.querySelector('[data-editor-title]');
    const saveButton = document.querySelector('[data-editor-save]');
    const reloadButton = document.querySelector('[data-editor-reload]');

    let current = 0;
    let dirty = false;
    let maxBytes = 0;

    function setDirty(value) {
        dirty = value;
        if (!state) return;
        state.textContent = value ? 'Unsaved changes' : 'Saved';
        state.dataset.dirty = value ? 'true' : 'false';
    }

    function renderCursor() {
        if (!cursor) return;
        const before = text.value.slice(0, text.selectionStart);
        const lines = before.split('\n');
        cursor.textContent = `Ln ${lines.length}, Col ${lines[lines.length - 1].length + 1}`;
    }

    // The gutter is its own element so the line numbers stay aligned while the
    // text scrolls; the textarea does not wrap, which is what keeps them honest.
    function renderGutter() {
        if (!gutter) return;
        const count = text.value.split('\n').length;
        let numbers = '';
        for (let line = 1; line <= count; line += 1) numbers += `${line}\n`;
        gutter.textContent = numbers;
        gutter.scrollTop = text.scrollTop;
    }

    function render() {
        renderGutter();
        renderCursor();
    }

    async function loadScript(gameId) {
        if (!gameId) {
            text.value = '';
            render();
            return;
        }
        setStatus(status, 'Loading script', '');
        const { response, data } = await getJson(`/api/games/${gameId}/script`, getToken());
        if (!response.ok || !data.success) {
            setStatus(status, data.error || 'Could not load the script.', 'error');
            return;
        }
        current = data.gameId;
        text.value = data.text || '';
        maxBytes = Number(data.limit) || 0;
        if (title) title.textContent = data.title;
        if (limit) limit.textContent = maxBytes ? `${Math.round(maxBytes / 1024)} KB limit` : '';
        setDirty(false);
        render();
        setStatus(status, text.value
            ? 'Loaded from the server.'
            : 'This game has no script yet; saving makes one.', '');
    }

    async function saveScript() {
        if (!current) return;
        if (maxBytes && text.value.length > maxBytes) {
            setStatus(status, `This script is over the ${Math.round(maxBytes / 1024)} KB limit.`, 'error');
            return;
        }
        saveButton.disabled = true;
        saveButton.textContent = 'Saving';
        try {
            const response = await fetch(`/api/games/${current}/script`, {
                method: 'PUT',
                headers: {
                    'Content-Type': 'application/json',
                    Authorization: `Bearer ${getToken()}`
                },
                body: JSON.stringify({ text: text.value })
            });
            const data = await response.json();
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not save the script.');
            setDirty(false);
            setStatus(status, `Saved ${formatGameCount(data.bytes, 'byte', 'bytes')}.`, 'success');
        } catch (error) {
            setStatus(status, error.message || 'Could not save the script.', 'error');
        } finally {
            saveButton.disabled = false;
            saveButton.textContent = 'Save';
        }
    }

    function discardConfirmed() {
        return !dirty || window.confirm('Discard the unsaved changes?');
    }

    picker.addEventListener('change', () => {
        if (!discardConfirmed()) {
            picker.value = String(current);
            return;
        }
        loadScript(Number(picker.value)).catch((error) => {
            setStatus(status, error.message || 'Could not load the script.', 'error');
        });
        window.history.replaceState(null, '', `/editor?game=${picker.value}`);
    });

    text.addEventListener('input', () => {
        setDirty(true);
        render();
    });
    text.addEventListener('scroll', () => {
        if (gutter) gutter.scrollTop = text.scrollTop;
    });
    for (const event of ['click', 'keyup', 'select']) {
        text.addEventListener(event, renderCursor);
    }
    text.addEventListener('keydown', (event) => {
        if (event.key === 'Tab') {
            event.preventDefault();
            text.setRangeText('  ', text.selectionStart, text.selectionEnd, 'end');
            setDirty(true);
            render();
            return;
        }
        if (event.key === 'Enter') {
            const start = text.selectionStart;
            const lineStart = text.value.lastIndexOf('\n', start - 1) + 1;
            const match = text.value.slice(lineStart, start).match(/^[ \t]*/);
            const indent = match ? match[0] : '';
            if (indent) {
                event.preventDefault();
                text.setRangeText(`\n${indent}`, start, text.selectionEnd, 'end');
                setDirty(true);
                render();
            }
            return;
        }
        if (event.key === 's' && (event.ctrlKey || event.metaKey)) {
            event.preventDefault();
            saveScript().catch(() => {});
        }
    });

    saveButton.addEventListener('click', () => {
        saveScript().catch(() => {});
    });
    reloadButton.addEventListener('click', () => {
        if (!discardConfirmed()) return;
        loadScript(current).catch((error) => {
            setStatus(status, error.message || 'Could not load the script.', 'error');
        });
    });
    window.addEventListener('beforeunload', (event) => {
        if (!dirty) return;
        event.preventDefault();
        event.returnValue = '';
    });

    requireSession()
        .then(async (session) => {
            if (!session) return;
            const { response, data } = await getJson('/api/games/mine', getToken());
            if (!response.ok || !data.success) {
                throw new Error(data.error || 'Could not load your games.');
            }
            const games = data.games || [];
            picker.innerHTML = '';
            for (const game of games) {
                const option = document.createElement('option');
                option.value = String(game.id);
                option.textContent = game.title;
                picker.appendChild(option);
            }
            if (games.length === 0) {
                picker.disabled = true;
                text.disabled = true;
                saveButton.disabled = true;
                reloadButton.disabled = true;
                setStatus(status, 'Publish a game first; its script can be edited here after that.', '');
                return;
            }
            const requested = Number(new URLSearchParams(window.location.search).get('game'));
            const chosen = games.some((game) => game.id === requested) ? requested : games[0].id;
            picker.value = String(chosen);
            await loadScript(chosen);
        })
        .catch((error) => {
            setStatus(status, error.message || 'Could not load the editor.', 'error');
        });
}

function selectedGameId() {
    const match = window.location.pathname.match(/^\/games\/(\d+)/);
    return match ? Number(match[1]) : 0;
}

function renderServerList(container, servers) {
    if (!container) return;
    container.innerHTML = '';
    if (!servers || servers.length === 0) {
        container.appendChild(emptyMessage('No live servers yet.'));
        return;
    }

    for (const server of servers) {
        const row = document.createElement('article');
        row.className = 'server-row';
        const address = document.createElement('strong');
        address.textContent = `${server.host}:${server.port}`;
        const meta = document.createElement('span');
        const playerCount = Number(server.playerCount || 0);
        meta.textContent = `${server.status} - ${playerCount} player${playerCount === 1 ? '' : 's'}${server.processId ? ` - PID ${server.processId}` : ''}`;
        row.append(address, meta);
        container.appendChild(row);
    }
}

function renderGameStats(container, game) {
    if (!container) return;
    container.innerHTML = '';
    const rows = [
        ['Parts', String(game.stats?.partsCount || 0)],
        ['Dynamic', String(game.stats?.dynamicPartsCount || 0)],
        ['Spawns', String(game.stats?.spawnCount || 0)],
        ['Created', formatDate(game.createdAt)],
        ['Updated', formatDate(game.updatedAt)],
        ['Plays', String(game.stats?.launchCount || 0)]
    ];
    rows.forEach(([label, value]) => container.appendChild(statTile(label, value)));
}

function initGamePage() {
    const root = document.querySelector('[data-game-page]');
    if (!root) return;

    const gameId = selectedGameId();
    const title = document.querySelector('[data-game-title]');
    const owner = document.querySelector('[data-game-owner]');
    const description = document.querySelector('[data-game-description]');
    const banner = document.querySelector('[data-game-banner]');
    const stats = document.querySelector('[data-game-stats]');
    const servers = document.querySelector('[data-game-servers]');
    const status = document.querySelector('[data-game-status]');
    const playButton = document.querySelector('[data-play-game]');
    const launchCommand = document.querySelector('[data-launch-command]');
    let currentGame = null;

    async function loadGame() {
        if (!gameId) {
            setStatus(status, 'Game not found.', 'error');
            return;
        }

        const { response, data } = await getJson(`/api/games/${gameId}`, getToken());
        if (response.status === 404) {
            setStatus(status, 'Game not found.', 'error');
            return;
        }
        if (!response.ok || !data.success) throw new Error(data.error || 'Could not load game.');

        currentGame = data.game;
        document.title = `${currentGame.title} - RBLX Clone`;
        title.textContent = currentGame.title;
        owner.textContent = `By ${currentGame.ownerUsername}`;
        description.textContent = currentGame.description || 'No description yet.';
        banner.src = currentGame.bannerUrl || '/assets/games/default-game-banner.png';
        renderGameStats(stats, currentGame);
        renderServerList(servers, data.servers);

        const likeSlot = document.querySelector('[data-game-like]');
        if (likeSlot) {
            likeSlot.innerHTML = '';
            likeSlot.appendChild(createLikeControl(currentGame, status));
        }
    }

    playButton.addEventListener('click', async () => {
        if (!currentGame) return;
        const token = getToken();
        if (!token) {
            window.location.href = `/login?next=${encodeURIComponent(window.location.pathname)}`;
            return;
        }

        playButton.disabled = true;
        playButton.textContent = 'Starting';
        setStatus(status, '', '');

        try {
            const { response, data } = await postJson(`/api/games/${currentGame.id}/play`, { launchClient: true }, token);
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not start game.');

            if (launchCommand) {
                launchCommand.hidden = false;
                launchCommand.textContent = data.launch.command;
            }

            // A failed client launch used to render as a green "Server ready" line,
            // which hid the reason no player window appeared. Report it as an error
            // and keep the command visible so the client can be started by hand.
            const address = `${data.server.host}:${data.server.port}`;
            if (data.server?.running === false) {
                // The backend reserved an address without starting anything on it,
                // so calling it ready promises a listener that does not exist.
                const how = data.server.command ? ` Start it with: ${data.server.command}` : '';
                setStatus(status, `No game server is running on ${address}.${how}`, 'error');
            } else if (data.player?.launched) {
                setStatus(status, `Player opening on ${address}.`, 'success');
            } else {
                const reason = data.player?.reason ? ` ${data.player.reason}` : '';
                setStatus(status, `Server ready on ${address}, but the player did not open.${reason}`, 'error');
            }
            renderServerList(servers, [data.server]);
            await loadGame();
        } catch (error) {
            setStatus(status, error.message || 'Could not start game.', 'error');
        } finally {
            playButton.disabled = false;
            playButton.textContent = 'Play';
        }
    });

    loadGame().catch((error) => {
        setStatus(status, error.message || 'Could not load game.', 'error');
    });
}

function initAdminPage() {
    const root = document.querySelector('[data-admin-page]');
    if (!root) return;

    const statsGrid = document.querySelector('[data-admin-stats]');
    const usersBody = document.querySelector('[data-admin-users]');
    const gamesBody = document.querySelector('[data-admin-games]');
    const instancesBody = document.querySelector('[data-admin-instances]');
    const reportsBody = document.querySelector('[data-admin-reports]');
    const userReportsBody = document.querySelector('[data-admin-user-reports]');
    const appealsBody = document.querySelector('[data-admin-appeals]');
    const replayPanel = document.querySelector('[data-admin-replay-panel]');
    const replayTitle = document.querySelector('[data-admin-replay-title]');
    const replayNote = document.querySelector('[data-admin-replay-note]');
    const replayStage = document.querySelector('[data-admin-replay-stage]');
    const replayPlay = document.querySelector('[data-admin-replay-play]');
    const replayClose = document.querySelector('[data-admin-replay-close]');
    const status = document.querySelector('[data-admin-status]');
    const refreshButton = document.querySelector('[data-admin-refresh]');

    function adminRow(cells) {
        const row = document.createElement('tr');
        for (const cell of cells) {
            const td = document.createElement('td');
            if (cell instanceof Node) {
                td.appendChild(cell);
            } else {
                td.textContent = String(cell);
            }
            row.appendChild(td);
        }
        return row;
    }

    function adminEmptyRow(body, columns, message) {
        const row = document.createElement('tr');
        const cell = document.createElement('td');
        cell.colSpan = columns;
        cell.className = 'admin-empty';
        cell.textContent = message;
        row.appendChild(cell);
        body.appendChild(row);
    }

    function gameLink(game) {
        const link = document.createElement('a');
        link.className = 'game-title-link';
        link.href = gameHref(game);
        link.textContent = game.title;
        return link;
    }

    async function deleteGame(game, button) {
        const confirmed = window.confirm(`Delete "${game.title}"? Its world file is removed too.`);
        if (!confirmed) return;

        button.disabled = true;
        button.textContent = 'Deleting';
        try {
            const response = await fetch(`/api/admin/games/${game.id}`, {
                method: 'DELETE',
                headers: { Authorization: `Bearer ${getToken()}` }
            });
            const data = await response.json();
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not delete the game.');
            setStatus(status, `Deleted "${game.title}".`, 'success');
            await load();
        } catch (error) {
            setStatus(status, error.message || 'Could not delete the game.', 'error');
            button.disabled = false;
            button.textContent = 'Delete';
        }
    }

    function renderStats(stats) {
        if (!statsGrid) return;
        statsGrid.innerHTML = '';
        statsGrid.append(
            statTile('Users', String(stats.userCount)),
            statTile('Games', String(stats.gameCount)),
            statTile('Public games', String(stats.publicGameCount)),
            statTile('Live servers', String(stats.instanceCount)),
            statTile('Launches', String(stats.launchCount)),
            statTile('Playtime', formatPlaytime(stats.playtimeSeconds)),
            statTile('Open appeals', String(stats.openAppealCount ?? 0))
        );
    }

    function moderationLabel(user) {
        if (user.banned) return 'Banned';
        if (user.suspendedUntil) return `Suspended until ${formatDate(user.suspendedUntil)}`;
        if (user.cheatStrikes > 0) {
            return `${user.cheatStrikes} strike${user.cheatStrikes === 1 ? '' : 's'}`;
        }
        return 'Clear';
    }

    async function moderateUser(user, action, button) {
        if (action === 'ban') {
            const confirmed = window.confirm(`Ban "${user.username}" for cheating?`);
            if (!confirmed) return;
        }
        button.disabled = true;
        try {
            const response = await fetch(`/api/admin/users/${user.id}/moderation`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    Authorization: `Bearer ${getToken()}`
                },
                body: JSON.stringify({ action })
            });
            const data = await response.json();
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not change the account.');
            await load();
            setStatus(status, data.message || `Updated "${user.username}".`, 'success');
        } catch (error) {
            setStatus(status, error.message || 'Could not change the account.', 'error');
            button.disabled = false;
        }
    }

    // The balance is earned by playtime, so the dashboard reads it and offers a
    // small box to give coins on top of it rather than an editable number.
    async function giveCoins(user, input, button) {
        const amount = Math.trunc(Number(input.value));
        if (!Number.isFinite(amount) || amount <= 0) {
            setStatus(status, 'Enter a whole number of coins to give.', 'error');
            return;
        }
        button.disabled = true;
        try {
            const response = await fetch(`/api/admin/users/${user.id}/coins`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    Authorization: `Bearer ${getToken()}`
                },
                body: JSON.stringify({ amount })
            });
            const data = await response.json();
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not give coins.');
            await load();
            setStatus(status, data.message || `Gave ${amount} coins to "${user.username}".`, 'success');
        } catch (error) {
            setStatus(status, error.message || 'Could not give coins.', 'error');
            button.disabled = false;
        }
    }

    function renderUsers(users) {
        if (!usersBody) return;
        usersBody.innerHTML = '';
        if (!users || users.length === 0) {
            adminEmptyRow(usersBody, 11, 'No accounts yet.');
            return;
        }
        for (const user of users) {
            const name = document.createElement('a');
            name.className = 'game-title-link';
            name.href = profileHref(user);
            name.textContent = user.username;

            // A restricted account is the one case where the button changes what
            // it does, so the label says which one it is.
            const restricted = Boolean(user.banned || user.suspendedUntil);
            const cell = document.createElement('div');
            const stateText = document.createElement('span');
            stateText.className = 'admin-reason';
            stateText.textContent = moderationLabel(user);
            const button = makeButton(
                restricted ? 'Pardon' : 'Ban',
                restricted ? 'button small' : 'button danger small',
                () => {}
            );
            button.addEventListener('click', () => {
                moderateUser(user, restricted ? 'pardon' : 'ban', button).catch(() => {});
            });
            cell.append(stateText, button);

            const coinCell = document.createElement('div');
            const balance = document.createElement('span');
            balance.textContent = String(user.coins ?? 0);
            const grant = document.createElement('input');
            grant.type = 'number';
            grant.className = 'input coin-grant-input';
            grant.min = '1';
            grant.max = '100000';
            grant.value = '10';
            grant.setAttribute('aria-label', `Coins to give ${user.username}`);
            const giveButton = makeButton('Give', 'button secondary small', () => {});
            giveButton.addEventListener('click', () => {
                giveCoins(user, grant, giveButton).catch(() => {});
            });
            coinCell.append(balance, grant, giveButton);

            usersBody.appendChild(adminRow([
                user.id,
                name,
                user.isAdmin ? 'Admin' : 'Player',
                user.gameCount,
                user.friendCount,
                coinCell,
                formatPlaytime(user.playtimeSeconds),
                formatDate(user.createdAt),
                user.clientDetections ?? 0,
                user.cheatStrikes ?? 0,
                cell
            ]));
        }
    }

    function renderGames(games) {
        if (!gamesBody) return;
        gamesBody.innerHTML = '';
        if (!games || games.length === 0) {
            adminEmptyRow(gamesBody, 8, 'No games published yet.');
            return;
        }
        for (const game of games) {
            const remove = makeButton('Delete', 'button danger small', () => {});
            remove.addEventListener('click', () => deleteGame(game, remove));
            gamesBody.appendChild(adminRow([
                game.id,
                gameLink(game),
                game.ownerUsername || game.ownerId,
                game.isPublic ? 'Public' : 'Private',
                formatGameCount(game.stats?.partsCount, 'part', 'parts'),
                game.stats?.launchCount ?? 0,
                formatDate(game.updatedAt),
                remove
            ]));
        }
    }

    function renderInstances(instances) {
        if (!instancesBody) return;
        instancesBody.innerHTML = '';
        if (!instances || instances.length === 0) {
            adminEmptyRow(instancesBody, 5, 'No game servers are running.');
            return;
        }
        for (const instance of instances) {
            instancesBody.appendChild(adminRow([
                instance.id.slice(0, 8),
                `${instance.host}:${instance.port}`,
                instance.status,
                instance.playerCount,
                formatDate(instance.startedAt)
            ]));
        }
    }

    function renderReports(reports) {
        if (!reportsBody) return;
        reportsBody.innerHTML = '';
        if (!reports || reports.length === 0) {
            adminEmptyRow(reportsBody, 5, 'No client problems reported.');
            return;
        }
        for (const report of reports) {
            const reason = document.createElement('span');
            reason.className = 'admin-reason';
            reason.textContent = report.reason || '';
            reportsBody.appendChild(adminRow([
                formatDate(report.createdAt),
                report.username || 'guest',
                report.stage || 'unknown',
                report.serverAddress || '—',
                reason
            ]));
        }
    }

    function renderUserReports(reports) {
        if (!userReportsBody) return;
        userReportsBody.innerHTML = '';
        if (!reports || reports.length === 0) {
            adminEmptyRow(userReportsBody, 4, 'No reports filed.');
            return;
        }
        for (const report of reports) {
            const target = document.createElement('span');
            target.textContent = `${report.targetType === 'game' ? 'Game' : 'Player'}: ${report.targetLabel || report.targetId}`;
            const reason = document.createElement('span');
            reason.className = 'admin-reason';
            reason.textContent = report.reason || '';
            userReportsBody.appendChild(adminRow([
                formatDate(report.createdAt),
                report.reporterUsername || 'unknown',
                target,
                reason
            ]));
        }
    }

    async function reviewAppeal(appeal, action, button) {
        button.disabled = true;
        try {
            const response = await fetch(`/api/admin/appeals/${appeal.id}`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    Authorization: `Bearer ${getToken()}`
                },
                body: JSON.stringify({ action })
            });
            const data = await response.json();
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not answer the appeal.');
            setStatus(status, data.message || 'Appeal answered.', 'success');
            await load();
        } catch (error) {
            setStatus(status, error.message || 'Could not answer the appeal.', 'error');
            button.disabled = false;
        }
    }

    // The recording behind an action, drawn from above: the path the client
    // claimed in one line, the path the server accepted in another. A hop, a climb
    // or an honest run all look different here, which is what an appeal turns on.
    let replayClip = null;
    let replayMarker = null;
    let replayTimer = 0;

    function replayPath(frames, keyX, keyZ, bounds) {
        return frames
            .map((frame, index) => {
                const x = ((frame[keyX] / 100) - bounds.minX) / bounds.width * 100;
                const z = ((frame[keyZ] / 100) - bounds.minZ) / bounds.depth * 100;
                return `${index === 0 ? 'M' : 'L'}${x.toFixed(2)},${z.toFixed(2)}`;
            })
            .join(' ');
    }

    function moveReplayMarker(index) {
        if (!replayClip || !replayMarker) return;
        const frame = replayClip.frames[index];
        if (!frame) return;
        const bounds = replayClip.bounds;
        replayMarker.setAttribute('cx', String(((frame.x / 100) - bounds.minX) / bounds.width * 100));
        replayMarker.setAttribute('cy', String(((frame.z / 100) - bounds.minZ) / bounds.depth * 100));
        replayMarker.setAttribute('r', '2.4');
    }

    function playReplay() {
        if (!replayClip || replayClip.frames.length === 0) return;
        cancelAnimationFrame(replayTimer);
        const frames = replayClip.frames;
        const startedAt = performance.now() - frames[0].atMs;
        replayPlay.textContent = 'Pause';
        const step = (now) => {
            const elapsed = now - startedAt;
            let index = 0;
            while (index + 1 < frames.length && frames[index + 1].atMs <= elapsed) index += 1;
            moveReplayMarker(index);
            if (index + 1 < frames.length) {
                replayTimer = requestAnimationFrame(step);
            } else {
                replayPlay.textContent = 'Play';
            }
        };
        replayTimer = requestAnimationFrame(step);
    }

    async function showReplay(appeal) {
        if (!replayPanel || !replayStage) return;
        try {
            const response = await fetch(`/api/admin/infractions/${appeal.infractionId}/replay`, {
                headers: { Authorization: `Bearer ${getToken()}` }
            });
            const data = await response.json();
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not load the replay.');
            const frames = data.frames || [];
            if (frames.length === 0) throw new Error('This action has no recording.');

            const xs = frames.map((frame) => frame.x / 100).concat(frames.map((frame) => frame.ax / 100));
            const zs = frames.map((frame) => frame.z / 100).concat(frames.map((frame) => frame.az / 100));
            const minX = Math.min(...xs);
            const minZ = Math.min(...zs);
            const bounds = {
                minX,
                minZ,
                width: Math.max(Math.max(...xs) - minX, 1),
                depth: Math.max(Math.max(...zs) - minZ, 1)
            };
            replayClip = { frames, bounds };

            replayStage.innerHTML = '';
            const canvas = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
            canvas.setAttribute('viewBox', '0 0 100 100');
            canvas.setAttribute('class', 'replay-canvas');
            const claimed = document.createElementNS('http://www.w3.org/2000/svg', 'path');
            claimed.setAttribute('d', replayPath(frames, 'x', 'z', bounds));
            claimed.setAttribute('class', 'replay-claimed');
            const accepted = document.createElementNS('http://www.w3.org/2000/svg', 'path');
            accepted.setAttribute('d', replayPath(frames, 'ax', 'az', bounds));
            accepted.setAttribute('class', 'replay-accepted');
            replayMarker = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            replayMarker.setAttribute('class', 'replay-marker');
            canvas.append(accepted, claimed, replayMarker);
            replayStage.appendChild(canvas);

            replayTitle.textContent = `Replay: ${data.username} - ${data.kind}`;
            replayNote.textContent = [data.proof, data.expected].filter(Boolean).join(' | ');
            replayPanel.hidden = false;
            moveReplayMarker(0);
            playReplay();
        } catch (error) {
            setStatus(status, error.message || 'Could not load the replay.', 'error');
        }
    }

    if (replayClose) {
        replayClose.addEventListener('click', () => {
            cancelAnimationFrame(replayTimer);
            replayPanel.hidden = true;
        });
    }
    if (replayPlay) {
        replayPlay.addEventListener('click', () => {
            if (replayPlay.textContent === 'Pause') {
                cancelAnimationFrame(replayTimer);
                replayPlay.textContent = 'Play';
            } else {
                playReplay();
            }
        });
    }

    function renderAppeals(appeals) {
        if (!appealsBody) return;
        appealsBody.innerHTML = '';
        if (!appeals || appeals.length === 0) {
            adminEmptyRow(appealsBody, 5, 'No appeals filed.');
            return;
        }
        for (const appeal of appeals) {
            const message = document.createElement('span');
            message.className = 'admin-reason';
            message.textContent = appeal.message || '';
            if (appeal.infractionKind) {
                const target = document.createElement('span');
                target.className = 'admin-reason';
                target.textContent =
                    ` — appeals a ${appeal.infractionKind}: ${appeal.infractionReason || 'no reason recorded'}`;
                message.append(target);
            }
            // The measurement and the rule it broke, so the decision rests on what
            // the server saw rather than on the label alone.
            if (appeal.infractionProof) {
                const proof = document.createElement('span');
                proof.className = 'admin-reason';
                proof.textContent = ` — proof: ${appeal.infractionProof}`;
                message.append(proof);
            }
            if (appeal.infractionExpected) {
                const expected = document.createElement('span');
                expected.className = 'admin-reason';
                expected.textContent = ` — expected: ${appeal.infractionExpected}`;
                message.append(expected);
            }
            const state = document.createElement('span');
            state.textContent = appeal.status === 'open'
                ? 'Open'
                : `${appeal.status} by ${appeal.reviewer || 'an admin'}`;
            const actions = document.createElement('div');
            if (appeal.infractionHasReplay) {
                const replayButton = makeButton('Replay', 'button secondary small', () => {});
                replayButton.addEventListener('click', () => showReplay(appeal));
                actions.append(replayButton);
            }
            if (appeal.status === 'open') {
                const accept = makeButton('Accept', 'button small', () => {});
                accept.addEventListener('click', () => reviewAppeal(appeal, 'accept', accept));
                const deny = makeButton('Deny', 'button danger small', () => {});
                deny.addEventListener('click', () => reviewAppeal(appeal, 'deny', deny));
                actions.append(accept, deny);
            } else if (!appeal.infractionHasReplay) {
                actions.textContent = '—';
            }
            appealsBody.appendChild(adminRow([
                formatDate(appeal.createdAt),
                appeal.username || appeal.userId,
                message,
                state,
                actions
            ]));
        }
    }

    async function load() {
        setStatus(status, 'Loading site data', '');
        if (refreshButton) refreshButton.disabled = true;
        try {
            const { response, data } = await getJson('/api/admin/overview', getToken());
            if (response.status === 401) {
                window.location.href = `/login?next=${encodeURIComponent('/admin')}`;
                return;
            }
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not load admin data.');
            renderStats(data.stats);
            renderUsers(data.users);
            renderGames(data.games);
            renderInstances(data.instances);
            renderReports(data.clientReports);
            renderUserReports(data.reports);
            renderAppeals(data.appeals);
            setStatus(status, '', '');
        } catch (error) {
            setStatus(status, error.message || 'Could not load admin data.', 'error');
        } finally {
            if (refreshButton) refreshButton.disabled = false;
        }
    }

    refreshButton?.addEventListener('click', () => {
        load().catch(() => {});
    });

    load().catch((error) => {
        setStatus(status, error.message || 'Could not load admin data.', 'error');
    });
}

// Display preferences are per browser, not per account, and they change how every
// page looks, so they are applied before any page-specific setup runs.
function applyDisplayPreferences() {
    const reduced = localStorage.getItem('reduceMotion') === '1';
    document.documentElement.classList.toggle('reduce-motion', reduced);

    // "system" removes the attribute so the prefers-color-scheme rules in the
    // stylesheet take over; an explicit choice pins the theme in this browser.
    const theme = localStorage.getItem('theme') || 'system';
    if (theme === 'light' || theme === 'dark') {
        document.documentElement.dataset.theme = theme;
    } else {
        delete document.documentElement.dataset.theme;
    }
    return reduced;
}

// Account standing: the account's own moderation record, from the same store the
// admins read. An appeal is offered only while the account is actually restricted
// and has nothing already waiting.
function infractionLabel(kind) {
    switch (kind) {
        case 'warning': return 'Warning';
        case 'suspension': return 'Suspension';
        case 'ban': return 'Ban';
        case 'pardon': return 'Lifted';
        case 'detection': return 'Detection';
        default: return 'Note';
    }
}

function initStanding() {
    const panel = document.querySelector('[data-standing-panel]');
    const token = getToken();
    if (!panel || !token) return;

    const badge = panel.querySelector('[data-standing-badge]');
    const summary = panel.querySelector('[data-standing-summary]');
    const status = panel.querySelector('[data-standing-status]');
    const rows = panel.querySelector('[data-standing-infractions]');
    const appealList = panel.querySelector('[data-standing-appeals]');
    const appealForm = panel.querySelector('[data-appeal-form]');
    const appealStatus = panel.querySelector('[data-appeal-status]');
    const appealTarget = panel.querySelector('[data-appeal-infraction]');
    const appealTargetLabel = panel.querySelector('[data-appeal-target]');

    function row(cells) {
        const element = document.createElement('tr');
        for (const value of cells) {
            const cell = document.createElement('td');
            if (value instanceof Node) {
                cell.appendChild(value);
            } else {
                cell.textContent = value;
            }
            element.appendChild(cell);
        }
        return element;
    }

    // An appeal is about one action, so the button that starts one names it. The
    // action id rides in a hidden field; the form stays out of the way until a row
    // is chosen.
    function chooseAppeal(entry) {
        if (!appealForm || !appealTarget) return;
        appealTarget.value = String(entry.id);
        if (appealTargetLabel) {
            appealTargetLabel.hidden = false;
            appealTargetLabel.textContent =
                `Appealing the ${infractionLabel(entry.kind).toLowerCase()} from ${formatDateTime(entry.createdAt)}.`;
        }
        appealForm.hidden = false;
        appealForm.scrollIntoView({ behavior: 'smooth', block: 'center' });
        const message = panel.querySelector('#appeal-message');
        if (message) message.focus();
    }

    function render(data) {
        const strikes = Number(data.strikes || 0);
        const label = data.banned ? 'Banned' : data.suspendedUntil ? 'Suspended' : strikes > 0 ? 'Warned' : 'Clear';
        if (badge) {
            badge.textContent = label;
            badge.className = `standing-badge ${data.standing || 'clear'}`;
        }
        if (summary) {
            summary.textContent = data.banned
                ? 'This account is banned: it can still use the site but cannot join a game. Every entry that led here is below.'
                : data.suspendedUntil
                    ? `Suspended until ${formatDateTime(data.suspendedUntil)}${data.suspendedHoursRemaining ? `, about ${data.suspendedHoursRemaining}h left` : ''}. It can be appealed below.`
                    : strikes > 0
                        ? `${strikes} strike${strikes === 1 ? '' : 's'} recorded. The next confirmed detection suspends this account.`
                        : 'Nothing is recorded against this account.';
        }

        if (rows) {
            rows.innerHTML = '';
            const infractions = data.infractions || [];
            if (infractions.length === 0) {
                rows.appendChild(row(['—', 'Nothing recorded', '', '']));
            }
            for (const entry of infractions) {
                let appealCell = '—';
                if (entry.appealStatus) {
                    appealCell = `Appealed (${entry.appealStatus})`;
                } else if (entry.canAppeal) {
                    const button = document.createElement('button');
                    button.type = 'button';
                    button.className = 'button secondary small';
                    button.textContent = 'Appeal';
                    button.addEventListener('click', () => chooseAppeal(entry));
                    appealCell = button;
                }
                rows.appendChild(row([
                    formatDateTime(entry.createdAt),
                    `${infractionLabel(entry.kind)}${entry.automatic ? ' (detection)' : ''}`,
                    entry.expiresAt ? formatDateTime(entry.expiresAt) : '—',
                    appealCell
                ]));
            }
        }

        if (appealList) {
            appealList.innerHTML = '';
            const appeals = data.appeals || [];
            if (appeals.length === 0) {
                appealList.appendChild(emptyMessage('No appeals filed.'));
            }
            for (const appeal of appeals) {
                const card = document.createElement('div');
                card.className = 'standing-appeal';
                const head = document.createElement('p');
                head.textContent = `${formatDateTime(appeal.createdAt)} - ${appeal.status}`;
                const message = document.createElement('p');
                message.textContent = appeal.message;
                card.append(head, message);
                if (appeal.reviewNote) {
                    const note = document.createElement('p');
                    note.className = 'admin-reason';
                    note.textContent = `Admin: ${appeal.reviewNote}`;
                    card.appendChild(note);
                } else if (appeal.reviewer) {
                    const note = document.createElement('p');
                    note.className = 'admin-reason';
                    note.textContent = `Reviewed by ${appeal.reviewer}`;
                    card.appendChild(note);
                }
                appealList.appendChild(card);
            }
        }

        if (appealForm && !data.canAppeal) {
            appealForm.hidden = true;
            if (appealTarget) appealTarget.value = '';
            if (appealTargetLabel) appealTargetLabel.hidden = true;
        }
        if (appealStatus) {
            if (data.hasOpenAppeal) {
                setStatus(appealStatus, 'An appeal is waiting for an admin.', '');
            } else if (!data.canAppeal) {
                setStatus(appealStatus, '', '');
            }
        }
    }

    async function load() {
        try {
            const { response, data } = await getJson('/api/account/standing', token);
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not load account standing.');
            render(data);
        } catch (error) {
            setStatus(status, error.message || 'Could not load account standing.', 'error');
        }
    }

    appealForm?.addEventListener('submit', async (event) => {
        event.preventDefault();
        const submit = appealForm.querySelector('button[type="submit"]');
        submit.disabled = true;
        setStatus(appealStatus, '', '');
        try {
            const { response, data } = await postJson('/api/account/appeal', {
                infractionId: Number(appealTarget ? appealTarget.value : 0),
                message: appealForm.querySelector('#appeal-message').value.trim()
            }, token);
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not send the appeal.');
            appealForm.reset();
            setStatus(appealStatus, data.message || 'Appeal sent.', 'success');
            await load();
        } catch (error) {
            setStatus(appealStatus, error.message || 'Could not send the appeal.', 'error');
        } finally {
            submit.disabled = false;
        }
    });

    load().catch(() => {});
}

function initSettings() {
    const root = document.querySelector('[data-settings]');
    if (!root) return;

    // The standing panel stands on its own: the rest of the page needs the
    // password form, this does not.
    initStanding();

    const reducedToggle = document.querySelector('[data-reduce-motion]');
    if (reducedToggle) {
        reducedToggle.checked = applyDisplayPreferences();
        reducedToggle.addEventListener('change', () => {
            localStorage.setItem('reduceMotion', reducedToggle.checked ? '1' : '0');
            applyDisplayPreferences();
        });
    }

    const form = document.querySelector('[data-password-form]');
    if (!form) return;

    const token = getToken();
    if (!token) {
        window.location.href = '/login';
        return;
    }

    const usernameOutput = document.querySelector('[data-settings-username]');
    const idOutput = document.querySelector('[data-settings-id]');
    const coinsOutput = document.querySelector('[data-settings-coins]');
    const status = document.querySelector('[data-password-status]');

    verifySession()
        .then((session) => {
            if (!session) {
                window.location.href = '/login';
                return;
            }
            if (usernameOutput) usernameOutput.textContent = session.username;
            if (idOutput) idOutput.textContent = String(session.userId);
            if (coinsOutput) coinsOutput.textContent = String(Number(session.coins || 0));
        })
        .catch(() => {});

    form.addEventListener('submit', async (event) => {
        event.preventDefault();
        const submit = form.querySelector('button[type="submit"]');
        const currentPassword = form.querySelector('#current-password').value;
        const newPassword = form.querySelector('#new-password').value;
        const confirmPassword = form.querySelector('#confirm-password').value;

        if (newPassword !== confirmPassword) {
            setStatus(status, 'The new passwords do not match.', 'error');
            return;
        }

        submit.disabled = true;
        setStatus(status, '', '');
        try {
            const { response, data } = await postJson('/api/account/password',
                { currentPassword, newPassword }, getToken());
            if (!response.ok || !data.success) {
                throw new Error(data.error || 'Could not change the password.');
            }
            // Changing the password signs out every session, this one included,
            // so it keeps going with the token the server just issued.
            if (data.token) localStorage.setItem(tokenKey, data.token);
            form.reset();
            setStatus(status, 'Password changed.', 'success');
        } catch (error) {
            setStatus(status, error.message || 'Could not change the password.', 'error');
        } finally {
            submit.disabled = false;
        }
    });

    const themeSelect = document.querySelector('[data-theme-select]');
    if (themeSelect) {
        themeSelect.value = localStorage.getItem('theme') || 'system';
        themeSelect.addEventListener('change', () => {
            localStorage.setItem('theme', themeSelect.value);
            applyDisplayPreferences();
        });
    }

    const usernameForm = document.querySelector('[data-username-form]');
    const usernameStatus = document.querySelector('[data-username-status]');
    if (usernameForm) {
        usernameForm.addEventListener('submit', async (event) => {
            event.preventDefault();
            const submit = usernameForm.querySelector('button[type="submit"]');
            submit.disabled = true;
            setStatus(usernameStatus, '', '');
            try {
                const { response, data } = await postJson('/api/account/username', {
                    username: usernameForm.querySelector('#new-username').value.trim(),
                    currentPassword: usernameForm.querySelector('#username-password').value
                }, getToken());
                if (!response.ok || !data.success) throw new Error(data.error || 'Could not change the username.');
                // The token carries the username, so the new one replaces the old
                // session rather than leaving a stale claim behind.
                saveSession(data);
                if (usernameOutput) usernameOutput.textContent = data.username;
                usernameForm.reset();
                setStatus(usernameStatus, `Signed in as ${data.username}.`, 'success');
                initNavAuth();
            } catch (error) {
                setStatus(usernameStatus, error.message || 'Could not change the username.', 'error');
            } finally {
                submit.disabled = false;
            }
        });
    }

    const recoveryForm = document.querySelector('[data-recovery-form]');
    const recoveryStatus = document.querySelector('[data-recovery-status]');
    const recoveryOutput = document.querySelector('[data-recovery-code]');
    if (recoveryForm) {
        recoveryForm.addEventListener('submit', async (event) => {
            event.preventDefault();
            const submit = recoveryForm.querySelector('button[type="submit"]');
            submit.disabled = true;
            setStatus(recoveryStatus, '', '');
            try {
                const { response, data } = await postJson('/api/account/recovery', {
                    currentPassword: recoveryForm.querySelector('#recovery-password').value
                }, getToken());
                if (!response.ok || !data.success) throw new Error(data.error || 'Could not create a recovery code.');
                recoveryForm.reset();
                if (recoveryOutput) {
                    recoveryOutput.textContent = data.recoveryCode;
                    recoveryOutput.hidden = false;
                }
                setStatus(recoveryStatus, 'New code created. The old one no longer works.', 'success');
            } catch (error) {
                setStatus(recoveryStatus, error.message || 'Could not create a recovery code.', 'error');
            } finally {
                submit.disabled = false;
            }
        });
    }

    const exportButton = document.querySelector('[data-export]');
    const deleteForm = document.querySelector('[data-delete-form]');
    const deleteStatus = document.querySelector('[data-delete-status]');
    if (exportButton) {
        exportButton.addEventListener('click', async () => {
            exportButton.disabled = true;
            try {
                const { response, data } = await getJson('/api/account/export', getToken());
                if (!response.ok) throw new Error(data.error || 'Could not build the export.');
                const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
                const url = URL.createObjectURL(blob);
                const link = document.createElement('a');
                link.href = url;
                link.download = 'rblx-account.json';
                link.click();
                URL.revokeObjectURL(url);
            } catch (error) {
                setStatus(deleteStatus, error.message || 'Could not build the export.', 'error');
            } finally {
                exportButton.disabled = false;
            }
        });
    }

    if (deleteForm) {
        deleteForm.addEventListener('submit', async (event) => {
            event.preventDefault();
            if (!window.confirm('Delete this account? Its games and world files are removed too, and this cannot be undone.')) return;
            const submit = deleteForm.querySelector('button[type="submit"]');
            submit.disabled = true;
            setStatus(deleteStatus, '', '');
            try {
                const { response, data } = await postJson('/api/account/delete', {
                    password: deleteForm.querySelector('#delete-password').value
                }, getToken());
                if (!response.ok || !data.success) throw new Error(data.error || 'Could not delete the account.');
                clearSession();
                window.location.href = '/';
            } catch (error) {
                setStatus(deleteStatus, error.message || 'Could not delete the account.', 'error');
                submit.disabled = false;
            }
        });
    }
}

// The login page carries the recovery form for accounts whose password is gone.
function initRecovery() {
    const form = document.querySelector('[data-recovery-form]');
    if (!form) return;

    const status = document.querySelector('[data-recovery-status]');
    const output = document.querySelector('[data-recovery-new-code]');
    const refreshCaptcha = initCaptcha(form);

    form.addEventListener('submit', async (event) => {
        event.preventDefault();
        const submit = form.querySelector('button[type="submit"]');
        const username = form.querySelector('#recovery-username').value.trim();
        const recoveryCode = form.querySelector('#recovery-code').value.trim();
        const newPassword = form.querySelector('#recovery-password').value;
        const confirmPassword = form.querySelector('#recovery-confirm').value;

        if (newPassword !== confirmPassword) {
            setStatus(status, 'The new passwords do not match.', 'error');
            return;
        }

        submit.disabled = true;
        setStatus(status, '', '');
        try {
            const { response, data } = await postJson('/api/account/recover', {
                username,
                recoveryCode,
                newPassword,
                captchaId: form.querySelector('[data-captcha-id]')?.value || '',
                captchaAnswer: form.querySelector('input[name="captchaAnswer"]')?.value || ''
            });
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not reset the password.');
            form.reset();
            if (output) {
                output.textContent = data.recoveryCode;
                output.hidden = false;
            }
            setStatus(status, 'Password reset. The code above replaces the one you used.', 'success');
        } catch (error) {
            refreshCaptcha();
            setStatus(status, error.message || 'Could not reset the password.', 'error');
        } finally {
            submit.disabled = false;
        }
    });
}

// Games are reported from their own page; the id is in the address.
function initReportGame() {
    const button = document.querySelector('[data-report-game]');
    if (!button) return;

    const match = window.location.pathname.match(/^\/games\/(\d+)$/);
    if (!match) {
        button.hidden = true;
        return;
    }
    const gameId = Number(match[1]);

    button.addEventListener('click', async () => {
        const reason = window.prompt('What is wrong with this game?');
        if (!reason) return;
        try {
            const { response, data } = await postJson('/api/reports', {
                targetType: 'game',
                targetId: gameId,
                reason
            }, getToken());
            if (!response.ok || !data.success) throw new Error(data.error || 'Could not send the report.');
            window.alert('Report sent to the site admin.');
        } catch (error) {
            window.alert(error.message || 'Could not send the report.');
        }
    });
}

// The developer portal: the tokens a headless program signs in with, and the
// switch that lets the trainer play one of this account's games. The token is
// shown once, when it is made, because the site keeps only its hash.
async function initDevelopersPage() {
    const root = document.querySelector('[data-developers]');
    if (!root) return;
    if (!(await requireSession())) return;

    const token = getToken();
    const status = root.querySelector('[data-dev-status]');
    const tokenList = root.querySelector('[data-dev-tokens]');
    const gameList = root.querySelector('[data-dev-games]');
    const form = root.querySelector('[data-dev-token-form]');
    const output = root.querySelector('[data-dev-token-output]');
    const outputValue = root.querySelector('[data-dev-token-value]');
    const copyButton = root.querySelector('[data-dev-token-copy]');

    function renderTokens(data) {
        tokenList.innerHTML = '';
        const tokens = data.tokens || [];
        if (tokens.length === 0) {
            tokenList.appendChild(emptyMessage('No AI tokens yet.'));
            return;
        }
        tokens.forEach((devToken) => {
            const row = document.createElement('div');
            const label = document.createElement('strong');
            label.textContent = devToken.label;
            const meta = document.createElement('p');
            meta.className = 'onboarding-hint';
            const used = devToken.lastUsedAt
                ? `last used ${formatDateTime(devToken.lastUsedAt)}`
                : 'never used';
            const state = devToken.revoked ? ' · revoked' : '';
            meta.textContent = `Created ${formatDate(devToken.createdAt)} · ${used}${state}`;
            row.append(label, meta);

            if (!devToken.revoked) {
                const revoke = document.createElement('button');
                revoke.className = 'button danger';
                revoke.type = 'button';
                revoke.textContent = 'Revoke';
                revoke.addEventListener('click', async () => {
                    revoke.disabled = true;
                    try {
                        const { response, data: result } = await postJson(
                            `/api/dev/tokens/${devToken.id}/revoke`, {}, token);
                        if (!response.ok || !result.success) {
                            throw new Error(result.error || 'Could not revoke the token.');
                        }
                        setStatus(status, `Revoked "${devToken.label}".`, 'success');
                        await loadTokens();
                    } catch (error) {
                        revoke.disabled = false;
                        setStatus(status, error.message || 'Could not revoke the token.', 'error');
                    }
                });
                row.appendChild(revoke);
            }
            tokenList.appendChild(row);
        });
    }

    function renderGames(games) {
        gameList.innerHTML = '';
        if (games.length === 0) {
            gameList.appendChild(emptyMessage('You have no games yet. Publish one first.'));
            return;
        }
        games.forEach((game) => {
            const row = document.createElement('form');
            row.className = 'dashboard-search';

            const heading = document.createElement('p');
            heading.textContent = `${game.title}${game.isPublic ? '' : ' (private)'}`;

            const compatibleLabel = document.createElement('label');
            compatibleLabel.className = 'field';
            const compatible = document.createElement('input');
            compatible.type = 'checkbox';
            compatible.checked = Boolean(game.ai?.compatible);
            const compatibleText = document.createElement('span');
            compatibleText.textContent = 'AI compatible';
            compatibleLabel.append(compatible, compatibleText);

            const courseLabel = document.createElement('label');
            courseLabel.className = 'field';
            courseLabel.textContent = 'Course';
            const course = document.createElement('select');
            course.className = 'input';
            [['map', 'Walk to a part'], ['doors', 'Doors hotel to door 100']].forEach(([value, text]) => {
                const option = document.createElement('option');
                option.value = value;
                option.textContent = text;
                course.appendChild(option);
            });
            course.value = game.ai?.course === 'doors' ? 'doors' : 'map';
            courseLabel.appendChild(course);

            const goalLabel = document.createElement('label');
            goalLabel.className = 'field';
            goalLabel.textContent = 'Goal part';
            const goal = document.createElement('input');
            goal.className = 'input';
            goal.placeholder = 'FinishLine';
            goal.value = game.ai?.goal || '';
            goalLabel.appendChild(goal);

            const save = document.createElement('button');
            save.className = 'button';
            save.type = 'submit';
            save.textContent = 'Save';
            row.append(heading, compatibleLabel, courseLabel, goalLabel, save);

            row.addEventListener('submit', async (event) => {
                event.preventDefault();
                save.disabled = true;
                try {
                    const { response, data } = await postJson(`/api/games/${game.id}/ai`, {
                        compatible: compatible.checked,
                        course: course.value,
                        goal: goal.value
                    }, token);
                    if (!response.ok || !data.success) {
                        throw new Error(data.error || 'Could not save the AI setting.');
                    }
                    setStatus(status, `${game.title}: AI setting saved.`, 'success');
                    await loadGames();
                } catch (error) {
                    save.disabled = false;
                    setStatus(status, error.message || 'Could not save the AI setting.', 'error');
                }
            });
            gameList.appendChild(row);
        });
    }

    async function loadTokens() {
        const { response, data } = await getJson('/api/dev/tokens', token);
        if (!response.ok || !data.success) throw new Error(data.error || 'Could not load tokens.');
        renderTokens(data);
    }

    async function loadGames() {
        const { response, data } = await getJson('/api/games/mine', token);
        if (!response.ok || !data.success) throw new Error(data.error || 'Could not load your games.');
        renderGames(data.games || []);
    }

    form.addEventListener('submit', async (event) => {
        event.preventDefault();
        const submit = form.querySelector('button[type="submit"]');
        submit.disabled = true;
        output.hidden = true;
        try {
            const { response, data } = await postJson('/api/dev/tokens', {
                label: form.querySelector('#dev-token-label').value
            }, token);
            if (!response.ok || !data.success) {
                throw new Error(data.error || 'Could not create the token.');
            }
            outputValue.textContent = data.token;
            output.hidden = false;
            form.reset();
            setStatus(status, 'Token created. Copy it before leaving this page.', 'success');
            await loadTokens();
        } catch (error) {
            setStatus(status, error.message || 'Could not create the token.', 'error');
        } finally {
            submit.disabled = false;
        }
    });

    copyButton.addEventListener('click', async () => {
        try {
            await navigator.clipboard.writeText(outputValue.textContent || '');
            setStatus(status, 'Token copied.', 'success');
        } catch (error) {
            setStatus(status, 'Could not copy it; select the token and copy it by hand.', 'error');
        }
    });

    try {
        await Promise.all([loadTokens(), loadGames()]);
    } catch (error) {
        setStatus(status, error.message || 'Could not load the developer portal.', 'error');
    }
}

document.addEventListener('DOMContentLoaded', () => {
    applyDisplayPreferences();
    initNavAuth();
    initNavCurrent();
    initStandingNotice();
    initLogoutButtons();
    initLogin();
    initSignup();
    initRecovery();
    initReportGame();
    initDashboard();
    initSearch();
    initProfilePage();
    initAvatar();
    initGamesPage();
    initCreateGamePage();
    initEditorPage();
    initGamePage();
    initAdminPage();
    initSettings();
    initDevelopersPage();
});
