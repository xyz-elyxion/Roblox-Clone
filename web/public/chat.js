// The chat page is the website's own chat: one room per published game, four
// channels in every room, messages kept by WebApp.exe in the data file. This
// file reads a room, polls it while it is open and posts into it. It invents
// nothing -- no seed conversations, no made-up members -- so a page served
// without the backend says so instead of showing a conversation that never
// happened.

const chatBaseTitle = document.title;
const chatPollMs = 4000;

const chatShell = document.querySelector('[data-chat-shell]');
const chatLog = document.querySelector('[data-chat-log]');
const chatGuilds = document.querySelector('[data-guilds]');
const serverTitle = document.querySelector('[data-server-title]');
const serverMeta = document.querySelector('[data-server-meta]');
const channelList = document.querySelector('[data-channel-list]');
const channelTitle = document.querySelector('[data-channel-title]');
const channelTopic = document.querySelector('[data-channel-topic]');
const chatComposer = document.querySelector('[data-composer]');
const messageInput = document.querySelector('[data-message-input]');
const sendButton = chatComposer.querySelector('button[type="submit"]');
const chatHint = document.querySelector('[data-chat-hint]');
const memberGroups = document.querySelector('[data-member-groups]');
const membersToggle = document.querySelector('[data-toggle-members]');
const channelsToggle = document.querySelector('[data-toggle-channels]');
const chatBackdrop = document.querySelector('[data-chat-backdrop]');

const chatState = {
    servers: [],
    channels: [],
    gameId: 0,
    channel: 'general',
    lastId: 0,
    lastUser: '',
    lastMinutes: -1,
    lastDay: '',
    signedIn: false,
    canPost: false,
    userId: 0,
    username: '',
    maxLength: 500,
    polling: false,
    sending: false,
    pollTimer: 0,
    pollCount: 0,
    unreadTotal: 0
};

function chatInitials(name) {
    const words = String(name || '?').trim().split(/\s+/).filter(Boolean);
    if (words.length >= 2) {
        return (words[0][0] + words[1][0]).toUpperCase();
    }
    const letters = String(words[0] || '?').replace(/[^a-z0-9]/gi, '');
    return (letters.slice(0, 2) || '?').toUpperCase();
}

function chatAvatar(name) {
    const avatar = document.createElement('span');
    avatar.className = 'chat-avatar';
    avatar.setAttribute('aria-hidden', 'true');
    avatar.textContent = chatInitials(name);
    return avatar;
}

function chatDate(iso) {
    const date = new Date(iso);
    return Number.isNaN(date.getTime()) ? null : date;
}

function chatTimeLabel(date) {
    return date ? date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }) : '';
}

function chatDayKey(date) {
    return `${date.getFullYear()}-${date.getMonth()}-${date.getDate()}`;
}

function chatDayLabel(date) {
    const today = new Date();
    if (chatDayKey(date) === chatDayKey(today)) {
        return 'Today';
    }
    return date.toLocaleDateString([], { weekday: 'short', day: 'numeric', month: 'short' });
}

function chatMinutes(date) {
    return date.getHours() * 60 + date.getMinutes();
}

function chatToken() {
    return localStorage.getItem('authToken');
}

async function chatJson(url, options) {
    const settings = options || {};
    const headers = Object.assign({}, settings.headers || {});
    const token = chatToken();
    if (token) {
        headers.Authorization = `Bearer ${token}`;
    }
    let body;
    if (settings.body) {
        headers['Content-Type'] = 'application/json';
        body = JSON.stringify(settings.body);
    }

    let response;
    try {
        response = await fetch(url, {
            method: settings.method || 'GET',
            headers,
            body,
            cache: 'no-store'
        });
    } catch (error) {
        return { ok: false, offline: true, message: 'The chat server is not reachable.' };
    }

    let data = {};
    try {
        data = await response.json();
    } catch (error) {
        data = {};
    }
    return {
        ok: response.ok,
        status: response.status,
        data,
        message: data.error || `The chat answered ${response.status}.`
    };
}

function chatSetHint(text, kind, signIn) {
    chatHint.classList.toggle('is-error', kind === 'error');
    chatHint.replaceChildren();
    chatHint.append(document.createTextNode(text));
    if (signIn) {
        chatHint.append(document.createTextNode(' '));
        const link = document.createElement('a');
        link.href = '/login?next=%2Fchat';
        link.textContent = 'Sign in';
        chatHint.append(link);
    }
}

