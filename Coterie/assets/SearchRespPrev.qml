import bb.cascades 1.4

Container {
    property alias title : username.text
    property alias imageSource : userimg.imageSource
    property alias status : timedate.text
    property alias description : comment.text
    horizontalAlignment: HorizontalAlignment.Fill
    verticalAlignment: VerticalAlignment.Fill
    topPadding: 15.0
    bottomPadding: 3.0
    rightPadding: ui.du(1.0)
    leftPadding: ui.du(1.0)
    Container {
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        
        }
        Container {
            ImageView {
                loadEffect: ImageViewLoadEffect.FadeZoom
                maxHeight: 100
                maxWidth: 100
                minHeight: 100
                minWidth: 100
                id: userimg
                //imageSource: "asset:///images/qrcode%20(2).png"
            
            }
        }
        Container {
            leftPadding: ui.du(1)
            layout: StackLayout {
                orientation: LayoutOrientation.TopToBottom
            
            }
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                
                }
                Container {
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: .55
                    }
                    Label {
                        id: username
                        // text: "Mahesh Reddy"
                        textStyle.fontWeight: FontWeight.W500
                        textStyle.fontSize: FontSize.Small
                        textStyle.color: Color.create("#ff54c9ff")
                    
                    }
                }
                Container {
                    horizontalAlignment: HorizontalAlignment.Right
                    layoutProperties: StackLayoutProperties {
                        spaceQuota: .45
                    }
                    Label {
                        horizontalAlignment: HorizontalAlignment.Right
                        id: timedate
                        //   text: "6 may 2015 10:40"
                        opacity: .7
                        textStyle.fontWeight: FontWeight.W300
                        textStyle.color: Color.create("#68b38b")
                        textStyle.fontStyle: FontStyle.Italic
                    
                    }
                }
            }
            Container {
                Label {
                    id: comment                    
                    // text: "testing for rg gogmeoeme kjasf  asjn hsadg adsjnvhyjads gjasdhvs dvjkfdsaybwek"
                    multiline: true
                    textStyle.fontSize: FontSize.Small
                    textStyle.fontWeight: FontWeight.W200
                }
            }
        }
    }

}
