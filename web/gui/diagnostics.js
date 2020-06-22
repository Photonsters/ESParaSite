var debug = 2;

// On Page load show graphs
document.addEventListener("DOMContentLoaded", function () {
  generateTableHead("0", "Ambient");
  getData("0", "Ambient");
  generateTableHead("1", "Print Chamber");
  getData("1", "Chamber");
  generateTableHead("2", "EEPROM");
  getData("2", "Eeprom");
  generateTableHead("3", "Enclosure");
  getData("3", "Enclosure");
  generateTableHead("4", "I2C Bus");
  getData("4", "I2C");
  generateTableHead("5", "Optics");
  getData("5", "Optics");
  generateTableHead("6", "File System Info");
  getData("6", "FSInfo");
  generateTableHead("7", "File System Contents");
  getData("7", "FSContents");
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
      var raw = this.responseText;
      var raw = raw.replace(/(null)/g, "");
      var json = raw.replace(/(\]\[)/g, ",");
      var data = JSON.parse(json);
      updateTable(data, elementID, dataset);
    }
  };
  if (debug == 1) {
    if (dataset == "Ambient") {
      xhttp.open("GET", "json/readAmbient.json", true);
    } else if (dataset == "Chamber") {
      xhttp.open("GET", "json/readChamber.json", true);
    } else if (dataset == "Eeprom") {
      xhttp.open("GET", "json/readEeprom.json", true);
    } else if (dataset == "Enclosure") {
      xhttp.open("GET", "json/readEnclosure.json", true);
    } else if (dataset == "I2C") {
      xhttp.open("GET", "json/readI2C.json", true);
    } else if (dataset == "Optics") {
      xhttp.open("GET", "json/readOptics.json", true);
    } else if (dataset == "FSInfo") {
      xhttp.open("GET", "json/readFSInfo.json", true);
    } else if (dataset == "FSContents") {
      xhttp.open("GET", "json/readFSList.json", true);
    }
  } else if (debug == 2) {
    if (dataset == "Ambient") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readAmbient=" + new Date().getTime(),
        true
      );
    } else if (dataset == "Chamber") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readChamber=" + new Date().getTime(),
        true
      );
    } else if (dataset == "Eeprom") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readEeprom=" + new Date().getTime(),
        true
      );
    } else if (dataset == "Enclosure") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readEnclosure=" + new Date().getTime(),
        true
      );
    } else if (dataset == "I2C") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readI2C=" + new Date().getTime(),
        true
      );
    } else if (dataset == "Optics") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readOptics=" + new Date().getTime(),
        true
      );
    } else if (dataset == "FSInfo") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readFSInfo=" + new Date().getTime(),
        true
      );
    } else if (dataset == "FSContents") {
      xhttp.open(
        "GET",
        "http://esparasite.local/api?readFSList=" + new Date().getTime(),
        true
      );
    }
  } else {
    if (dataset == "Ambient") {
      xhttp.open("GET", "api?readAmbient=" + new Date().getTime(), true);
    } else if (dataset == "Chamber") {
      xhttp.open("GET", "api?readChamber=" + new Date().getTime(), true);
    } else if (dataset == "Eeprom") {
      xhttp.open("GET", "api?readEeprom=" + new Date().getTime(), true);
    } else if (dataset == "Enclosure") {
      xhttp.open("GET", "api?readEnclosure=" + new Date().getTime(), true);
    } else if (dataset == "I2C") {
      xhttp.open("GET", "api?readI2C=" + new Date().getTime(), true);
    } else if (dataset == "Optics") {
      xhttp.open("GET", "api?readOptics=" + new Date().getTime(), true);
    } else if (dataset == "FSInfo") {
      xhttp.open("GET", "api?readFSInfo=" + new Date().getTime(), true);
    } else if (dataset == "FSContents") {
      xhttp.open("GET", "api?readFSList=" + new Date().getTime(), true);
    }
  }
  xhttp.send();
}

