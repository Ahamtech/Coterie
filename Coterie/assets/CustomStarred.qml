import bb.cascades 1.4

Container {
    property alias title: contactNameLabel.text
    property alias status: timestampLabel.text
    Container {
        horizontalAlignment: HorizontalAlignment.Fill
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        Container {
            layout: DockLayout {
            
            }
            Container {
                ImageView {
                    
                    loadEffect: ImageViewLoadEffect.FadeZoom
                    maxHeight: 100
                    minHeight: 100
                    maxWidth: 100
                    minWidth: 100
                    scalingMethod: ScalingMethod.AspectFill
                    imageSource: "asset:///images/icons.png"
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
                }
            }
        
        }
        Container {
            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }
            verticalAlignment: VerticalAlignment.Center
            Label {
                id: contactNameLabel
//                                        text: "Mahesh Reddy"
                textStyle.fontSize: FontSize.Medium
                textStyle.fontWeight: FontWeight.W500
                textStyle.color: Color.create("#68b38b")
            }
        }
        Container {
            topPadding: 3
            verticalAlignment: VerticalAlignment.Center
            Label {
                id: timestampLabel
//                                       text: "Apr 21,9:54 PM"
                horizontalAlignment: HorizontalAlignment.Right
                textStyle.fontSize: FontSize.Small
                textStyle.fontWeight: FontWeight.W300
                textStyle.fontStyle: FontStyle.Italic
                textStyle.color: Color.create("#477b96")
                verticalAlignment: VerticalAlignment.Center
            }
        }
    }
}
