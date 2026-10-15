import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D

// 第 2 章交互演示主窗口。
// 左侧 Qt Quick 3D 仅解释空间关系；右侧图像完全由 C++ CPU 光线追踪器生成。
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

    // 顶栏展示当前章节、渲染分辨率以及最近一帧 CPU 耗时。
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

        // 主工作区保持左右并列，便于把三维场景和相机最终看到的画布直接对照。
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

                        // 教学用三维场景。sceneView 的 observerCamera 只是旁观者相机，
                        // 不参与右侧光线追踪；书中位于原点的相机由下方 Node 单独绘制。
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

                            // 从斜上方观察整个教学场景，使概念相机、视口和球体同时可见。
                            PerspectiveCamera {
                                id: observerCamera
                                position: Qt.vector3d(650, 440, 920)
                                eulerRotation: Qt.vector3d(-18, 35, 0)
                                fieldOfView: 46
                                clipFar: 3000
                            }

                            // 这些灯光只用于增强左侧模型的空间感，不会影响右侧纯色渲染结果。
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

                            // 中性地面提供稳定的空间参照，不属于书中的待渲染场景。
                            Model {
                                source: "#Cube"
                                position: Qt.vector3d(0, -220, -300)
                                scale: Qt.vector3d(8, 0.015, 8)
                                castsShadows: false
                                receivesShadows: true
                                materials: [PrincipledMaterial {
                                    baseColor: '#9c216d'
                                    roughness: 1.0
                                }]
                            }

                            // 坐标映射：书本场景的 +Z 指向远方，而此处 Quick 3D 相机朝 -Z。
                            // 因此所有书本坐标 (x,y,z) 在左侧显示为 (100x,100y,-100z)。
                            // 三条细长立方体分别绘制 Z、X、Y 坐标轴。
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

                            // 书中的概念相机位于原点。灰色立方体表示机身，橙色圆柱表示镜头；
                            // 它与 observerCamera 分离，避免“观察教学场景”和“参与算法的相机”混淆。
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

                            // 投影平面大小为 1x1 个书本场景单位，位于 z=1。
                            // 由于上面的坐标映射，它在 Quick 3D 中位于 z=-100、宽高各 100。
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

                            // 同一个 SphereModel 同时驱动左侧 3D 委托和右侧 CPU 渲染器，
                            // 从数据源上保证两边不会出现位置不同步。
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

                                    // pickable 使 View3D.pick() 能从鼠标位置反查具体模型。
                                    pickable: true
                                    position: Qt.vector3d(centerX * 100, centerY * 100, -centerZ * 100)

                                    // 内置 #Sphere 的直径为 100；半径 r 应显示为直径 200r，故缩放 2r。
                                    scale: Qt.vector3d(radius * 2, radius * 2, radius * 2)
                                    castsShadows: true
                                    receivesShadows: true

                                    materials: [PrincipledMaterial {
                                        baseColor: sphereColor
                                        roughness: 0.52

                                        // 轻微自发光只标记当前选择，不改变 C++ 中的球体固有颜色。
                                        emissiveFactor: sphereIndex === sceneController.selectedSphere
                                                        ? Qt.vector3d(0.12, 0.12, 0.12)
                                                        : Qt.vector3d(0, 0, 0)
                                    }]
                                }
                            }
                        }

                        // 透明 MouseArea 覆盖 View3D，把二维指针事件转换成球体选择和位置增量。
                        // 拖动修改书本 X/Y 坐标；滚轮修改深度 Z。所有边界校验由 C++ 控制器完成。
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
                                // pick() 返回射线首先击中的 Quick 3D 对象；objectName 保存模型行索引。
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

                                // 每 100 个屏幕像素对应 1 个书本场景单位；屏幕 Y 方向需要翻转。
                                const dx = (mouse.x - previousX) * 0.01
                                const dy = -(mouse.y - previousY) * 0.01
                                sceneController.moveSelectedBy(dx, dy, 0)
                                previousX = mouse.x
                                previousY = mouse.y
                            }

                            onReleased: movingSphere = false
                            onCanceled: movingSphere = false

                            onWheel: function(wheel) {
                                // 正滚轮把球体拉近相机，因此书本场景中的正 Z 值减小。
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
                            // 查询参数形成随 revision 变化的 URL，强制 QML 在新帧发布后重新
                            // 调用 RayImageProvider，而不是继续使用上一帧的图片缓存。
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

        // 底部控制区直接绑定当前球体坐标；滑块变化交给 C++ 限幅并调度新帧。
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

