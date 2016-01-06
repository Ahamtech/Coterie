import bb.cascades 1.4

Container {
    property alias title: contactNameLabel.text
    property alias description: messagebdy.text
    property alias status: timestampLabel.text
    property alias imageSource: imagepic.imageSource
    property alias mute: muter.visible
    property alias active: activity.filterColor
    leftPadding: ui.du(1)
    rightPadding: ui.du(1)
    topPadding: ui.du(1.0)
    bottomPadding: ui.du(1.0)
    Container {
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        Container {
            layout: DockLayout {
            
            }
            Container {
                ImageView {
                    id: imagepic
                    loadEffect: ImageViewLoadEffect.FadeZoom
                    maxHeight: 100
                    minHeight: 100
                    maxWidth: 100
                    minWidth: 100
                    scalingMethod: ScalingMethod.AspectFill
                }
            }
            Container {
                ImageView {
                    imageSource: Application.themeSupport.theme.colorTheme.style == 2 ? "asset:///images/messages/blackcircle.png" : "asset:///images/messages/whitecircle.png"
                    scalingMethod: ScalingMethod.AspectFill
                    maxHeight: 100
                    minHeight: 100
                    maxWidth: 100
                    minWidth: 100
                    visible: false
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
                leftMargin: ui.du(1.0)
                Container {
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1
                    }
                    horizontalAlignment: HorizontalAlignment.Left
                    Label {
                        id: contactNameLabel
                        //                        text: "Mahesh Reddy"
                        textStyle.fontSize: FontSize.Medium
                        textStyle.fontWeight: FontWeight.W500
                        textStyle.color: Color.create("#68b38b")
                    }
                }
                Container {
                    verticalAlignment: VerticalAlignment.Center
                    horizontalAlignment: HorizontalAlignment.Right
                    Label {
                        id: timestampLabel
                        //                        text: "Apr 21,9:54 PM"
                        horizontalAlignment: HorizontalAlignment.Right
                        textStyle.fontSize: FontSize.Small
                        textStyle.fontWeight: FontWeight.W300
                        textStyle.fontStyle: FontStyle.Italic
                        textStyle.color: Color.create("#477b96")
                    }
                }
            }
            Container {
                
                verticalAlignment: VerticalAlignment.Center
                horizontalAlignment: HorizontalAlignment.Fill
                topPadding: 10.0
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                Container {
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1
                    }
                    Label {
                        id: messagebdy
                        text: " "
                        //                        text: "https://secure.gravatar.com/avatar/00b 7 e1 3 f7023ebf6a0cfd20f9a8f3272.jpg?s=72&d=https%3A%2F%2Fslack.global.ssl.fastly.net%2F3654%2Fimg%2Favatars%2Fava_0005-72.png"
                        multiline: false
                        textStyle.fontWeight: FontWeight.W400
                    }
                }
                Container {
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    verticalAlignment: VerticalAlignment.Center
                    horizontalAlignment: HorizontalAlignment.Right
                    
                    Container {
                        
                        ImageView {
                            id: muter
                            visible: false
                            verticalAlignment: VerticalAlignment.Center
                            imageSource: "asset:///images/newicons/bellcancel.png"
                            filterColor: ui.palette.primary
                            maxHeight: 40
                            maxWidth: 40
                        }
                    }
                    Container {
                        leftPadding: 15
                        verticalAlignment: VerticalAlignment.Center
                        ImageView {
                            id: activity
                            verticalAlignment: VerticalAlignment.Center
                            imageSource: "asset:///images/newicons/online.png"
                            maxHeight: 30
                            maxWidth: 30
                            visible: true
                            filterColor: Color.create("#00bfb9ba")
                            //ff76ec3d
                        }
                    }
                }
            }
        }
    }
}
