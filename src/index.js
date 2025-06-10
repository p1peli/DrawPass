//const charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
const upperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const lowerChars = "abcdefghijklmnopqrstuvwxyz";
const numberChars = "0123456789";
const specialChars = "!@#$%^&*()_+[]{}|;:,.<>?"; 

var includeNumbers = document.getElementById("includeNumbers").checked;
var includeSpecial = document.getElementById("includeSpecial").checked; 

function createCharset(nc, sc) {
    var charset = upperChars + lowerChars; 
    if (nc) {
        charset += numberChars; 
    }
    if (sc) {
        charset += specialChars; 
    }

    return charset
}

var c = document.getElementById("myCanvas");
var ctx = c.getContext("2d");
var isDrawing = false;
var tipSize = document.getElementById("tipRange").value;
var password_length = document.getElementById("lengthRange").value;
var points = 0;
document.getElementById("points").innerHTML = points;

ctx.fillStyle = "black";


const coordinates = [];

function getMousePos(canvas, evt) {
    const rect = canvas.getBoundingClientRect();

    const clientX = evt.clientX || (evt.touches && evt.touches[0] ? evt.touches[0].clientX : 0);
    const clientY = evt.clientY || (evt.touches && evt.touches[0] ? evt.touches[0].clientY : 0);

    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;

    return {
        x: (clientX - rect.left) * scaleX,
        y: (clientY - rect.top) * scaleY
    };    
}

function Draw(event, canvas) {
    event.preventDefault();

    var pos = getMousePos(canvas, event);

    ctx.beginPath();
    ctx.arc(pos.x, pos.y, tipSize, 0, Math.PI * 2);
    ctx.fill();
    coordinates.push({ x: pos.x, y: pos.y, time_stamp: Date.now() });
    convertToString();

    points += 1;
    document.getElementById("points").innerHTML = points;
}

async function convertToString() {
    if (coordinates.length > 0) {
        const inputString = JSON.stringify(coordinates) + ctx.fillStyle.toString(); //Adding color as Salt (this is mostly for fun, since it doesn't add any noticeable entropy)
        //console.log(inputString);
        const encoder = new TextEncoder();
        const data = encoder.encode(inputString);

        const hashBuffer = await crypto.subtle.digest('SHA-256', data); //Using SHA-256 to compress data to given length, max. 32 characters since with a bigger charset we need 8bits per character, if we use base64 encoding we could use 6bits per character to get around 44 characters
        const hashArray = Array.from(new Uint8Array(hashBuffer));

        var charset = createCharset(includeNumbers, includeSpecial); 
        var password = "";
        for (let i = 0; i < password_length; i++) {
            password += charset[hashArray[i % hashArray.length] % charset.length];
        }

        document.getElementById("password").textContent = password;
    }
}

// Buttons: 
const colorButtons = document.querySelectorAll(".colorcontainer button");

function changeColor(clr) {
    ctx.fillStyle = clr;
    console.log("[Log] Changed color to: ", clr);

    // Deselect all buttons
    colorButtons.forEach(btn => btn.classList.remove("selected-color"));

    event.target.classList.add("selected-color");
}

function clearCanvas(canvas) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    coordinates.length = 0;
    points = 0;
    document.getElementById("password").innerHTML = "";
    document.getElementById("points").innerHTML = points;
    console.log("[Log] Cleared Canvas");
}

//Copy Pwd to Clipboard
function copyPwd() {
    var password = document.getElementById("password").textContent; 

    if (!password) {
        console.warn("[Log] No password to copy.");
        return;
    }

    navigator.clipboard.writeText(password)
    .then(() => {
        console.log("[Log] Copied password to clipboard!");
    })
    .catch(err => {
        console.error("[Log] Error while copying: ", err);
    });
}


// Mouse Controls: 
c.addEventListener('mousedown', function () {
    isDrawing = true;
});

c.addEventListener('mouseup', function () {
    isDrawing = false;
});

c.addEventListener('mouseleave', function () {
    isDrawing = false;
});

c.addEventListener('mousemove', function (event) {
    if (isDrawing) {
        Draw(event, c);
    }
});

// Touch Controls: (Mobile support)
c.addEventListener('touchstart', function (event) {
    event.preventDefault();
    isDrawing = true;
    Draw(event, myCanvas);
}, { passive: false }); // Ensure this is still { passive: false }

c.addEventListener('touchend', function () {
    isDrawing = false;
});

c.addEventListener('touchcancel', function () { // When a touch is interrupted
    isDrawing = false;
});

c.addEventListener('touchmove', function (event) {
    event.preventDefault();
    if (isDrawing) {
        Draw(event, c);
    }
}, { passive: false }); // Use { passive: false } to allow preventDefault


// Sliders: 
var tip_slider = document.getElementById("tipRange");

tip_slider.oninput = function () {
    tipSize = this.value;
}

var length_slider = document.getElementById("lengthRange");

length_slider.oninput = function () {
    password_length = this.value;
    convertToString();  
}

//Checkboxes: 
var numberBox = document.getElementById("includeNumbers"); 

numberBox.onchange = function () {
    includeNumbers = this.checked; 
    console.log("[Log] Changed includeNumbers to: ", includeNumbers)
    convertToString(); 
}

var specialBox = document.getElementById("includeSpecial"); 

specialBox.onchange = function() {
    includeSpecial = this.checked; 
    console.log("[Log] Changed includeSpecial to: ", includeSpecial)
    convertToString(); 
}