function url() {
    return "http://imagebin.ca/upload.php";
}

function contentKey() {
    return "f";
}

function setup() {
    provider.addPostItem("t", "file", "text/plain");
    provider.addPostItem("name", "plasmasharedimage", "text/plain");
    provider.addPostItem("tags", "kde", "text/plain");
    provider.addPostItem("adult", "f", "text/plain");
}

function handleResultData(data) {
    var res = data.match("'http://imagebin.ca/view/.+\.html'");
    if (res == null) {
        provider.error(data);
        return;
    }
    provider.success(res[0].replace("'", ""));
}
