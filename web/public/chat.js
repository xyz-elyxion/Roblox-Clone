// A Discord-shaped chat page: one server, four channels, a message log and a
// member list. It is a preview page, so a message stays in this tab instead of
// being posted to an API -- the game client's chat is a separate feature and
// would not be reachable from here even if it were wired up.

const chatChannels = [
    {
        id: 'general',
        title: 'general',
        topic: 'Whatever the current build is doing.',
        messages: [
            {
                author: 'Nova',
                at: '9:41',
                text: 'Pushed a new roof to Sky Platformer. The spawn pads sit on the second ' +
                    'platform now, so nobody falls into the void while the world loads.'
            },
            {
                author: 'Nova',
                at: '9:43',
                text: 'If you joined in the last hour and spawned in the air, that was me. Sorry.'
            },
            {
                author: 'Kit',
                at: '9:58',
                text: 'Took the long way round the lava on purpose and made it to the goal in ' +
                    '38 seconds. The tide is fair.'
            },
            {
                author: 'Patch',
                at: '10:02',
                bot: true,
                text: 'Build 353bb418 is on main: sound, a synthesised bank and workspace:PlaySound. ' +
                    'Instances launched before it will not hear the new clients.'
            },
            {
                author: 'Mira',
                at: '10:14',
                text: 'Anyone free to playtest the doors game after lunch? I want a second pair of ' +
                    'eyes on the wardrobe before it ships.'
            }
        ]
    },
    {
        id: 'world-building',
        title: 'world-building',
        topic: 'Parts, spawns and the things that eat players.',
        messages: [
            {
                author: 'Kit',
                at: '8:52',
                text: 'The glowing blocks are just a neon material and a slightly saturated colour. ' +
                    'Studio shows it correctly, which is more than I expected.'
            },
            {
                author: 'Nova',
                at: '9:05',
                text: 'Keep the moving platforms under 8 studs a second. Above that the physics step ' +
                    'starts dropping players through the top face.'
            },
            {
                author: 'Kit',
                at: '9:07',
                text: 'Noted. Dropped mine to 6 and it stopped happening.'
            },
            {
                author: 'Mira',
                at: '9:30',
                text: 'Doors Hotel needs a second staircase. One exit for a whole floor is a queue, ' +
                    'not a level.'
            }
        ]
    },
    {
        id: 'bug-reports',
        title: 'bug-reports',
        topic: 'One report per message: what you did, what happened, what you expected.',
        messages: [
            {
                author: 'Mira',
                at: '7:48',
                text: 'Stairs at roughly 40, 8, 120 have no collision. Walked straight through them ' +
                    'twice, both times on the way up from the lower platform.'
            },
            {
                author: 'Nova',
                at: '8:02',
                text: 'Reproduced. The steps were saved as a group with zero scale on one axis, so the ' +
                    'collider is a plane. Fixing it in the world, not the engine.'
            },
            {
                author: 'Kit',
                at: '8:20',
                text: 'Separate thing: the chat box keeps the draft after a respawn. Told it was fixed ' +
                    'last week, still there on build 4cb423eb.'
            },
            {
                author: 'Patch',
                at: '8:21',
                bot: true,
                text: 'Reported to the tracker as CHAT-DRAFT-2. Nothing has been changed yet.'
            }
        ]
    },
    {
        id: 'sound-design',
        title: 'sound-design',
        topic: 'Ten synthesised sounds, one mixer, no audio files.',
        messages: [
            {
                author: 'Nova',
                at: '10:20',
                text: 'The bank is generated in code now: footsteps, jump, door, wardrobe, entity, ' +
                    'death, lap, lava, chime, elevator. No files to drift out of step with the build.'
            },
            {
                author: 'Mira',
                at: '10:22',
                text: 'The lava bed is doing its job. It is also the loudest thing in the level by a ' +
                    'wide margin, so I turned it down in the script rather than in the mixer.'
            },
            {
                author: 'Nova',
                at: '10:24',
                text: 'A script calls workspace:PlaySound("lava", part.Position, 0.5). The server ' +
                    'checks the name, so a typo fails on the line that made it.'
            },
            {
                author: 'Kit',
                at: '10:31',
                text: 'Requesting a sound for a door that is being held shut. A slow creak plus the ' +
                    'latch, not just the slam.'
            }
        ]
    }
];

