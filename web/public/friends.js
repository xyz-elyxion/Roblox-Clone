// The friends page: the roster, the requests waiting on an answer, and a search
// for the next person to add. It reads the same /api/me/social answer the
// dashboard does and draws the cards with the same helper, so the two pages
// cannot disagree about who is a friend or which buttons a relationship has.

(function () {
    const page = document.querySelector('[data-friends-page]');
    if (!page) {
        return;
    }

    const status = page.querySelector('[data-friends-status]');
    const friendsList = page.querySelector('[data-friend-list]');
    const friendsCount = page.querySelector('[data-friend-count]');
    const incomingList = page.querySelector('[data-incoming]');
    const outgoingList = page.querySelector('[data-outgoing]');
    const requestsCount = page.querySelector('[data-request-count]');
    const searchForm = page.querySelector('[data-friend-search]');
    const searchInput = page.querySelector('[data-friend-search-input]');
    const searchStatus = page.querySelector('[data-friend-search-status]');
    const searchResults = page.querySelector('[data-friend-search-results]');

    // Friends need an account; the page says so instead of showing three empty
    // lists that look like a bug.
    function showSignedOut() {
        friendsList.replaceChildren(emptyMessage(
            'Friends belong to an account. Sign in to see the people you added and the requests waiting for you.'));
        incomingList.replaceChildren(emptyMessage('Sign in to see requests.'));
        outgoingList.replaceChildren(emptyMessage('Sign in to send requests.'));
        if (searchForm) {
            searchForm.hidden = true;
        }
        if (friendsCount) {
            friendsCount.textContent = '';
        }
        if (requestsCount) {
            requestsCount.textContent = '';
        }
        setStatus(status, 'Reading this page is open to anyone; the lists need an account.', 'error');
        const link = document.createElement('a');
        link.href = '/login?next=%2Ffriends';
        link.textContent = 'Sign in';
        status.append(document.createTextNode(' '), link);
    }

    async function load() {
        if (!getToken()) {
            showSignedOut();
            return;
        }

        setStatus(status, '', '');
        const { response, data } = await getJson('/api/me/social', getToken());
        if (!response.ok || !data.success) {
            setStatus(status, data.error || 'Could not load friends.', 'error');
            return;
        }

        const friends = data.friends || [];
        const online = friends.filter((friend) => friend.online).length;
        if (friendsCount) {
            friendsCount.textContent = friends.length === 0
                ? 'No friends yet'
                : `${friends.length} friend${friends.length === 1 ? '' : 's'} · ${online} online`;
        }
        if (requestsCount) {
            const waiting = (data.incomingRequests || []).length;
            requestsCount.textContent = waiting === 0 ? '' : `${waiting} waiting`;
        }

        renderUserList(friendsList, friends,
            'No friends yet. Search below to add someone.', load);
        renderUserList(incomingList, data.incomingRequests,
            'No requests waiting.', load);
        renderUserList(outgoingList, data.outgoingRequests,
            'You have not sent a request.', load);
    }

    async function search() {
        if (!getToken()) {
            setStatus(status, 'Sign in to add friends.', 'error');
            return;
        }
        const query = searchInput.value.trim();
        if (!query) {
            return;
        }

        setStatus(searchStatus, '', '');
        searchResults.replaceChildren(emptyMessage('Searching people'));
        const { response, data } = await getJson(
            `/api/users/search?q=${encodeURIComponent(query)}`, getToken());
        if (!response.ok || !data.success) {
            setStatus(searchStatus, data.error || 'Search failed.', 'error');
            return;
        }

        // A card's button changes the relationship, so the search runs again with
        // the roster after it, and the results keep showing where they were.
        renderUserList(searchResults, data.users,
            `No people found for "${query}".`, async () => {
                await load();
                await search();
            });
    }

    if (searchForm) {
        searchForm.addEventListener('submit', (event) => {
            event.preventDefault();
            search().catch((error) => {
                setStatus(searchStatus, error.message || 'Search failed.', 'error');
            });
        });
    }

    load().catch((error) => {
        setStatus(status, error.message || 'Could not load friends.', 'error');
    });
})();
