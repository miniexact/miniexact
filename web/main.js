let isReady = false;
let focused_on_error = false;

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

    if(input.value.length > 0) {
        run_()
    }
}

function run_() {
    if(!isReady)
        return;

    isReady = false;
    run_element.disabled = true;
    focused_on_error = false;

    stdout.value = "";
    stderr.value = "";

    let algorithm = document.querySelector('input[name="algorithm"]:checked').value;
    let enumerate = document.getElementById("option-enumerate").checked ? 'e' : ' ';
    let verbose = document.getElementById("option-verbose").checked ? 'v' : ' ';
    let print_options = document.getElementById("option-print").checked ? 'p' : ' ';
    let problem = input.value;
    Module.solve(problem, algorithm, enumerate, verbose, print_options);
    localStorage.setItem('input', input.value);

    isReady = true;
    run_element.disabled = false;
}

function matchError(e) {
    let matcher = /^\[MINIEXACT\] \[ERROR\] Parse error at (\d+):(\d+) \(pos (\d+)\)/g;
    let arr = matcher.exec(e);
    if(arr !== null && !focused_on_error) {
	let line = arr[1];
	let col = arr[2];
	let pos = arr[3];
	input.focus();
	input.select();
	input.selectionStart = pos;
	input.selectionEnd = pos;
	focused_on_error = true;
    }
}

window.onload = function() {
    let i = window.localStorage.getItem('input');
    if(i != null && i.length > 0) {
        input.value = i;
    }
}

var Module = {
    'print': function(text) { stdout.value += text + "\n"; resize_textarea.call(stdout); },
    'printErr': function(text) { stderr.value += text + "\n"; matchError(text); resize_textarea.call(stderr); },
    'onRuntimeInitialized': function() { ready(); }
};

document.onkeydown = function (e) {
    if((e.ctrlKey || e.shiftKey) && e.key == "Enter") {
        e.preventDefault();
        run_();
    }
}

const tx = document.getElementsByTagName("textarea");
for (let i = 0; i < tx.length; i++) {
  tx[i].setAttribute("style", "height:" + (tx[i].scrollHeight) + "px;overflow-y:hidden;");
  tx[i].addEventListener("input", resize_textarea, false);
}

function resize_textarea() {
  this.style.height = "auto";
  this.style.height = (this.scrollHeight) + "px";
}
