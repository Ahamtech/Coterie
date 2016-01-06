import bb.cascades 1.4
import "../moment.js" as Moment
import bb.system 1.2
import bb.device 1.4

NavigationPane {
    id: mainnav
    property variant userid
    property variant token
    property variant currentusr
    property variant fileactive

    property variant endpoint: "https://slack.com/api/"
    onCreationCompleted: {
        app.textChanged.connect(infoa)
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
    function infoa(data){
        console.log("share qml "+data)
        textshare.text = data
    }
    attachedObjects: [
        ComponentDefinition {
            id: opt
            Option {
            
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
                
                Label {
                    id: textshare
                }
            
            }
        }
    }
}