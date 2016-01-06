
import "moment.js" as Moment
import bb.cascades 1.4
import bb.device 1.4
import bb.system 1.2

Page {
    property variant fileinfotemp: []
    property variant activedownload
    onCreationCompleted: {
        getInfo()
        app.attachmentProgres.connect(attprogress)
        app.attachmentFinish.connect(attfinish)
        Qt.deleteComment = deleteComment
        Qt.activeshare = []
        //        app.attachProgress.connect()
    }
    onActivedownloadChanged: {
    }
    function attfinish() {
        imageso.imageSource = Qt.mediaPath + fileinfotemp.file.name
        invoke.query.uri = Qt.mediaPath + fileinfotemp.file.name
        downloadinde.value = 0
    }
    function attprogress(rem, total) {
        downloadinde.value = rem / total
    }
    property variant vpoint: 0
    attachedObjects: [
        DisplayInfo {
            id: display
        },
        SystemPrompt {
            id: commentbox
            title: qsTr("Enter Comment")
            rememberMeChecked: false
            includeRememberMe: false
            confirmButton.enabled: true
            cancelButton.enabled: true
            inputField.emptyText: qsTr("Comment")
            onFinished: {
                if (result == SystemUiResult.ConfirmButtonSelection) {
                    postComment(commentbox.inputFieldTextEntry())
                }
            }
        },
        Sheet {
            id: sharefile
            onOpened: {
                channelslist.dataModel.clear()
                for (var key in Qt.mapper) {
                    if (Qt.mapper[key].mode == "groups" || Qt.mapper[key].mode == "channel" ||  Qt.mapper[key].mode == "users")
                        channelslist.dataModel.append(Qt.mapper[key])
                }
            }
            content: Page {
                titleBar: TitleBar {
                    title: qsTr("Share")
                    acceptAction: ActionItem {
                        title: qsTr("Share")
                        onTriggered: {
                            fileShare(Qt.activeshare)
                        }
                    }
                    dismissAction: ActionItem {
                        title: qsTr("Close")
                        onTriggered: {
                            sharefile.close()
                        }
                    }
                }
                ListView {
                    id: channelslist
                    dataModel: ArrayDataModel {

                    }
                    onTriggered: {
                        toggleSelection(indexPath)
                        console.log(indexPath)
                        console.log(JSON.stringify(channelslist.dataModel.data([ indexPath ])))
                        Qt.activeshare = channelslist.dataModel.data([ indexPath ]).id
                    }
                    listItemComponents: [
                        ListItemComponent {
                            type: ""
                            StandardListItem {
                                title: ListItemData.name
                            }
                        }
                    ]
                }
            }
        }
    ]
    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                topPadding: ui.du(1)
                SegmentedControl {
                    id: segcontrol
                    onSelectedIndexChanged: {
                        if (selectedIndex == 1) {
                            scrollview.scrollToPoint(second.layoutFrame.x, second.layoutFrame.y)
                        } else
                            scrollview.scrollToPoint(first.layoutFrame.x, first.layoutFrame.y)
                    }
                    options: [
                        Option {
                            id: one
                            text: qsTr("File Info")
                            value: "1"
                            selected: true
                        },
                        Option {
                            id: two
                            text: qsTr("Comments")
                            value: "2"
                        }
                    ]
                }
            }
        }
    }
    Container {
        topPadding: ui.du(2.0)
        bottomPadding: ui.du(2.0)
        horizontalAlignment: HorizontalAlignment.Fill
        attachedObjects: [
            LayoutUpdateHandler {
                id: handler
            }
        ]
        layout: StackLayout {
        }
        Container {
            verticalAlignment: VerticalAlignment.Fill
            ScrollView {
                id: scrollview
                onTouch: {
                    if (event.isUp()) {
                        if (vpoint > display.pixelSize.width / 2) {
                            segcontrol.setSelectedIndex(1)
                            scrollview.scrollToPoint(second.layoutFrame.x, second.layoutFrame.y)
                        } else {
                            segcontrol.setSelectedIndex(0)
                            scrollview.scrollToPoint(first.layoutFrame.x, first.layoutFrame.y)
                        }
                    }
                }
                onViewableAreaChanging: {
                    vpoint = viewableArea.x
                }
                scrollRole: ScrollRole.Main
                scrollViewProperties.scrollMode: ScrollMode.Horizontal
                horizontalAlignment: HorizontalAlignment.Fill
                Container {
                    horizontalAlignment: HorizontalAlignment.Fill
                    minHeight: display.physicalSize.height
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Container {
                        id: oneContent
                        minWidth: handler.layoutFrame.width
                        maxWidth: handler.layoutFrame.width
                        verticalAlignment: VerticalAlignment.Fill
                        attachedObjects: [
                            LayoutUpdateHandler {
                                id: first
                            }
                        ]
                        Container {
                            id: scrollcont
                            horizontalAlignment: HorizontalAlignment.Fill
                            verticalAlignment: VerticalAlignment.Fill
                            ScrollView {
                                scrollRole: ScrollRole.Main
                                scrollViewProperties.scrollMode: ScrollMode.Vertical
                                horizontalAlignment: HorizontalAlignment.Fill
                                verticalAlignment: VerticalAlignment.Fill
                                Container {
                                    horizontalAlignment: HorizontalAlignment.Fill
                                    verticalAlignment: VerticalAlignment.Fill
                                    Container {
                                        horizontalAlignment: HorizontalAlignment.Fill
                                        Container {

                                            leftPadding: ui.du(2.0)
                                            rightPadding: ui.du(2.0)
                                            Label {
                                                id: text
                                                multiline: true
                                                textFormat: TextFormat.Html
                                            }
                                        }
                                        ImageView {
                                            id: imageso
                                            horizontalAlignment: HorizontalAlignment.Center
                                            verticalAlignment: VerticalAlignment.Center
                                            scalingMethod: ScalingMethod.AspectFit
                                            gestureHandlers: [
                                                TapHandler {

                                                    onTapped: {
                                                        app.invokePictureView(imageso.imageSource)

                                                    }
                                                }
                                            ]
                                        }
                                        ProgressIndicator {
                                            id: downloadinde
                                        }
                                    }
                                    Container {
                                        maxWidth: display.pixelSize.width - ui.du(4)
                                        topPadding: ui.du(2)
                                        bottomPadding: ui.du(5.0)
                                        bottomMargin: ui.du(5.0)
                                        leftPadding: ui.du(2.0)
                                        rightPadding: ui.du(2.0)
                                        Container {
                                            rightPadding: ui.du(2)
                                            Label {
                                                id: filetitle
                                                textStyle.fontSize: FontSize.Medium
                                                multiline: true
                                            }
                                        }
                                        Container {
                                            layout: StackLayout {
                                                orientation: LayoutOrientation.LeftToRight
                                            }
                                            topPadding: ui.du(1)
                                            bottomPadding: 5.0
                                            Container {
                                                ImageView {
                                                    id: fileuploaduser
                                                    imageSource: "asset:///images/icons.png"
                                                    minHeight: 96
                                                    minWidth: 90
                                                }
                                            }
                                            Container {
                                                layout: StackLayout {
                                                }
                                                leftPadding: ui.du(2.0)
                                                leftMargin: ui.du(2.0)
                                                Container {
                                                    Label {
                                                        id: username
                                                        textStyle.fontSize: FontSize.Medium
                                                        textStyle.fontWeight: FontWeight.W500
                                                    }
                                                }
                                                Container {
                                                    layout: StackLayout {
                                                        orientation: LayoutOrientation.LeftToRight
                                                    }
                                                    Label {
                                                        layoutProperties: StackLayoutProperties {
                                                            spaceQuota: 0.35
                                                        }
                                                        id: filesize
                                                        textStyle.fontSize: FontSize.Medium
                                                        textStyle.fontWeight: FontWeight.W300
                                                    }
                                                    Label {
                                                        layoutProperties: StackLayoutProperties {
                                                            spaceQuota: 0.65
                                                        }
                                                        id: timedate
                                                        textStyle.fontSize: FontSize.Medium
                                                        textStyle.fontWeight: FontWeight.W300
                                                    }
                                                }
                                            }
                                        }
                                        Container {
                                            layout: StackLayout {
                                                orientation: LayoutOrientation.LeftToRight
                                            }
                                            topPadding: 5.0
                                            topMargin: 5.0
                                            //                                            leftPadding: ui.du(2.0)
                                            Label {
                                                layoutProperties: StackLayoutProperties {
                                                    spaceQuota: 0.3
                                                }
                                                textStyle.fontWeight: FontWeight.W500
                                                //                                                text: qsTr("Shared with :")
                                            }
                                            Container {
                                                layoutProperties: StackLayoutProperties {
                                                    spaceQuota: 0.70
                                                }
                                                rightPadding: ui.du(2)
                                                Label {
                                                    multiline: true
                                                    id: share
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Container {
                        id: twoContent
                        minWidth: handler.layoutFrame.width
                        maxWidth: handler.layoutFrame.width
                        attachedObjects: [
                            LayoutUpdateHandler {
                                id: second
                            }
                        ]
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Fill
                        Container {
                            id: inivisibleCommentsContent
                            horizontalAlignment: HorizontalAlignment.Fill

                            verticalAlignment: VerticalAlignment.Bottom
                            Label {
                                horizontalAlignment: HorizontalAlignment.Fill
                                verticalAlignment: VerticalAlignment.Center
                                textStyle.textAlign: TextAlign.Center
                                text: qsTr("No Comments Yet")
                                textStyle.fontSize: FontSize.Large
                            }
                        }
                        Container {

                            ListView {
                                visible: false
                                id: comments
                                dataModel: ArrayDataModel {

                                }
                                onCreationCompleted: {
                                }
                                listItemComponents: [
                                    ListItemComponent {
                                        type: "header"
                                    },
                                    ListItemComponent {
                                        type: ""
                                        CustomCommentList {
                                            description: ListItemData.comment
                                            title: Qt.getUserNameById(ListItemData.user)
                                            status: Moment.moment.unix(ListItemData.timestamp).locale(Qt.local).format('LLL')
                                            imageSource: Qt.mediaPath + ListItemData.user + ".png"

                                            contextActions: [
                                                ActionSet {
                                                    ActionItem {
                                                        title: qsTr("Copy") + Retranslate.onLanguageChanged
                                                        imageSource: "asset:///images/BBicons/ic_copy.png"
                                                        onTriggered: {
                                                            Qt.copytoClip(ListItemData.comment)
                                                        }
                                                    }
                                                    DeleteActionItem {
                                                        title: qsTr("Delete")
                                                        imageSource: "asset:///images/BBicons/ic_delete.png"
                                                        onTriggered: {
                                                            Qt.deleteComment(ListItemData.id)
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
            }
        }
    }
    actions: [
        ActionItem {
            title: qsTr("Open in Browser")
            imageSource: "asset:///images/BBicons/ic_browser.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                app.invokeBrowser(fileinfotemp.file["url"])
            }
        },
        ActionItem {
            title: qsTr("Copy Link")
            imageSource: "asset:///images/BBicons/ic_copy_link.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                Qt.copytoClip(fileinfotemp.file.url)
            }
        },
        ActionItem {
            title: qsTr("Comment")
            ActionBar.placement: ActionBarPlacement.Signature
            imageSource: "asset:///images/iconswhite/ic_bbm.png"
            onTriggered: {
                commentbox.show()
            }
        },
        ActionItem {
            title: qsTr("Share")
            imageSource: "asset:///images/BBicons/ic_share.png"
            onTriggered: {
                sharefile.open()
            }
        }
        //        ActionItem {
        //            title: qsTr("Download")
        //            imageSource: "asset:///images/BBicons/ic_save_as.png"
        //            ActionBar.placement: ActionBarPlacement.OnBar
        //            onTriggered: {
        //                app.downloadAttachment()
        //            }
        //        }
    ]
    actionBarAutoHideBehavior: ActionBarAutoHideBehavior.HideOnScroll
    actionBarVisibility: ChromeVisibility.Default
    resizeBehavior: PageResizeBehavior.None
    function getInfo() {
        var doc = new XMLHttpRequest();
        var url = endpoint + "files.info?token=" + token + "&file=" + fileactive
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        fileinfotemp = info
                        filetitle.text = info.file.title
                        username.text = Qt.getUserNameById(info.file.user)
                        fileuploaduser.imageSource = Qt.mediaPath + info.file.user + ".png"
                        timedate.text = Moment.moment(info.file.timestamp * 1000).locale(Qt.local).format('LLL')
                        filesize.text = Qt.formatSizeUnits(info.file.size)
                        comments.dataModel.clear()
                        comments.dataModel.append(info.comments)
                        two.text = qsTr("Comments") + " - " + comments.dataModel.size()
                        inivisibleCommentsContent.visible = ! comments.dataModel.size() > 0
                        comments.visible = comments.dataModel.size() > 0
                        console.log("file type", info.file.filetype)
                        if (info.file.filetype == "png" || info.file.filetype == "jpg") {
                            // downaload image
                            console.log(info.file.thumb_360, info.file.name)
                            app.downloadFiles(info.file.url_download, info.file.name)
                        } else {
                            if (info.file.preview)
                                text.text = info.file.preview
                        }
                    } else {
                        displayError(qsTr("Unable to access files"))
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
    function postComment(data) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "files.comments.add?token=" + token + "&file=" + fileactive + "&comment=" + data
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var ina = JSON.parse(doc.responseText)
                    if (ina.ok == true) {
                        comments.dataModel.append(ina.comment)
                        two.text = qsTr("Comments") + " - " + comments.dataModel.size()
                        inivisibleCommentsContent.visible = ! comments.dataModel.size() > 0
                        comments.visible = comments.dataModel.size() > 0
                    } else {
                        displayError(qsTr("Unable to Post"))
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
    function editComment(data) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "files.comments.add?token=" + token + "&file=" + fileactive + "&comment=" + data
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    console.log(doc.status + doc.statusText)
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
    function deleteComment(id) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "files.comments.delete?token=" + token + "&file=" + fileactive + "&id=" + id
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    var info = JSON.parse(doc.responseText)
                    if (info.ok == true) {
                        for (var i = 0; i < comments.dataModel.size(); i ++) {
                            if (comments.dataModel.data([ i ]).id == id)
                                comments.dataModel.removeAt(i)
                            two.text = qsTr("Comments") + " - " + comments.dataModel.size()
                            inivisibleCommentsContent.visible = ! comments.dataModel.size() > 0
                            comments.visible = comments.dataModel.size() > 0
                        }
                    } else {
                        displayError(qsTr("Unable to delete comment"))
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
    function fileShare(id) {
        var doc = new XMLHttpRequest();
        var url = endpoint + "files.share?token=" + token + "&file=" + fileactive + "&channel=" + id
        console.log(url)
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                if (doc.status == 200) {
                    sharefile.close()
                    displayError(qsTr("Sucessfully shared file"))
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
}
