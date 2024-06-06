import "./styles.css";
import * as THREE from 'three';

import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';

// 

import mqtt from "mqtt";
document.getElementById("app").innerHTML = `
<h1>Hello World!</h1>
<div>
  This example shows how to start a basic scene with a helpful axes coordinate system in the top right of the scene.
</div>
`;

var geometry, material, mesh, renderer, scene;
const cylinderRadius = 0.1;
const cylinderLength = 30;

const heading = document.querySelector('#heading');

function main() {
    const canvas = document.querySelector("#c");

    renderer = new THREE.WebGLRenderer({ canvas, alpha: true });

    const fov = 75;
    const aspect = 2; // the canvas default
    const near = 0.1;
    const far = 1000;
    const camera = new THREE.PerspectiveCamera(fov, aspect, near, far);
    camera.position.z = 2;

    const controls = new OrbitControls(camera, canvas);
    controls.update();

    scene = new THREE.Scene();
    // scene.background = new THREE.Color("lightblue");
    scene.background = new THREE.Color("black");

    const sceneView = {
        left: 0,
        bottom: 0,
        width: 1,
        height: 1
    };

    const coordScene = new THREE.Scene();
    coordScene.background = new THREE.Color(scene.background);

    const coordSceneRatio = 0.1;
    const coordSceneSize = 1 - coordSceneRatio;
    const coordSceneView = {
        left: sceneView.left + coordSceneSize,
        bottom: sceneView.bottom + coordSceneSize,
        width: sceneView.width * coordSceneRatio,
        height: sceneView.height * coordSceneRatio
    };

    const coordCamera = new THREE.OrthographicCamera(
        10 / -2,
        10 / 2,
        10 / 2,
        10 / -2,
        0.011,
        1000
    );
    // const coordCamera = new THREE.OrthographicCamera(
    //     100 / -2,
    //     100 / 2,
    //     100 / 2,
    //     100 / -2,
    //     0.011,
    //     1000
    // );
    coordCamera.position.copy(camera.position);

    // const axesHelper = new THREE.AxesHelper(5);
    // coordScene.add(axesHelper);

    //////////////////////////////////////////////////////////////////////////////////
    // BASIC SCENE
    //////////////////////////////////////////////////////////////////////////////////
    function addLight(x, y, z) {
        const color = 0xffffff;
        const intensity = 1;
        const light = new THREE.DirectionalLight(color, intensity);
        light.position.set(x, y, z);
        scene.add(light);
    }
    addLight(-1, 2, 4);
    addLight(1, -1, -2);


    // geometry = new THREE.BoxGeometry(0.8, 0.4, 0.05);
    // material = new THREE.MeshNormalMaterial();

    // mesh = new THREE.Mesh(geometry, material);
    // scene.add(mesh);


    // loadGltf(mesh);
    const loader = new GLTFLoader();
    loader.load(
        // 'assets/5543 LSM6DS3-LIS3MDL.gltf',
        'assets/OE-SOX-flames.gltf',
        function (gltf) {
            mesh = gltf.scene;
            scene.add(mesh);
        },
        function (xhr) {
            console.log((xhr.loaded / xhr.total * 100) + '% loaded');
        },
        function (error) {
            console.log('An error happened');
        }
    );

    // axes, slightly offset
    var targetPoint = new THREE.Vector3(-0.1, -0.1, -0.1); // make sure to update this on window resize
    const cylinderX = new THREE.Mesh(
        new THREE.CylinderGeometry(cylinderRadius, cylinderRadius, cylinderLength), // radius,height,length
        new THREE.MeshBasicMaterial({ color: 0xff0000 }) // Red - X
    );
    cylinderX.position.copy(targetPoint.clone().add(new THREE.Vector3(cylinderLength / 2, 0, 0)));
    cylinderX.rotation.set(0, 0, Math.PI / 2);
    scene.add(cylinderX);

    const cylinderY = new THREE.Mesh(
        new THREE.CylinderGeometry(cylinderRadius, cylinderRadius, cylinderLength),
        new THREE.MeshBasicMaterial({ color: 0x00ff00 }) // Green - Y
    );
    cylinderY.position.copy(targetPoint.clone().add(new THREE.Vector3(0, cylinderLength / 2, 0)));
    scene.add(cylinderY);

    const cylinderZ = new THREE.Mesh(
        new THREE.CylinderGeometry(cylinderRadius, cylinderRadius, cylinderLength),
        new THREE.MeshBasicMaterial({ color: 0x0000ff }) // Blue - Z
    );
    cylinderZ.position.copy(targetPoint.clone().add(new THREE.Vector3(0, 0, cylinderLength / 2)));
    cylinderZ.rotation.set(Math.PI / 2, 0, 0);
    scene.add(cylinderZ);

    function resizeRendererToDisplaySize(renderer) {
        const canvas = renderer.domElement;
        const pixelRatio = window.devicePixelRatio;
        const width = (canvas.clientWidth * pixelRatio) | 0;
        const height = (canvas.clientHeight * pixelRatio) | 0;
        const needResize = canvas.width !== width || canvas.height !== height;
        if (needResize) {
            renderer.setSize(width, height, false);
        }
        return needResize;
    }

    function renderSceneInfo(scene, camera, left, bottom, width, height) {
        const canvas = renderer.domElement;
        if (resizeRendererToDisplaySize(renderer)) {
            camera.aspect = canvas.clientWidth / canvas.clientHeight;
            camera.updateProjectionMatrix();
        }

        // take the coordinate from 0-1 space, and put them
        // in screen space, baed on the size of the canvas.
        var nleft = Math.floor(canvas.width * left);
        var nbottom = Math.floor(canvas.height * bottom);
        var nwidth = Math.floor(canvas.width * width);
        var nheight = Math.floor(canvas.height * height);
        renderer.setViewport(nleft, nbottom, nwidth, nheight);
        renderer.setScissor(nleft, nbottom, nwidth, nheight);
        renderer.setScissorTest(true);
        // renderer.setClearColor("lightblue");
        renderer.render(scene, camera);
    }

    let renderRequested = false;

    function render() {
        renderRequested = undefined;
        renderSceneInfo(
            scene,
            camera,
            sceneView.left,
            sceneView.bottom,
            sceneView.width,
            sceneView.height
        );

        coordScene.background = scene.background;
        coordCamera.position.copy(camera.position);
        coordCamera.position.sub(controls.target);
        coordCamera.lookAt(coordScene.position);

        renderSceneInfo(
            coordScene,
            coordCamera,
            coordSceneView.left,
            coordSceneView.bottom,
            coordSceneView.width,
            coordSceneView.height
        );

        controls.update();
    }
    render();

    function requestRenderIfNotRequested() {
        if (!renderRequested) {
            renderRequested = true;
            requestAnimationFrame(render);
        }
    }

    controls.addEventListener("change", requestRenderIfNotRequested);
    window.addEventListener("resize", requestRenderIfNotRequested);


}

