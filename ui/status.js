
//On Page load show graphs
window.onload = function () {

};


let table = document.getElementById("dataTable");

generateTableHead(table);
getData();

function generateNetworkTableHead(table) {
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

            let row = table.insertRow();
            cell = row.insertCell();
            text = "wifi SSID";
            cell.appendChild(text);
            cell = row.insertCell();
            text = document.createTextNode(obj["ssid"]);
            cell.appendChild(text);

            let row = table.insertRow();
            cell = row.insertCell();
            text = "Signal Strength";
            cell.appendChild(text);
            cell = row.insertCell();
            text = document.createTextNode(obj[k]["rssi"]);
            cell.appendChild(text);

            let row = table.insertRow();
            cell = row.insertCell();
            text = "IP address";
            cell.appendChild(text);
            cell = row.insertCell();
            text = document.createTextNode(obj[k]["ipaddr"]);
            cell.appendChild(text);

            let row = table.insertRow();
            cell = row.insertCell();
            text = "MDNS Active";
            cell.appendChild(text);
            cell = row.insertCell();
            if (document.createTextNode(obj[k]["MDNS Status"]) == 1) {
                text = "True";
            } else {
                text = "False";
            }
            cell.appendChild(text);

            let row = table.insertRow();
            cell = row.insertCell();
            text = "MDNS Name";
            cell.appendChild(text);
            cell = row.insertCell();
            text = concat(document.createTextNode(obj[k]["mdnsN"]),".local");
            cell.appendChild(text);


            //sortTable(); //does not work properly with human readable timestamps
        } else {
        };
    };
    xhttp.open("GET", "http://esparasite.local/guiFeed?readHistory=1", true); //Handle readHistory server on ESP8266
    xhttp.send();
}