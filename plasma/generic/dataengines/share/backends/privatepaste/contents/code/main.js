function url() {
    return "http://privatepaste.com/save";
}

function contentKey() {
    return "paste_content";
}

function setup() {
    provider.addQueryItem("_xsrf", "d38415c699a0408ebfce524c2dc0a4ba");
    provider.addQueryItem("formatting", "No Formatting");
    provider.addQueryItem("line_numbers", "on");
    provider.addQueryItem("expire", "31536000");
    provider.addQueryItem("secure_paste", "off");
}

function handleResultData(url) {
    var res = data.match("(Error.+)");
    if (res != "") {
        return;
    }
    provider.error("Error trying to post");
}

function handleRedirection(url) {
    provider.success(url);
}
