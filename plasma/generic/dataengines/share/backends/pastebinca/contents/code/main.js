function url() {
    return "http://pastebin.ca/quiet-paste.php";
}

function contentKey() {
    return "content";
}

function setup() {
    provider.addArgument("api", "yhDgkQ9mSjoOTVrTX4XqP4jRDasxZYXX");
    provider.addArgument("description", "");
    provider.addArgument("type", "1");
    provider.addArgument("expiry", "1%20day");
    provider.addArgument("name", "");
}

function handleResultData(data) {
    if (data.search("SUCCESS") == -1) {
        provider.error(data);
        return;
    }
    provider.success(data.replace("SUCCESS:", "http://pastebin.ca/"));
}
