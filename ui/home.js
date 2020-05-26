//Graphs https://www.chartjs.org
var timeStamp = [];
var cTValues = [];
var cHValues = [];
var aTValues = [];
var aHValues = [];
var lTValues = [];
var sTValues = [];
var lOValues = [];

//On Page load show graphs
document.addEventListener("DOMContentLoaded", function () {
    generateTableHead();
    getData();
    showGraph();
});

//setInterval(function () {
// Call a function repetatively with 5 Second interval
//   getData();
//}, 5000); //5000mSeconds update rate


function generateTableHead() {
    var table = document.getElementById("dataTable");
    var thead = table.createTHead();
    var row = thead.insertRow();
    row.appendChild(createHeadCell("Time Stamp"));
    row.appendChild(createHeadCell("Chamber Temperature (°C)"));
    row.appendChild(createHeadCell("Chamber Humidity (%)"));
    row.appendChild(createHeadCell("Ambient Temperature (°C)"));
    row.appendChild(createHeadCell("Ambient Humidity (%)"));
    row.appendChild(createHeadCell("LED Temperature (°C)"));
    row.appendChild(createHeadCell("Screen Temperature (°C)"));
    row.appendChild(createHeadCell("LED"));
}

function createHeadCell(text) {
    var th = document.createElement("th");
    th.appendChild(document.createTextNode(text));
    return th;
}

function createRowCell(text) {
    var td = document.createElement("td");
    td.appendChild(document.createTextNode(text));
    return td;
}

function getData() {
    var table = document.getElementById("dataTable");
    var xhttp = new XMLHttpRequest();
    var offset = new Date().getTimezoneOffset();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            timeStamp.length = 0;
            cTValues.length = 0;
            cHValues.length = 0;
            aTValues.length = 0;
            aHValues.length = 0;
            var json = this.responseText;
            var obj = JSON.parse(json);

            for (var i = table.rows.length - 1; i > 0; i--) {
                table.deleteRow(i);
            }

            for (var k in obj) {
                var row = table.insertRow();
                var d = (obj[k]["ts"]);
                var utc = d + (offset * 60);  //This converts to UTC 00:00
                dutc = new Date(utc * 1000);

                timeStamp.push(dutc.toLocaleTimeString());
                cTValues.push(obj[k]["ct"]);
                cHValues.push(obj[k]["ch"]);
                aTValues.push(obj[k]["at"]);
                aHValues.push(obj[k]["ah"]);

                row.appendChild(createRowCell(dutc.toLocaleTimeString()));
                row.appendChild(createRowCell(obj[k]["ct"]));
                row.appendChild(createRowCell(obj[k]["ch"]));
                row.appendChild(createRowCell(obj[k]["at"]));
                row.appendChild(createRowCell(obj[k]["ah"]));
                row.appendChild(createRowCell(obj[k]["lt"]));
                row.appendChild(createRowCell(obj[k]["st"]));
                row.appendChild(createRowCell(obj[k]["lo"] ? "On" : "Off"));

            }

            updateGraph();

        }
    }

    xhttp.open("GET", "readHistory.html", true); //Handle readHistory server on ESP8266
    xhttp.send();
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
                backgroundColor: 'rgba( 146, 160, 254, 1)', //Dot marker color
                borderColor: 'rgba(146, 160, 254, 1)', //Graph Line Color
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

function updateGraph() {
    showGraph();
}