// Updates the table with the new data received
function updateTable(data, elementID, dataset) {
  var tbody = document.getElementById("tableBody" + elementID);
  tbody.innerHTML = "";
  if (dataset == "Ambient") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Ambient Temperature"));
    row.appendChild(createRowCell(data["ambTempC"] + " " + "°C"));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Ambient Humidity"));
    row.appendChild(createRowCell(data["ambHumid"] + " %"));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Barometric Pressure"));
    row.appendChild(createRowCell(data["ambPres"] + " hpa"));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Altitude"));
    row.appendChild(createRowCell(trimFloat(data["ambAlt"]) + " m"));
    var row = tbody.insertRow();
  } else if (dataset == "Chamber") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Chamber Temperature"));
    row.appendChild(createRowCell(data["cmbTempC"] + " " + "°C"));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Chamber Humidity"));
    row.appendChild(createRowCell(data["cmbHumid"] + " %"));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Dew Point"));
    row.appendChild(createRowCell(trimFloat(data["cmbDewPt"]) + " " + "°C"));
    var row = tbody.insertRow();
  } else if (dataset == "Eeprom") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Last Write Timestamp"));
    row.appendChild(createRowCell(data["lastWrtTS"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("First On Timestamp"));
    row.appendChild(createRowCell(data["firstOnTS"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Recorded LED Life Seconds"));
    row.appendChild(createRowCell(data["eprmLedLS"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Recorded Screen Life Seconds"));
    row.appendChild(createRowCell(data["eprmScrLS"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Recorded Vat Life Seconds"));
    row.appendChild(createRowCell(data["eprmVatLS"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Last EEPROM Segment Written"));
    row.appendChild(createRowCell(data["lastSegAddr"]));
    var row = tbody.insertRow();
  } else if (dataset == "Enclosure") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Case Temperature"));
    row.appendChild(createRowCell(data["caseTempC"] + " " + "°C"));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Printer Life Seconds"));
    row.appendChild(createRowCell(data["printerLS"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Current LED Life Seconds"));
    row.appendChild(createRowCell(data["curLedLS"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Current Screen Life Seconds"));
    row.appendChild(createRowCell(data["curScrLS"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Current Vat Life Seconds"));
    row.appendChild(createRowCell(data["curVatLS"]));
    var row = tbody.insertRow();
  } else if (dataset == "I2C") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("SDA Pin"));
    row.appendChild(createRowCell(data["sdaPin"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("SCL Pin"));
    row.appendChild(createRowCell(data["sclPin"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Chamber Sensor"));
    row.appendChild(createRowCell(data["dhtExist"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Ambient Sensor"));
    row.appendChild(createRowCell(data["bmeExist"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Screen Sensor"));
    row.appendChild(createRowCell(data["mlxExist"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Light Sensor"));
    row.appendChild(createRowCell(data["siExist"]));
    var row = tbody.insertRow();
  } else if (dataset == "Optics") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("LED UV Index"));
    row.appendChild(createRowCell(data["uvIdx"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("LED Visible"));
    row.appendChild(createRowCell(data["visLux"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("LED IR"));
    row.appendChild(createRowCell(data["irLux"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("LED Temperature"));
    row.appendChild(createRowCell(data["ledTempC"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Screen Temperature"));
    row.appendChild(createRowCell(data["scrTempC"]));
    var row = tbody.insertRow();
  } else if (dataset == "FSInfo") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Total FileSystem Bytes"));
    row.appendChild(createRowCell(data["tFsB"]));
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Used FileSystem Bytes"));
    row.appendChild(createRowCell(data["uFsB"]));
    var row = tbody.insertRow();
  } else if (dataset == "FSContents") {
    var row = tbody.insertRow();
    row.appendChild(createRowCell("Filename"));
    row.appendChild(createRowCell("Size (Bytes)"));
    data.forEach(function (dataItem) {
      var row = tbody.insertRow();
      row.appendChild(createRowCell(dataItem["fName"]));
      row.appendChild(createRowCell(dataItem["fSize"]));
    });
  }
}

// helper method
function createHeadCell(text) {
  var th = document.createElement("th");
  th.setAttribute("colspan", "2");
  th.setAttribute("width", "100%");
  th.appendChild(document.createTextNode(text));
  return th;
}

// helper method
function createRowCell(text) {
  var td = document.createElement("td");
  td.setAttribute("width", "50%");
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

function trimFloat(value) {
  var trimmed = (Math.round((value * 100)) / 100);
  return trimmed;
}