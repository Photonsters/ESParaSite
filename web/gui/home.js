var debug = 2;

//Graphs https://www.chartjs.org
var chartInstance;

var timeStamps = [];
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
  showGraph();
  getData();
});

// Generates the html table header part
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
  var tbody = document.createElement("tbody");
  tbody.setAttribute("id", "tableBody");
  table.appendChild(tbody);
}

// Creates the chart, sets visuals for it, links the datasources to the global variables
function showGraph() {
  var ctx = document.getElementById("Chart").getContext("2d");
  chartInstance = new Chart(ctx, {
    type: "line",
    data: {
      labels: timeStamps, //Bottom Labeling
      datasets: [
        {
          label: "Chamber Temperature",
          fill: false, //Try with true
          pointBackgroundColor: "rgba(215 ,13 ,13 ,1)",
          pointRadius: "3",
          backgroundColor: "rgba(215 ,13 ,13 ,1)", //Dot marker color
          borderColor: "rgba(215 ,13 ,13 ,1)", //Graph Line Color
          fontColor: "rgba(236 ,236, 236, 1)",
          data: cTValues,
        },
        {
          label: "Screen Temperature",
          fill: false, //Try with true
          pointBackgroundColor: "rgba( 234, 119, 4, 1)",
          pointRadius: "3",
          backgroundColor: "rgba( 234, 119, 4, 1)", //Dot marker color
          borderColor: "rgba( 234, 119, 4, 1)", //Graph Line Color
          fontColor: "rgba(236 ,236, 236, 1)",
          data: sTValues,
        },
        {
          label: "Ambient Temperature",
          fill: false, //Try with true
          pointBackgroundColor: "rgba(82, 99, 255, 1)",
          pointRadius: "3",
          backgroundColor: "rgba(82, 99, 255, 1)", //Dot marker color
          borderColor: "rgba(82, 99, 255, 1)", //Graph Line Color
          fontColor: "rgba(236 ,236, 236, 1)",
          data: aTValues,
        },
        {
          label: "Ambient Humidity",
          fill: false, //Try with true
          pointBackgroundColor: "rgba( 146, 160, 254, 1)",
          pointRadius: "3",
          backgroundColor: "rgba( 146, 160, 254, 1)", //Dot marker color
          borderColor: "rgba(146, 160, 254, 1)", //Graph Line Color
          fontColor: "rgba(236 ,236, 236, 1)",
          data: aHValues,
        },
      ],
    },
    options: {
      legend: {
        labels: {
          fontColor: "rgba(204 ,204, 204, 1)",
        },
      },
      title: {
        display: false,
        text: "",
      },
      maintainAspectRatio: false,
      elements: {
        line: {
          tension: 0.5, //Smoothening (Curved) of data lines
        },
      },
      scales: {
        yAxes: [
          {
            ticks: {
              beginAtZero: true,
              color: "rgba(236 ,236, 236, 1)",
              fontColor: "rgba(236 ,236, 236, 1)",
            },
            gridLines: {
              color: "rgba(204 ,204, 204, 1)",
            },
          },
        ],
        xAxes: [
          {
            ticks: {
              color: "rgba(236 ,236, 236, 1)",
              fontColor: "rgba(204 ,204, 204, 1)",
            },
            gridLines: {
              color: "rgba(204 ,204, 204, 1)",
            },
          },
        ],
      },
    },
  });
}

// Fetches the data to show and calls the chart and table update
function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var raw = this.responseText;
      var raw = raw.replace(/(null)/g, "");
      var json = raw.replace(/(\]\[)/g, ",");
      var data = JSON.parse(json);
      updateGraphAndTable(data);
    }
  };
  if (debug == 1) {
    xhttp.open("GET", "json/readHistory.json", true);
    xhttp.send();
  } else if (debug == 2) {
    xhttp.open(
      "GET",
      "http://esparasite.local/api?readHistory=" + new Date().getTime(),
      true
    );
    xhttp.send();
  } else {
    xhttp.open("GET", "api?readHistory=" + new Date().getTime(), true);
    xhttp.send();
  }
}

// Updates the chart and table with the new data received
function updateGraphAndTable(data) {
  var tbody = document.getElementById("tableBody");
  var offset = new Date().getTimezoneOffset();
  timeStamps.length = 0;
  cTValues.length = 0;
  sTValues.length = 0;
  aTValues.length = 0;
  aHValues.length = 0;

  tbody.innerHTML = "";

  data.forEach(function (dataItem) {
    var row = tbody.insertRow();
    
    var utc = dataItem["ts"];
    dutc = new Date(utc * 1000);

    timeStamps.push(dutc.toLocaleTimeString());
    cTValues.push(dataItem["ct"]);
    sTValues.push(dataItem["st"]);
    aTValues.push(dataItem["at"]);
    aHValues.push(dataItem["ah"]);

    row.appendChild(createRowCell(dutc.toLocaleTimeString()));
    row.appendChild(createRowCell(dataItem["ct"]));
    row.appendChild(createRowCell(dataItem["ch"]));
    row.appendChild(createRowCell(dataItem["at"]));
    row.appendChild(createRowCell(dataItem["ah"]));
    row.appendChild(createRowCell(dataItem["lt"]));
    row.appendChild(createRowCell(dataItem["st"]));
    row.appendChild(createRowCell(dataItem["lo"] ? "On" : "Off"));
  });

  chartInstance.update();

  // Get the data again in 5 seconds
  setTimeout(function () {
    getData();
  }, 5000);
}

// helper method
function createHeadCell(text) {
  var th = document.createElement("th");
  th.appendChild(document.createTextNode(text));
  return th;
}

// helper method
function createRowCell(text) {
  var td = document.createElement("td");
  td.appendChild(document.createTextNode(text));
  return td;
}
