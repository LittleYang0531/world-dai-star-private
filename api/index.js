const sonolus = require("sonolus-express");
const express = require("express");
const fs = require("fs");
const { isArray, isObject, isString } = require("util");
const path = require("path");
const tracker = require("ackee-tracker");
const port = 3000;
var app = new express();
var config = JSON.parse(fs.readFileSync(path.join(__dirname, "../config.json")));

ackeeTracker.create('https://ackee.littleyang.me').record('a59ca492-3bc4-44dc-b39e-9dc83751e0c3');

function DFS(json, dep) {
    if (!isObject(json)) {
        if (isString(json)) return [ json ];
        else return json;
    } for (const key in json) {
        if (key == "type") continue;
        json[key] = DFS(json[key], dep + 1);
    } return json;
}

function DFS2(json, dep = 0) {
    if (!isObject(json)) {
        if (isString(json)) return [ json ];
        else return json;
    } for (const key in json) {
        if (key == "url") {
            json[key] = config["config.dataUrlPrefix"] + json["hash"];
            continue;
        } if (key == "type" || key == "name" || key == "url" || key == "hash") continue;
        if (dep > 0 && json[key]["name"] != undefined) { json[key] = json[key]["name"]; continue; }
        json[key] = DFS2(json[key], dep + 1);
    } return json;
}

var data = JSON.parse(fs.readFileSync(path.join(__dirname, "../data.json")));
var search = DFS(JSON.parse(fs.readFileSync(path.join(__dirname, "../search.json"))));
var sonolusApp = new sonolus.Sonolus(app, {
    levels: { search: { options: search["levels"] } },
    skins: { search: { options: search["skins"] } },
    backgrounds: { search: { options: search["backgrounds"] } },
    effects: { search: { options: search["effects"] } },
    particles: { search: { options: search["particles"] } },
    engines: { search: { options: search["engines"] } }
});

sonolusApp.db.info = {
    title: [ config["info.title"] ],
    banner: {
        type: "ServerBanner",
        hash: config["info.bannerHash"],
        url: config["config.dataUrlPrefix"] + config["info.bannerHash"]
    }
}

sonolusApp.db.levels = DFS2(data["levels"]);
sonolusApp.db.skins = DFS2(data["skins"]);
sonolusApp.db.backgrounds = DFS2(data["backgrounds"]);
sonolusApp.db.effects = DFS2(data["effects"]);
sonolusApp.db.particles = DFS2(data["particles"]);
sonolusApp.db.engines = DFS2(data["engines"]);

sonolusApp.levelListHandler = (sonolus, query, page) => {
    var dataSet = sonolus.db.levels, items = [];
    for (var i = 0; i < dataSet.length; i++) {
        var ok = true;
        if (query[0] != '' && dataSet[i]["title"][0].indexOf(query[0]) == -1) ok = false;
        if (query[1] != '' && dataSet[i]["artists"][0].indexOf(query[1]) == -1) ok = false;
        if (query[2] != '' && dataSet[i]["author"][0].indexOf(query[2]) == -1) ok = false;
        if (query[3] != '' && dataSet[i]["rating"] < query[3]) ok = false;
        if (query[4] != '' && dataSet[i]["rating"] > query[4]) ok = false;
        if (ok) items.push(dataSet[i]); 
    }
    var offset = page * config["list.pageNumber"], infos = [];
    for (var i = offset; i < offset + config["list.pageNumber"] && i < items.length; i++) infos.push(items[i]);
    return {
        pageCount: (items.length == 0 ? 0 : Math.round((items.length - 1) / config["list.pageNumber"]) + 1),
        infos: infos
    };
}
sonolusApp.skinListHandler = (sonolus, query, page) => {
    var dataSet = sonolus.db.skins, items = [];
    for (var i = 0; i < dataSet.length; i++) {
        var ok = true;
        if (query[0] != '' && dataSet[i]["title"][0].indexOf(query[0]) == -1) ok = false;
        if (ok) items.push(dataSet[i]); 
    }
    var offset = page * config["list.pageNumber"], infos = [];
    for (var i = offset; i < offset + config["list.pageNumber"] && i < items.length; i++) infos.push(items[i]);
    return {
        pageCount: (items.length == 0 ? 0 : Math.round((items.length - 1) / config["list.pageNumber"]) + 1),
        infos: infos
    };
}
sonolusApp.backgroundListHandler = (sonolus, query, page) => {
    var dataSet = sonolus.db.backgrounds, items = [];
    for (var i = 0; i < dataSet.length; i++) {
        var ok = true;
        if (query[0] != '' && dataSet[i]["title"][0].indexOf(query[0]) == -1) ok = false;
        if (ok) items.push(dataSet[i]); 
    }
    var offset = page * config["list.pageNumber"], infos = [];
    for (var i = offset; i < offset + config["list.pageNumber"] && i < items.length; i++) infos.push(items[i]);
    return {
        pageCount: (items.length == 0 ? 0 : Math.round((items.length - 1) / config["list.pageNumber"]) + 1),
        infos: infos
    };
}
sonolusApp.effectListHandler = (sonolus, query, page) => {
    var dataSet = sonolus.db.effects, items = [];
    for (var i = 0; i < dataSet.length; i++) {
        var ok = true;
        if (query[0] != '' && dataSet[i]["title"][0].indexOf(query[0]) == -1) ok = false;
        if (ok) items.push(dataSet[i]); 
    }
    var offset = page * config["list.pageNumber"], infos = [];
    for (var i = offset; i < offset + config["list.pageNumber"] && i < items.length; i++) infos.push(items[i]);
    return {
        pageCount: (items.length == 0 ? 0 : Math.round((items.length - 1) / config["list.pageNumber"]) + 1),
        infos: infos
    };
}
sonolusApp.particleListHandler = (sonolus, query, page) => {
    var dataSet = sonolus.db.particles, items = [];
    for (var i = 0; i < dataSet.length; i++) {
        var ok = true;
        if (query[0] != '' && dataSet[i]["title"][0].indexOf(query[0]) == -1) ok = false;
        if (ok) items.push(dataSet[i]); 
    }
    var offset = page * config["list.pageNumber"], infos = [];
    for (var i = offset; i < offset + config["list.pageNumber"] && i < items.length; i++) infos.push(items[i]);
    return {
        pageCount: (items.length == 0 ? 0 : Math.round((items.length - 1) / config["list.pageNumber"]) + 1),
        infos: infos
    };
}
sonolusApp.engineListHandler = (sonolus, query, page) => {
    var dataSet = sonolus.db.engines, items = [];
    for (var i = 0; i < dataSet.length; i++) {
        var ok = true;
        if (query[0] != '' && dataSet[i]["title"][0].indexOf(query[0]) == -1) ok = false;
        if (ok) items.push(dataSet[i]); 
    }
    var offset = page * config["list.pageNumber"], infos = [];
    for (var i = offset; i < offset + config["list.pageNumber"] && i < items.length; i++) infos.push(items[i]);
    return {
        pageCount: (items.length == 0 ? 0 : Math.round((items.length - 1) / config["list.pageNumber"]) + 1),
        infos: infos
    };
}

app.use(express.static(path.join(__dirname, "../public/web")));

app.listen(port, () => {
    console.log("Server listening at port", port);
})