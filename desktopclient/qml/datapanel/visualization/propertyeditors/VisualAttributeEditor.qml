import QtQuick 2.1
import "../../../Global.js" as Global

Item {
    id : displayEditorColumn

    property var currentEditor

    onCurrentEditorChanged: {
         if ( currentEditor){
             if ( editorColumn2.state == "minimized"){
                 editorColumn1.state = "minimized"
                 editorColumn2.state = "maximized"
                 propertyEditor2.setSource(currentEditor.qmlUrl,{"editor" : currentEditor})

             }else {
                 editorColumn2.state = "minimized"
                 editorColumn1.state = "maximized"
                 propertyEditor1.setSource(currentEditor.qmlUrl,{"editor" : currentEditor})
             }
          }
     }
    Rectangle {
        id : editorsLabel
        width : parent.width
        height : 18
        color : Global.alternatecolor3
        Text{
            text : qsTr("Property Editor")
            font.weight: Font.DemiBold
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Rectangle {
        id : editorColumn1
        color : Global.alternatecolor2
        border.color: "lightgrey"
        border.width: 1
        anchors.right: parent.right
        anchors.top: editorsLabel.bottom
        anchors.topMargin: 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 3
        state : "maximized"
        Loader {
            id : propertyEditor1
            anchors.fill : parent


        }
        states: [
            State { name: "maximized"
                    PropertyChanges { target: editorColumn1; opacity : 1 }
            },
            State {
                name : "minimized"
                    PropertyChanges { target: editorColumn1; opacity : 0 }
            }

        ]
        transitions: [
            Transition {
                NumberAnimation { properties: "opacity"; duration : 500 ; easing.type: Easing.InOutCubic }
            }
        ]

    }

    Rectangle {
        id : editorColumn2
        color : Global.alternatecolor2
        border.color: "lightgrey"
        border.width: 1
        anchors.right: parent.right
        anchors.top: editorsLabel.bottom
        anchors.topMargin: 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 3
        Loader {
            id : propertyEditor2
            anchors.fill : parent


        }
        states: [
            State { name: "maximized"
                    PropertyChanges { target: editorColumn2; opacity : 1 }
            },
            State {
                name : "minimized"
                    PropertyChanges { target: editorColumn2; opacity : 0 }
            }

        ]
        transitions: [
            Transition {
                NumberAnimation { properties: "opacity"; duration : 500 ; easing.type: Easing.InOutCubic }
            }
        ]

    }
}

