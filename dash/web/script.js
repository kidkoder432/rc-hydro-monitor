const scale = (num, inMin, inMax, outMin, outMax) =>
    ((num - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin;

eel.expose(close_window);
function close_window() {
    window.close();
}

function round(n, p) {
    return Math.round(n * Math.pow(10, p)) / Math.pow(10, p)
}

function hsvToRgb(h, s, v) {
    var r, g, b;

    var i = Math.floor(h * 6);
    var f = h * 6 - i;
    var p = v * (1 - s);
    var q = v * (1 - f * s);
    var t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0:
            (r = v), (g = t), (b = p);
            break;
        case 1:
            (r = q), (g = v), (b = p);
            break;
        case 2:
            (r = p), (g = v), (b = t);
            break;
        case 3:
            (r = p), (g = q), (b = v);
            break;
        case 4:
            (r = t), (g = p), (b = v);
            break;
        case 5:
            (r = v), (g = p), (b = q);
            break;
    }

    return `rgb(${Math.round(r * 255)},${Math.round(g * 255)},${Math.round(
        b * 255
    )})`;
}

function bg(x, a, b) {
    return hsvToRgb(scale(x, a, b, 0, 0.3333), 1, 1);
}

eel.expose(recv);
function recv(v, a) {
    console.log(v, a);
    const vd = document.getElementById("volt");
    const ad = document.getElementById("amps");

    const vp = document.getElementById("vp");
    const ap = document.getElementById("ap");

    if (a < 0) {
        ad.style.width = 100 + "%";
        ad.style.backgroundColor = "red";
    } else {
        ad.style.width = scale(a, 0, 3, 0, 100) + "%";
        ad.style.backgroundColor = bg(a, 0, 3);
    }

    if (v < 7) {
        vd.style.width = 100 + "%";
        vd.style.backgroundColor = "red";
    } else {
        vd.style.width = scale(v, 7, 9, 0, 100) + "%";
        vd.style.backgroundColor = bg(v, 7, 9);
    }

    ap.innerHTML = round(a,2) + " A";
    vp.innerHTML = round(v, 2) + " V ";
}

eel.expose(t);
function t(ts, lrt, r) {
    e = document.getElementById("time");
    if (r) {
        e.innerHTML = `Current Time: ${ts} <span class='r'>(last received ${Math.round(
            lrt
        )}s ago)</span>`;
    } else {
        e.innerHTML = `Current Time: ${ts} (last received ${Math.round(
            lrt
        )}s ago)`;
    }
}

eel.expose(setRem);
function setRem(id, startTime, currentTime, startVal, currentVal, finalVal) {
    let slope = (currentVal - startVal) / (currentTime - startTime);
    let t = round((finalVal - currentVal) / slope, 2);

    console.log(slope, t);

    let e = document.getElementById(id);
    e.innerHTML = Math.max(0, t);
}

eel.expose(setHTML);
function setHTML(id, h) {
    let e = document.getElementById(id);
    e.innerHTML = h;
}
