import bb.cascades 1.3
Container {

    property alias head: sectionheader.text
    layout: StackLayout {

    }
    horizontalAlignment: HorizontalAlignment.Fill
    verticalAlignment: VerticalAlignment.Fill
    background: ui.palette.primary
    leftPadding: ui.du(4.0)
    Label {
        textStyle.fontSize: FontSize.Large
        verticalAlignment: VerticalAlignment.Center
        id: sectionheader
        textStyle.color: ui.palette.primary
    }
}
