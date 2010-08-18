function url() {
    return "http://wklej.org";
}

function contentKey() {
    return "body";
}

function setup() {
    provider.addQueryItem("autor", "kde");
    provider.addQueryItem("syntax", "text");
}

function handleResultData(data) {
    var res = provider.parseXML("title", data);
    if (res == "") {
        provider.error(data);
    }
    var id = res.split(" ")[1].replace("#", "http://wklej.org/id/");
    provider.success(id);
}

