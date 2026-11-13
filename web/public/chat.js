// The chat page is the website's own chat: one room per published game, four
// channels in every room, messages kept by WebApp.exe in the data file. This
// file reads a room, polls it while it is open and posts into it. It invents
// nothing -- no seed conversations, no made-up members -- so a page served
// without the backend says so instead of showing a conversation that never
// happened.

const chatBaseTitle = document.title;
const chatPollMs = 4000;
// One attachment per message, the same ceiling the upload route enforces: the
// request body is capped at two mebibytes, so the page says so before sending.
const chatUploadLimit = 2 * 1024 * 1024;

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
const attachButton = chatComposer.querySelector('[data-attach]');
const recordButton = chatComposer.querySelector('[data-record]');
const fileInput = chatComposer.querySelector('[data-file-input]');
const composerTray = chatComposer.querySelector('[data-composer-tray]');
const composerFile = chatComposer.querySelector('[data-composer-file]');
const clearAttachment = chatComposer.querySelector('[data-clear-attachment]');
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
    uploading: false,
    pendingUpload: null,
    recording: false,
    recordedGame: 0,
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
    // Posting refusals are the one thing a reader has to hear straight away; the
    // ordinary reminder of how Enter works waits for a pause.
    chatHint.setAttribute('role', kind === 'error' ? 'alert' : 'status');
    chatHint.setAttribute('aria-live', kind === 'error' ? 'assertive' : 'polite');
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

