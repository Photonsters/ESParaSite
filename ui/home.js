//Graphs https://www.chartjs.org
var timeStamp = [];
var cTValues = [];
var cHValues = [];
var aTValues = [];
var aHValues = [];
var lTValues = [];
var sTValues = [];
var lOValues = [];

let table = document.getElementById("dataTable");

function generateTableHead(table) {
    let thead = table.createTHead();
    let row = thead.insertRow();
    let th = document.createElement("th");
    let text = document.createTextNode("TimeStamp");
    th.appendChild(text);
    row.appendChild(th);
    th = document.createElement("th");
    text = document.createTextNode("Chamber Temperature (°C)");
    th.appendChild(text);
    row.appendChild(th);
    th = document.createElement("th");
    text = document.createTextNode("Chamber Humidity (%)");
    th.appendChild(text);
    row.appendChild(th);
    th = document.createElement("th");
    text = document.createTextNode("Ambient Temperature (°C)");
    th.appendChild(text);
    row.appendChild(th);
    th = document.createElement("th");
    text = document.createTextNode("Ambient Humidity (%)");
    th.appendChild(text);
    row.appendChild(th);
    th = document.createElement("th");
    text = document.createTextNode("LED Temperature (°C)");
    th.appendChild(text);
    row.appendChild(th);
    th = document.createElement("th");
    text = document.createTextNode("Screen Temperature (°C)");
    th.appendChild(text);
    row.appendChild(th);
    th = document.createElement("th");
    text = document.createTextNode("LED On");
    th.appendChild(text);
    row.appendChild(th);
}

function sortTable() {
    var table, rows, switching, i, x, y, shouldSwitch;
    table = document.getElementById("dataTable");
    switching = true;
    /* Make a loop that will continue until
    no switching has been done: */
    while (switching) {
        // Start by saying: no switching is done:
        switching = false;
        rows = table.rows;
        /* Loop through all table rows (except the
        first, which contains table headers): */
        for (i = 1; i < (rows.length - 1); i++) {
            // Start by saying there should be no switching:
            shouldSwitch = false;
            /* Get the two elements you want to compare,
            one from current row and one from the next: */
            x = rows[i].getElementsByTagName("TD")[0];
            y = rows[i + 1].getElementsByTagName("TD")[0];
            // Check if the two rows should switch place:
            if (x.innerHTML.toLowerCase() < y.innerHTML.toLowerCase()) {
                // If so, mark as a switch and break the loop:
                shouldSwitch = true;
                break;
            }
        }
        if (shouldSwitch) {
            /* If a switch has been marked, make the switch
            and mark that a switch has been done: */
            rows[i].parentNode.insertBefore(rows[i + 1], rows[i]);
            switching = true;
        }
    }
}



