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

    console.log(stdout.parentNode.dataset.replicatedValue)
}

function run_() {
    if(!isReady)
        return;

    stdout.value = "";
    stderr.value = "";

    let algorithm = document.querySelector('input[name="algorithm"]:checked').value;
    let enumerate = document.getElementById("option-enumerate").checked ? 'e' : ' ';
    let verbose = document.getElementById("option-verbose").checked ? 'v' : ' ';
    let print_options = document.getElementById("option-print").checked ? 'p' : ' ';
    Module.solve(input.value, algorithm, enumerate, verbose, print_options);
}

var Module = {
    'print': function(text) { stdout.value += text + "\n"; stdout.parentNode.dataset.replicatedValue = stdout.value; },
    'printErr': function(text) { stderr.value += text + "\n"; stderr.parentNode.dataset.replicatedValue = stderr.value; },
    'onRuntimeInitialized': function() { ready(); }
};

document.onkeydown = function (e) {
    if((e.ctrlKey || e.shiftKey) && e.key == "Enter") {
        e.preventDefault();
        run_();
    }
}