// Attachments are uploaded first and posted second: the file goes up as a raw
// body with its name in a header, the server answers with an id and its scan
// verdict, and the message that follows cites that id. The page never decides
// whether a file is safe; it shows what the server said about it.
function chatUploadName(name) {
    // The name travels in a header, which is ASCII, and is shown in the room, so
    // anything else is folded to a dash rather than dropped.
    let out = '';
    for (const character of String(name || '')) {
        const code = character.codePointAt(0);
        out += code >= 32 && code < 127 ? character : '-';
    }
    out = out.replace(/["\\]/g, '-').trim().slice(0, 80);
    return out || 'attachment';
}

function chatBytes(count) {
    const size = Number(count) || 0;
    if (size < 1024) {
        return `${size} B`;
    }
    if (size < 1024 * 1024) {
        return `${(size / 1024).toFixed(size < 10 * 1024 ? 1 : 0)} KB`;
    }
    return `${(size / (1024 * 1024)).toFixed(1)} MB`;
}

async function chatUpload(file) {
    if (!file || chatState.gameId === 0) {
        return null;
    }
    if (file.size > chatUploadLimit) {
        chatSetHint(`Files are limited to ${chatBytes(chatUploadLimit)}.`, 'error');
        return null;
    }

    const headers = {
        'Content-Type': 'application/octet-stream',
        'X-Upload-Name': chatUploadName(file.name)
    };
    const token = chatToken();
    if (token) {
        headers.Authorization = `Bearer ${token}`;
    }

    chatState.uploading = true;
    chatSyncComposer();
    chatSetHint(`Uploading ${chatUploadName(file.name)} …`);

    let response;
    try {
        response = await fetch(`/api/games/${chatState.gameId}/uploads`, {
            method: 'POST',
            headers,
            body: file
        });
    } catch (error) {
        chatState.uploading = false;
        chatSyncComposer();
        chatSetHint('The upload could not be sent. Check the connection.', 'error');
        return null;
    }

    let data = {};
    try {
        data = await response.json();
    } catch (error) {
        data = {};
    }
    chatState.uploading = false;
    chatSyncComposer();

    if (!response.ok) {
        // A blocked file comes back with the scanner's name for it, which is the
        // part a reader needs to see: "Trojan.Downloader", not just "refused".
        const label = data.detection ? ` (${data.detection})` : '';
        chatSetHint(`${data.error || 'The file was refused.'}${label}`, 'error');
        return null;
    }

    const upload = data.upload || null;
    if (upload && upload.scan === 'suspicious') {
        chatSetHint(`Kept and labelled: ${upload.name} looked like ` +
            `${upload.detection || 'a mismatched file'}.`, 'error');
    }
    return upload;
}

function chatSyncPending() {
    const upload = chatState.pendingUpload;
    composerTray.hidden = !upload;
    composerFile.textContent = upload
        ? `${upload.name} · ${chatBytes(upload.bytes)} · ${upload.scan === 'suspicious' ? 'flagged' : 'clean'}`
        : '';
    attachButton.disabled = !chatState.canPost || chatState.uploading;
    // The microphone stays live while recording, because it is also the stop
    // button; attaching a second file mid-recording is what gets locked out.
    recordButton.disabled = !chatState.canPost || chatState.uploading;
    attachButton.disabled = !chatState.canPost || chatState.uploading || chatState.recording;
    recordButton.setAttribute('aria-pressed', chatState.recording ? 'true' : 'false');
    recordButton.classList.toggle('is-recording', chatState.recording);
}

// One attachment, drawn from the JSON the server sent: a picture is shown, a clip
// gets a player, anything else is a link to save. The scan verdict is a label, so
// a file the scanner doubted is readable before it is opened.
function chatAttachmentNode(attachment) {
    const wrap = document.createElement('div');
    wrap.className = 'chat-attachment';
    const mime = String(attachment.mime || '');
    const flagged = attachment.scan === 'suspicious';
    if (flagged) {
        wrap.classList.add('is-flagged');
    }

    if (mime.startsWith('image/')) {
        const image = document.createElement('img');
        image.className = 'chat-attachment-image';
        image.src = attachment.url;
        image.alt = attachment.name;
        image.loading = 'lazy';
        wrap.append(image);
    } else if (mime.startsWith('audio/')) {
        const player = document.createElement('audio');
        player.className = 'chat-attachment-audio';
        player.controls = true;
        player.preload = 'none';
        player.src = attachment.url;
        wrap.append(player);
    } else {
        const link = document.createElement('a');
        link.className = 'chat-attachment-file';
        link.href = attachment.url;
        link.rel = 'noopener';
        link.textContent = attachment.name;
        wrap.append(link);
    }

    const meta = document.createElement('span');
    meta.className = 'chat-attachment-meta';
    const size = chatBytes(attachment.bytes);
    const named = mime.startsWith('image/') || mime.startsWith('audio/');
    meta.textContent = named ? `${attachment.name} · ${size}` : size;
    wrap.append(meta);

    if (flagged) {
        const badge = document.createElement('span');
        badge.className = 'chat-scan-badge';
        badge.textContent = `Flagged: ${attachment.detection || 'unknown'}`;
        badge.title = 'The file was kept, but the site scanner was not sure about it.';
        wrap.append(badge);
    }
    return wrap;
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

    if (message.body) {
        const text = document.createElement('p');
        text.className = 'chat-message-text';
        text.textContent = message.body;
        body.append(text);
    }

    if (message.attachment) {
        body.append(chatAttachmentNode(message.attachment));
    }

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
    // The list is rebuilt whenever a room's counts change, so the button a reader
    // is on is remembered by its room and focused again afterwards.
    const focused = document.activeElement;
    const keepFocus = focused && focused.dataset ? focused.dataset.server : null;
    chatGuilds.replaceChildren();
    for (const server of chatState.servers) {
        const button = document.createElement('button');
        button.type = 'button';
        button.className = 'chat-guild';
        button.dataset.server = String(server.id);
        button.textContent = chatInitials(server.title);
        // The initials are a visual tab; the room's name is what a reader needs.
        button.setAttribute('aria-label', server.online > 0
            ? `${server.title} — ${server.online} online`
            : server.title);
        button.title = server.title;
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
    for (const button of chatGuilds.querySelectorAll('[data-server]')) {
        if (keepFocus !== null && button.dataset.server === keepFocus) {
            button.focus();
            break;
        }
    }
    chatRefreshBadges();
}

function chatRenderChannels() {
    // Every poll rebuilds this list; without this the channel the keyboard is on
    // would disappear under it.
    const focused = document.activeElement;
    const keepFocus = focused && focused.dataset ? focused.dataset.channel : null;
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
    for (const button of channelList.querySelectorAll('[data-channel]')) {
        if (keepFocus !== null && button.dataset.channel === keepFocus) {
            button.focus();
            break;
        }
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
    sendButton.disabled = !open || !chatState.canPost || chatState.sending ||
        (!messageInput.value.trim() && !chatState.pendingUpload);
    chatSyncPending();

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
    // A file uploaded for one room is not a file for the next one; the server would
    // refuse the post, so the tray is emptied rather than left holding it.
    chatState.pendingUpload = null;
    chatSyncComposer();
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
    const attachment = chatState.pendingUpload;
    if ((!text && !attachment) || chatState.sending || !chatState.canPost ||
        chatState.gameId === 0) {
        return;
    }

    chatState.sending = true;
    chatSyncComposer();

    const response = await chatJson(`/api/chat/${chatState.gameId}/messages`, {
        method: 'POST',
        body: {
            channel: chatState.channel,
            body: text,
            attachmentId: attachment ? attachment.id : 0
        }
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
    chatState.pendingUpload = null;
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

let chatRecorder = null;
let chatRecordTimer = 0;
let chatRecordChunks = [];

async function chatStartRecording() {
    if (chatState.recording || !chatState.canPost || chatState.gameId === 0) {
        return;
    }
    if (!navigator.mediaDevices || typeof window.MediaRecorder !== 'function') {
        chatSetHint('This browser cannot record audio.', 'error');
        return;
    }

    let stream;
    try {
        stream = await navigator.mediaDevices.getUserMedia({ audio: true });
    } catch (error) {
        chatSetHint('The microphone was refused. The site needs permission to record a clip.',
            'error');
        return;
    }

    // WebM/Opus is what every current browser records and what the upload route
    // accepts. A browser without it still records; the server says what it made of
    // the bytes, and an unsupported container is refused rather than guessed at.
    const preferred = 'audio/webm;codecs=opus';
    const options = window.MediaRecorder.isTypeSupported(preferred)
        ? { mimeType: preferred }
        : undefined;

    chatRecordChunks = [];
    chatState.recordedGame = chatState.gameId;
    chatRecorder = options ? new MediaRecorder(stream, options) : new MediaRecorder(stream);
    chatRecorder.addEventListener('dataavailable', (event) => {
        if (event.data && event.data.size > 0) {
            chatRecordChunks.push(event.data);
        }
    });
    chatRecorder.addEventListener('stop', () => {
        stream.getTracks().forEach((track) => track.stop());
        const recordedGame = chatState.recordedGame;
        const blob = new Blob(chatRecordChunks, { type: 'audio/webm' });
        chatRecordChunks = [];
        chatState.recording = false;
        window.clearTimeout(chatRecordTimer);
        chatSyncComposer();
        if (recordedGame === 0 || recordedGame !== chatState.gameId) {
            chatSetHint('The room changed while recording, so the clip was dropped.');
            return;
        }
        if (blob.size === 0) {
            chatSetHint('The clip was empty, so nothing was sent.', 'error');
            return;
        }
        chatFinishRecording(blob);
    });

    try {
        chatRecorder.start();
    } catch (error) {
        stream.getTracks().forEach((track) => track.stop());
        chatSetHint('This browser could not start recording.', 'error');
        return;
    }
    chatState.recording = true;
    chatSyncComposer();
    chatSetHint('Recording a voice clip. Press Voice again to stop, or it stops by itself at 60 s.');
    chatRecordTimer = window.setTimeout(() => chatStopRecording(), 60000);
}

function chatStopRecording() {
    if (chatRecorder && chatState.recording) {
        chatRecorder.stop();
    }
}

async function chatFinishRecording(blob) {
    const file = new File([blob], `voice-${Date.now()}.webm`, { type: 'audio/webm' });
    const upload = await chatUpload(file);
    if (!upload) {
        return;
    }
    chatState.pendingUpload = upload;
    chatSyncComposer();
    // A clip is a message on its own, so it is posted as soon as it is stored;
    // anything already typed becomes its caption.
    await chatSend();
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

    messageInput.addEventListener('input', () => {
        chatResizeInput();
        chatSyncComposer();
    });

    attachButton.addEventListener('click', () => {
        fileInput.click();
    });

    fileInput.addEventListener('change', async () => {
        const file = fileInput.files && fileInput.files[0];
        fileInput.value = '';
        if (!file) {
            return;
        }
        const upload = await chatUpload(file);
        if (upload) {
            chatState.pendingUpload = upload;
            chatSyncComposer();
            messageInput.focus();
        }
    });

    recordButton.addEventListener('click', () => {
        if (chatState.recording) {
            chatStopRecording();
        } else {
            chatStartRecording();
        }
    });

    clearAttachment.addEventListener('click', () => {
        chatState.pendingUpload = null;
        chatSyncComposer();
    });

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
