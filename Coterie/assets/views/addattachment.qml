import bb.cascades 1.4
import bb.cascades.pickers 1.0
import bb.system 1.2
Page {
    property variant selectfile
    titleBar: TitleBar {
        title: qsTr("Add Attachment")
    }
    onCreationCompleted: {
        console.log(JSON.stringify(channels))
        for (var i = 0; i < channels.length; i ++) {
            var a = opt.createObject()
            a.text = "#"+ channels[i].name
            a.value = channels[i].id
            drop.add(a)
        }
        console.log(JSON.stringify(ims))
        for (var i = 0; i < ims.length; i ++) {
            var a = opt.createObject()
            a.text = "@"+ ims[i].name
            a.value = ims[i].id
            drop.add(a)
        }
        
        for (var i = 0; i < groups.length; i ++) {
            var a = opt.createObject()
            a.text = "~"+groups[i].name
            a.value = groups[i].id
            drop.add(a)
        }
    }
    ScrollView {

        Container {
            topPadding: ui.du(2.0)
            leftPadding: ui.du(2.0)
            rightPadding: ui.du(2.0)
            bottomPadding: ui.du(2.0)
           
           
            Container {
                topPadding: ui.du(2)
                bottomPadding: ui.du(2.0)

                TextField {
                    hintText: qsTr("Enter title of the file")
                    id: filetitle
                }
                DropDown {
                    id: drop
                    title: qsTr("Share with")
                    enabled: true
                }
            }
            Container {
                id: upload
                visible: true
                preferredHeight: ui.du(50)
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Fill
                background: Color.LightGray
                layout: DockLayout {
                
                }
                ImageButton {
                    verticalAlignment: VerticalAlignment.Center
                    horizontalAlignment: HorizontalAlignment.Center
                    defaultImageSource: "asset:///images/newicons/icon_092b.png"
                    preferredHeight: 150
                    preferredWidth: 150
                    onClicked: {
                        filePicker.open()
                    }
                }
            
            }
            Container {
                id: image
                visible: false
                preferredHeight: ui.du(50)
                horizontalAlignment: HorizontalAlignment.Fill
                verticalAlignment: VerticalAlignment.Center
                background: Color.LightGray
                
                ImageView {
                    id: myImageView
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment: VerticalAlignment.Center
                    imageSource: "asset:///images/Various%20Artists%20-%20Django%20Unchained.jpg"
                    scalingMethod: ScalingMethod.AspectFit
                    //                    filterColor: Application.themeSupport.theme.colorTheme.primary
                }
            }
        }
    }
    actions: [
        ActionItem {
            ActionBar.placement: ActionBarPlacement.Signature
            imageSource: "asset:///images/newicons/icon_053.png"
            title: "upload"
            onTriggered: {
                progressDialog.show()
                app.uploadFile(selectfile, token, drop.selectedValue, filetitle.text);
            }
        }
    ]
    attachedObjects: [
        SystemProgressDialog {
            id: progressDialog
            title: qsTr("Adding Attachment")
            body: qsTr("Attachment has been uploaded")
            autoUpdateEnabled: true
            progress: 45
            customButton.enabled: false
            confirmButton.enabled: false
            cancelButton.enabled: false
            activityIndicatorVisible: true

        },
        FilePicker {
            id: filePicker
            type: FileType.Picture
            title: "Select Picture"
            directories: [ "/accounts/1000/shared/" ]
            onFileSelected: {
                console.log("FileSelected signal received : " + selectedFiles);
                upload.visible=false
                image.visible = true
                //make sure to prepend "file://" when using as a source for an ImageView or MediaPlayer
                myImageView.imageSource = "file://" + selectedFiles[0];
                selectfile = selectedFiles[0]
//                app.uploadFile(selectedFiles[0], token, "", "");

            }
        },
        ComponentDefinition {
            id: opt
            Option {
            
            }
        }
    ]
    function upload() {
        app.uploadFile(selectfile, token, drop.selectedValue, filetitle.text);
    }
}