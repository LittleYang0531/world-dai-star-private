auto SonolusCommunityInfo = [](client_conn conn, http_request request, param argv) {
    auto $_GET = getParam(request);
    bool isLogin = checkLogin(request);
    Json::Value ItemDetails;
    ItemDetails["actions"].resize(0);
    bool isLike = isLogin && (db.query("SELECT uid FROM LikeTable WHERE targetType = \"" + string(argv[0]) + "\" AND targetName = \"" + string(argv[1]) + "\" AND uid = \"" + getUserProfile(request).id + "\"").size());
    if (appConfig[string(argv[0]) + ".enableLike"].asBool()) ItemDetails["actions"].append((isLike ? unlikeCommunityObject : likeCommunityObject).toJsonObject());
    if (appConfig[string(argv[0]) + ".enableComment"].asBool()) ItemDetails["actions"].append(commentCommunityObject.toJsonObject());
    if (appConfig[string(argv[0]) + ".enableRating"].asBool()) {
        int defaultRating = 5;
        if (isLogin) {
            auto myRating = db.query("SELECT rating FROM Rating WHERE targetType = \"" + string(argv[0]) + "\" AND targetName = \"" + string(argv[1]) + "\" AND uid = \"" + getUserProfile(request).id + "\"");
            if (myRating.size() != 0) defaultRating = atoi(myRating[0]["rating"].c_str());
        } Search ratingSearch = ratingCommunityObject;
        ratingSearch.options[0].slider.def = defaultRating;
        ItemDetails["actions"].append(ratingSearch.toJsonObject());
    }
    ItemDetails["topComments"].resize(0);
    if (appConfig[string(argv[0]) + ".enableLike"].asBool()) {
        int likes = db.query("SELECT uid FROM LikeTable WHERE targetType = \"" + string(argv[0]) + "\" AND targetName = \"" + string(argv[1]) + "\"").size();
        ItemDetails["topComments"].append(ItemCommunityComment("", "System", 0, "Likes: " + to_string(likes), {}).toJsonObject());
    }
    if (appConfig[string(argv[0]) + ".enableComment"].asBool()) {
        int comments = commentsNumber("targetType = \"" + argv[0] + "\" AND targetName = \"" + argv[1] + "\"");
        ItemDetails["topComments"].append(ItemCommunityComment("", "System", 0, "Comments: " + to_string(comments) + " / " + to_string((comments - 1) / appConfig[argv[0] + ".pageSize.community"].asInt() + 1) + " pages", {}).toJsonObject());
    }
    if (appConfig[string(argv[0]) + ".enableRating"].asBool()) {
        int ratings = db.query("SELECT uid FROM Rating WHERE targetType = \"" + string(argv[0]) + "\" AND targetName = \"" + string(argv[1]) + "\"").size();
        double rating = atof(db.query("SELECT AVG(rating) FROM Rating WHERE targetType = \"" + string(argv[0]) + "\" AND targetName = \"" + string(argv[1]) + "\"")[0]["AVG(rating)"].c_str());
        ItemDetails["topComments"].append(ItemCommunityComment("", "System", 0, "Rating: " + to_string(rating) + "(" + to_string(ratings) + ")", {}).toJsonObject());
    }
    quickSendObj(ItemDetails);
};

