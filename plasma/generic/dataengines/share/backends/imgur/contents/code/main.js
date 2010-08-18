function url() {
    return "http://imgur.com/api/upload";
}

function contentKey() {
    return "image";
}

function setup() {
    // key associated with plasma-devel@kde.org
    // thanks to Alan Schaaf of Imgur (alan@imgur.com)
    provider.addPostItem("key", "d0757bc2e94a0d4652f28079a0be9379", "text/plain");
}

function handleResultData(data) {
    var res = provider.parseXML("original_image", data);
    if (res == "") {
        provider.error(data);
        return;
    }
    provider.success(res);
}