const chatMembers = [
    { name: 'Nova', role: 'Builder', online: true },
    { name: 'Patch', role: 'Bot', online: true, bot: true },
    { name: 'Kit', role: 'Tester', online: true },
    { name: 'Mira', role: 'Level design', online: true },
    { name: 'Juno', role: 'Builder', online: false },
    { name: 'Ravi', role: 'Tester', online: false }
];

const chatShell = document.querySelector('[data-chat-shell]');
const chatLog = document.querySelector('[data-chat-log]');
const channelTitle = document.querySelector('[data-channel-title]');
const channelTopic = document.querySelector('[data-channel-topic]');
const chatComposer = document.querySelector('[data-composer]');
const messageInput = document.querySelector('[data-message-input]');
const channelButtons = Array.from(document.querySelectorAll('[data-channel]'));
const memberGroups = document.querySelector('[data-member-groups]');
const membersToggle = document.querySelector('[data-toggle-members]');
const channelsToggle = document.querySelector('[data-toggle-channels]');
const chatBackdrop = document.querySelector('[data-chat-backdrop]');

const CHAT_PREVIEW_NOTE =
    'Preview page: these messages stay in this tab. The game client keeps its own chat.';

function chatInitials(name) {
    const letters = String(name || '?').replace(/[^a-z0-9]/gi, '');
    return (letters.slice(0, 2) || '?').toUpperCase();
}

// Grouping compares clock minutes, so a message at 23:58 and one at 00:01 stay
// apart rather than counting as three minutes apart by subtraction.
function chatMinutes(at) {
    const [hours, minutes] = String(at).split(':').map(Number);
    return (hours || 0) * 60 + (minutes || 0);
}

function chatAvatar(name) {
    const avatar = document.createElement('span');
    avatar.className = 'chat-avatar';
    avatar.setAttribute('aria-hidden', 'true');
    avatar.textContent = chatInitials(name);
    return avatar;
}

function chatActiveChannel() {
    const active = channelButtons.find((button) => button.classList.contains('is-active'));
    return chatChannels.find((channel) => channel.id === active?.dataset.channel) || chatChannels[0];
}

function chatMessageElement(message, previous) {
    const grouped = previous !== null && previous.author === message.author &&
        Math.abs(chatMinutes(message.at) - chatMinutes(previous.at)) <= 5;

    const row = document.createElement('article');
    row.className = grouped ? 'chat-message is-continuation' : 'chat-message';
    row.append(chatAvatar(message.author));

    const body = document.createElement('div');
    body.className = 'chat-message-body';

    if (!grouped) {
        const head = document.createElement('p');
        head.className = 'chat-message-head';

        const author = document.createElement('span');
        author.className = 'chat-author';
        author.textContent = message.author;
        head.append(author);

        if (message.bot) {
            const tag = document.createElement('span');
            tag.className = 'chat-bot';
            tag.textContent = 'BOT';
            head.append(tag);
        }

        const time = document.createElement('time');
        time.className = 'chat-time';
        time.textContent = message.at;
        head.append(time);

        body.append(head);
    }

    const text = document.createElement('p');
    text.className = 'chat-message-text';
    text.textContent = message.text;
    body.append(text);

    row.append(body);
    return row;
}

function chatDivider(label) {
    const divider = document.createElement('div');
    divider.className = 'chat-divider';
    const text = document.createElement('span');
    text.textContent = label;
    divider.append(text);
    return divider;
}

function chatScrollToBottom() {
    chatLog.scrollTop = chatLog.scrollHeight;
}

