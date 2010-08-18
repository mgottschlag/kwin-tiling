function url() {
    return "http://paste.opensuse.org";
}

function contentKey() {
    return "code";
}

function setup() {
    provider.addQueryItem("name", "KDE");
    provider.addQueryItem("title", "mypaste");
    provider.addQueryItem("email", "");
    provider.addQueryItem("lang", "text");
    provider.addQueryItem("website", "");
    provider.addQueryItem("submit", "submit");
}

function handleResultData(url) {
    var res = data.match("(Info.+)");
    if (res != "") {
        return;
    }
    provider.error("Error trying to post");
}

function handleRedirection(url) {
    provider.success(url);
}