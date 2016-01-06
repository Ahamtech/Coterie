import bb.cascades 1.4

Page {
    onCreationCompleted: {
        var info = Qt.mapper[useractive.id].members
        var asa = new Array()
        for(var i =0; i < info.length; i++ ){
            if(Qt.mapper[info[i]]){
                asa.push(info[i])
            }
        }
        channlemembers.dataModel.clear()
        channlemembers.dataModel.append(asa)
    }
    titleBar: TitleBar {
        title: qsTr("Channel Members")
    }
    ListView {
        id: channlemembers
        dataModel: ArrayDataModel {

        }
        listItemComponents: [
            ListItemComponent {
                type: ""
                StandardListItem {
                    title: Qt.mapper[ListItemData].name
                    status: Qt.mapper[ListItemData].profile.email
                    imageSource: Qt.mediaPath + Qt.mapper[ListItemData].id + ".png"
                }
            }
        ]
    }
}