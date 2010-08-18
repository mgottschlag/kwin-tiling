function url() {
    return "http://wstaw.org";
}

function contentKey() {
    return "pic";
}

function setup() {
}

function handleResultData(data) {
    var res = data.match("value=\"http://wstaw.org/m/.+\"");
    if (res == null) {
        provider.error(data);
        return;
    }
    provider.success(res[0].replace("value=", "").replace("\"", ""));
}
