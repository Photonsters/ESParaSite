//On Page load show graphs
document.addEventListener("DOMContentLoaded", function () {
  generateTableHead();
  getData();
  generateStatusTableHead();
  getStatusData();
});

// Generates the html table header part
function generateTableHead() {
  var table = document.getElementById("dataTable");
  var thead = table.createTHead();
  var row = thead.insertRow();
  row.appendChild(createHeadCell("Network"));
  var tbody = document.createElement("tbody");
  tbody.setAttribute("id", "tableBody");
  table.appendChild(tbody);
}

// Generates the html table header part
function generateStatusTableHead() {
  var table = document.getElementById("dataTable2");
  var thead = table.createTHead();
  var row = thead.insertRow();
  row.appendChild(createHeadCell("Printer Status"));
  var tbody = document.createElement("tbody");
  tbody.setAttribute("id", "tableBody2");
  table.appendChild(tbody);
}

// Fetches the data to show and calls the chart and table update
function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var json = this.responseText;
      var data = JSON.parse(json);
      updateTable(data);
    }
  };
  xhttp.open(
    "GET",
    "http://esparasite.local/guiFeed?rn=1" + new Date().getTime(),
    true
  ); //Handle readHistory server on ESP8266
  xhttp.send();
}

// Fetches the data to show and calls the chart and table update
function getStatusData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var json = this.responseText;
      var data = JSON.parse(json);
      updateStatusTable(data);
    }
  };
  xhttp.open(
    "GET",
    "http://esparasite.local/guiFeed?rs=1" + new Date().getTime(),
    true
  ); //Handle readHistory server on ESP8266
  xhttp.send();
}

// Updates the chart and table with the new data received
function updateTable(data) {
  var tbody = document.getElementById("tableBody");
  tbody.innerHTML = "";

  var row = tbody.insertRow();
  row.appendChild(createRowCell("Wifi Network:"));
  row.appendChild(createRowCell(data["ssid"]));
  var row = tbody.insertRow();
  row.appendChild(createRowCell("Signal Strength:"));
  var signal = convertRSSI(Math.abs(parseInt(data["rssi"]), 10));
  row.appendChild(createRowCell(signal));
  var row = tbody.insertRow();
  row.appendChild(createRowCell("IP Address:"));
  row.appendChild(createRowCell(data["ipaddr"]));
  var row = tbody.insertRow();

  if ((data["mdnsS"] = 1)) {
    row.appendChild(createRowCell("mDNS Status:"));
    row.appendChild(createRowCell("Enabled"));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("mDNS Name:"));
    row.appendChild(createRowCell(data["mdnsN"] + ".local"));
  }

}

function updateStatusTable(data) {
  var tbody = document.getElementById("tableBody2");
  tbody.innerHTML = "";

  var offset = new Date().getTimezoneOffset();
  var d = parseInt(data["lwts"]);
  var utc = d + offset * 60; //This converts to UTC 00:00
  dutc = new Date(utc * 1000);

  var row = tbody.insertRow();
  row.appendChild(createRowCell("Case Temperature:"));
  row.appendChild(createRowCell(data["castc"] + " °C"));
  var row = tbody.insertRow();
  row.appendChild(createRowCell("Last EEPROM Write:"));
  row.appendChild(createRowCell(dutc));
  var row = tbody.insertRow();
  row.appendChild(createRowCell("Printer Hours:"));
  row.appendChild(createRowCell(convertSeconds(data["prls"])));
  var row = tbody.insertRow();
  row.appendChild(createRowCell("Screen Hours:"));
  row.appendChild(createRowCell(convertSeconds(data["scrls"])));
  var row = tbody.insertRow();
  row.appendChild(createRowCell("Vat Hours:"));
  row.appendChild(createRowCell(convertSeconds(data["vatls"])));
  var row = tbody.insertRow();
  row.appendChild(createRowCell("LED Hours:"));
  row.appendChild(createRowCell(convertSeconds(data["ledls"])));

  // Get the data again in 5 seconds
  setTimeout(function () {
    getData();
  }, 5000);
}

// helper method
function createHeadCell(text) {
  var th = document.createElement("th");
  th.setAttribute("colspan", "2");
  th.appendChild(document.createTextNode(text));
  return th;
}

// helper method
function createRowCell(text) {
  var td = document.createElement("td");
  td.appendChild(document.createTextNode(text));
  return td;
}

function convertRSSI(value) {
  if (value >= 80) {
    return "Poor";
  } else if (value < 80 && value > 67) {
    return "Fair";
  } else if (value <= 67 && value > 30) {
    return "Good";
  } else if (value <= 30 && value > 67) {
    return "Excellent";
  }
}

function convertSeconds(seconds) {
  if (seconds <= 86400) {
    var hhmmss = new Date(seconds * 1000).toISOString().substr(11, 8);
    return hhmmss;
  } else {
    var dayshhmmss =
      Math.floor(seconds / 86400) +
      " Days " +
      new Date(seconds * 1000).toISOString().substr(11, 8);
    return dayshhmmss;
  }
}
