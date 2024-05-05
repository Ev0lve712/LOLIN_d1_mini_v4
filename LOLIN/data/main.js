var endpoint = window.location.hostname == "127.0.0.1" ? "192.168.0.100" : window.location.hostname;
var accelerometerPre;
var px, py, pz;
var currentDataIndex = 0;
var currentState = -1;

var cvs = document.getElementById('accel-canvas');
var ctx = cvs.getContext('2d');
var imageData = ctx.getImageData(0, 0, cvs.width, cvs.height);

var sentences = [];
var currentSentence = 0;
var wasDetectingMovement = false;
var manualOverride = false;
var logContainer;
function* routine() {
    var pre;
    logContainer = document.getElementById('log');
    pre = document.createElement('pre');
    logContainer.appendChild(pre);

    while (true) {
        if (currentSentence == sentences.length) {
            yield;
        } else {
            yield* typedSentenceRoutine(pre, sentences[currentSentence].text, sentences[currentSentence].speed);
            currentSentence++;
        }
    }
}

function* typedSentenceRoutine(element, text, speed) {
    var baseText = element.innerHTML;
    var newText = "";
    var letter = 0;
    while (letter < text.length) {
        var lettersPerTick = speed >= 1 ? speed : 1;
        var ticksPerLetter = speed < 1 ? 1 / speed : 1;
        newText += text.substring(letter, letter + lettersPerTick).replace(/[ ]/g, "&nbsp;");
        element.innerHTML = baseText + newText;
        logContainer.scrollTop = logContainer.scrollHeight;
        letter += lettersPerTick;
        for (var i = 0; i < ticksPerLetter; i++) {
            yield;
        }
    }
}

function* delayTicks(ticks) {
    var i = 0;
    while (i < ticks) {
        i++;
        yield;
    }
}

var d = new Date();
sentences.push({
    text: d.getDate() + "-" + (d.getMonth() + 1) + "-" + d.getFullYear() + " " + d.getHours() + ":" + d.getMinutes() + "\n",
    speed: 1
});
sentences.push({ text: "Домашний сервер MONOLITcorp\n", speed: 1 });
sentences.push({ text: "Номер [FUAX-3722-1628]\n", speed: 1 });
sentences.push({ text: "Проверка Статуса", speed: 1 });
sentences.push({ text: "........\n", speed: 0.1 });
sentences.push({ text: "Статус: [Онлайн]\n", speed: 1 });

var r = routine();
setInterval(() => {
    r.next();
}, 16);

function setOpen(open) {
    var xhr = new XMLHttpRequest();
    var fd = new FormData();
    fd.append("open", open ? 1 : 0);
    xhr.open("POST", `http://${endpoint}/set_open`, true);
    xhr.send(fd);
}

function setState(state) {
    var xhr = new XMLHttpRequest();
    var fd = new FormData();
    fd.append("state", state);
    xhr.open("POST", `http://${endpoint}/set_state`, true);
    xhr.send(fd);
}

console.log(`ws://${endpoint}:81/`);
var connection = new WebSocket(`ws://${endpoint}:81/`);
connection.binaryType = 'arraybuffer';

connection.onopen = () => {
    console.log("connected");
};

connection.onerror = (error) => {
    console.log('WebSocket Error ', error);
};

connection.onmessage = (e) => {
	let Str = e.data;
	let Info = Str.split('_');
	sentences.push({ text: "Температура: " + Info[0] + "\n" , speed: 0.7 });
	sentences.push({ text: "Влажность: " + Info[1] + "\n" , speed: 0.7 });
	sentences.push({ text: "Свобоной памяти: " + Info[2] + " KB\n" , speed: 0.7 });
};

function getTimeStamp() {
    d = new Date();
    var h = d.getHours().toString().padStart(2, "0");
    var m = d.getMinutes().toString().padStart(2, "0");
    var s = d.getSeconds().toString().padStart(2, "0");
    return `${h}:${m}:${s}`;
}

function setPixel(x, y, r, g, b) {
    var i = (y * cvs.width + x) * 4;
    imageData.data[i + 0] = r;
    imageData.data[i + 1] = g;
    imageData.data[i + 2] = b;
    imageData.data[i + 3] = 255;
}

function draw(x, y1, y2, c) {
    ctx.beginPath();
    ctx.strokeStyle = c;
    ctx.moveTo(x * 2, y1);
    ctx.lineTo(x * 2 + 2, y2);
    ctx.stroke();
}

connection.onclose = () => {
    console.log('WebSocket connection closed');
};

window.onbeforeunload = function () {
    connection.onclose = function () { }; // disable onclose handler first
    connection.close();
};

document.getElementById('button-wol').addEventListener('click', ()=>{
	sentences.push({ text: "Команда на включение отправлена\n", speed: 0.7 });
	connection.send("wake");
});

document.getElementById('button-send').addEventListener('click', ()=>{
	let input = document.getElementById('TextInput');
	sentences.push({ text: "::" + input.value + "отправлено на LCD \n", speed: 1 });
	connection.send("::" + input.value);
});

document.getElementById('button-refresh').addEventListener('click', ()=>{
	sentences.push({ text: "Запрос обновить показатели.\n", speed: 1 });
    connection.send("refresh");
});

document.getElementById('SetClock').addEventListener('click', ()=>{
	let inpHourse = document.getElementById('Hourse');
	let inpMinuts = document.getElementById('Minuts');
	sentences.push({ text: "Время установлено " + inpHourse.value + ":" + inpMinuts.value + "\n", speed: 0.7 });
    connection.send("setClock");
});

for (var i = 0; i < stateButtons.length; i++) {
    let state = stateButtons[i].getAttribute('data-state');
    stateButtons[i].addEventListener('mousedown', () => {
        setState(state);
    });
}