function chatNotice(text) {
    const note = document.createElement('p');
    note.className = 'chat-note';
    note.textContent = text;
    return note;
}

function chatEmpty() {
    const empty = document.createElement('div');
    empty.className = 'chat-empty';

    const heading = document.createElement('p');
    heading.className = 'chat-empty-title';
    heading.textContent = chatState.channel ? `#${chatState.channel} is quiet` : 'No messages yet';
    empty.append(heading);

    const text = document.createElement('p');
    text.textContent = chatState.canPost
        ? 'Nothing here yet. Start the conversation.'
        : 'Nothing here yet. Sign in to post the first message.';
    empty.append(text);

    return empty;
}

function chatDivider(label) {
    const divider = document.createElement('div');
    divider.className = 'chat-divider';
    const text = document.createElement('span');
    text.textContent = label;
    divider.append(text);
    return divider;
}

function chatBadge(node, count) {
    const unread = Number(count) || 0;
    const existing = node.querySelector('.chat-badge');
    if (unread <= 0) {
        if (existing) {
            existing.remove();
        }
        return;
    }
    if (existing) {
        existing.textContent = unread > 99 ? '99+' : String(unread);
        existing.title = `${unread} unread`;
        return;
    }
    const badge = document.createElement('span');
    badge.className = 'chat-badge';
    badge.textContent = unread > 99 ? '99+' : String(unread);
    badge.title = `${unread} unread`;
    node.append(badge);
}

function chatSyncTitle() {
    if (chatState.unreadTotal > 0) {
        const count = chatState.unreadTotal > 99 ? '99+' : String(chatState.unreadTotal);
        document.title = `(${count}) ${chatBaseTitle}`;
    } else {
        document.title = chatBaseTitle;
    }
}

function chatRefreshBadges() {
    let total = 0;
    for (const server of chatState.servers) {
        const unread = Number(server.unread) || 0;
        total += unread;
        const button = chatGuilds.querySelector(`.chat-guild[data-server="${server.id}"]`);
        if (!button) {
            continue;
        }
        chatBadge(button, unread);
        button.title = server.online > 0
            ? `${server.title} — ${server.online} online`
            : server.title;
    }
    chatState.unreadTotal = total;
    chatSyncTitle();
}

function chatUpdateChannelBadges(channels) {
    for (const channel of channels) {
        const button = channelList.querySelector(`[data-channel="${channel.id}"]`);
        if (button) {
            chatBadge(button, channel.unread);
        }
    }
}

function chatAtBottom() {
    return chatLog.scrollHeight - chatLog.scrollTop - chatLog.clientHeight < 120;
}

function chatScrollToBottom() {
    chatLog.scrollTop = chatLog.scrollHeight;
}

function chatAppendMessage(message) {
    if (message.id <= chatState.lastId) {
        return;
    }

    const when = chatDate(message.at);
    if (when) {
        const day = chatDayKey(when);
        if (day !== chatState.lastDay) {
            chatState.lastDay = day;
            chatState.lastUser = '';
            chatState.lastMinutes = -1;
            const empty = chatLog.querySelector('.chat-empty');
            if (empty) {
                empty.remove();
            }
            chatLog.append(chatDivider(chatDayLabel(when)));
        }
    }

    const minutes = when ? chatMinutes(when) : -1;
    const grouped = message.username === chatState.lastUser &&
        Math.abs(minutes - chatState.lastMinutes) <= 5;

    const row = document.createElement('article');
    row.className = grouped ? 'chat-message is-continuation' : 'chat-message';
    row.append(chatAvatar(message.username));

    const body = document.createElement('div');
    body.className = 'chat-message-body';

    if (!grouped) {
        const head = document.createElement('p');
        head.className = 'chat-message-head';

        const author = document.createElement('span');
        author.className = 'chat-author';
        author.textContent = message.username;
        head.append(author);

        if (message.mine) {
            const tag = document.createElement('span');
            tag.className = 'chat-bot';
            tag.textContent = 'YOU';
            head.append(tag);
        }

        const time = document.createElement('time');
        time.className = 'chat-time';
        time.textContent = chatTimeLabel(when);
        head.append(time);

        body.append(head);
    }

    const text = document.createElement('p');
    text.className = 'chat-message-text';
    text.textContent = message.body;
    body.append(text);

    row.append(body);
    chatLog.append(row);

    chatState.lastUser = message.username;
    chatState.lastMinutes = minutes;
    chatState.lastId = Math.max(chatState.lastId, message.id);
}

