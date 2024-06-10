const colors = ["red", "blue", "green", "purple", "orange"];

function draw() {
    let size = parseInt(document.getElementById('size').value);
    let graph = document.getElementById("graph");

    // Clear the graph and resize it to the current size.
    removeAllChildNodes(graph)
    let width = 2 * size;
    graph.style.gridTemplateRows = "repeat(" + width.toFixed() + "," + (700 / width).toFixed() + "px)";
    graph.style.gridTemplateColumns = "repeat(" + width.toFixed() + "," + (700 / width).toFixed() + "px)";


    // Loop over every cell in the graph and create a div with 4 black borders.
    // size + 1 for the ghost bubbles whose top and left borders form the bottom and right borders of the graph.
    let lineWeight = (30 / size).toFixed();
    for(let r = -size; r < size + 1; r++) {
        for(let c = -size; c < size + 1; c++) {
            var div = document.createElement("div");
            div.id = (-r).toFixed() + "," + c.toFixed();

            div.style.gridRowStart = r + size + 1;
            div.style.gridRowEnd = r + size + 2;
            div.style.gridColumnStart = c + size + 1;
            div.style.gridColumnEnd = c + size + 2;

            if(r < size) {
                if(c == 0) {
                    // y-axis.
                    div.style.borderLeft = lineWeight + "px solid black";
                }
                else {
                    // Dotted non-axis lines.
                    div.style.borderLeft = 1 + "px dotted black";
                }
            }
            if(c < size) {
                if(r == 0) {
                    div.style.borderTop = lineWeight + "px solid black";
                }
                else {
                    div.style.borderTop = 1 + "px dotted black";
                }
            }
            graph.appendChild(div);
        }
    }

    for(let i = 1; i <= 5; i++) {
        let walk = document.getElementById('walk' + i.toFixed()).value;
        drawWalk(walk, colors[i - 1], size);
    }
}

// For each step, change the corresponding border to red.
function drawWalk(walk, color, size) {
    let x = 0;
    let y = 0;
    let xStep = 1;
    let yStep = 1;

    if(walk.includes("0") || walk.includes("1")) {
        // Reverse the order (assumes 01010101 is read right to left, flip if that's been changed)
        walk = walk.split("").reverse().join("");

        // Change from 01 format to HV format.
        walk = walk.replaceAll("0", "H");
        walk = walk.replaceAll("1", "V");
    }

    console.log(walk);

    for(const char of walk) {
        if(char == "H") {
            let newX = x + xStep;
            drawStep(x, y, newX, y, color, size);
            x = newX;
            yStep = -yStep;
        }
        else if(char == "V") {
            let newY = y + yStep;
            drawStep(x, y, x, newY, color, size);
            y = newY;
            xStep = -xStep;
        }
    }
}

// Clears the graph.
function removeAllChildNodes(parent) {
    while(parent.firstChild) {
        parent.removeChild(parent.firstChild);
    }
}

function drawStep(x1, y1, x2, y2, color, size) {
    let r = 0;
    let c = 0;
    let borderLocation = "";
    if(x2 == x1 + 1) {
        r = y1;
        c = x1;
        borderLocation = "borderTop";
    }
    else if(x2 == x1 - 1) {
        r = y1;
        c = x2;
        borderLocation = "borderTop";
    }
    else if(y2 == y1 + 1) {
        r = y1 + 1;
        c = x1;
        borderLocation = "borderLeft";
    }
    else {
        r = y2 + 1;
        c = x1;
        borderLocation = "borderLeft";
    }

    if(r > -size && r <= size && c >= -size && c < size) {
        let div = document.getElementById(r.toFixed() + "," + c.toFixed());
        let border = (50 / size).toFixed() + "px solid " + color;
        div.style[borderLocation] = border;
    }
}