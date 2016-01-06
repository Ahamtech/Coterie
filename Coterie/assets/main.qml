import bb.cascades 1.4
import bb.device 1.4
import bb.system 1.2
import bb.platform 1.3

import "moment.js" as Moment
TabbedPane {
    id: mainTab
    property variant self
    property variant teams
    property variant channels
    property variant groups
    property variant users
    property variant bots
    property variant ims
    property variant mainlist
    property variant vpoint: 0
    property variant files
    property variant fav
    property variant token
    property variant stared
    property variant endpoint: "https://slack.com/api/"
    property variant fetchingimages: false
    property variant mediaPath: filepathname.media
    property variant fileactive
    property variant useractive
    property variant theme
    property variant acctype
    property variant primarycolor
    property bool auto_theme_enable

    onCreationCompleted: {

        var infoan = app.getAppSettingsByType("theme")
        if (infoan[0]) {
            if (infoan[0].value == "Dark") {
                Application.themeSupport.setVisualStyle(VisualStyle.Dark)
            } else {
                Application.themeSupport.setVisualStyle(VisualStyle.Bright)
            }
            var clr1 = app.getAppSettingsByType("color1")
            var clr2 = app.getAppSettingsByType("color2")
            if (clr1.length > 0 && clr2.length > 0)
                Application.themeSupport.setPrimaryColor(Color.create(clr1[0]["value"]), Color.create(clr2[0]["value"]))
        }
        var inf = app.getActiveAccount()
        var info = inf[0]
        if (inf.length > 0) {
            loading.open()
            token = info.value
            acctype = info.type
            getActiveSocket(info.value)
        } else {
            login.open()
        }
        app.incomingMsg.connect(response)
        app.reconnectass.connect(recon)
        //        app.avatarDownloadFinish.connect(avatarDone)
        Qt.getUserNameById = getUserNameById
        Qt.getGroupNameById = getGroupNameById
        Qt.mimetoimage = mimetoimage
        Qt.tstoDate = tstoDate
        Qt.staredtoName = staredtoName
        Qt.checkMuteChannels = app.checkMuteChannels
        Qt.mediaPath = mediaPath
        Qt.local = app.getLocale()
        Qt.mapper = new Object()
        Qt.formatSizeUnits = formatSizeUnits
        Qt.copytoClip = app.copyToClipBoard
        Qt.updateMute = updateMute
        Qt.removeStar = removeStar
        Qt.displayError = displayError
        Qt.mainlist = new Array()
        Application.setCover(multiCover)
    }
    function response(data) {
        console.log("Incoming data " + JSON.stringify(data))
        var incoming = JSON.parse(data)
        switch (incoming.type) {
            case "message":
                {
                    if (incoming.subtype == "message_deleted") {
                        app.deleteMsgByTs(incoming.deleted_ts)
                    } else if (incoming.subtype == "message_changed") {
                        app.updateMsgs(incoming.message.text, incoming.message.ts)
                    } else {
                        var type = "in"
                        var unread_count = 0
                        if (incoming.user == self.id) {
                            type = "out"
                            unread_count = 0
                        } else {
                            var chec = app.checkMuteChannels(incoming.user)
                            if (chec == true) {
                            } else {
                                if (acctype != "primary") {
                                    noti.title = Qt.mapper[incoming.channel].name
                                    noti.body = incoming.text
                                    noti.notify()
                                }
                            }
                            unread_count = 1
                        }
                        var intext = ""
                        if (incoming.subtype == "file_comment") {
                            incoming.text = qsTr("Comment file") + " " + incoming.file.name + "\n"
                            incoming.text += incoming.message.comment.comment
                            intext = incoming.text
                            app.insertMsgs(incoming.channel, incoming.comment.user, intext, incoming.ts, "file", incoming.file.id)
                        } else if (incoming.subtype == "file_share") {
                            incoming.text = qsTr("uploaded file") + " " + incoming.file.name
                            intext = incoming.text
                            app.insertMsgs(incoming.channel, incoming.user, intext, incoming.ts, "file", incoming.file.id)
                        } else if (incoming.subtype == "pinned_item") {
                            incoming.text = qsTr("Pinned Item") + "\n"
                            if (incoming.item_type == "F")
                                intext = qsTr("Pinned Item") + "\n" + incoming.item.name;
                            else {
                                att = incoming.attachments[0]
                                intext = incoming.text
                                intext += att.text
                            }
                            app.insertMsgs(incoming.channel, incoming.comment.user, intext, incoming.ts, "pin", "")
                        } else if (incoming.subtype == "bot_message") {
                            if (incoming.text)
                                intext = incoming.text
                            if (incoming.attachments)
                                intext = incoming.attachments[0].fallback
                            app.insertMsgs(incoming.channel, incoming.bot_id, intext, incoming.ts, "", "")
                        } else {
                            intext = incoming.text
                            var reg = /<@([^>]+)>/g;
                            var menuser = incoming.text.match(reg)
                            if (menuser != null) {
                                var found = menuser[0];
                                var newstr = found.substring(2, found.length - 1);
                                var regname = "@" + Qt.mapper[newstr].name;
                                intext = incoming.text.replace(reg, regname);
                            }
                            app.insertMsgs(incoming.channel, incoming.user, intext, incoming.ts, "", "")
                        }
                        for (var i = 0; i < channellist.dataModel.size(); i ++) {
                            if (incoming.channel == channellist.dataModel.data([ i ])['id']) {
                                var asa = channellist.dataModel.data([ i ])
                                asa['latest']['text'] = intext
                                asa["latest"]["ts"] = incoming.ts
                                asa.unread_count += unread_count
                                channellist.dataModel.replace(i, asa)
                                channellist.dataModel.move(i, 0)
                            }
                        }
                        screenmessagebig.text = intext
                        screenmessagesmall.text = intext
                        screentimebig.text = Qt.tstoDate(incoming.ts)
                        screentimesmall.text = Qt.tstoDate(incoming.ts)
                        screentitlebig.text = Qt.mapper[incoming.channel].name
                        screentitlesmall.text = Qt.mapper[incoming.channel].name
                        bigCover.update()
                        smallCover.update()
                    }
                    break

                }
            case "star_added":
                if (incoming.item.type == "message") {
                    staredlist.dataModel.insert(0, (incoming.item))
                }
                checkFavList()
                break
            case "star_removed":
                if (incoming.item.type == "message") {
                    for (var i = 0; staredlist.dataModel.size(); i ++) {
                        if (staredlist.dataModel.data([ i ]).message.ts == incoming.item.message.ts)
                            staredlist.dataModel.removeAt(i)
                    }
                }
                checkFavList()
                break
            case "im_marked":
            case "channel_marked":
            case "group_marked":
                {
                    for (var i = 0; i < channellist.dataModel.size(); i ++) {
                        if (incoming.channel == channellist.dataModel.data([ i ])['id']) {
                            var asa = channellist.dataModel.data([ i ])
                            asa.unread_count = 0
                            channellist.dataModel.replace(i, asa)
                        }
                    }
                }
                break
            case "channel_joined":
            case "group_joined":
                var tea = incoming.channel
                tea.mode = "channel"
                Qt.mapper[tea.id] = tea
                channellist.dataModel.insert(0, tea)
                break
            case "channel_left":
            case "group_left":
            case "channel_archive":
            case "group_archive":
                for (var i = 0; i < channellist.dataModel.size(); i ++) {
                    if (incoming.channel == channellist.dataModel.data([ i ])['id']) {
                        channellist.dataModel.removeAt(i)
                    }
                }
                break

            case "presence_change":
                Qt.mapper[incoming.user].presence = incoming.presence
                //                for (var i = 0; i < imslist.dataModel.size(); i ++) {
                //                    if (incoming.user == imslist.dataModel.data([ i ])['usrid']) {
                //                        var asa = imslist.dataModel.data([ i ])
                //                        imslist.dataModel.replace(i, asa)
                //                    }
                //                }
                break
            case "user_change":
                Qt.mapper[incoming.user.id] = incoming.user
                app.downloadFiles(incoming.users.profile['image_72'], incoming.users.id + ".png")
        }

    }
    function render() {

        removeDeleteUsers()
        removeArchivedGroups()
        removeArchiveChannels()
        removeArchiveIms()
        deletedBots()

        getAvatars()
        filesList()
        getStaredList()
        renderAccounts()
        Qt.mainlist = [];

        for (var i = 0; i < channels.length; i ++) {
            Qt.mainlist.push(channels[i])
        }
        for (var i = 0; i < groups.length; i ++) {
            Qt.mainlist.push(groups[i])
        }
        for (var i = 0; i < ims.length; i ++) {
            Qt.mainlist.push(ims[i])
        }

        Qt.mainlist = sortArray(Qt.mainlist, "ts")
        Qt.mainlist = Qt.mainlist.reverse()

        channellist.dataModel.clear()

        for (var i = 0; i < Qt.mainlist.length; i ++) {
            channellist.dataModel.append(Qt.mainlist[i])
        }

        teamusers.dataModel.clear()
        teamusers.dataModel.append(users)

        acctab.description = teams.domain
        profilename.title = teams.domain

        loading.close()
    }
    function recon() {
        var inf = app.getActiveAccount()
        var info = inf[0]
        token = info.value
        getActiveSocket(info.value)
    }

    function createcolor(data) {
        return Color.create(data)
    }

    Menu.definition: [
        MenuDefinition {
            actions: [
                ActionItem {
                    title: qsTr("Review")
                    imageSource: "asset:///images/BBicons/ic_compose.png"
                    attachedObjects: [
                        Invocation {
                            id: invoke
                            query: InvokeQuery {
                                invokeTargetId: "sys.appworld"
                                uri: "appworld://content/59959969"
                            }
                        }
                    ]
                    onTriggered: {
                        invoke.trigger("bb.action.OPEN")
                    }
                },
                ActionItem {
                    title: qsTr("Settings")
                    imageSource: "asset:///images/iconswhite/settings4.png"
                    onTriggered: {
                        settingspage.open()
                    }
                },
                ActionItem {
                    title: qsTr("About Us")
                    imageSource: "asset:///images/iconswhite/anchor_icon.png"
                    onTriggered: {
                        aboutuspage.open()
                    }
                },
                ActionItem {
                    title: qsTr("Send Feedback")
                    imageSource: "asset:///images/BBicons/ic_feedback.png"
                    onTriggered: {
                        sendfeed.trigger("bb.action.SENDEMAIL")
                    }
                    attachedObjects: [
                        Invocation {
                            id: sendfeed
                            query.mimeType: "text/plain"
                            query.invokeTargetId: "sys.pim.uib.email.hybridcomposer"
                            query.uri: "mailto:coterie@ahamtech.in?subject=Coterie-Feedback"
                        }
                    ]
                }
            ]
        }
    ]
    Tab {
        title: qsTr("Chats")
        imageSource: "asset:///images/iconswhite/Chat_icon.png"
        description: qsTr("")
        NavigationPane {
            id: mainnav
            attachedObjects: [
                ComponentDefinition {
                    id: channel
                    source: "asset:///views/channel.qml"
                }
            ]
            Page {
                titleBar: TitleBar {
                    id: profilename
                    title: " "

                    //                    kind: TitleBarKind.FreeForm
                    //                    kindProperties: FreeFormTitleBarKindProperties {
                    //                        Container {
                    //                            Label {
                    //                                id: profilename
                    //                                text: "profile name"
                    //                            }
                    //                        }
                    //                        Container {
                    //                            layout: StackLayout {
                    //                                orientation: LayoutOrientation.LeftToRight
                    //                            }
                    //                            Container {
                    //                                verticalAlignment: VerticalAlignment.Center
                    //                                leftPadding: ui.du(2)
                    //                                TextField {
                    //                                    id: searchbox
                    //                                    hintText: qsTr("Search")
                    //                                    onTextChanging: {
                    //
                    //                                    }
                    //                                }
                    //                            }
                    //                            Container {
                    //                                verticalAlignment: VerticalAlignment.Center
                    //                                leftPadding: ui.du(2)
                    //                                rightPadding: ui.du(1)
                    //                                ImageView {
                    //                                    imageSource: "asset:///images/BBicons/ic_search.png"
                    //                                    maxHeight: 60
                    //                                    maxWidth: 60
                    //                                    verticalAlignment: VerticalAlignment.Center
                    //                                    gestureHandlers: TapHandler {
                    //                                        onTapped: {
                    //                                            searchbox.text = ""
                    //                                            //                                            searchcontainer.visible = false
                    //                                            //                                            selectionitems.visible = true
                    //                                            //                                            taskslist.visible = true
                    //                                        }
                    //                                    }
                    //
                    //                                }
                    //                            }
                    //                        }
                    //                    }

                }
                Container {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment: VerticalAlignment.Fill
                    Container {
                        layout: DockLayout {
                        }
                        ListView {
                            id: channellist
                            dataModel: ArrayDataModel {
                            }
                            onTriggered: {
                                useractive = dataModel.data(indexPath)
                                mainnav.push(channel.createObject())
                            }
                            listItemComponents: [
                                ListItemComponent {
                                    type: ""
                                    CustomIMchatlist {
                                        title: ListItemData.name
                                        description: if (ListItemData.latest) {
                                            return ListItemData.latest.text
                                        } else {
                                            return (" " )
                                        }
                                        status: Qt.tstoDate(ListItemData.latest.ts)
                                        imageSource: if (ListItemData.unread_count > 0) {
                                            return "asset:///images/icons.png"
                                        } else {
                                            return "asset:///images/newicons/grey.png"
                                        }
                                        unread :if (ListItemData.unread_count > 0) {
                                            return true
                                        } else {
                                            return false 
                                        } 
                                        mute: Qt.checkMuteChannels(ListItemData.id)
                                    }
                                }
                            ]
                            horizontalAlignment: HorizontalAlignment.Center
                            verticalAlignment: VerticalAlignment.Center
                        }
                    }

                }
                actions: [
                    ActionItem {
                        title: qsTr("Create Channel")
                        onTriggered: {
                            newchannel.show()
                        }
                        imageSource: "asset:///images/iconswhite/Add_icon.png"
                    },
                    ActionItem {
                        title: qsTr("Create Group")
                        onTriggered: {
                            newgroup.show()
                        }
                        imageSource: "asset:///images/iconswhite/Add_icon.png"
                    }
                ]
                actionBarAutoHideBehavior: ActionBarAutoHideBehavior.Disabled
                actionBarVisibility: ChromeVisibility.Compact
            }
        }
    }
    Tab {
        id: filestab
        title: qsTr("Files")
        imageSource: "asset:///images/iconswhite/Attachment_icon.png"

        NavigationPane {

            id: filesnav
            attachedObjects: [
                ComponentDefinition {
                    id: filesview
                    source: "asset:///fileview.qml"
                },
                ComponentDefinition {
                    id: addnewfile
                    source: "asset:///views/addattachment.qml"
                }
            ]
            Page {

                titleBar: TitleBar {
                    kind: TitleBarKind.FreeForm
                    kindProperties: FreeFormTitleBarKindProperties {
                        Container {
                            topPadding: 10
                            SegmentedControl {
                                id: filefiltercheck1
                                options: [
                                    Option {
                                        text: qsTr("Everyone")
                                        value: "every"
                                        selected: true

                                    },
                                    Option {
                                        text: qsTr("Just You")
                                        value: "you"

                                    }

                                ]
                                onSelectedValueChanged: {
                                    filesList()
                                }

                            }

                        }
                    }
                }

                actionBarAutoHideBehavior: ActionBarAutoHideBehavior.Default
                actionBarVisibility: ChromeVisibility.Compact
                Container {
                    Container {
                        topPadding: ui.du(2)
                        bottomPadding: ui.du(2)
                        leftPadding: ui.du(2)
                        rightPadding: ui.du(2)

                        DropDown {
                            id: filefiltercheck2
                            options: [
                                Option {
                                    text: qsTr("All Files")
                                    value: "all"
                                    selected: true
                                    imageSource: "asset:///images/newicons/all.png"
                                },
                                Option {
                                    text: qsTr("Posts")
                                    imageSource: "asset:///images/newicons/pos.png"
                                    value: "posts"
                                },
                                Option {
                                    imageSource: "asset:///images/newicons/snp.png"
                                    text: qsTr("Snippets")
                                    value: "snippets"
                                },
                                Option {
                                    imageSource: "asset:///images/newicons/img.png"
                                    text: qsTr("Images")
                                    value: "images"
                                },
                                Option {
                                    imageSource: "asset:///images/newicons/pdf.png"
                                    text: qsTr("PDF Files")
                                    value: "pdf"
                                },
                                Option {
                                    imageSource: "asset:///images/newicons/doc.png"
                                    text: qsTr("Google Docs")
                                    value: "gdocs"
                                },
                                Option {
                                    imageSource: "asset:///images/newicons/zip.png"
                                    text: qsTr("Zip")
                                    value: "zips"
                                }
                            ]
                            onSelectedValueChanged: {
                                filesList()
                            }
                            title: qsTr("Filter")
                        }
                    }
                    Container {
                        Container {
                            visible: true
                            id: fileloading
                            horizontalAlignment: HorizontalAlignment.Fill
                            ActivityIndicator {
                                horizontalAlignment: HorizontalAlignment.Center
                                preferredHeight: 100
                                preferredWidth: 100
                                running: true
                            }
                            Container {
                                horizontalAlignment: HorizontalAlignment.Fill
                                Label {
                                    horizontalAlignment: HorizontalAlignment.Center
                                    text: qsTr("Loading .......")
                                }
                            }

                        }
                        Container {
                            horizontalAlignment: HorizontalAlignment.Center
                            id: fileempty
                            visible: false
                            Container {
                                horizontalAlignment: HorizontalAlignment.Fill
                                ImageView {
                                    horizontalAlignment: HorizontalAlignment.Center
                                    imageSource: "asset:///images/newicons/files.png"
                                }
                            }
                            Label {
                                horizontalAlignment: HorizontalAlignment.Center
                                text: qsTr("No files uploaded  Yet")
                                textStyle.textAlign: TextAlign.Center
                            }
                        }
                        ListView {
                            id: filelist
                            dataModel: ArrayDataModel {
                                onItemAdded: {
                                    fileempty.visible = ! size() > 0
                                }

                            }
                            onTriggered: {
                                fileactive = dataModel.data(indexPath).id
                                filesnav.push(filesview.createObject())
                            }
                            listItemComponents: [
                                ListItemComponent {
                                    type: ""
                                    CustomFileList {
                                        filename: ListItemData.title
                                        username: Qt.getUserNameById(ListItemData.user)
                                        filesize: Qt.formatSizeUnits(ListItemData.size)
                                        time: Moment.moment(ListItemData.timestamp * 1000).locale(Qt.local).format('LLL')
                                        // image: Qt.mediaPath + ListItemData.user + ".png"
                                        priv: ! ListItemData['is_public']
                                        img: Qt.mimetoimage(ListItemData.filetype)
                                    }
                                }
                            ]
                        }

                    }
                }
                actions: [
                    ActionItem {
                        title: qsTr("Upload")
                        ActionBar.placement: ActionBarPlacement.Signature
                        imageSource: "asset:///images/BBicons/ic_attach.png"
                        onTriggered: {
                            filesnav.push(addnewfile.createObject())
                        }
                    }
                ]
            }
        }
    }

    Tab {
        title: qsTr("Starred Items")
        imageSource: "asset:///images/iconswhite/Like_icon.png"
        Page {

            titleBar: TitleBar {
                title: qsTr("Starred Items")
            }
            actionBarVisibility: ChromeVisibility.Compact
            Container {
                horizontalAlignment: HorizontalAlignment.Center

                verticalAlignment: VerticalAlignment.Center
                layout: DockLayout {

                }
                Container {
                    id: starempty
                    visible: false
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment: VerticalAlignment.Center
                    Container {
                        horizontalAlignment: HorizontalAlignment.Fill
                        ImageView {
                            horizontalAlignment: HorizontalAlignment.Center
                            imageSource: "asset:///images/newicons/fav.png"
                        }
                    }
                    Label {
                        text: qsTr("No starred  items  Yet")
                        textStyle.textAlign: TextAlign.Center
                        verticalAlignment: VerticalAlignment.Center
                    }
                }
                ListView {
                    id: staredlist
                    dataModel: ArrayDataModel {

                    }
                    listItemComponents: [
                        ListItemComponent {
                            type: ""
                            SearchResp {
                                imageSource: Qt.mediaPath + ListItemData.message.user + ".png"
                                title: Qt.mapper[ListItemData.channel].name
                                description: ListItemData.message.text
                                status: Qt.tstoDate(ListItemData.message.ts)
                                contextActions: [
                                    ActionSet {
                                        ActionItem {
                                            title: qsTr("Copy")
                                            imageSource: "asset:///images/BBicons/ic_copy.png"
                                            onTriggered: {
                                                Qt.copytoClip(ListItemData.message.text)
                                            }
                                        }
                                        DeleteActionItem {
                                            title: qsTr("Remove From Star")
                                            imageSource: "asset:///images/BBicons/ic_delete.png"
                                            onTriggered: {
                                                Qt.removeStar(ListItemData.message.ts, ListItemData.channel)
                                            }
                                        }
                                    }
                                ]
                            }
                        }
                    ]
                }
            }
        }
    }
    Tab {
        id: serachTab
        title: qsTr("Search")
        imageSource: "asset:///images/BBicons/ic_search.png"
        onTriggered: {
            searchField.requestFocus()
        }
        Page {
            actionBarVisibility: ChromeVisibility.Compact
            titleBar: TitleBar {
                title: qsTr("Search")
            }
            Container {
                layout: DockLayout {

                }
                Container {
                    Container {
                        attachedObjects: [
                            LayoutUpdateHandler {
                                id: searchhandler
                            }
                        ]

                        verticalAlignment: VerticalAlignment.Top
                        leftPadding: ui.du(1.0)
                        rightPadding: ui.du(1.0)
                        bottomPadding: ui.du(1.0)
                        topPadding: ui.du(1.0)
                        TextField {
                            verticalAlignment: VerticalAlignment.Top
                            enabled: true
                            id: searchField
                            hintText: qsTr("Search")
                            input.submitKey: SubmitKey.Search
                            input.onSubmitted: {
                                searchList.dataModel.clear()
                                activityloader.running = true
                                searchResponseText.text = qsTr("Loading")
                                search(text, true)
                            }
                        }

                    }
                    Container {
                        id: searchContainer
                        layout: StackLayout {

                        }
                        horizontalAlignment: HorizontalAlignment.Fill
                        verticalAlignment: VerticalAlignment.Fill

                        Container {
                            ListView {
                                id: searchList
                                dataModel: ArrayDataModel {

                                }
                                function itemType(data, index) {
                                    return data.type
                                }
                                listItemComponents: [
                                    ListItemComponent {
                                        type: "previous"
                                        SearchRespPrev {
                                            description: ListItemData.text
                                            title: ListItemData.username
                                            status: Moment.moment.unix(ListItemData.ts).locale(Qt.local).format('LLL')
                                            imageSource: Qt.mediaPath + ListItemData.user + ".png"
                                            contextActions: ActionSet {
                                                ActionItem {
                                                    title: qsTr("Copy")
                                                    imageSource: "asset:///images/BBicons/ic_copy.png"
                                                    onTriggered: {
                                                        Qt.copytoClip(ListItemData.text)
                                                    }
                                                }
                                            }
                                        }
                                    },
                                    ListItemComponent {
                                        type: "now"
                                        SearchResp {
                                            description: ListItemData.text
                                            title: ListItemData.username
                                            status: Moment.moment.unix(ListItemData.ts).locale(Qt.local).format('LLL')
                                            imageSource: Qt.mediaPath + ListItemData.user + ".png"
                                            contextActions: ActionSet {
                                                ActionItem {
                                                    title: qsTr("Copy")
                                                    imageSource: "asset:///images/BBicons/ic_copy.png"
                                                    onTriggered: {
                                                        Qt.copytoClip(ListItemData.text)
                                                    }
                                                }
                                            }
                                        }
                                    },
                                    ListItemComponent {
                                        type: "next"
                                        SearchRespNext {
                                            description: ListItemData.text
                                            title: ListItemData.username
                                            status: Moment.moment.unix(ListItemData.ts).locale(Qt.local).format('LLL')
                                            imageSource: Qt.mediaPath + ListItemData.user + ".png"
                                            contextActions: ActionSet {
                                                ActionItem {
                                                    title: qsTr("Copy")
                                                    imageSource: "asset:///images/BBicons/ic_copy.png"
                                                    onTriggered: {
                                                        Qt.copytoClip(ListItemData.text)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                ]
                            }
                        }
                    }
                }
                Container {
                    verticalAlignment: VerticalAlignment.Center
                    horizontalAlignment: HorizontalAlignment.Center
                    id: loader
                    Container {
                        horizontalAlignment: HorizontalAlignment.Center
                        ActivityIndicator {
                            horizontalAlignment: HorizontalAlignment.Center
                            id: activityloader
                            running: false
                            preferredHeight: 100
                            preferredWidth: 100

                        }
                    }
                    Container {
                        horizontalAlignment: HorizontalAlignment.Center

                        Label {
                            horizontalAlignment: HorizontalAlignment.Center
                            id: searchResponseText
                            //                    text: "Search here.."
                            verticalAlignment: VerticalAlignment.Center
                        }
                    }

                }

            }
        }
    }
    Tab {
        title: qsTr("Team Directory")
        imageSource: "asset:///images/iconswhite/Contact_book_icon.png"
        Page {

            titleBar: TitleBar {
                title: qsTr("Team Directory")
            }

            actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
            actionBarVisibility: ChromeVisibility.Compact
            ListView {
                id: teamusers
                dataModel: ArrayDataModel {

                }
                listItemComponents: [
                    ListItemComponent {
                        type: ""
                        StandardListItem {
                            title: if (ListItemData.real_name) {
                                return ListItemData.real_name
                            } else {
                                return ListItemData.name
                            }
                            description: ListItemData.profile.title
                            status: ListItemData.profile.email
                            imageSource: Qt.mediaPath + ListItemData.id + ".png"
                            imageSpaceReserved: true
                        }
                    }
                ]
            }
        }
    }
    Tab {
        title: qsTr("Accounts")
        id: acctab
        imageSource: "asset:///images/iconswhite/Lock_icon.png"
        Page {
            titleBar: TitleBar {
                title: qsTr("Switch Accounts")
            }
            actionBarVisibility: ChromeVisibility.Compact
            ListView {
                id: accounts
                dataModel: ArrayDataModel {

                }
                onTriggered: {
                    var i = accounts.dataModel.data(indexPath)
                    acctype = i.type
                    switchAccount(i.value)
                }
                listItemComponents: [
                    ListItemComponent {
                        type: ""
                        CustomSwitchAct {
                            account: ListItemData.account
                            type: ListItemData.type
                            primary: ListItemData.active
                        }
                    }
                ]
            }
            actions: [
                ActionItem {
                    title: qsTr("Add Another Account")
                    onTriggered: {
                        login.open()
                    }
                },
                ActionItem {
                    title: qsTr("Logout")
                    onTriggered: {
                        var acc = app.getActiveAccount()
                        var ina = acc[0]
                        if (ina.type == "primary") {

                        } else {
                            app.logOut(ina.value)
                            var ans = app.getPrimaryAccount()
                            switchAccount(ans[0].value)
                        }
                    }
                }
            ]
        }
    }
    attachedObjects: [
        Notification {
            id: noti
            type: NotificationType.HubOff
            iconUrl: "asset:///grey.png"
        },
        DisplayInfo {
            id: display
        },
        MultiCover {
            id: multiCover
            SceneCover {
                id: bigCover
                // Use this cover when a large cover is required
                MultiCover.level: CoverDetailLevel.High
                content: Container {
                    attachedObjects: [
                        ImagePaintDefinition {
                            id: imagePaint
                            imageSource: "asset:///images/px_by_Gre3g.png"
                            repeatPattern: RepeatPattern.XY
                        }
                    ]
                    background: imagePaint.imagePaint
                    // Your large cover layout
                    Container {
                        verticalAlignment: VerticalAlignment.Top
                        horizontalAlignment: HorizontalAlignment.Left
                        Label {
                            id: screentitlebig
                            text: qsTr("No New Messages")
                            textStyle.fontSizeValue: 10.0
                            textStyle.color: Color.create("#68b38b")
                            textStyle.fontWeight: FontWeight.W500
                        }
                    }
                    Container {

                        verticalAlignment: VerticalAlignment.Top
                        horizontalAlignment: HorizontalAlignment.Right
                        Label {
                            textStyle.color: Color.create("#477b96")

                            id: screentimebig
                        }
                    }
                    Container {

                        horizontalAlignment: HorizontalAlignment.Left
                        Label {
                            id: screenmessagebig
                            textStyle.fontSizeValue: 10.0
                            multiline: true
                            textStyle.color: Color.White
                        }
                    }
                }
                function update() {
                    // Update the large cover dynamically
                }
            } // sceneCover HIGH

            SceneCover {
                id: smallCover
                // Use this cover when a small cover is required
                MultiCover.level: CoverDetailLevel.Medium
                content: Container {
                    attachedObjects: [
                        ImagePaintDefinition {
                            id: imagePaint2
                            imageSource: "asset:///images/px_by_Gre3g.png"
                            repeatPattern: RepeatPattern.XY
                        }
                    ]
                    background: imagePaint2.imagePaint
                    // Your small cover layout
                    Container {
                        verticalAlignment: VerticalAlignment.Top
                        horizontalAlignment: HorizontalAlignment.Left
                        background: Color.Black
                        Label {
                            text: "No New Messages"
                            id: screentitlesmall
                            textStyle.fontWeight: FontWeight.W500
                            textStyle.fontSizeValue: 5.0
                            textStyle.color: Color.create("#68b38b")
                        }
                    }
                    Container {
                        verticalAlignment: VerticalAlignment.Top
                        horizontalAlignment: HorizontalAlignment.Right
                        Label {
                            textStyle.color: Color.create("#477b96")
                            id: screentimesmall
                        }
                    }
                    Container {
                        horizontalAlignment: HorizontalAlignment.Left
                        Label {
                            id: screenmessagesmall
                            textStyle.fontSizeValue: 10.0
                            multiline: true
                            textStyle.color: Color.White
                        }
                    }
                }
                function update() {
                    // Update the small cover dynamically
                }
            } // sceneCover MEDIUM

        },
        Sheet {
            id: loading
            content: Page {
                Container {
                    attachedObjects: [
                        ImagePaintDefinition {
                            id: loadingImagePaint
                            repeatPattern: RepeatPattern.XY
                        }
                    ]
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment: VerticalAlignment.Center
                    layout: DockLayout {

                    }
                    background: loadingImagePaint.imagePaint
                    Container {
                        horizontalAlignment: HorizontalAlignment.Fill
                        verticalAlignment: VerticalAlignment.Center
                        ImageView {
                            preferredHeight: 300
                            preferredWidth: 300

                            onCreationCompleted: {
                                imageani.play()
                            }
                            horizontalAlignment: HorizontalAlignment.Center
                            imageSource: "asset:///images/icons.png"
                            animations: [
                                SequentialAnimation {
                                    onEnded: {
                                        imageani.play()
                                    }
                                    id: imageani

                                    FadeTransition {

                                        duration: 2000
                                        easingCurve: StockCurve.CubicOut
                                        toOpacity: 0.2
                                    }
                                    FadeTransition {

                                        duration: 2000
                                        easingCurve: StockCurve.CubicOut
                                        toOpacity: 1
                                    }
                                }
                            ]
                        }
                    }

                    Label {
                        text: qsTr("Loading ")
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Center
                    }
                }
            }
        },
        Sheet {
            id: login
            onOpened: {
                var uran = "https://slack.com/oauth/authorize?client_id=2826514116.4307473998&redirect_uri=http://localhost&scope=client&state=2334324"
                webWindow.url = uran
            }
            content: Page {
                titleBar: TitleBar {
                    dismissAction: [
                        ActionItem {
                            title: qsTr("Cancel")
                            onTriggered: {
                                if (app.settingsCountByType("primary") > 0) {
                                    login.close()
                                } else {
                                    showerror.body = "Primary account is needed"
                                    showerror.show()
                                }
                            }
                        }
                    ]
                    acceptAction: [
                        ActionItem {
                            title: qsTr("Send to Authorization")
                            onTriggered: {
                                var uran = "https://slack.com/oauth/authorize?client_id=2826514116.4307473998&redirect_uri=http://localhost&scope=client&state=2334324"
                                webWindow.url = uran
                            }
                        }
                    ]
                    title: qsTr("Authorization")
                }
                Container {
                    layout: DockLayout {
                    }
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment: VerticalAlignment.Fill
                    ScrollView {
                        Container {
                            layout: DockLayout {
                            }
                            horizontalAlignment: HorizontalAlignment.Fill
                            verticalAlignment: VerticalAlignment.Center
                            WebView {
                                id: webWindow
                                onUrlChanged: {
                                    console.log("we" + url.toString())
                                    if (url.toString().indexOf("authorize") > 0 || url.toString().indexOf("approve_access") > 0) {
                                    } else {
                                        if (url.toString().indexOf("localhost") > 0) {
                                            var items = url.toString().split("?")
                                            var cod = items[1].split("=")
                                            getInitToken(cod[1])
                                        }
                                    }
                                }
                                onLoadProgressChanged: {
                                    webviewprgoress.value = loadProgress / 100
                                }
                                settings.zoomToFitEnabled: true
                                verticalAlignment: VerticalAlignment.Fill
                                horizontalAlignment: HorizontalAlignment.Fill
                            }
                        }
                    }
                    ProgressIndicator {
                        id: webviewprgoress
                        value: 0.1
                        horizontalAlignment: HorizontalAlignment.Fill
                        verticalAlignment: VerticalAlignment.Bottom
                        onValueChanged: {
                            if (value <= 0.4) {
                                loadtext.textStyle.color = Color.Red
                            }
                            if (value > 0.4) {
                                loadtext.textStyle.color = Color.Yellow
                            }
                            if (value >= 0.7) {
                                loadtext.textStyle.color = Color.Green
                                pageloadindicator.visible = false
                            }
                        }
                    }
                    Container {
                        verticalAlignment: VerticalAlignment.Fill
                        horizontalAlignment: HorizontalAlignment.Fill
                        id: pageloadindicator
                        layout: DockLayout {

                        }
                        Label {
                            id: loadtext
                            textStyle.fontSize: FontSize.Large
                            horizontalAlignment: HorizontalAlignment.Center
                            verticalAlignment: VerticalAlignment.Center
                            text: "Page is loading ......"
                        }
                    }
                }
            }
        },
        Sheet {
            id: settingspage
            content: NavigationPane {
                id: settings
                Page {
                    titleBar: TitleBar {
                        dismissAction: [
                            ActionItem {
                                title: qsTr("Close")
                                onTriggered: {
                                    settingspage.close()
                                }
                            }
                        ]
                        title: qsTr("Settings")
                    }
                    onCreationCompleted: {
                        if (theme == "Dark") {
                            clrtheme.setSelectedIndex(0)
                        }
                        if (theme == "Light") {
                            clrtheme.setSelectedIndex(1)
                        }
                        if (primarycolor) {
                            var optionslist = primaryDropDown.options;
                            for (var i = 0; i < optionslist.length; i ++) {
                                if (primarycolor == optionslist[i].value) {
                                    primaryDropDown.setSelectedIndex(i);
                                }
                            }
                        }
                    }
                    ScrollView {
                        Container {
                            Header {
                                title: qsTr("Select Theme")
                            }
                            Container {
                                leftPadding: ui.du(2)
                                rightPadding: ui.du(2)
                                topPadding: 20.0
                                DropDown {
                                    id: clrtheme
                                    title: qsTr("Set Theme")
                                    Option {
                                        text: qsTr("Dark")
                                        value: "Dark"
                                    }
                                    Option {
                                        text: qsTr("Light")
                                        value: "Light"
                                    }
                                    onSelectedValueChanged: {
                                        app.insertAppSettings("theme", selectedValue)
                                        theme = selectedValue
                                        if (selectedValue == "Dark") {
                                            Application.themeSupport.setVisualStyle(VisualStyle.Dark)
                                        } else if (selectedValue == "Light") {
                                            Application.themeSupport.setVisualStyle(VisualStyle.Bright)
                                        }
                                    }
                                }
                            }
                            Header {
                                title: qsTr("Display") + Retranslate.onLanguageChanged
                            }
                            Container {
                                leftPadding: ui.du(2)
                                topPadding: 20.0
                                rightPadding: ui.du(2)
                                horizontalAlignment: HorizontalAlignment.Fill
                                bottomPadding: 10
                                DropDown {
                                    id: primaryDropDown
                                    title: qsTr("Primary colour") + Retranslate.onLanguageChanged
                                    Option {
                                        text: qsTr("Red") + Retranslate.onLanguageChanged
                                        value: "redtheme"
                                    }
                                    Option {
                                        text: qsTr("Green") + Retranslate.onLanguageChanged
                                        value: "greentheme"
                                    }
                                    Option {
                                        text: qsTr("Yellow") + Retranslate.onLanguageChanged
                                        value: "yellowtheme"
                                    }
                                    Option {
                                        text: qsTr("Blue") + Retranslate.onLanguageChanged
                                        value: "bluetheme"
                                    }
                                    Option {
                                        text: qsTr("Grey") + Retranslate.onLanguageChanged
                                        value: "greytheme"
                                    }
                                    onSelectedValueChanged: {
                                        app.insertAppSettings("primarycolor", selectedValue)
                                        primarycolor = selectedValue;
                                        var prim = primaryDropDown.selectedValue
                                        var themecolor = {
                                            "redtheme": {
                                                base: "#cc3333",
                                                primary: "#ff3333"
                                            },
                                            "greytheme": {
                                                base: "#e6e6e6",
                                                primary: "#f0f0f0"
                                            },
                                            "bluetheme": {
                                                base: "#087099",
                                                primary: "#0092cc"
                                            },
                                            "yellowtheme": {
                                                base: "#b7b327",
                                                primary: "#dcd427"
                                            },
                                            "greentheme": {
                                                base: "#5c7829",
                                                primary: "#779933"
                                            }
                                        }
                                        Application.themeSupport.setPrimaryColor(createcolor(themecolor[prim].base), createcolor(themecolor[prim].primary))
                                        app.insertAppSettings("color1", themecolor[prim].base)
                                        app.insertAppSettings("color2", themecolor[prim].primary)
                                        app.insertAppSettings("primary", prim.toString())
                                    }
                                }
                            }
                            Container {
                                Header {
                                    title: qsTr("Notification And Sounds ") + Retranslate.onLanguageChanged
                                }
                                Container {
                                    leftPadding: ui.du(2)
                                    topPadding: 20.0
                                    rightPadding: ui.du(2)
                                    horizontalAlignment: HorizontalAlignment.Fill
                                    bottomPadding: 10
                                    Button {
                                        text: qsTr("Settings")
                                        onClicked: {
                                            app.invokeSystemSettings()
                                        }
                                        horizontalAlignment: HorizontalAlignment.Fill
                                        verticalAlignment: VerticalAlignment.Center
                                    }
                                }
                            }
                            Container {
                                Header {
                                    title: "Headless"
                                }
                                Container {
                                    leftPadding: ui.du(2)
                                    topPadding: 20.0
                                    rightPadding: ui.du(2)
                                    horizontalAlignment: HorizontalAlignment.Fill
                                    bottomPadding: 10
                                    Label {
                                        text: "Please Stop the application in Device Monitor -> Coterie "
                                        multiline: true
                                    }
                                    Button {
                                        text: qsTr("Open Device Monitor")
                                        onClicked: {
                                            app.headlessStop()
                                        }
                                        horizontalAlignment: HorizontalAlignment.Fill
                                        verticalAlignment: VerticalAlignment.Center
                                    }
                                    Divider {
                                    }
                                    Label {
                                        text: "Headless restart would take 20 seconds to start"
                                    }
                                    Button {
                                        text: qsTr("Re-Start Headless")
                                        onClicked: {
                                            showerror.body = "Headless restart would take 20 seconds to start"
                                            showerror.show()
                                            app.headlessRestart()
                                        }
                                        horizontalAlignment: HorizontalAlignment.Fill
                                        verticalAlignment: VerticalAlignment.Center
                                    }
                                }
                            }
                        }
                    }
                }
            }
        },
        Sheet {
            id: aboutuspage
            content: NavigationPane {
                id: aboutus
                Page {
                    titleBar: TitleBar {
                        dismissAction: [
                            ActionItem {
                                title: qsTr("Close")
                                onTriggered: {
                                    aboutuspage.close()
                                }
                            }
                        ]
                        title: qsTr("A Little About Us")
                    }
                    actionBarVisibility: ChromeVisibility.Compact
                    ScrollView {
                        Container {
                            Container {
                                layout: GridLayout {
                                }
                                horizontalAlignment: HorizontalAlignment.Fill
                                Container {
                                    layout: StackLayout {
                                        orientation: LayoutOrientation.TopToBottom
                                    }
                                    Label {
                                        text: "<html><body><p style='font-size:14'>AHAMTECH</p><p><span style='font-size:10;'>INDIA</span></p></body></html>"
                                        textFormat: TextFormat.Html
                                        multiline: true
                                    }

                                    Label {
                                        textFormat: TextFormat.Html
                                        text: "<html><body><a href='http://www.ahamtech.in'><p>www.ahamtech.in</p></a></body></html>"
                                        //<img src='asset:///images/test/Link.png' height='10' width='10'/>
                                    }
                                }
                                Container {
                                    leftPadding: ui.du(1)
                                    topPadding: ui.du(2)
                                    rightPadding: ui.du(2)
                                    horizontalAlignment: HorizontalAlignment.Right
                                    verticalAlignment: VerticalAlignment.Top
                                    layout: GridLayout {
                                        columnCount: 3
                                    }
                                    ImageView {
                                        imageSource: "asset:///images/qrcode%20(2).png"
                                        horizontalAlignment: HorizontalAlignment.Right
                                        verticalAlignment: VerticalAlignment.Center
                                        scalingMethod: ScalingMethod.AspectFit
                                    }
                                }
                            }
                            Header {
                                title: qsTr("What We Do")
                            }
                            Container {
                                horizontalAlignment: HorizontalAlignment.Center
                                Label {
                                    text: qsTr("Team AHAMTECH stumbles on programming , designing and developing native BlackBerry applications for both public and enterprise.")
                                    multiline: true
                                    textStyle.fontStyle: FontStyle.Italic
                                    horizontalAlignment: HorizontalAlignment.Center
                                    verticalAlignment: VerticalAlignment.Center
                                    autoSize.maxLineCount: 5
                                }
                                Label {
                                    text: "Anchored in ANANTAPUR\r\n           Andhra Pradesh\r\n             make in INDIA"
                                    multiline: true
                                    horizontalAlignment: HorizontalAlignment.Center
                                    verticalAlignment: VerticalAlignment.Top
                                }
                            }
                            Header {
                                title: "About Coterie"
                            }
                            Container {
                                topPadding: ui.du(2)
                                layout: StackLayout {
                                    orientation: LayoutOrientation.LeftToRight
                                }
                                ImageView {
                                    layoutProperties: StackLayoutProperties {
                                        spaceQuota: -1.0
                                    }
                                    imageSource: "asset:///images/icons.png"
                                    maxHeight: 100
                                    maxWidth: 100
                                    horizontalAlignment: HorizontalAlignment.Center
                                    scalingMethod: ScalingMethod.AspectFill
                                }
                                Label {
                                    layoutProperties: StackLayoutProperties {
                                        spaceQuota: 0.1
                                    }
                                    text: "Coterie is a third part for 'Slack'  also a pure native app for BlackBerry 10, we request you to send us any suggestions, feedback and improvements required."
                                    multiline: true
                                    textStyle.fontStyle: FontStyle.Italic
                                }
                            }
                            Container {
                                leftPadding: ui.du(2.0)
                                rightPadding: ui.du(2.0)
                                Label {
                                    text: "We will try our level best to update the app depending on API support and productivity for the users."
                                    multiline: true
                                    textStyle.fontStyle: FontStyle.Italic
                                }
                                Label {
                                    text: "Some of the features may not be available due to API limitations and coding challenges we are facing, future updates depend on user base and responses we get. So please feel free to send us your comments and suggestions."
                                    multiline: true
                                    textStyle.fontStyle: FontStyle.Italic
                                }
                                Button {
                                    preferredWidth: ui.du(30)
                                    text: qsTr("Send Feedback") + Retranslate.onLanguageChanged
                                    horizontalAlignment: HorizontalAlignment.Center
                                    onClicked: {
                                        translate.trigger("bb.action.SENDEMAIL") + Retranslate.onLanguageChanged
                                    }
                                    color: Color.create("#f1cd31")
                                    imageSource: "asset:///images/BBicons/ic_feedback.png"
                                }
                            }
                            Header {
                                title: qsTr("Under The Hood")
                            }
                            Label {
                                text: "<html><body><span><b>        Lead Programmer</b></span><span><i>    Hari Kishan Reddy</i></span><br/>
                                <span><b>Fellow Programmer</b></span><span><i>    John Ankanna</i></span>
                                <span><b>UI Designer</b></span><span><i>    Sandeep Eranti</i></span>
                                <span><b>Front End Programmer</b></span><span><i>    Mahesh Reddy</i></span></body></html>"
                                textFormat: TextFormat.Html
                                multiline: true
                            }
                            Header {

                            }
                        }
                        attachedObjects: [
                            Invocation {
                                id: translate
                                query.mimeType: "text/plain"
                                query.invokeTargetId: "sys.pim.uib.email.hybridcomposer"
                                query.uri: "mailto:coterie@ahamtech.in?subject=Coterie-Feedback"
                            }
                        ]
                    }
                }
            }
        },

        SystemPrompt {
            id: newchannel
            title: qsTr("Create New Channel")
            rememberMeChecked: false
            includeRememberMe: false
            confirmButton.enabled: true
            cancelButton.enabled: true
            onFinished: {
                if (result == SystemUiResult.ConfirmButtonSelection) {
                    createChannel(inputFieldTextEntry())
                }
            }
        },
        SystemPrompt {
            id: newgroup
            title: qsTr("Create New Groups")
            rememberMeChecked: false
            includeRememberMe: false
            confirmButton.enabled: true
            cancelButton.enabled: true
            onFinished: {
                if (result == SystemUiResult.ConfirmButtonSelection) {
                    createGroups(inputFieldTextEntry())
                }
            }
        },
        SystemToast {
            id: showerror
            position: SystemUiPosition.BottomCenter
        },
        Screenshot {
            id: screenshot
        }
    ]
    showTabsOnActionBar: false
    activeTab: Tab {

    }

    function getInitToken(code) {
        var doc = new XMLHttpRequest();
        var getkeylink = "https://slack.com/api/oauth.access?client_id=2826514116.4307473998&redirect_uri=http://localhost&client_secret=df34a61c571b9ea71fd29cd76882ceb5&scope=&scope=client&code=" + code
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var desk = JSON.parse(doc.responseText)
                    showerror.body = qsTr("New Account is added")
                    showerror.show()
                    if (app.settingsCountByType("primary") > 0) {
                        app.insertSettings("secondary", desk.access_token)
                    } else {
                        app.insertSettings("primary", desk.access_token)
                        app.activePrimary()
                    }
                    token = desk.access_token
                    getActiveSocket(desk.access_token)
                    login.close()
                } else {
                }
            }
        }
        doc.open("Get", getkeylink);
        doc.setRequestHeader("Content-Type", "application/json");
        doc.setRequestHeader("User-Agent", "BB10/RB Client");
        doc.setRequestHeader("Content-Encoding", "UTF-8");
        doc.send();
    }
    function getActiveSocket(token) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "rtm.start?token=" + token
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var data = JSON.parse(doc.responseText)
                    if (data.ok == true) {
                        showerror.body = qsTr("Connected To Server")
                        showerror.show()
                        app.updateSettings(token, data.team.domain, true)
                        console.log(data.url)
                        app.connectSocket(data.url)
                        teams = data.team
                        self = data.self
                        channels = data.channels
                        groups = data.groups
                        users = data.users
                        bots = data.bots
                        ims = data.ims
                        render()
                    } else {
                        showerror.body = qsTr("Error while Connecting to server")
                        showerror.show()
                    }
                } else {
                    console.log(doc.status + doc.statusText)
                }
            }
        }
        doc.open("GET", url);
        doc.setRequestHeader("Content-Type", "apappplication/json");
        doc.setRequestHeader("User-Agent", "BB10");
        doc.setRequestHeader("Content-Encoding", "UTF-8");
        doc.send();
    }
    function removeDeleteUsers() {
        var usr = new Array()
        var map = []
        for (var i = 0; i < users.length; i ++) {
            if (! users[i].deleted) {
                var ura = users[i]
                ura.mode = "users"
                Qt.mapper[ura.id] = ura
                usr.push(ura)
            }
        }
        users = usr
    }
    function removeArchiveChannels() {
        var usr = new Array()
        var dat = new Array()
        for (var i = 0; i < channels.length; i ++) {
            if (channels[i]["is_archived"] == false && channels[i]["is_member"] == true) {
                var tea = channels[i]
                tea.mode = "channel"
                Qt.mapper[tea.id] = tea
                tea['ts'] = tea['latest']['ts'].split(".")[0]
                usr.push(tea)
            }
        }
        channels = usr
    }
    function removeArchivedGroups() {
        var usr = new Array()
        for (var i = 0; i < groups.length; i ++) {
            if (! groups[i]["is_archived"]) {
                console.log(JSON.stringify(groups[i]))
                var ura = groups[i]
                ura.mode = "group"
                Qt.mapper[ura.id] = ura
                if (ura['latest'])
                    ura['ts'] = ura['latest']['ts'].split(".")[0]
                usr.push(ura)
            }
        }
        groups = usr
    }
    function removeArchiveIms() {
        var usr = new Array()
        for (var i = 0; i < ims.length; i ++) {
            if (ims[i]["is_open"] == true) {
                var ura = ims[i]
                if (Qt.mapper[ura.user]) {
                    ura['name'] = Qt.mapper[ura.user].name
                    if (Qt.mapper[ura.user].real_name)
                        ura['name'] = Qt.mapper[ura.user].real_name
                    ura['usrid'] = Qt.mapper[ura.user].id
                    if (ura['latest'])
                        ura['ts'] = ura['latest']['ts'].split('.')[0];
                    else
                        ura['ts'] = i
                    ura['mode'] = "users"
                    Qt.mapper[ura.id] = ura
                    usr.push(ura)
                }
            }
        }
        ims = null
        ims = usr
    }
    function deletedBots() {
        var usr = new Array()
        for (var i = 0; i < bots.length; i ++) {
            if (bots[i].deleted == false) {
                var ura = bots[i]
                ura['mode'] = "bots"
                Qt.mapper[ura.id] = ura
                usr.push(ura)
            }
        }
        bots = usr
    }
    function getUserNameById(userid) {
        var filename
        for (var i = 0; i < users.length; i ++) {
            if (users[i].id == userid) {
                if (users[i].real_name != "") {
                    return users[i].real_name
                } else {
                    return users[i].name
                }
            }
        }
    }
    function getGroupNameById(grpid) {
        for (var i = 0; i < groups.length; i ++) {
            if (groups[i].id == grpid) {
                return groups[i].name
            }
        }
    }
    function createChannel(text) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "channels.create?token=" + token + "&name=" + text
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == false) {
                        displayError(info.error)
                    } else {
                        displayError(qsTr("New Channel is created"))
                        channels.push(info)
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
    function createGroups(text) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "groups.create?token=" + token + "&name=" + text
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == false) {
                        displayError(info.error)
                    } else {
                        displayError(qsTr("New Channel is created"))
                        channels.push(info)
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
    function archiveChannel(text) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "channels.archive?token=" + token + "&channel=" + text
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == false) {
                        displayError(info.error)
                    } else {

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
    function filesList() {
        fileloading.visible = true
        filelist.enabled = false
        var doc = new XMLHttpRequest();
        var url = endpoint + "files.list?token=" + token
        if (filefiltercheck1.selectedValue == "you") {
            url += "&user=" + self.id
        }
        url += "&types=" + filefiltercheck2.selectedValue
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        files = info.files
                        filelist.dataModel.clear()
                        filelist.dataModel.append(info.files)
                        fileempty.visible = ! filelist.dataModel.size() > 0
                    } else {
                        displayError(qsTr("Unable to access files"))
                    }
                } else {
                    console.log(doc.status + doc.statusText)
                }
                fileloading.visible = false
                filelist.enabled = true
            }
        }
        doc.open("GET", url);
        doc.setRequestHeader("Content-Type", "apappplication/json");
        doc.setRequestHeader("User-Agent", "BB10");
        doc.send();
    }

    function search(text, highlight) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "search.messages?token=" + token + "&query=" + text
        if (highlight == true) {
            url += "&highlight=true"
        }
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == false) {
                        displayError(info.error)
                    } else {
                        searchList.dataModel.clear()
                        for (var i = 0; i < info.messages.matches.length; i ++) {
                            if (info.messages.matches[i].previous) {
                                var prev = info.messages.matches[i].previous
                                prev.type = "previous"
                                searchList.dataModel.append(prev)

                            }
                            var now = info.messages.matches[i]
                            now.type = "now"
                            searchList.dataModel.append(now)
                            if (info.messages.matches[i].next) {
                                var prev = info.messages.matches[i].next
                                prev.type = "next"
                                searchList.dataModel.append(prev)
                            }
                        }
                        if (searchList.dataModel.size() > 0) {
                            searchContainer.visible = true
                            loader.visible = false
                        } else {
                            activityloader.running = false
                            loader.visible = true
                            searchContainer.visible = false
                            searchResponseText.text = "No Matching Results Found"
                        }
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

    function getStaredList() {
        var doc = new XMLHttpRequest();
        var url = endpoint + "stars.list?token=" + token + "&user=" + self.id
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    var ata = new Array()
                    var ana = new Array()
                    for (var i = 0; i < info.items.length; i ++) {
                        if (info.items[i].type == "message") {
                            ata.push(info.items[i])
                        } else if (info.items[i].type == "channel") {
                            //                            info.items[i].name = staredtoName(info.items[i])
                            ana.push(info.items[i])
                        }
                    }
                    stared = ana
                    checkFavList()
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

    function checkFavList() {
        if (staredlist.dataModel.size() == 0)
            starempty.visible = true;
        else
            starempty.visible = false
    }
    function staredtoName(data) {
        var name
        switch (data.type) {
            case "im":
                {
                    for (var i = 0; i < ims.length; i ++) {
                        if (ims[i].id == data.channel) {
                            name = getUserNameById(ims[i].user)
                        }
                    }
                    break;
                }
            case "channel":
                {
                    for (var i = 0; i < channels.length; i ++) {
                        if (channels[i].id == data.channel) {
                            name = channels[i].name
                        }
                    }
                    break;
                }
        }
        return name
    }

    function removeStar(ts, id) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "stars.remove?token=" + token + "&channel=" + id + "&timestamp=" + ts
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        displayError(qsTr("Removed Star"))
                    } else {
                        displayError(qsTr("Error Removed Starred"))
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
    function sortArray(data, key) {
        var array_copy = []
        var key_copy = []
        for (var i = 0; i < data.length; i ++) {
            key_copy.push(data[i][key])
        }
        key_copy.sort(function (a, b) {
                return a - b
            })
        for (var i = 0; i < key_copy.length; i ++) {
            for (var j = 0; j < data.length; j ++) {
                if (data[j][key] == key_copy[i]) {
                    array_copy.push(data[j])
                }
            }

        }
        return array_copy
    }
    function renderAccounts() {
        var inf = app.getSettings()
        accounts.dataModel.clear()
        accounts.dataModel.append(inf)
    }
    function switchAccount(toke) {
        app.disconnectSocketForSwitchAccount();
        token = toke
        getActiveSocket(token)
    }
    function getAvatars() {
        fetchingimages = true
        //        app.downloadFiles(teams.icon['image_88'],teams.id+".png")
        for (var i = 0; i < users.length; i ++) {
            app.downloadFiles(users[i].profile['image_72'], users[i].id + ".png")
        }
        for (var i = 0; i < bots.length; i ++) {
            if (bots[i].icons) {
                var imgurl = bots[i].icons['image_48']
                if (bots[i].icons['image_64']) {
                    imgurl = bots[i].icons['image_64']
                }
                app.downloadFiles(imgurl, bots[i].id + ".png")
            }
        }
        fetchingimages = false

    }

    function tstoDate(ts) {
        var tsr = ts.split('.')
        return Moment.moment.unix(tsr[0]).locale(Qt.local).format('MMM D, h:mm A')
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
    function mimetoimage(type) {
        switch (type) {
            case "png":
            case "jpg":
            case "jpeg":
                return "asset:///images/newicons/img.png"
            case "doc":
                return "asset:///images/newicons/doc.png"
            case "pdf":
                return "asset:///images/newicons/pdf.png"
            case "post":
                return "asset:///images/newicons/pos.png"
            case "zip":
                return "asset:///images/newicons/zip.png"
            case "text":
                return "asset:///images/newicons/snp.png"
            default:
                return "asset:///images/newicons/all.png"
        }

    }
    function updateMute(type, id) {
        console.log("tyup e" + type)
        switch (type) {
            case "channel":
                {
                    for (var i = 0; i < channellist.dataModel.size(); i ++) {
                        var anasa = channellist.dataModel.data([ i ])
                        if (id == anasa["id"]) {
                            var more = anasa
                            channellist.dataModel.replace(i, more)
                        }
                    }
                }
                break
            case "users":

                for (var i = 0; i < imslist.dataModel.size(); i ++) {
                    var anasa = imslist.dataModel.data([ i ])
                    if (id == anasa["id"]) {
                        var more = anasa
                        imslist.dataModel.replace(i, more)
                    }
                }
                break
            case "group":
                for (var i = 0; i < grouplist.dataModel.size(); i ++) {
                    var anasa = grouplist.dataModel.data([ i ])
                    if (id == anasa["id"]) {
                        var more = anasa
                        grouplist.dataModel.replace(i, more)
                    }
                }
                break

        }
    }
    shortcuts: [
        Shortcut {
            key: "q"
            onTriggered: {
                console.log("q triggered")
                screenshot.captureDisplay()
            }
        }
    ]
}