function chatRenderLog(messages, reset) {
    if (reset) {
        chatLog.replaceChildren();
        chatState.lastUser = '';
        chatState.lastMinutes = -1;
        chatState.lastDay = '';
    }
    for (const message of messages) {
        chatAppendMessage(message);
    }
    if (reset && messages.length === 0) {
        chatLog.append(chatEmpty());
    }
}

function chatRenderGuilds() {
    chatGuilds.replaceChildren();
    for (const server of chatState.servers) {
        const button = document.createElement('button');
        button.type = 'button';
        button.className = 'chat-guild';
        button.dataset.server = String(server.id);
        button.textContent = chatInitials(server.title);
        button.title = server.online > 0
            ? `${server.title} — ${server.online} online`
            : server.title;
        if (server.id === chatState.gameId) {
            button.classList.add('is-active');
            button.setAttribute('aria-current', 'true');
        }
        button.addEventListener('click', () => {
            if (server.id !== chatState.gameId) {
                chatOpenRoom(server.id, 'general');
            }
        });
        chatGuilds.append(button);
    }
    chatRefreshBadges();
}

function chatRenderChannels() {
    channelList.replaceChildren();
    for (const channel of chatState.channels) {
        const button = document.createElement('button');
        button.type = 'button';
        button.className = 'chat-channel';
        button.dataset.channel = channel.id;

        const hash = document.createElement('span');
        hash.className = 'chat-hash';
        hash.setAttribute('aria-hidden', 'true');
        hash.textContent = '#';

        const name = document.createElement('span');
        name.textContent = channel.title;
        button.append(hash, name);

        if (channel.id === chatState.channel) {
            button.classList.add('is-active');
            button.setAttribute('aria-current', 'true');
        }

        chatBadge(button, channel.unread);

        button.addEventListener('click', () => {
            chatSelectChannel(channel.id);
            if (window.matchMedia('(max-width: 900px)').matches) {
                chatSetChannels(false);
            }
        });
        channelList.append(button);
    }
}

function chatRenderMembers(members) {
    const online = members.filter((member) => member.online);
    const offline = members.filter((member) => !member.online);
    const groups = [
        { label: 'Online', members: online },
        { label: 'Offline', members: offline }
    ];

    memberGroups.replaceChildren();

    for (const group of groups) {
        if (group.members.length === 0) {
            continue;
        }

        const section = document.createElement('section');
        section.className = 'chat-member-group';

        const heading = document.createElement('h2');
        heading.className = 'chat-group-title';
        heading.textContent = `${group.label} — ${group.members.length}`;
        section.append(heading);

        for (const member of group.members) {
            const row = document.createElement('div');
            row.className = member.online ? 'chat-member' : 'chat-member is-offline';
            row.append(chatAvatar(member.username));

            const text = document.createElement('div');
            text.className = 'chat-member-text';

            const name = document.createElement('span');
            name.className = 'chat-member-name';
            name.textContent = member.username;
            text.append(name);

            if (member.id === chatState.userId) {
                const tag = document.createElement('span');
                tag.className = 'chat-bot';
                tag.textContent = 'YOU';
                text.append(tag);
            }

            const role = document.createElement('span');
            role.className = 'chat-member-role';
            role.textContent = member.role || 'Player';
            text.append(role);

            row.append(text);
            section.append(row);
        }

        memberGroups.append(section);
    }
}

function chatRenderHead(game) {
    if (!game) {
        return;
    }
    serverTitle.textContent = game.title;
    const owner = game.owner ? ` · ${game.owner}` : '';
    serverMeta.textContent = `${game.online} online${owner}`;

    const current = chatState.channels.find((channel) => channel.id === chatState.channel);
    channelTitle.textContent = chatState.channel;
    channelTopic.textContent = current ? current.topic : '';
    messageInput.placeholder = `Message #${chatState.channel}`;
}

function chatSyncComposer() {
    const open = chatState.gameId > 0;
    messageInput.disabled = !open || !chatState.canPost;
    sendButton.disabled = !open || !chatState.canPost || chatState.sending;

    if (!open) {
        chatSetHint('Open a game to read its chat.', 'error');
        messageInput.placeholder = 'No room open';
        return;
    }
    if (chatState.canPost) {
        chatSetHint('Enter sends, Shift+Enter starts a new line.');
        messageInput.placeholder = `Message #${chatState.channel}`;
    } else if (!chatState.signedIn) {
        chatSetHint('Reading is open to everyone; posting needs an account.', 'error', true);
        messageInput.placeholder = 'Sign in to send a message';
    } else {
        chatSetHint('Your account is restricted, so posting is closed. Settings has the record.',
            'error');
        messageInput.placeholder = 'Posting is closed for this account';
    }
}