function animate() {
    requestAnimationFrame(animate);
    renderer.render(scene, camera);
}

var correct = false;
var setRef = false;
var refPos = new THREE.Quaternion(0, 0, 0, 1);

function mqtt_setup() {
    const clientId = 'mqttjs_' + Math.random().toString(16).substr(2, 8)
    // const host = 'ws://' + location.host + ':81';
    const host = 'ws://' + 'sensorbox.local' + ':81';
    const options = {
        keepalive: 60,
        clientId: clientId,
        protocolId: 'MQTT',
        protocolVersion: 4,
        clean: true,
        reconnectPeriod: 1000,
        connectTimeout: 30 * 1000,
    }
    console.log('Connecting mqtt client');
    const client = mqtt.connect(host, options)
    client.on('error', (err) => {
        console.log('Connection error: ', err)
        client.end()
    })
    client.on('reconnect', () => {
        console.log('Reconnecting...')
    })
    client.on('connect', () => {
        console.log(`Client connected: ${clientId}`)
        client.subscribe('imu/quat9', { qos: 0 })
        client.subscribe('imu/hdg', { qos: 0 })
    })
    client.on('message', (topic, message, packet) => {
        var jsonObj = JSON.parse(message.toString());
        if (topic === 'imu/quat9') {
            // var targetQuaternion = new THREE.Quaternion(jsonObj.z, jsonObj.y, jsonObj.w, jsonObj.x);
            var targetQuaternion = new THREE.Quaternion(jsonObj.x, jsonObj.y, jsonObj.z, jsonObj.w);
            if (setRef) {
                refPos = targetQuaternion.clone().conjugate();
                setRef = false;
                // refpos.textContent = qtext("Reference", targetQuaternion);
            }
            if (correct) {
                mesh.quaternion.slerp(targetQuaternion.multiply(refPos), 1);
            } else {
                mesh.quaternion.slerp(targetQuaternion, 1);
            }
        }
        if (topic === 'imu/hdg') {
            //  heading.textContent = "Heading: " + message + "°";
        }
    })
}
mqtt_setup();
main();
// animate();