function chatRenderLog(channel) {
    chatLog.replaceChildren();

    const note = document.createElement('p');
    note.className = 'chat-note';
    note.textContent = CHAT_PREVIEW_NOTE;
    chatLog.append(note, chatDivider('Today'));

    let previous = null;
    for (const message of channel.messages) {
        chatLog.append(chatMessageElement(message, previous));
        previous = message;
    }

    chatScrollToBottom();
}

function chatAppendMessage(message) {
    const rows = chatLog.querySelectorAll('.chat-message');
    const previousRow = rows[rows.length - 1];
    const previous = previousRow ? previousRow.dataset : null;

    chatLog.append(chatMessageElement(message, previous));
    const last = chatLog.querySelector('.chat-message:last-of-type');
    if (last) {
        last.dataset.author = message.author;
        last.dataset.at = message.at;
    }
    chatScrollToBottom();
}

function chatSelectChannel(id) {
    const channel = chatChannels.find((entry) => entry.id === id) || chatChannels[0];

    for (const button of channelButtons) {
        const active = button.dataset.channel === channel.id;
        button.classList.toggle('is-active', active);
        if (active) {
            button.setAttribute('aria-current', 'true');
        } else {
            button.removeAttribute('aria-current');
        }
    }

    channelTitle.textContent = channel.title;
    channelTopic.textContent = channel.topic;
    messageInput.placeholder = `Message #${channel.title}`;

    chatRenderLog(channel);
}

function chatRenderMembers() {
    const groups = [
        { label: 'Online', members: chatMembers.filter((member) => member.online) },
        { label: 'Offline', members: chatMembers.filter((member) => !member.online) }
    ];

    memberGroups.replaceChildren();

    for (const group of groups) {
        if (group.members.length === 0) continue;

        const section = document.createElement('section');
        section.className = 'chat-member-group';

        const heading = document.createElement('h2');
        heading.className = 'chat-group-title';
        heading.textContent = `${group.label} — ${group.members.length}`;
        section.append(heading);

        for (const member of group.members) {
            const row = document.createElement('div');
            row.className = member.online ? 'chat-member' : 'chat-member is-offline';
            row.append(chatAvatar(member.name));

            const text = document.createElement('div');
            text.className = 'chat-member-text';

            const name = document.createElement('span');
            name.className = 'chat-member-name';
            name.textContent = member.name;
            text.append(name);

            if (member.bot) {
                const tag = document.createElement('span');
                tag.className = 'chat-bot';
                tag.textContent = 'BOT';
                text.append(tag);
            }

            const role = document.createElement('span');
            role.className = 'chat-member-role';
            role.textContent = member.role;
            text.append(role);

            row.append(text);
            section.append(row);
        }

        memberGroups.append(section);
    }
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

function chatResizeInput() {
    messageInput.style.height = 'auto';
    messageInput.style.height = `${Math.min(messageInput.scrollHeight, 180)}px`;
}

function chatSend() {
    const text = messageInput.value.trim();
    if (!text) return;

    const stored = (localStorage.getItem('username') || '').trim();
    const message = {
        author: stored || 'Guest',
        at: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
        text
    };

    chatActiveChannel().messages.push(message);
    chatAppendMessage(message);

    messageInput.value = '';
    chatResizeInput();
    messageInput.focus();
}

function chatShowUser() {
    const stored = (localStorage.getItem('username') || '').trim();
    const name = stored || 'Guest';
    document.querySelector('[data-user-name]').textContent = name;
    document.querySelector('[data-user-initials]').textContent = chatInitials(name);
    document.querySelector('[data-user-status]').textContent = stored ? 'signed in' : 'not signed in';
}

document.addEventListener('DOMContentLoaded', () => {
    chatShowUser();
    chatRenderMembers();
    chatSelectChannel('general');

    for (const button of channelButtons) {
        button.addEventListener('click', () => {
            chatSelectChannel(button.dataset.channel);
            if (window.matchMedia('(max-width: 900px)').matches) {
                chatSetChannels(false);
            }
        });
    }

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
        if (event.key !== 'Escape') return;
        chatSetChannels(false);
        chatSetMembers(false);
    });
});
