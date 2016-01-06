import bb.cascades 1.4
import bb.cascades.pickers 1.0
import bb.system 1.2

NavigationPane {
    property variant token
    property variant selectfile
    onCreationCompleted: {
        app.memoChanged.connect(memocha)

        var info = app.getActiveAccount()[0]
        token = info.value
        var optin = app.getChannels()
        for (var i = 0; i < optin.length; i ++) {
            var a = opt.createObject()
            a.text = optin[i].name
            a.value = optin[i].id
            drop.add(a)
        }
    }
    function memocha(data,id){
        console.log("share qml "+data,id)
        imagesource.imageSource = data
        selectfile = data.split('://')[1]
    }

    attachedObjects: [
        ComponentDefinition {
            id: opt
            Option {

            }
        },
        FilePicker {
            id: filePicker
            type: FileType.Picture
            title: "Select Picture"
            directories: [ "/accounts/1000/shared/" ]
            onFileSelected: {
                selectfile = selectedFiles[0];
                myImageView.imageSource = "file://" + selectedFiles[0];
            }
        }
    ]
    Page {
        titleBar: TitleBar {
            title: qsTr("Share with")
        }
        ScrollView {

            Container {
                topPadding: ui.du(2.0)
                leftPadding: ui.du(2.0)
                rightPadding: ui.du(2.0)
                bottomPadding: ui.du(2.0)
                TextField {
                    hintText: qsTr("Enter title ")
                    id: filetitle
                }
                DropDown {
                    id: drop
                    title: qsTr("Share with")
                    enabled: true
                }

                Container {
                    preferredHeight: ui.du(50)
                    background: Color.LightGray
                    horizontalAlignment: HorizontalAlignment.Fill
                    ImageView {
                        id: imagesource
                        horizontalAlignment: HorizontalAlignment.Center
                        verticalAlignment: VerticalAlignment.Center
                        imageSource: "asset:///images/Various%20Artists%20-%20Django%20Unchained.jpg"
                        scalingMethod: ScalingMethod.AspectFit
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
//                    progressDialog.show()
                    upload()
                }
            }
        ]
        attachedObjects: [
            SystemProgressDialog {
                id: progressDialog
                title: qsTr("Uploading file")
                body: qsTr("File has been uploaded")
                autoUpdateEnabled: true
                progress: 45
                customButton.enabled: false
                confirmButton.enabled: false
                cancelButton.enabled: false
                activityIndicatorVisible: true

            }
        ]
    }
    function upload() {
        app.uploadFile(selectfile, token, drop.selectedValue, filetitle.text);
    }
}