string getRef(int len = 32) {
	string res = "", key = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	for (int i = 0; i < len; i++) res += key[rand() % key.size()];
	return res;
}
auto SonolusCommunitySubmit = [](client_conn conn, http_request request, param argv) {
    request.postdata = json_decode(request.postdata)["values"].asString();
    auto $_POST = postParam(request);
    if (!checkLogin(request)) quickSendMsg(401);
    UserProfile profile = getUserProfile(request);
    Json::Value SubmitItemCommunityActionResponse;
    SubmitItemCommunityActionResponse["shouldUpdateCommunity"] = true;
    bool isLike = db.query("SELECT uid FROM LikeTable WHERE targetType = \"" + argv[0] + "\" AND targetName = \"" + argv[1] + "\" AND uid = \"" + profile.id + "\"").size();
    auto myRating = db.query("SELECT rating FROM Rating WHERE targetType = \"" + argv[0] + "\" AND targetName = \"" + argv[1] + "\" AND uid = \"" + profile.id + "\"");
    int rating = myRating.size() == 0 ? 5 : atoi(myRating[0]["rating"].c_str());
    if ($_POST["type"] == "like" && appConfig[argv[0] + ".enableLike"].asBool() && !isLike)
        db.execute("INSERT INTO LikeTable (targetType, targetName, uid) VALUES (\"" + argv[0] + "\", \"" + argv[1] + "\", \"" + profile.id + "\")");
    if ($_POST["type"] == "dislike" && appConfig[argv[0] + ".enableLike"].asBool() && isLike)
        db.execute("DELETE FROM LikeTable WHERE targetType = \"" + argv[0] + "\" AND targetName = \"" + argv[1] + "\" AND uid = \"" + profile.id + "\"");
    if ($_POST["type"] == "comment" && appConfig[argv[0] + ".enableComment"].asBool()) {
        $_POST["content"] = quote_encode(urldecode($_POST["content"]));
        string name = getRef();
        db.execute("INSERT INTO Comment (name, targetType, targetName, uid, time, content) VALUES (\"" + name + "\", \"" + argv[0] + "\", \"" + argv[1] + "\", \"" + profile.id + "\", " + to_string(getMilliSeconds()) + ", \"" + $_POST["content"] + "\")");
        int comments = commentsNumber("targetType = \"" + argv[0] + "\" AND targetName = \"" + argv[1] + "\"");
        SubmitItemCommunityActionResponse["shouldNavigateCommentsToPage"] = (comments - 1) / appConfig[argv[0] + ".pageSize.community"].asInt();
    }
    if ($_POST["type"] == "rating" && appConfig[argv[0] + ".enableRating"].asBool()) {
        int rating2 = $_POST["rating"] == "" ? rating : atoi($_POST["rating"].c_str());
        if (myRating.size() == 0) db.execute("INSERT INTO Rating (targetType, targetName, uid, rating) VALUES (\"" + argv[0] + "\", \"" + argv[1] + "\", \"" + profile.id + "\", \"" + to_string(rating2) + "\")");
        else db.execute("UPDATE Rating SET rating = \"" + to_string(rating2) + "\" WHERE targetType = \"" + argv[0] + "\" AND targetName = \"" + argv[1] + "\" AND uid = \"" + profile.id + "\"");
    }
    if ($_POST["type"].substr(0, 5) == "reply" && appConfig[argv[0] + ".enableRating"].asBool()) {
        string commentName = $_POST["type"].substr(6);
        ItemCommunityComment original = commentsList("name = \"" + commentName + "\"", "time ASC", 1, 1)[0];
        string content = str_replace("\n", "\n> ", "> " + original.content) + "\n\n" + urldecode($_POST["content"]);
        content = quote_encode(content);
        string name = getRef();
        db.execute("INSERT INTO Comment (name, targetType, targetName, uid, time, content) VALUES (\"" + name + "\", \"" + argv[0] + "\", \"" + argv[1] + "\", \"" + profile.id + "\", " + to_string(getMilliSeconds()) + ", \"" + content + "\")");
        int comments = commentsNumber("targetType = \"" + argv[0] + "\" AND targetName = \"" + argv[1] + "\"");
        SubmitItemCommunityActionResponse["shouldNavigateCommentsToPage"] = (comments - 1) / appConfig[argv[0] + ".pageSize.community"].asInt();
    }
    quickSendObj(SubmitItemCommunityActionResponse);
};

auto SonolusCommunity = [](client_conn conn, http_request request, param argv) {
    if (request.method == "GET" || request.method == "get") SonolusCommunityInfo(conn, request, argv);
    else if (request.method == "POST" || request.method == "post") SonolusCommunitySubmit(conn, request, argv);
    else quickSendMsg(405);
};