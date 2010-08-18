function url() {
    return "http://api.simplest-image-hosting.net/upload:image,default";
}

function contentKey() {
    return "fileName";
}

function setup() {
}

function handleResultData(data) {
    var res = data.match("800\n(http://.+)\n");
    if (res == "") {
        provider.error(data);
        return;
    }
    provider.success(data.replace("800", "").replace("\n", ""));
}
