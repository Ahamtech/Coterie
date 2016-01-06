import bb.cascades 1.4

Container {
    property alias filename: filename.text
    property alias username: username.text
    property alias filesize: filesize.text
    property alias time: timestamp.text
    //    property alias image: avatar.imageSource
    property alias img : imagetype.imageSource
    property alias priv: fileprive.visible
    onCreationCompleted: {

    }
    topPadding: ui.du(1.0)
    leftPadding: ui.du(1.0)
    rightPadding: ui.du(1.0)
    bottomPadding: ui.du(1.0)
    layout: StackLayout {
        orientation: LayoutOrientation.LeftToRight

    }
    Container {
        verticalAlignment: VerticalAlignment.Top
        id: filetype
        horizontalAlignment: HorizontalAlignment.Center
        ImageView {
            id: imagetype
            minHeight: 70.0
            minWidth: 80.0
            maxHeight: 70
            maxWidth: 80
        }
    
    }
    Container {
        leftPadding: ui.du(1)
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }

        Container {
            Container {
                Label {
                    id: filename
//                    text: "file name .png .jpgfile name .png .jpgfile name .png .jpg"
                    textStyle.fontSize: FontSize.Medium
                }
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                Label {
                    id: username
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1
                    }
//                    text: "mahesh blazer blazer blazer blazer"
                    textStyle.color: Color.create("#ff54c9ff")
                    textStyle.fontWeight: FontWeight.W500
                }
                Label {
                    id: filesize
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: -1
                    }
//                    text: "KB png files"
                    textStyle.fontWeight: FontWeight.W200
                    opacity: 0.7
                }
            }
            Container {
                verticalAlignment: VerticalAlignment.Center
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                horizontalAlignment: HorizontalAlignment.Fill
                Container {
                    verticalAlignment: VerticalAlignment.Center
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: -1
                    }
                    id: fileprive
                    Container {

                        verticalAlignment: VerticalAlignment.Center

                        ImageView {
                            filterColor: ui.palette.primary
                            verticalAlignment: VerticalAlignment.Center
                            imageSource: "asset:///images/newicons/lock.png"
                            maxHeight: 40
                            maxWidth: 40
                            minHeight: 40
                            minWidth: 40
                        }
                    

                    }
                }

                Container {
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: 1
                    }

                    Label {
                        textStyle.color: Color.create("#68b38b")
                        id: timestamp
                        horizontalAlignment: HorizontalAlignment.Right
//                        text: "time stamp time stamp "

                    }

                }

            }

        }
        
        bottomPadding: 5
    }
//    Container {
//
//        horizontalAlignment: HorizontalAlignment.Fill
//        preferredHeight: 2
//        background: Color.LightGray
//
//        topMargin: ui.du(2.0)
//
//    }

}
