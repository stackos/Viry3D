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
const Engine = {
    Init: null,
    Done: null,
    Update: null,
    events: new Array(),
    down: false,
};

function OnMouseDown(e) {
    const x = e.clientX - gl.canvas.offsetLeft;
    const y = e.clientY - gl.canvas.offsetTop;

    Engine.events.push({
        type: "MouseDown",
        x: x,
        y: y,
    });
    Engine.down = true;
}

function OnMouseMove(e) {
    const x = e.clientX - gl.canvas.offsetLeft;
    const y = e.clientY - gl.canvas.offsetTop;

    if (Engine.down) {
        Engine.events.push({
            type: "MouseMove",
            x: x,
            y: y,
        });
    }
}

function OnMouseUp(e) {
    const x = e.clientX - gl.canvas.offsetLeft;
    const y = e.clientY - gl.canvas.offsetTop;

    Engine.events.push({
        type: "MouseUp",
        x: x,
        y: y,
    });
    Engine.down = false;
}

function IsMobilePlatform() {
    return /Android|webOS|iPhone|iPod|BlackBerry/i.test(navigator.userAgent);
}

function Render() {
    const msg = {
        events: Engine.events,
    };

    Engine.Update(JSON.stringify(msg));

    // clear events
    Engine.events.splice(0, Engine.events.length);

    window.requestAnimationFrame(Render);
}

function Main() {
    Engine.Init = Module.cwrap("InitEngine", null, ["string"]);
    Engine.Done = Module.cwrap("DoneEngine", null, ["string"]);
    Engine.Update = Module.cwrap("UpdateEngine", null, ["string"]);

    let glesv3 = false;
    const canvas = Module.canvas;
    let context = GL.createContext(canvas, {
        majorVersion: 2,
        minorVersion: 0,
    })
    if (context == 0) {
        context = GL.createContext(canvas, {
            majorVersion: 1,
            minorVersion: 0,
        })
    } else {
        glesv3 = true;
    }
    GL.makeContextCurrent(context);
    gl = Module.ctx;

    if (IsMobilePlatform()) {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    } else {
        canvas.width = 1280;
        canvas.height = 720;
    }

    canvas.onmousedown = function (e) {
        OnMouseDown(e);
    };
    canvas.onmousemove = function (e) {
        OnMouseMove(e);
    };
    canvas.onmouseup = function (e) {
        OnMouseUp(e);
    };

    const canvas_width = canvas.width;
    const canvas_height = canvas.height;

    const msg = {
        name: "Viry3D",
        width: canvas_width,
        height: canvas_height,
        glesv3: glesv3,
    };

    Engine.Init(JSON.stringify(msg));

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