function chatShowUser() {
    const name = chatState.username || 'Guest';
    document.querySelector('[data-user-name]').textContent = name;
    document.querySelector('[data-user-initials]').textContent = chatInitials(name);
    document.querySelector('[data-user-status]').textContent =
        chatState.signedIn ? 'signed in' : 'not signed in';
}

function chatApplyRoom(data, reset) {
    chatState.channel = data.channel || 'general';
    chatState.channels = data.channels || [];
    chatState.maxLength = Number(data.maxMessageLength) || 500;
    messageInput.maxLength = chatState.maxLength;

    const you = data.you || {};
    chatState.signedIn = Boolean(you.signedIn);
    chatState.canPost = Boolean(you.canPost);
    chatState.userId = Number(you.id) || 0;
    chatState.username = you.username || '';

    const known = chatState.servers.find((entry) => entry.id === data.game.id);
    if (known) {
        known.online = Number(data.game.online) || 0;
        known.unread = Number(data.game.unread) || 0;
    }

    chatRenderHead(data.game);
    chatRenderChannels();
    chatRefreshBadges();
    if (reset && data.hidden > 0) {
        chatLog.append(chatNotice(
            `${data.hidden} message${data.hidden === 1 ? '' : 's'} hidden because of a block.`));
    }
    chatRenderLog(data.messages || [], reset);
    chatRenderMembers(data.members || []);
    chatShowUser();
    chatSyncComposer();
    if (reset) {
        chatScrollToBottom();
    }
}

function chatFatal(message) {
    chatState.gameId = 0;
    chatState.canPost = false;
    chatState.unreadTotal = 0;
    chatSyncTitle();
    chatLog.replaceChildren(chatNotice(message));
    memberGroups.replaceChildren();
    channelList.replaceChildren();
    chatGuilds.replaceChildren();
    serverTitle.textContent = 'Chat';
    serverMeta.textContent = 'not connected';
    chatSyncComposer();
    chatShowUser();
    if (chatState.pollTimer) {
        window.clearInterval(chatState.pollTimer);
        chatState.pollTimer = 0;
    }
}

async function chatOpenRoom(gameId, channel) {
    chatState.gameId = gameId;
    chatState.lastId = 0;
    const wanted = channel || 'general';

    const response = await chatJson(
        `/api/chat/${gameId}?channel=${encodeURIComponent(wanted)}&after=0`);
    if (!response.ok) {
        chatFatal(response.offline
            ? 'The chat is served by WebApp.exe, and this page cannot reach it. Start the website and reload.'
            : response.message);
        return;
    }

    chatApplyRoom(response.data, true);
    chatRenderGuilds();
    chatUpdateUrl();
    chatStartPolling();
}

async function chatSelectChannel(channel) {
    if (channel === chatState.channel || chatState.gameId === 0) {
        return;
    }
    await chatOpenRoom(chatState.gameId, channel);
}

function chatUpdateUrl() {
    const url = `/chat?server=${chatState.gameId}&channel=${encodeURIComponent(chatState.channel)}`;
    if (`${window.location.pathname}${window.location.search}` !== url) {
        window.history.replaceState(null, '', url);
    }
}

async function chatPoll() {
    if (chatState.polling || chatState.gameId === 0) {
        return;
    }
    if (document.visibilityState !== 'visible') {
        return;
    }

    chatState.polling = true;
    try {
        const query = new URLSearchParams({
            channel: chatState.channel,
            after: String(chatState.lastId)
        });
        const response = await chatJson(`/api/chat/${chatState.gameId}?${query}`, {});
        if (!response.ok) {
            return;
        }
        const data = response.data;
        if (data.channel !== chatState.channel) {
            return;
        }

        const you = data.you || {};
        chatState.canPost = Boolean(you.canPost);
        chatState.signedIn = Boolean(you.signedIn);
        chatState.username = you.username || chatState.username;
        chatState.userId = Number(you.id) || chatState.userId;

        const atBottom = chatAtBottom();
        chatRenderHead(data.game);
        chatUpdateChannelBadges(data.channels || []);
        chatRenderMembers(data.members || []);
        chatRenderLog(data.messages || [], false);

        chatState.pollCount += 1;
        if (chatState.pollCount % 3 === 0) {
            await chatRefreshServers();
        }
        chatShowUser();
        chatSyncComposer();
        if (atBottom) {
            chatScrollToBottom();
        }
    } finally {
        chatState.polling = false;
    }
}

