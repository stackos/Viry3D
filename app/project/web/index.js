/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

let gl = null;
let InitEngine = null;
let DoneEngine = null;
let UpdateEngine = null;

function IsMobilePlatform() {
    return /Android|webOS|iPhone|iPod|BlackBerry/i.test(navigator.userAgent);
}

function Render() {
    UpdateEngine("");

    window.requestAnimationFrame(Render);
}

function Main() {
    InitEngine = Module.cwrap("InitEngine", null, ["string"]);
    DoneEngine = Module.cwrap("DoneEngine", null, ["string"]);
    UpdateEngine = Module.cwrap("UpdateEngine", null, ["string"]);

    const canvas = Module.canvas;
    GL.makeContextCurrent(GL.createContext(canvas, {}));
    gl = Module.ctx;
    
    if (IsMobilePlatform()) {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    } else {
        canvas.width = 1280;
        canvas.height = 720;
    }

    let canvas_width = canvas.width;
    let canvas_height = canvas.height;

    let msg = {
        name: "Viry3D",
        width: canvas_width,
        height: canvas_height,
        glesv3: false,
    };

    InitEngine(JSON.stringify(msg));

    Render();
}

var Module = {
    preRun: [],
    postRun: function () {
        Main();
    },
    print: (function () {
        return function (text) {
            if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
            console.log(text);
        };
    })(),
    printErr: function (text) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        if (0) { // XXX disabled for safety typeof dump == 'function') {
            dump(text + '\n'); // fast, straight to the real console
        } else {
            console.error(text);
        }
    },
    canvas: (function () {
        var canvas = document.getElementById("canvas");

        // As a default initial behavior, pop up an alert when webgl context is lost. To make your
        // application robust, you may want to override this behavior before shipping!
        // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
        canvas.addEventListener("webglcontextlost", function (e) {
            alert("WebGL context lost. You will need to reload the page.");
            e.preventDefault();
        }, false);

        return canvas;
    })()
};

window.onerror = function (event) {
    Module.setStatus = function (text) {
        if (text) Module.printErr("[post-exception status] " + text);
    };
};
