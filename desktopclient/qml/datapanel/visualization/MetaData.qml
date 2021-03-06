import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.1
import UIContextModel 1.0
import LayerManager 1.0
import "../../workbench/propertyform" as MetaData
import "../../controls" as Controls
import "../../Global.js" as Global
import "../.." as Base

Item {
    id : metatdata
    width : 210
    height : parent.height

    function iconSource(name) {
        if ( name === "")
            name = "redbuttonr.png"
         var iconP = "../../images/" + name
         return iconP

     }


    Rectangle {
        id : layersLabel
        width : parent.width + 10
        height : 18
        color : Global.alternatecolor3
        Text{
            text : qsTr("Layers")
            font.weight: Font.DemiBold
            x : 5
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    Row {
        width: parent.width
        anchors.top: layersLabel.bottom
        anchors.topMargin: 2
        height : parent.height - layersLabel.height - 5
        spacing : 3
        Rectangle {
            id : layerContainer
            width : 210
            height : parent.height
            color : Global.alternatecolor2
            border.color: "lightgrey"
            border.width: 1


            Component {
                id: highlight

                Rectangle {
                    width: layersList.width; height: 18
                    color: Global.selectedColor; radius: 2
                    y: (layersList && layersList.currentItem) ? layersList.currentItem.y : 0
                    Behavior on y {
                        SpringAnimation {
                            spring: 3
                            damping: 0.2
                        }
                    }
                }
            }
            ListView {
                id : layersList
                model : manager.layers
                anchors.fill: parent
                anchors.margins: 4
                delegate: Component{
                    Item {
                        id : layerRow
                        width: parent.width
                        height: 18

                        Row {
                            spacing: 2
                            width : parent.width
                            Image {
                                id : image
                                width : 16; height :16
                                source : iconSource(iconPath)
                                fillMode: Image.PreserveAspectFit
                            }
                            Text{
                                text : name
                                width : parent.width - image.width
                                font.pointSize: 8
                                elide: Text.ElideMiddle
                                MouseArea{
                                    anchors.fill: parent
                                    onClicked: {
                                        layersList.currentIndex = index
                                    }
                                }
                            }

                        }
                    }
                }

                highlight: highlight
                focus: true
                clip : true

            }
        }
        Rectangle{
            width : parent.width - layerContainer.width - 8
            height : layerContainer.height
            ScrollView{
                anchors.fill: parent
                ListView{
                    id : metatdatalist
                }

            }
            border.color: "lightgrey"
            border.width: 1

        }
    }
}
