import bb.cascades 1.4
import "../moment.js" as Moment
import bb.system 1.2
import bb.device 1.4

NavigationPane {
    id: mainnav
    property variant userid
    property variant token
    property variant currentusr
    property variant fileactive
    
    property variant endpoint: "https://slack.com/api/"
    onCreationCompleted: {
        app.memoChanged.connect(infoa)
        app.incomingMessage.connect(incoming)
        var info = app.getAppSettingsByType("theme")
        if (info[0]) {
            if (info[0].value == "Dark") {
                Application.themeSupport.setVisualStyle(VisualStyle.Dark)
            } else {
                Application.themeSupport.setVisualStyle(VisualStyle.Bright)
            }
            var clr1 = app.getAppSettingsByType("color1")
            var clr2 = app.getAppSettingsByType("color2")
            if (clr1.length > 0 && clr2.length > 0)
                Application.themeSupport.setPrimaryColor(Color.create(clr1[0]["value"]), Color.create(clr2[0]["value"]))
        }
        Qt.tstoDate = tstoDate
        Qt.copytoClip = app.copyToClipBoard
        Qt.getChannelById = getChannelById
        Qt.local = app.getLocale()
        Qt.deleteItem = deleteItem
        Qt.filePath = filepathname.media
        Qt.displayError = showerror
        Qt.starItem = starItem
        Qt.formatSizeUnits = formatSizeUnits        
        Qt.editmessage = editmsg
        Qt.tempedit = 0
    }
    function getChannelById(did) {
        return app.getChannelById(did)[0].name
    }
    attachedObjects: [
        SystemToast {
            id: showerror
            position: SystemUiPosition.BottomCenter
        },
        ComponentDefinition {
            id: fileview
            source: "asset:///cards/fileview.qml"
        },
        SystemPrompt {
            id: editmsg
            title: qsTr("Edit message")
            onFinished: {
                if (result == SystemUiResult.ConfirmButtonSelection) {
                    updateItem(inputFieldTextEntry())
                }
            }
            returnKeyAction: SystemUiReturnKeyAction.Submit
        }
    ]

    function incoming(type, text) {
        if (userid == text) {
            if (type == "new") {
                var ts = chathistory.dataModel.data([ chathistory.dataModel.size() - 1 ]).ts.toString()
                chathistory.dataModel.append(app.getMessagesByTs(userid, ts));
                markRead()
            }
        }
        if (type == "delete") {
            for (var i = 0; i < chathistory.dataModel.size(); i ++) {
                if (chathistory.dataModel.data([ i ]).ts == text)
                    chathistory.dataModel.removeAt(i)
            }
        } else if (type == "update") {
            for (var i = 0; i < chathistory.dataModel.size(); i ++) {
                if (chathistory.dataModel.data([ i ]).ts == text)
                    chathistory.dataModel.replace(i, app.getTs(text)[0])
            }
        }
    }
    function infoa(data) {
        var pim = data
        var usrid = pim.split(':')
        userid = usrid[4]
        render(userid)
        var info = app.getActiveAccount()[0]
        console.log(JSON.stringify(app.getChannelById(userid)))
        var userinfo = app.getChannelById(userid)[0]

        channelname.text = userinfo.name
        channelavatar.image = Qt.filePath + userid + ".png"
        currentusr = userinfo
        token = info.value
        checkMute()
        sendinput.requestFocus()
        console.log("checking histotu " + app.checkChannelHistory(userid))
        if (! app.checkChannelHistory(userid)) {
            console.log("getting history ")
            channelname.text = qsTr("Loading Data")
            getHistory()
        }
        markRead()
    }

    function render(idea) {
        chathistory.dataModel.clear()
        chathistory.dataModel.append(app.loadChat(idea, chathistory.dataModel.size()))
        chathistory.scrollToPosition([ ScrollPosition.End ], ScrollAnimation.None)
    }
    Page {
        property variant v_var
        titleBar: TitleBar {
            kind: TitleBarKind.FreeForm
            kindProperties: FreeFormTitleBarKindProperties {
                content: Container {
                    layout: DockLayout {

                    }
                    Container {
                        leftPadding: ui.du(2.0)

                        ImageView {
                            id: channelavatar
                        }
                        Label {
                            id: channelname
//                            text: "Title"
                            textStyle.fontStyle: FontStyle.Default
                            textStyle.fontWeight: FontWeight.W500
                            textStyle.fontSize: FontSize.Large
                            textStyle.color: Color.create("#68b38b")
                        }
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
                                        console.log("riggres")
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
                onTriggered: {
                    var da = dataModel.data(indexPath)
                    if (da.type == "file") {
                        fileactive = dataModel.data(indexPath).id
                        mainnav.push(fileview.createObject())
                    }
                }
                listItemComponents: [
                    ListItemComponent {
                        type: ""
                        Newchatbuuble {
                            username: Qt.getChannelById(ListItemData.user)
                            img: Qt.filePath + ListItemData.user + ".png"
                            message: ListItemData.text
                            time: Qt.tstoDate(ListItemData.ts)
                            fileview: if(ListItemData.type){if(ListItemData.type  == "file") return true}else {return false}
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
                            onActionSetAdded: {
                                console.log("action item")
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
                enabled: false
                input.submitKeyFocusBehavior: SubmitKeyFocusBehavior.Keep
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
                    onSubmitted: {
                        if (sendinput.text) {
                            sendMessage(sendinput.text)
                        }
                    }
                }
                title: qsTr("Send message") + Retranslate.onLanguageChanged
            }
        ]
    }
    function checkMute() {
        var info = app.checkMuteChannels(userid)
        if (info == true) {
            mutetimer.imageSource = "asset:///images/newicons/bellcancel.png"
        } else {
            mutetimer.imageSource = "asset:///images/newicons/bell.png"
        }
    }
    function muteChannel() {
        var info = app.checkMuteChannels(userid)
        console.log("checking mute - " + info)
        if (info == false) {
            app.insertMuteChannels(userid)
        } else {
            app.deleteMuteChannels(userid)
        }

        checkMute()
    }
    function markRead() {
        var lastitem = chathistory.dataModel.value(chathistory.dataModel.size() - 1)
        var doc = new XMLHttpRequest();
        var mode
        switch (currentusr.type) {
            case "channel":
                mode = "channels.mark"
                break
            case "ims":
                mode = "im.mark"
                break
            case "group":
                mode = "groups.mark"
                break

        }
        var url = endpoint + mode + "?token=" + token + "&channel=" + userid + "&ts=" + lastitem.ts
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    chathistory.scrollToPosition([ ScrollPosition.End ], 200)
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
        var url = endpoint + "chat.postMessage?token=" + token + "&channel=" + userid + "&text=" + msg + "&as_user=true"
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    sendinput.text = ""
                    console.log(doc.responseText)
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
        switch (currentusr.type) {
            case "channel":
                mode = "channels.history"
                break
            case "ims":
                mode = "im.history"
                break
            case "group":
                mode = "groups.history"
                break

        }
        var url = endpoint + mode + "?token=" + token + "&channel=" + userid
        console.log("gathering url " + url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var infocac = JSON.parse(doc.responseText)
                    app.deleteHistory(userid)
                    var infocache = infocac.messages
                    for (var i = 0; i < infocache.length; i ++) {
                        var ms = infocache[i]
                        var msg = new Object()
                        msg['channel'] = userid
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
                                    msg['text'] =  qsTr("Commented on") + " "+ ms.file.name + "\n"
                                    msg['text'] += ms.comment.comment
                                    msg['user'] = ms.comment.user
                                    msg['type'] = "file"
                                    msg['id'] = ms.file.id
                                    break
                                case "file_created":
                                    msg['text'] =  qsTr("Created ") + " "+ ms.file.name + "\n"
                                    msg['text'] += ms.comment.comment
                                    msg['user'] = ms.comment.user
                                    msg['type'] = "file"
                                    msg['id'] = ms.file.id
                                    break
                                case "file_share":
                                    msg['text'] =  qsTr("Shared ") + " "+ ms.file.name + "\n"
                                    msg['user'] = ms.user
                                    msg['type'] = "file"
                                    msg['id'] = ms.file.id
                                    break
                                case "pinned_item":
                                    msg['text'] =  qsTr("Pinned Item") + "\n"
                                    if(ms.item_type == "F")
                                        msg['text'] = qsTr("Pinned Item")+ "\n" + ms.item.name
                                    else
                                        msg['text'] += ms.attachments[0].fallback
                                    msg['user'] = ms.user
                                    msg['type'] = "pin"
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
                                var found = menuser[0];
                                var newstr = found.substring(2, found.length - 1);
                                var regname = "@"+ Qt.getChannelById(newstr)
                                var intext = msg.text.replace(reg, regname);
                                msg.text = intext
                                }
                        }
                        app.inserMessaging(msg)
                    }
                    app.setChannelHistory(userid)
                    channelname.text = currentusr.name
                    render(userid)
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
    function tstoDate(ts) {
        var tsr = ts.split('.')
        return Moment.moment.unix(tsr[0]).locale(Qt.local).format('MMM D, h:mm A')
    }
    function deleteItem(ts, id) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "chat.delete?token=" + token + "&channel=" + userid + "&ts=" + ts
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        displayError(qsTr("Item Deleted"))
                    } else {
                        displayError(qsTr("Error Deleting Item"))
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
        var url = endpoint + "chat.update?token=" + token + "&channel=" + userid + "&ts=" + Qt.tempedit.ts + "&text=" + ts
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                    } else {
                        displayError(qsTr("Error Updating Item"))
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
        var url = endpoint + "stars.add?token=" + token + "&channel=" + userid + "&timestamp=" + ts
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        displayError(qsTr("Item Starred"))
                    } else {
                        displayError(qsTr("Error Item Starred"))
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
    function displayError(text) {
        showerror.body = text
        showerror.show()
    }
    function formatSizeUnits(bytes) {
        if (bytes >= 1000000000) {
            bytes = (bytes / 1000000000).toFixed(2) + ' ' + qsTr('GB');
        } else if (bytes >= 1000000) {
            bytes = (bytes / 1000000).toFixed(2) + ' ' + qsTr(' MB');
        } else if (bytes >= 1000) {
            bytes = (bytes / 1000).toFixed(2) + ' ' + qsTr('KB');
        } else if (bytes > 1) {
            bytes = bytes + ' ' + qsTr('bytes');
        } else if (bytes == 1) {
            bytes = bytes + ' ' + qsTr('byte');
        } else {
            bytes = qsTr("0 byte");
        }
        return bytes;
    }
}