import bb.cascades 1.4
import bb.system 1.2
import "../cards"
Page {
    property variant id

    function response(data) {
        var incom = JSON.parse(data)
        if (incom.channel == id && incom.type == "message") {
            if (incom.subtype == "message_deleted") {
                for (var i = 0; i < chathistory.dataModel.size(); i ++) {
                    if (chathistory.dataModel.data([ i ]).ts == incom.deleted_ts)
                        chathistory.dataModel.removeAt(i)
                }
            } else if (incom.subtype == "message_changed") {
                for (var i = 0; i < chathistory.dataModel.size(); i ++) {
                    if (chathistory.dataModel.data([ i ]).ts == incom.message.ts)
                        chathistory.dataModel.replace(i, incom.message)
                }
            } else {
                if (incom.subtype == "file_comment") {
                    incom.type = "file"
                    incom.id = incom.file.id
                    incom.user = incom.comment.user
                    incom.text = qsTr("Comment file") + " " + incom.file.name + "\n"
                    incom.text += incom.comment.comment
                } else if (incom.subtype == "file_share") {
                    incom.type = "file"
                    incom.id = incom.file.id
                    incom.user = incom.user
                    incom.text = qsTr("Uploaded file") + " " + incom.file.name
                }
                else if(incom.subtype == "bot_message"){
                    if (incom.attachments){
                        incom.text = incom.attachments[0].fallback
                        incom.user = incom.bot_id
                    }
                } 
                else {
                    var reg = /<@([^>]+)>/g;
                    var menuser = incom.text.match(reg)
                    if (menuser){
                        var foundana = menuser[0];
                        var newstr = foundana.substring(2, foundana.length - 1);
                        var regname = "@" +Qt.mapper[newstr].name;
                        console.log("regnme " + regname)
                        var intext = incom.text.replace(reg, regname);
                        incom.text = intext
                        console.log("regnme " + incom.text)
                    }
                }
                chathistory.dataModel.append(incom)
                chathistory.scrollToPosition([ ScrollPosition.End ], ScrollAnimation.Smooth)
            }
        }
    }
    onCreationCompleted: {
        Qt.deleteItem = deleteItem
        Qt.starItem = starItem
        Qt.editmessage = editmsg
        Qt.tempedit = 0
        app.incomingMsg.connect(response)
        id = useractive.id
        channelname.text = Qt.mapper[useractive.id].name
        if (useractive.mode == "users")
            channelemail.text = Qt.mapper[useractive.usrid].profile.email
        var hist = app.getMsgsById(useractive.id)
        chathistory.dataModel.clear()
        chathistory.dataModel.append(hist)
        chathistory.scrollToItem([ ScrollPosition.End ], ScrollAnimation.None)
        checkMute()
        if (! Qt.mapper[id]["history"]) {
            channelname.text = qsTr("loading History...")
            sendinput.hintText = qsTr("loading History...")
            getHistory()
        } else {
            console.log("Skipped history " + Qt.mapper[id]["history"])
        }
        markRead()
    }

    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            content: Container {
                layout: DockLayout {
                }
                Container {
                    leftPadding: ui.du(2.0)
                    verticalAlignment: VerticalAlignment.Center
                    layout: StackLayout {
                        orientation: LayoutOrientation.TopToBottom

                    }
                    Container {
                        Label {
                            id: channelname
                            text: ""
                            textStyle.color: Application.themeSupport.theme.colorTheme.style == 2 ? Color.create("#c7dfe4") : Color.create("#323232")
                            textStyle.fontSize: FontSize.Medium
                            textStyle.fontWeight: FontWeight.W500
                        }
                    }
                    Container {
                        Label {
                            id: channelemail
                            text: ""
                            textStyle.fontStyle: FontStyle.Italic
                        }
                    }
                    gestureHandlers: [
                        TapHandler {
                            onTapped: {
                                if (useractive.mode == "group" || useractive.mode == "channel")
                                    mainnav.push(channelmembers.createObject())
                            }
                        }
                    ]

                }
                Container {
                    layout: StackLayout {
                    }
                    horizontalAlignment: HorizontalAlignment.Right
                    rightPadding: ui.du(2.0)
                    verticalAlignment: VerticalAlignment.Center
                    ImageView {
                        maxHeight: 80
                        maxWidth: 80
                        id: mutetimer
                        imageSource: "asset:///images/newicons/bell.png"
                        gestureHandlers: [
                            TapHandler {
                                onTapped: {
                                    muteChannel()
                                }
                            }
                        ]
                        filterColor: Application.themeSupport.theme.colorTheme.primary
                        opacity: 1.0
                        horizontalAlignment: HorizontalAlignment.Left
                        verticalAlignment: VerticalAlignment.Top
                        implicitLayoutAnimationsEnabled: false
                    }
                }
            }
        }
        visibility: ChromeVisibility.Default
        branded: TriBool.False
        appearance: TitleBarAppearance.Default
        scrollBehavior: TitleBarScrollBehavior.Sticky
    }

    Container {
        ListView {
            id: chathistory
            dataModel: ArrayDataModel {
            }

            scrollRole: ScrollRole.Main
            attachedObjects: [

                ListScrollStateHandler {
                    id: scrollhnadler
                    onScrollingChanged: {
                        if (scrolling) {
                            sendinput
                        }
                    }
                },
                ComponentDefinition {
                    id: filesvi
                    source: "asset:///fileview.qml"
                }
            ]
            onTriggered: {
                var da = dataModel.data(indexPath)
                if (da.type == "file") {
                    fileactive = dataModel.data(indexPath).id
                    mainnav.push(filesvi.createObject())
                }
            }

            listItemComponents: [
                ListItemComponent {
                    type: ""
                    Newchatbuuble {
                        username: Qt.mapper[ListItemData.user].name
                        img: Qt.mediaPath + ListItemData.user + ".png"
                        message: ListItemData.text
                        time: Qt.tstoDate(ListItemData.ts)
                        fileview: if (ListItemData.type) {
                            if (ListItemData.type == "file") return true
                        } else {
                            return false
                        }

                        contextActions: ActionSet {
                            ActionItem {
                                title: qsTr("Copy")
                                imageSource: "asset:///images/BBicons/ic_copy.png"
                                onTriggered: {
                                    Qt.copytoClip(ListItemData.text)
                                }
                            }
                            ActionItem {
                                title: qsTr("Edit")
                                imageSource: "asset:///images/BBicons/ic_edit.png"
                                onTriggered: {
                                    Qt.tempedit = ListItemData
                                    Qt.editmessage.inputField.defaultText = ListItemData.text
                                    Qt.editmessage.show()
                                }
                            }
                            ActionItem {
                                title: qsTr("Star")
                                imageSource: "asset:///images/BBicons/ic_favorite.png"
                                onTriggered: {
                                    Qt.starItem(ListItemData.ts, ListItemData.channel)
                                }
                            }
                            DeleteActionItem {
                                title: qsTr("Delete")
                                imageSource: "asset:///images/BBicons/ic_delete.png"
                                onTriggered: {
                                    Qt.deleteItem(ListItemData.ts, ListItemData.channel)
                                }
                            }
                        }

                    }
                }
            ]
        }
    }
    actions: [
        TextInputActionItem {
            id: sendinput
            input.submitKey: SubmitKey.Send
            hintText: qsTr("Send Message")
            enabled: true
            input.submitKeyFocusBehavior: SubmitKeyFocusBehavior.Keep
            ActionBar.placement: ActionBarPlacement.Signature
            imageSource: "asset:///images/BBicons/ic_contacts.png"
            title: ""
            input {

                onSubmitted: {
                    if (sendinput.text) {
                        sendMessage(sendinput.text)
                    }
                }
            }
        },
        ActionItem {
            id: sendbutton
            ActionBar.placement: ActionBarPlacement.OnBar
            imageSource: "asset:///images/BBicons/bar_send.png"
            onTriggered: {
                onSubmitted:
                {
                    if (sendinput.text) {
                        sendMessage(sendinput.text)
                    }
                }
            }
            title: qsTr("Send Message") + Retranslate.onLanguageChanged
        }

    ]
    attachedObjects: [
        ComponentDefinition {
            id: channelmembers
            source: "asset:///views/channelmembers.qml"
        },
        SystemPrompt {
            id: editmsg
            title: qsTr("Edit Message")
            onFinished: {
                if (result == SystemUiResult.ConfirmButtonSelection) {
                    updateItem(inputFieldTextEntry())
                }
            }
            returnKeyAction: SystemUiReturnKeyAction.Submit
        }
    ]
    function checkMute() {
        var info = app.checkMuteChannels(id)
        if (info == true) {
            mutetimer.imageSource = "asset:///images/newicons/bellcancel.png"
        } else {
            mutetimer.imageSource = "asset:///images/newicons/bell.png"
        }
    }
    function muteChannel() {
        var info = app.checkMuteChannels(id)
        if (info == false) {
            app.insertMuteChannels(id)

        } else {
            app.deleteMuteChannels(id)
        }
        var ty = Qt.mapper[id]["mode"]
        Qt.updateMute(ty, id)
        checkMute()
    }
    function markRead() {
        var lastitem = chathistory.dataModel.value(chathistory.dataModel.size() - 1)
        var doc = new XMLHttpRequest();
        var mode
        switch (Qt.mapper[id]["mode"]) {
            case "channel":
                mode = "channels.mark"
                break
            case "users":
                mode = "im.mark"
                break
            case "group":
                mode = "groups.mark"
                break
        }
        var url = endpoint + mode + "?token=" + token + "&channel=" + id + "&ts=" + lastitem.ts
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    chathistory.scrollToPosition([ ScrollPosition.End ], ScrollAnimation.None)
                } else {
                    console.log(doc.status + doc.statusText)
                }
            }
        }
        doc.open("GET", url);
        doc.setRequestHeader("Content-Type", "apappplication/json");
        doc.setRequestHeader("User-Agent", "BB10");
        doc.send();
    }
    function sendMessage(msg) {

        var doc = new XMLHttpRequest();
        var url = endpoint + "chat.postMessage?token=" + token + "&channel=" + id + "&text=" + msg + "&as_user=true"
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        sendinput.text = ""
                    } else {
                        Qt.displayError(qsTr("Error while sending message"))
                    }

                } else {
                    console.log(doc.status + doc.statusText)
                }
            }
        }
        doc.open("GET", url);
        doc.setRequestHeader("Content-Type", "apappplication/json");
        doc.setRequestHeader("User-Agent", "BB10");
        doc.send();
    }
    function getHistory() {
        var lastitem = chathistory.dataModel.value(chathistory.dataModel.size() - 1)
        var doc = new XMLHttpRequest();
        var mode
        switch (useractive.mode) {
            case "channel":
                mode = "channels.history"
                break
            case "users":
                mode = "im.history"
                break
            case "group":
                mode = "groups.history"
                break
        }
        var url = endpoint + mode + "?token=" + token + "&channel=" + id
        console.log("history " + url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var infocac = JSON.parse(doc.responseText)
                    if (infocac.ok == true) {
                        app.deleteMsgById(id)
                        var infocache = infocac.messages
                        for (var i = 0; i < infocache.length; i ++) {
                            var ms = infocache[i]
                            var msg = new Object()
                            msg['channel'] = id
                            msg['text'] = ms.text
                            msg['ts'] = ms.ts
                            msg['user'] = ms.user
                            msg['type'] = ""
                            msg["id"] = ""
                            if (ms.bot_id)
                                msg['user'] = ms.bot_id
                            if (ms.subtype) {
                                switch (ms.subtype) {
                                    case "file_comment":
                                        msg['text'] = qsTr("Commented on file") + " " + ms.file.name + "\n"
                                        msg['text'] += ms.comment.comment
                                        msg['user'] = ms.comment.user
                                        msg['type'] = "file"
                                        msg['id'] = ms.file.id
                                        break
                                    case "file_created":
                                        msg['text'] = qsTr("Created a file") + " " + ms.file.name + "\n"
                                        msg['text'] += ms.comment.comment
                                        msg['user'] = ms.comment.user
                                        msg['type'] = "file"
                                        msg['id'] = ms.file.id
                                        break
                                    case "file_share":
                                        msg['text'] = qsTr("Uploaded file") + " " + ms.file.name + "\n"
                                        msg['user'] = ms.user
                                        msg['type'] = "file"
                                        msg['id'] = ms.file.id
                                        break
                                    case "group_join":
                                        msg['user'] = ms.user
                                        var reg = /@([^>]+)\|/g;
                                        var menuser = msg.text.match(reg)
                                        if (menuser && menuser.length > 0)
                                            var intext = msg.text.replace(reg, "");
                                            msg.text = intext
                                        break
                                    case "pinned_item":
                                        msg['text'] = qsTr("Pinned Item") + "\n"
                                        if (ms.item_type == "F")
                                            msg['text'] = qsTr("Pinned Item") + "\n" + ms.item.name;
                                        else
                                            msg['text'] += ms.attachments[0].fallback
                                        msg['user'] = ms.user
                                        msg['type'] = "pin"
                                        console.log(JSON.stringify(ms.ts))
                                        var reg = /<@([^>]+)>/g;
                                        var menuser = msg.text.match(reg)
                                        if (menuser && menuser.length > 0)
                                            var found = menuser[0];
                                            var newstr = found.substring(2, found.length - 1);
                                            console.log(JSON.stringify(newstr))
                                            var regname = "@"+Qt.mapper[newstr].name;
                                            var intext = msg.text.replace(reg, regname);
                                            msg.text = intext
                                        break
                                    case 'bot_message':
                                        if (ms.text)
                                            msg['text'] = ms.text
                                        if (ms.attachments)
                                            msg['text'] = ms.attachments[0].fallback
                                        msg['user'] = ms.bot_id
                                        break
                                }
                            }
                            else{
                                var reg = /<@([^>]+)>/g;
                                var menuser = ms.text.match(reg)
                                if (menuser != null){
                                    var foundas = menuser[0];
                                    var newstr = foundas.substring(2, foundas.length - 1);
                                    var regname = "@"+Qt.mapper[newstr].name;
                                    var intext = msg.text.replace(reg, regname);
                                    msg.text = intext
                                }
                            }
                            app.inserMessaging(msg)
                        }
                        sendinput.setEnabled("true")
                        channelname.text = Qt.mapper[useractive.id].name
                        sendinput.hintText = qsTr("Send Message")
                        Qt.mapper[id]["history"] = true
                        render(id)
                    } else {
                        Qt.displayError(qsTr("Error while retreving history"))
                    }
                } else {
                    console.log(doc.status + doc.statusText)
                }
            }
        }
        doc.open("GET", url);
        doc.setRequestHeader("Content-Type", "apappplication/json");
        doc.setRequestHeader("User-Agent", "BB10");
        doc.send();
    }
    function render(idea) {
        chathistory.dataModel.clear()
        chathistory.dataModel.append(app.getMsgsById(idea))
        chathistory.scrollToPosition([ ScrollPosition.End ], ScrollAnimation.None)
    }
    function deleteItem(ts, id) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "chat.delete?token=" + token + "&channel=" + id + "&ts=" + ts
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        Qt.displayError(qsTr("Item Deleted"))
                    } else {
                        Qt.displayError(qsTr("Error Deleting Item"))
                    }
                } else {
                    console.log(doc.status + doc.statusText)
                }
            }
        }
        doc.open("POST", url);
        doc.setRequestHeader("Content-Type", "apappplication/json");
        doc.setRequestHeader("User-Agent", "BB10");
        doc.send();
    }
    function updateItem(ts) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "chat.update?token=" + token + "&channel=" + id + "&ts=" + Qt.tempedit.ts + "&text=" + ts
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                    } else {
                        Qt.displayError(qsTr("Error Updating Item"))
                    }
                } else {
                    console.log(doc.status + doc.statusText)
                }
            }
        }
        doc.open("POST", url);
        doc.setRequestHeader("Content-Type", "apappplication/json");
        doc.setRequestHeader("User-Agent", "BB10");
        doc.send();
    }
    function starItem(ts, id) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "stars.add?token=" + token + "&channel=" + id + "&timestamp=" + ts
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        Qt.displayError(qsTr("Item Starred"))
                    } else {
                        Qt.displayError(qsTr("Error Item Starred"))
                    }
                } else {
                    console.log(doc.status + doc.statusText)
                }
            }
        }
        doc.open("POST", url);
        doc.setRequestHeader("Content-Type", "apappplication/json");
        doc.setRequestHeader("User-Agent", "BB10");
        doc.send();
    }
}
