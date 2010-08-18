function url() {
    return "http://pastebin.com/api_public.php";
}

function contentKey() {
    return "paste_code";
}

function setup() {
    provider.addQueryItem("paste_name", "Test");
}

function handleResultData(data) {
    if (data.search("ERROR") != -1) {
        provider.error(data);
        return;
    }
    provider.success(data);
}