async function chatRefreshServers() {
    const response = await chatJson('/api/chat', {});
    if (!response.ok) {
        return;
    }
    for (const server of response.data.servers || []) {
        const known = chatState.servers.find((entry) => entry.id === server.id);
        if (known) {
            known.online = Number(server.online) || 0;
            known.unread = Number(server.unread) || 0;
        }
    }
    chatRefreshBadges();
}

function chatStartPolling() {
    if (chatState.pollTimer) {
        window.clearInterval(chatState.pollTimer);
    }
    chatState.pollTimer = window.setInterval(chatPoll, chatPollMs);
}

async function chatSend() {
    const text = messageInput.value.trim();
    if (!text || chatState.sending || !chatState.canPost || chatState.gameId === 0) {
        return;
    }

    chatState.sending = true;
    chatSyncComposer();

    const response = await chatJson(`/api/chat/${chatState.gameId}/messages`, {
        method: 'POST',
        body: { channel: chatState.channel, body: text }
    });

    chatState.sending = false;

    if (!response.ok) {
        chatSetHint(response.message, 'error');
        chatSyncComposer();
        if (!chatState.canPost) {
            messageInput.value = '';
        }
        return;
    }

    messageInput.value = '';
    chatResizeInput();
    chatSyncComposer();
    messageInput.focus();

    if (response.data.message) {
        const atBottom = chatAtBottom();
        chatAppendMessage(response.data.message);
        if (atBottom) {
            chatScrollToBottom();
        }
    } else {
        await chatPoll();
    }
}

function chatResizeInput() {
    messageInput.style.height = 'auto';
    messageInput.style.height = `${Math.min(messageInput.scrollHeight, 180)}px`;
}

function chatSyncBackdrop() {
    const open = chatShell.classList.contains('channels-open') ||
        chatShell.classList.contains('members-open');
    chatBackdrop.hidden = !open;
}

function chatSetChannels(open) {
    chatShell.classList.toggle('channels-open', open);
    channelsToggle.setAttribute('aria-expanded', String(open));
    chatSyncBackdrop();
}

function chatSetMembers(open) {
    chatShell.classList.toggle('members-open', open);
    membersToggle.setAttribute('aria-pressed', String(open));
    chatSyncBackdrop();
}

async function chatLoad() {
    const response = await chatJson('/api/chat', {});
    if (!response.ok) {
        chatFatal(response.offline
            ? 'The chat is served by WebApp.exe, and this page cannot reach it. Start the website and reload.'
            : response.message);
        return;
    }

    chatState.servers = response.data.servers || [];
    chatState.maxLength = Number(response.data.maxMessageLength) || 500;

    if (chatState.servers.length === 0) {
        chatFatal('No public game has been published yet, so there is no room to open. ' +
            'Publish a game and its chat appears here.');
        chatLog.append(chatNotice('Games are created in Studio or on the create page.'));
        return;
    }

    const params = new URLSearchParams(window.location.search);
    const wanted = Number(params.get('server')) || 0;
    const server = chatState.servers.find((entry) => entry.id === wanted) || chatState.servers[0];
    await chatOpenRoom(server.id, params.get('channel') || 'general');
}

document.addEventListener('DOMContentLoaded', () => {
    chatShowUser();
    chatSyncComposer();

    chatComposer.addEventListener('submit', (event) => {
        event.preventDefault();
        chatSend();
    });

    messageInput.addEventListener('keydown', (event) => {
        if (event.key === 'Enter' && !event.shiftKey) {
            event.preventDefault();
            chatComposer.requestSubmit();
        }
    });

    messageInput.addEventListener('input', chatResizeInput);

    channelsToggle.addEventListener('click', () => {
        chatSetChannels(!chatShell.classList.contains('channels-open'));
    });

    membersToggle.addEventListener('click', () => {
        chatSetMembers(!chatShell.classList.contains('members-open'));
    });

    chatBackdrop.addEventListener('click', () => {
        chatSetChannels(false);
        chatSetMembers(false);
    });

    document.addEventListener('keydown', (event) => {
        if (event.key !== 'Escape') {
            return;
        }
        chatSetChannels(false);
        chatSetMembers(false);
    });

    document.addEventListener('visibilitychange', () => {
        if (document.visibilityState === 'visible') {
            chatPoll();
            chatRefreshServers();
        }
    });

    chatLoad();
});