function showGraph() {
    var ctx = document.getElementById("Chart").getContext('2d');
    var Chart2 = new Chart(ctx, {
        type: 'line',
        data: {
            labels: timeStamp,  //Bottom Labeling
            datasets: [{
                label: "Chamber Temperature",
                fill: false,  //Try with true
                pointBackgroundColor: 'rgba(255 ,255 ,255 ,1)',
                pointRadius: '2',
                backgroundColor: 'rgba(215 ,13 ,13 ,1)', //Dot marker color
                borderColor: 'rgba(215 ,13 ,13 ,1)', //Graph Line Color
                fontColor: 'rgba(236 ,236, 236, 1)',
                data: cTValues,
            }, {
                label: "Chamber Humidity",
                fill: false,  //Try with true
                pointBackgroundColor: 'rgba(255 ,255 ,255 ,1)',
                pointRadius: '2',
                backgroundColor: 'rgba( 234, 119, 4, 1)', //Dot marker color
                borderColor: 'rgba( 234, 119, 4, 1)', //Graph Line Color
                fontColor: 'rgba(236 ,236, 236, 1)',
                data: cHValues,
            },
            {
                label: "Ambient Temperature",
                fill: false,  //Try with true
                pointBackgroundColor: 'rgba(255 ,255 ,255 ,1)',
                pointRadius: '2',
                backgroundColor: 'rgba(82, 99, 255, 1)', //Dot marker color
                borderColor: 'rgba(82, 99, 255, 1)', //Graph Line Color
                fontColor: 'rgba(236 ,236, 236, 1)',
                data: aTValues,
            },
            {
                label: "Ambient Humidity",
                fill: false,  //Try with true
                pointBackgroundColor: 'rgba(255 ,255 ,255 ,1)',
                pointRadius: '2',
                backgroundColor: 'rgba( 59, 1, 160, 1)', //Dot marker color
                borderColor: 'rgba( 59, 1, 160, 1)', //Graph Line Color
                fontColor: 'rgba(236 ,236, 236, 1)',
                data: aHValues,
            }],
        },
        options: {
            legend: {
                labels: {
                    fontColor: 'rgba(204 ,204, 204, 1)',
                }
            },
            title: {
                display: false,
                text: ""
            },
            maintainAspectRatio: false,
            elements: {
                line: {
                    tension: 0.5 //Smoothening (Curved) of data lines
                }
            },
            scales: {
                yAxes: [{
                    ticks: {
                        beginAtZero: true,
                        color: 'rgba(236 ,236, 236, 1)',
                        fontColor: 'rgba(236 ,236, 236, 1)',
                    },
                    gridLines: {
                        color: 'rgba(204 ,204, 204, 1)'
                    },
                }],
                xAxes: [{
                    ticks: {
                        color: 'rgba(236 ,236, 236, 1)',
                        fontColor: 'rgba(204 ,204, 204, 1)',
                    },
                    gridLines: {
                        color: 'rgba(204 ,204, 204, 1)'
                    },
                }],
            }
        }
    });

}

//On Page load show graphs
window.onload = function () {
    generateTableHead(table);
};


let timerId = setTimeout(function tick() {
    getData();
    timerId = setTimeout(tick, 5000); // (*)
}, 5000);
//setInterval(function () {
// Call a function repetatively with 5 Second interval
//   getData();
//}, 5000); //5000mSeconds update rate

function getData() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            timeStamp.length = 0;
            cTValues.length = 0;
            cHValues.length = 0;
            aTValues.length = 0;
            aHValues.length = 0;
            lTValues.length = 0;
            sTValues.length = 0;
            lOValues.length = 0;
            var json = this.responseText;
            var obj = JSON.parse(json); //this does not happen

            for (var i = table.rows.length - 1; i > 0; i--) {
                table.deleteRow(i);
            }

            var offset = new Date().getTimezoneOffset();

            for (var k in obj) {

                var d = (obj[k]["ts"]);
                var utc = d + (offset * 60);  //This converts to UTC 00:00
                dutc = new Date(utc * 1000);

                timeStamp.push(dutc.toLocaleTimeString());
                cTValues.push(obj[k]["ct"]);
                cHValues.push(obj[k]["ch"]);
                aTValues.push(obj[k]["at"]);
                aHValues.push(obj[k]["ah"]);
                lTValues.push(obj[k]["lt"]);
                sTValues.push(obj[k]["st"]);
                lOValues.push(obj[k]["lo"]);

                let row = table.insertRow();
                cell = row.insertCell();
                text = document.createTextNode(dutc.toLocaleTimeString());
                cell.appendChild(text);
                cell = row.insertCell();
                text = document.createTextNode(obj[k]["ct"]);
                cell.appendChild(text);
                cell = row.insertCell();
                text = document.createTextNode(obj[k]["ch"]);
                cell.appendChild(text);
                cell = row.insertCell();
                text = document.createTextNode(obj[k]["at"]);
                cell.appendChild(text);
                cell = row.insertCell();
                text = document.createTextNode(obj[k]["ah"]);
                cell.appendChild(text);
                cell = row.insertCell();
                text = document.createTextNode(obj[k]["lt"]);
                cell.appendChild(text);
                cell = row.insertCell();
                text = document.createTextNode(obj[k]["st"]);
                cell.appendChild(text);
                cell = row.insertCell();
                text = document.createTextNode(obj[k]["lo"]);
                cell.appendChild(text);

            }
            //sortTable(); //does not work properly with human readable timestamps
            showGraph();  //Update Graphs
        } else {
        };
    };
    xhttp.open("GET", "http://esparasite.local/readHistory", true); //Handle readHistory server on ESP8266
    xhttp.send();
}

