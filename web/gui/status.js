var debug = 2;

//On Page load show graphs
document.addEventListener("DOMContentLoaded", function () {
  generateTableHead("0", "Network");
  getData("0", "Network");
  generateTableHead("1", "Printer Status");
  getData("1", "Status");
});

// Generates the html table header part
function generateTableHead(elementID, title) {
  var table = document.getElementById("infoTable" + elementID);
  var thead = table.createTHead();
  var row = thead.insertRow();
  row.appendChild(createHeadCell(title));
  var tbody = document.createElement("tbody");
  tbody.setAttribute("id", "tableBody" + elementID);
  table.appendChild(tbody);
}

// Fetches the data to show and calls the chart and table update
function getData(elementID, dataset) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var json = this.responseText;
      var data = JSON.parse(json);
      updateTable(data, elementID, dataset);
    }
  };
  if (debug == 1) {
    if (dataset == "Network") {
      xhttp.open("GET", "json/readNetwork.json", true);
    } else if (dataset == "Status") {
      xhttp.open("GET", "json/readStatus.json", true);
    }
  } else if (debug == 2) {
    if (dataset == "Network") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readNetwork=" + new Date().getTime(),
        true
      );
    } else if (dataset == "Status") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readStatus=" + new Date().getTime(),
        true
      );
    }
  } else {
    if (dataset == "Network") {
      xhttp.open("GET", "api?readNetwork=" + new Date().getTime(), true);
    } else if (dataset == "Status") {
      xhttp.open("GET", "api?readStatus=" + new Date().getTime(), true);
    }
  }
  xhttp.send();
}

// Updates the table with the new data received
function updateTable(data, elementID, dataset) {
  var tbody = document.getElementById("tableBody" + elementID);
  tbody.innerHTML = "";
  console.log(dataset);
  if (dataset == "Network") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Wifi Network:"));
    row.appendChild(createRowCell(data["ssid"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Signal Strength:"));
    var signal = convertRSSI(Math.abs(parseInt(data["rssi"]), 10));
    row.appendChild(createRowCell(signal));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("IP Address:"));
    row.appendChild(createRowCell(data["ipAddr"]));
    var row = tbody.insertRow();

    if ((data["mdnsS"] = 1)) {
      row.appendChild(createRowCell("mDNS Status:"));
      row.appendChild(createRowCell("Enabled"));
      var row = tbody.insertRow();
      row.appendChild(createRowCell("mDNS Name:"));
      row.appendChild(createRowCell(data["mdnsN"] + ".local"));
    }
  } else if (dataset == "Status") {
    var offset = new Date().getTimezoneOffset();
    var d = parseInt(data["lwts"]);
    var utc = d + offset * 60; //This converts to UTC 00:00
    dutc = new Date(utc * 1000);

    var row = tbody.insertRow();
    row.appendChild(createRowCell("Case Temperature:"));
    row.appendChild(createRowCell(data["castc"] + " " + "°C"));
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
  }
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
