function url() {
    return "http://imageshack.us/upload_api.php";
}

function contentKey() {
    return "fileupload";
}

function setup() {
    provider.addPostItem("key", "GHNTUVXYcd75a72f93afd1e797ffd9630154cc99", "text/plain");
}

function handleResultData(data) {
    var res = provider.parseXML("image_link", data);
    if (res == "") {
        provider.error(data);
        return;
    }
    provider.success(res);
}
