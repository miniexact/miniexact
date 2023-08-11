let isReady = false;

let input = document.getElementById("input");
let stdout = document.getElementById("stdout");
let stderr = document.getElementById("stderr");
let run_element = document.getElementById("run-btn");
let loader = document.getElementById("loading-indicator");

input.parentNode.dataset.replicatedValue = input.value;
stdout.parentNode.dataset.replicatedValue = stdout.value;
stderr.parentNode.dataset.replicatedValue = stderr.value;

function ready() {
    loader.style.display = "none";
    run_element.style.display = "block";
    isReady = true;
}

function run_() {
    if(!isReady)
        return;

    isReady = false;
    run_element.disabled = true;

    stdout.innerText = "";
    stderr.innerText = "";

    let algorithm = document.querySelector('input[name="algorithm"]:checked').value;
    let enumerate = document.getElementById("option-enumerate").checked ? 'e' : ' ';
    let verbose = document.getElementById("option-verbose").checked ? 'v' : ' ';
    let print_options = document.getElementById("option-print").checked ? 'p' : ' ';
    Module.solve(input.innerText, algorithm, enumerate, verbose, print_options);

    isReady = true;
    run_element.disabled = false;
}

function matchError(e) {
    let matcher = /^\[XCC\] \[ERROR\] Parse error at (\d+):(\d+)/g;
    let arr = matcher.exec(e);
}

var Module = {
    'print': function(text) { stdout.innerHTML += text + "<br>"; },
    'printErr': function(text) { stderr.innerHTML += text + "<br>"; matchError(text) },
    'onRuntimeInitialized': function() { ready(); }
};

document.onkeydown = function (e) {
    if((e.ctrlKey || e.shiftKey) && e.key == "Enter") {
        e.preventDefault();
        run_();
    }
}
