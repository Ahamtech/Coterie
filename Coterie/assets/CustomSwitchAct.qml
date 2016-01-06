import bb.cascades 1.4


Container {
    property alias account: actname.text
    property alias type: type.text
    property alias primary: active.visible
    verticalAlignment: VerticalAlignment.Fill
    Container {
        verticalAlignment: VerticalAlignment.Center
        horizontalAlignment: HorizontalAlignment.Fill
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        preferredHeight: 100
        Container {
            id: active
            preferredHeight: 100
            preferredWidth: 5
            background: Application.themeSupport.theme.colorTheme.primary
            minWidth: 20
            maxWidth: 20
            verticalAlignment: VerticalAlignment.Fill
        }
        Container {
            id: inactive
            visible: !active.visible
            preferredHeight: 100
            preferredWidth: 5
//            background: Color.create("#ff19ffff")
            minWidth: 40
            maxWidth: 40
            verticalAlignment: VerticalAlignment.Fill
        }
        Label {
            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }
            id: actname
//            text: "Ahamtech"
            verticalAlignment: VerticalAlignment.Center
            textStyle.fontSize: FontSize.Medium
        }
        Container {
            horizontalAlignment: HorizontalAlignment.Right
verticalAlignment: VerticalAlignment.Center
            rightPadding: 30
        Label {
            
            id: type
            opacity: 0.7
            verticalAlignment: VerticalAlignment.Center
        }
}
    }
    Container {
        preferredHeight: 2
        horizontalAlignment: HorizontalAlignment.Fill
        background: Color.LightGray

    }
}