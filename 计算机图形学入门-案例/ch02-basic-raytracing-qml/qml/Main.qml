import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D

ApplicationWindow {
    id: root

    width: 1440
    height: 860
    minimumWidth: 1100
    minimumHeight: 720
    visible: true
    title: qsTr("第二章 · 基础光线追踪")
    color: "#e7e9ec"

    font.family: "Microsoft YaHei"
    font.pixelSize: 14

    header: Rectangle {
        implicitHeight: 64
        color: "#ffffff"

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "#d7dbe0"
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            spacing: 16

            ColumnLayout {
                spacing: 1

                Label {
                    text: qsTr("第二章 · 基础光线追踪")
                    color: "#18212b"
                    font.pixelSize: 20
                    font.bold: true
                }

                Label {
                    text: qsTr("固定相机 · 1×1 视口 · 纯色球体")
                    color: "#68727e"
                    font.pixelSize: 12
                }
            }

            Item { Layout.fillWidth: true }

            Label {
                text: sceneController.renderWidth + " × " + sceneController.renderHeight
                color: "#4b5563"
                font.family: "Consolas"
            }

            Rectangle {
                width: 1
                height: 24
                color: "#d7dbe0"
            }

            Label {
                text: sceneController.renderMilliseconds.toFixed(1) + " ms"
                color: "#166534"
                font.family: "Consolas"
                font.bold: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 1

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 1

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 760
                color: "#ffffff"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 46
                        color: "#ffffff"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 18

                            Label {
                                text: qsTr("三维教学场景")
                                color: "#26313d"
                                font.pixelSize: 15
                                font.bold: true
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: qsTr("相机 O(0,0,0)  ·  视口 z=1")
                                color: "#7a8490"
                                font.family: "Consolas"
                                font.pixelSize: 12
                            }
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1
                            color: "#e4e7eb"
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        View3D {
                            id: sceneView
                            anchors.fill: parent
                            camera: observerCamera

                            environment: SceneEnvironment {
                                backgroundMode: SceneEnvironment.Color
                                clearColor: "#f3f5f7"
                                antialiasingMode: SceneEnvironment.MSAA
                                antialiasingQuality: SceneEnvironment.High
                            }

                            PerspectiveCamera {
                                id: observerCamera
                                position: Qt.vector3d(650, 440, 920)
                                eulerRotation: Qt.vector3d(-18, 35, 0)
                                fieldOfView: 46
                                clipFar: 3000
                            }

                            DirectionalLight {
                                eulerRotation: Qt.vector3d(-35, -30, 0)
                                brightness: 1.2
                                ambientColor: "#737b85"
                                castsShadow: true
                                shadowFactor: 35
                            }

                            PointLight {
                                position: Qt.vector3d(-350, 450, 250)
                                brightness: 5
                                quadraticFade: 0.000004
                            }

                            // Neutral ground plane gives the scene a stable spatial reference.
                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(0, -220, -300)
                                scale: Qt.vector3d(8, 0.015, 8)
                                castsShadows: false
                                receivesShadows: true
                                materials: [PrincipledMaterial {
                                    baseColor: "#e5e7eb"
                                    roughness: 1.0
                                }]
                            }

                            // Book-space +Z points into the scene; Quick 3D uses -Z here.
                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(0, 0, -300)
                                scale: Qt.vector3d(0.012, 0.012, 6)
                                castsShadows: false
                                materials: [PrincipledMaterial {
                                    baseColor: "#2563eb"
                                    emissiveFactor: Qt.vector3d(0.06, 0.11, 0.22)
                                    roughness: 0.7
                                }]
                            }

                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(0, 0, 0)
                                scale: Qt.vector3d(6, 0.012, 0.012)
                                castsShadows: false
                                materials: [PrincipledMaterial {
                                    baseColor: "#dc2626"
                                    roughness: 0.7
                                }]
                            }

                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(0, 150, 0)
                                scale: Qt.vector3d(0.012, 3, 0.012)
                                castsShadows: false
                                materials: [PrincipledMaterial {
                                    baseColor: "#16a34a"
                                    roughness: 0.7
                                }]
                            }

                            // Conceptual camera at the origin.
                            Node {
                                position: Qt.vector3d(0, 0, 20)

                                Model {
                                    source: "#Cube"
                                    scale: Qt.vector3d(0.48, 0.32, 0.38)
                                    materials: [PrincipledMaterial {
                                        baseColor: "#374151"
                                        metalness: 0.15
                                        roughness: 0.45
                                    }]
                                }

                                Model {
                                    source: "#Cylinder"
                                    position: Qt.vector3d(0, 0, -31)
                                    eulerRotation.x: 90
                                    scale: Qt.vector3d(0.17, 0.25, 0.17)
                                    materials: [PrincipledMaterial {
                                        baseColor: "#d97706"
                                        metalness: 0.1
                                        roughness: 0.35
                                    }]
                                }
                            }

                            // Projection plane: 1 x 1 world units at z = 1.
                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(0, 0, -100)
                                scale: Qt.vector3d(1, 1, 0.012)
                                opacity: 0.28
                                castsShadows: false
                                materials: [PrincipledMaterial {
                                    baseColor: "#38bdf8"
                                    roughness: 0.8
                                    alphaMode: PrincipledMaterial.Blend
                                }]
                            }

                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(0, 50, -100)
                                scale: Qt.vector3d(1.08, 0.018, 0.018)
                                castsShadows: false
                                materials: [PrincipledMaterial { baseColor: "#0284c7" }]
                            }

                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(0, -50, -100)
                                scale: Qt.vector3d(1.08, 0.018, 0.018)
                                castsShadows: false
                                materials: [PrincipledMaterial { baseColor: "#0284c7" }]
                            }

                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(50, 0, -100)
                                scale: Qt.vector3d(0.018, 1.08, 0.018)
                                castsShadows: false
                                materials: [PrincipledMaterial { baseColor: "#0284c7" }]
                            }

                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(-50, 0, -100)
                                scale: Qt.vector3d(0.018, 1.08, 0.018)
                                castsShadows: false
                                materials: [PrincipledMaterial { baseColor: "#0284c7" }]
                            }

                            Repeater3D {
                                model: sceneController.sphereModel

                                delegate: Model {
                                    required property real centerX
                                    required property real centerY
                                    required property real centerZ
                                    required property real radius
                                    required property color sphereColor
                                    required property int sphereIndex

                                    objectName: "sphere-" + sphereIndex
                                    source: "#Sphere"
                                    pickable: true
                                    position: Qt.vector3d(centerX * 100, centerY * 100, -centerZ * 100)
                                    scale: Qt.vector3d(radius * 2, radius * 2, radius * 2)
                                    castsShadows: true
                                    receivesShadows: true

                                    materials: [PrincipledMaterial {
                                        baseColor: sphereColor
                                        roughness: 0.52
                                        emissiveFactor: sphereIndex === sceneController.selectedSphere
                                                        ? Qt.vector3d(0.12, 0.12, 0.12)
                                                        : Qt.vector3d(0, 0, 0)
                                    }]
                                }
                            }
                        }

                        MouseArea {
                            id: sceneMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton
                            cursorShape: movingSphere ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                            property bool movingSphere: false
                            property real previousX: 0
                            property real previousY: 0

                            onPressed: function(mouse) {
                                const result = sceneView.pick(mouse.x, mouse.y)
                                if (result.objectHit && result.objectHit.objectName.indexOf("sphere-") === 0) {
                                    sceneController.selectedSphere = Number(result.objectHit.objectName.substring(7))
                                    movingSphere = true
                                    previousX = mouse.x
                                    previousY = mouse.y
                                }
                            }

                            onPositionChanged: function(mouse) {
                                if (!movingSphere || !(mouse.buttons & Qt.LeftButton))
                                    return

                                const dx = (mouse.x - previousX) * 0.01
                                const dy = -(mouse.y - previousY) * 0.01
                                sceneController.moveSelectedBy(dx, dy, 0)
                                previousX = mouse.x
                                previousY = mouse.y
                            }

                            onReleased: movingSphere = false
                            onCanceled: movingSphere = false

                            onWheel: function(wheel) {
                                sceneController.moveSelectedBy(0, 0, wheel.angleDelta.y > 0 ? -0.15 : 0.15)
                                wheel.accepted = true
                            }
                        }

                        Row {
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            anchors.margins: 14
                            spacing: 16

                            Label {
                                text: "X"
                                color: "#dc2626"
                                font.bold: true
                            }
                            Label {
                                text: "Y"
                                color: "#16a34a"
                                font.bold: true
                            }
                            Label {
                                text: "Z"
                                color: "#2563eb"
                                font.bold: true
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 620
                color: "#ffffff"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 46
                        color: "#ffffff"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 18

                            Label {
                                text: qsTr("相机画布")
                                color: "#26313d"
                                font.pixelSize: 15
                                font.bold: true
                            }

                            Item { Layout.fillWidth: true }

                            Label {
                                text: qsTr("CPU 光线追踪")
                                color: "#7a8490"
                                font.pixelSize: 12
                            }
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1
                            color: "#e4e7eb"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#eef1f4"

                        Image {
                            id: rayTracedImage
                            anchors.centerIn: parent
                            width: Math.min(parent.width - 40, parent.height - 40)
                            height: width
                            source: "image://raytracer/frame?revision=" + sceneController.renderRevision
                            sourceSize: Qt.size(sceneController.renderWidth, sceneController.renderHeight)
                            fillMode: Image.PreserveAspectFit
                            cache: false
                            smooth: true

                            Rectangle {
                                anchors.fill: parent
                                color: "transparent"
                                border.color: "#c8cdd3"
                                border.width: 1
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 152
            color: "#ffffff"

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: "#d7dbe0"
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                anchors.topMargin: 16
                anchors.bottomMargin: 16
                spacing: 24

                ColumnLayout {
                    Layout.preferredWidth: 190
                    Layout.maximumWidth: 220
                    spacing: 7

                    Label {
                        text: qsTr("球体")
                        color: "#4b5563"
                        font.pixelSize: 12
                    }

                    ComboBox {
                        Layout.fillWidth: true
                        model: sceneController.sphereModel
                        textRole: "name"
                        currentIndex: sceneController.selectedSphere
                        onActivated: sceneController.selectedSphere = currentIndex
                    }
                }

                Rectangle {
                    width: 1
                    Layout.fillHeight: true
                    color: "#e1e4e8"
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "X"; color: "#dc2626"; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: sceneController.selectedX.toFixed(2)
                            color: "#374151"
                            font.family: "Consolas"
                        }
                    }

                    Slider {
                        Layout.fillWidth: true
                        from: -3
                        to: 3
                        stepSize: 0.05
                        value: sceneController.selectedX
                        onMoved: sceneController.selectedX = value
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Y"; color: "#16a34a"; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: sceneController.selectedY.toFixed(2)
                            color: "#374151"
                            font.family: "Consolas"
                        }
                    }

                    Slider {
                        Layout.fillWidth: true
                        from: -2
                        to: 2
                        stepSize: 0.05
                        value: sceneController.selectedY
                        onMoved: sceneController.selectedY = value
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Z"; color: "#2563eb"; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: sceneController.selectedZ.toFixed(2)
                            color: "#374151"
                            font.family: "Consolas"
                        }
                    }

                    Slider {
                        Layout.fillWidth: true
                        from: 1.5
                        to: 8
                        stepSize: 0.05
                        value: sceneController.selectedZ
                        onMoved: sceneController.selectedZ = value
                    }
                }

                Button {
                    text: qsTr("重置场景")
                    Layout.alignment: Qt.AlignBottom
                    onClicked: sceneController.resetScene()
                }
            }
        }
    }
}

