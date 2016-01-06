import bb.cascades 1.4

Container {
    id: self
    topMargin: ui.du(2)
    leftMargin: ui.du(1)
    leftPadding: ui.du(1.0)
    rightPadding: ui.du(1.0)
    property bool isMockUp: false // Set to true when testing the formatting in QDE!!!
    property bool incoming
    property bool isSystem
    property alias img: readmessage.imageSource
    property alias time: timestampLabel.text
    property alias message: messagebdy.text
    property alias username: contactNameLabel.text
    property alias fileview: fileview.visible
    property alias imgs: imgsrc.imageSource
    property bool read
    property variant type
    Container {
        horizontalAlignment: HorizontalAlignment.Fill
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        leftPadding: ui.du(1)
        rightPadding: ui.du(1)
        topPadding: ui.du(1)
        background: Application.themeSupport.theme.colorTheme.style == 2 ? Color.create("#323232") : Color.create("#50c7dfe4")
        Container {
            Container {
                layout: DockLayout {
                }
                Container {
                    ImageView {
                        loadEffect: ImageViewLoadEffect.FadeZoom
                        maxHeight: 80
                        maxWidth: 80
                        minHeight: 80
                        minWidth: 80
                        id: readmessage
                    }
                }
                Container {
                    ImageView {
//                        imageSource: Application.themeSupport.theme.colorTheme.style == 2 ? "asset:///images/messages/blackcircle.png" : "asset:///images/messages/whitecircle.png"
                    }
                }
            }
            
        }
        Container {
            horizontalAlignment: HorizontalAlignment.Fill
            layout: StackLayout {
                orientation: LayoutOrientation.TopToBottom
            }
            leftPadding: ui.du(2)
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                leftMargin: ui.du(2)
                Container {
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1
                    }
                    horizontalAlignment: HorizontalAlignment.Left
                    Label {
                        id: contactNameLabel
//                          text: "Mahesh Reddy"
                        textStyle.fontSize: FontSize.Medium
                        textStyle.fontWeight: FontWeight.W500
//                        textStyle.color: ui.palette.primary
                        textStyle.color: Application.themeSupport.theme.colorTheme.style == 2 ? Color.create("#c7dfe4") : Color.create("#323232")
                    }
                }
                Container {
                    verticalAlignment: VerticalAlignment.Center
                    horizontalAlignment: HorizontalAlignment.Right
                    Label {
                        id: timestampLabel
//                                                text: "Apr 21,9:54 PM"
                        horizontalAlignment: HorizontalAlignment.Right
                        textStyle.fontSize: FontSize.Small
                        textStyle.fontWeight: FontWeight.W300
                        textStyle.fontStyle: FontStyle.Italic
                        textStyle.color: Color.create("#477b96")
                    }
                }
                Container {
                    topPadding: 8
                    leftPadding: 20
                    id: fileview
                    visible: false
                    ImageView {
                        id: imgsrc
                        imageSource: "asset:///images/files.png"
                        filterColor: ui.palette.primary
                        minHeight: 40
                        maxHeight: 40
                        maxWidth: 40
                        minWidth: 40
                    }
                }
            }
            Container {
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10.0
                Label {
                    id: messagebdy
//                      text: "https://secure.gravatar.com/avatar/Fava_0005-72.png"
                    multiline: true
                    textStyle.fontWeight: FontWeight.W400
                    textFormat: TextFormat.Html
                    content.flags: TextContentFlag.ActiveText | TextContentFlag.Emoticons
                    textStyle.color: Application.themeSupport.theme.colorTheme.style == 2 ? Color.White : Color.Black
                }
            }
        }
        bottomPadding: ui.du(2)
    }
        Container {
            opacity: 0.8
            background: ui.palette.primary
            horizontalAlignment: HorizontalAlignment.Fill
            preferredHeight: 2
        }
}