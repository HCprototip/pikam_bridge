
console.log("-EXE scripts");
updateOnlineStatus();

let noSleep = new NoSleep();
document.addEventListener('click', function enableNoSleep() {
    document.removeEventListener('click', enableNoSleep, false);
    noSleep.enable();
}, false);


let connectat = false;
var mode_visu = 0;
var tipo_contenido = "W";
var remolque_actual = 0;
var VisuFons = 0;
var type_cent = 0;


document.getElementById("P_content").style.display = "none";
// document.getElementById("T_content").style.display = "none"; // OBSOLETO
document.getElementById("E_content").style.display = "none";

window.addEventListener('DOMContentLoaded', function() { //DOMContentLoaded
    window.addEventListener('online', updateOnlineStatus);
    window.addEventListener('offline', updateOnlineStatus);
    updateOnlineStatus();
    
    // Ocultar botón BLE si no está disponible
    if (!navigator.bluetooth) {
        console.error('Web Bluetooth API no disponible en este navegador');
        document.getElementById("btn_connect_ble").style.display = "none";
        alert('Tu navegador no soporta Web Bluetooth. Por favor usa Chrome, Edge o Opera en Android/Windows/macOS.');
    }
});
window.addEventListener("resize", Resize);

document.addEventListener('webkitfullscreenchange', function(e) {
    screenchange();
}, false);
document.addEventListener('mozfullscreenchange', function(e) {
    screenchange();
}, false);
document.addEventListener('fullscreenchange', function(e) {
    screenchange();
}, false);

var msg_timerOut = setTimeout(msg_OUT, 3000);
var t_ble_watch = setInterval(ble_watch, 5000);
var t_reconnect;

function screenchange() {
    document.getElementById("minimiza").style.display = isFullScreen() ? "block" : "none";
    E_Image_OK(mode_visu);
}
function Reset_msg_tOUT() {
    clearTimeout(msg_timerOut);
    msg_timerOut = setTimeout(msg_OUT, 5000);
}
function msg_OUT() {
    console.log("TIME OUT");
    document.getElementById("conn_punt").style.animation = "stop";
    let obj = new Object();
    
    obj.cont = "W";
    obj.txt0 = "SIN DATOS";
    obj.txt1 = "";
    obj.pes = "-----";
    obj.img = "";
    Rebut(JSON.stringify(obj));
    connectat = false;
}
function ble_watch() {
    // Monitorear estado BLE
    if (!IsBLEConnected() && connectat) {
        EstatConn(false);
    }
}
function updateOnlineStatus(event) {
    // Para BLE no dependemos de navigator.onLine
    console.log("Navegador online:", navigator.onLine);
    
    if (!navigator.bluetooth) {
        console.error("Web Bluetooth no disponible");
        return;
    }
}
function Reload() {
    console.log("RELOAAAAD");
    location.reload();
}
function Resize(){
    ChartResice();
    E_Image_OK(mode_visu);
}
function DebugHTML(str) {
    document.getElementById("debug").innerHTML = str;
}
var  sw = true;
function Boto(quin) {
    if (quin > 0) {
        ws.send("P" + quin);
    }
    console.log("P" + quin);
    if (!navigator.onLine) {
        Reload();
    }
    if (!isFullScreen()) OpenFullscreen();
    if (quin == 0){
        sw = !sw;
        //document.getElementById("item2").style.animation = sw? "alarm 0.5s ease-in-out infinite alternate" : "stop";
    }
}
function BotoF(quin) {
    ws.send("F" + quin);
    console.log("F" + quin);
    if (!navigator.onLine) {
        Reload();
    }
    if (!isFullScreen()) OpenFullscreen();
}
function Switch(que){
    const obj = {
        MV: que
    }
    ws.send(JSON.stringify(obj));
}
function isFullScreen() {
    // Safari?
    // if (typeof(document.fullscreenElement) === 'undefined') {
    //     if (document.webkitCurrentFullScreenElement == null) return false;
    // }
    // if (document.fullscreenElement == null) return false;
    // return true;
    return document.fullscreenElement || document.webkitIsFullScreen || document.mozFullScreen;
}
function EstatConn(estat = false) {
    console.log("Estat:" + estat);
    connectat = estat;
    document.getElementById("conn_OK").style.display = estat ? "block" : "none";
    document.getElementById("conn_KO").style.display = estat ? "none" : "block";
    document.getElementById("conn_punt").style.animation = estat ? "animacio_punt_conn 2s infinite linear" : "stop";
    if (!estat) {
        let obj = new Object();
        obj.txt0 = "SIN CONEXION";
        obj.txt1 = "";
        obj.img = "MC_sinconex";
        Rebut(JSON.stringify(obj));
    }
}
function Minimiza() {
    console.log("MINI");
    if (document.exitFullscreen) document.exitFullscreen();
    else if (document.webkitExitFullscreen) document.webkitExitFullscreen();
    else if (document.mozCancelFullScreen) document.mozCancelFullScreen();
    else if (document.msExitFullscreen) document.msExitFullscreen();
}
function Rebut(data) {
    console.log("REBUT" + data);
    var msg = JSON.parse(data);
    if (msg == null) return;
    
    // Manejar recepción de configuración actual
    if (msg.GET_RC !== undefined) {
        document.getElementById('config_net').value = msg.GET_RC.NET || '';
        document.getElementById('config_adress').value = msg.GET_RC.ADRESS || '';
        document.getElementById('config_freq').value = msg.GET_RC.CHANNEL || '';
        document.getElementById('config_response').style.display = 'none';
        // No return - permitir procesar resto del JSON
    }
    
    // Manejar respuesta de configuración guardada
    if (msg.SET_RC_RESP !== undefined) {
        if (msg.SET_RC_RESP === true) {
            showConfigResponse('✓ Configuración guardada correctamente', true);
            setTimeout(() => {
                closeConfigModal();
            }, 2000);
        } else {
            showConfigResponse('✗ Error al guardar configuración', false);
        }
        // No return - permitir procesar resto del JSON
    }
    
    if (msg.txt_bot != undefined) {
        for (c in msg.txt_bot) {
            var txt = msg.txt_bot[c];
            document.getElementById(c).innerHTML = txt;
            document.getElementById(c).style.display = txt == "" ? "none" : "block";
        }
    }        
    if (msg.cont != undefined) {
        tipo_contenido = msg.cont;
        document.getElementById("P_content").style.display = tipo_contenido == "P" ? "block" : "none";
        document.getElementById("W_content").style.display = tipo_contenido == "W" ? "block" : "none";            
        document.getElementById("E_content").style.display = tipo_contenido == "E" ? "block" : "none";                    
        // document.getElementById("L_content").style.display = msg.cont == "L" ? "block" : "none";
        document.getElementById("T_content").style.display = tipo_contenido == "T" ? "block" : "none";  // OBSOLTE
        tipo_contenido = msg.cont;
        // TESTING RESETEJAR GRAFS ---------
        if (tipo_contenido != "E"){
            DeleteGrafs();
        }
    }
    if (msg.check_value != undefined){
        document.getElementById("check_ext").checked = msg.check_value;
    }
    if (msg.items != undefined) {
        it = TC.type_visu[mode_visu].items;
        c=0;        
            var alarma;
            var txt;
        for (c in it) {                         ////////////////// COSETES A MILLORAR  
            if (it[c].tipo == "total") break;  /////////// HAURIEM DE FER-HO DIFERENT...
            target = document.getElementById("item" + c);
            if (it[c].tipo == "neto") target = document.getElementById("neto"); ///// XAPUUSAAA!!!
            if (target == null) continue;
            switch (it[c].tipo){ 
                case "disp": 
                    valor = msg.items["item" + it[c].data];
                    alarma = valor[0];
                    txt = valor.slice(1); 
                    var anima = "none";
                    switch (alarma){
                        case 'A':
                            anima = "none";
                            break;
                        case 'B':
                            anima = "alarmB 0.8s ease-in-out infinite alternate";
                            break;
                        case 'C':
                            anima = "alarmC 0.5s ease-in-out infinite alternate";                                
                            break;
                    }
                    target.innerHTML = txt;                        
                    target.style.animation = anima; //sw? "alarm 0.5s ease-in-out infinite alternate" : "stop";
                break;
                case "neto": 
                    valor = msg.items["item" + it[c].data];
                    alarma = valor[0];
                    txt = valor.slice(1);
                    txt = valor.substring(1, 6);
                    var anima = "none";
                    switch (alarma){
                        case 'A':
                            anima = "none";
                            break;
                        case 'B':
                            anima = "alarmB 0.8s ease-in-out infinite alternate";
                            break;
                        case 'C':
                            anima = "alarmC 0.5s ease-in-out infinite alternate";                                
                            break;
                    }
                    target.innerHTML = txt;                        
                    target.style.animation = anima; //sw? "alarm 0.5s ease-in-out infinite alternate" : "stop";
                    break;
                case "F_Estiba":
                    valor = msg.items["item"+it[c].data];
                    txt = valor.slice(1);
                    value = -(parseInt(txt)) / 200;
                    target.style = "animation-delay: " + value + "s";
                    target.parentElement.childNodes[3].style = "transform: rotate(" + -(value+0.5)*18 + "deg)";
                    break;
                case "L_Estiba":
                    valor = msg.items["item6"];
                    txt = valor.slice(1); 
                    value = -(parseInt(txt)) / 200;
                    target.style = "animation-delay: " + value + "s";
                    break;
                case "Cert":
                    DebugHTML("CAL/VER: CEC10522: NOK");
                    console.log("que");
                    break;
            } 
        }
    }
    if (msg.printer != undefined) {
        document.getElementById("alt").innerHTML = msg.alb;
        document.getElementById("mat").innerHTML = msg.mat;
        return;
    }
    if (!connectat & (msg.img == undefined)) { ////////////////////////////////////////////////// PER RETORN DE 'SIN DATOS'
        msg.txt0 = "";
        msg.img = "";
    }
    if (msg.img != undefined) {
        if (msg.img == "") {
            document.getElementById("centerfix").innerHTML = "<div class='valor' id='valor'> </div> ";
            document.getElementById("ileft").style.display = "block";
            if (msg.txt0 == "") {
                document.getElementById("valor").className = "valor mega";
            } else {
                document.getElementById("valor").className = "valor petit";
            }
        } else {
            if (msg.img != "NoFoto") {
                document.getElementById("centerfix").innerHTML = "<img class='imagen' id='imatge' src='img/" + msg.img + ".jpg'>";
            } else {
                document.getElementById("centerfix").innerHTML = "";
            }
            document.getElementById("ileft").style.display = "none";
        }
    }
    if (msg.pes != undefined) {   /////////////////////////////// MIRA A VER ;)
        if (tipo_contenido == "E"){
            //////////////////////////////////////////////// 1era integració Certtificat
            // interpreto aqui pq si visualitzem pes estem a pantalla inicial
            // DebugHTML("CER/CAL: CDC10589:"  + (parseInt(msg.items.item10.slice(1)) == 1?  "OK" : "NOK"))
            if (document.getElementById("pesTotal") != null){
                document.getElementById("pesTotal").innerHTML = msg.pes;
            }
        } else {
            document.getElementById("valor").innerHTML = msg.pes;
        }
        /* document.getElementById("visor2").innerHTML = "
                   <div>14500_</div>"; */
    }
    if (msg.txt0 != undefined) {
        document.getElementById("txt0").innerHTML = msg.txt0;
    }
    if (msg.txt1 != undefined) {
        document.getElementById("txt1").innerHTML = msg.txt1;
    }
    if (msg.txt2 != undefined) {
        document.getElementById("txt_unidades").innerHTML = msg.txt2;
    }
    if (msg.al1 != undefined) {
        document.getElementById("alm1").style.backgroundColor = msg.al1 ? "#9C0000" : "#4CAF50";
        document.getElementById("alm1").className = msg.al1 ? "footer_alarma_on" : "footer_alarma_off";
    }
    if (msg.al2 != undefined) {
        document.getElementById("alm2").style.backgroundColor = msg.al2 ? "#9C0000" : "#4CAF50";
        document.getElementById("alm2").className = msg.al2 ? "footer_alarma_on" : "footer_alarma_off";
    }
    if (msg.tpe != undefined) {
        document.getElementById("txt_tipo_peso").innerHTML = msg.tpe;
        if (msg.tpe != "") {
            document.getElementById("tpe").innerHTML = msg.tpe;
        } // dependera del msg.tpe
    }
    if (msg.rem != undefined) {
        document.getElementById("rem").innerHTML = "REM" + msg.rem;
        remolque_actual = msg.rem;
    }
    if (msg.foot != undefined) {
        document.getElementById("footer").className = msg.foot == "si" ? "footer dins" : "footer fora";
    }
    if (msg.alb != undefined) {
        document.getElementById("alb").value = msg.alb;
    }
    if (msg.mat != undefined) {
        document.getElementById("mat").value = msg.mat;
    }
    if (msg.dt != undefined) {
        var D = new Date((msg.dt + _getTimeZoneOffsetInMs()) * 1000);
        // D = ;
        console.log(D.toLocaleString());
        console.log(D.toISOString());
        document.getElementById("dt").value = D.toISOString().slice(0, 16);
        console.log(msg.dt);
    }
    if (msg.tick != undefined) {
        document.getElementById("tick").textContent = msg.tick
    }
    if (msg.typeCent != undefined){
        TypeCent(msg.typeCent); 
    }
    if (msg.Etype != undefined){  
        Etype(msg.Etype); //////////////////////////     TD: YA LO PONDRE DONDE TOQUE
    }
    if (msg.VisuFons != undefined){
        VisuFons = msg.VisuFons; 
    }
}
function ActualizaDatos() {
    const obj = {
        alb: document.getElementById("alb").value,
        mat: document.getElementById("mat").value,
        dt: (Date.parse(document.getElementById("dt").value) / 1000).toFixed()
    };
    ws.send(JSON.stringify(obj));
}
function EnviaDatos() {
    const obj = {
        dt: (Date.now() / 1000).toFixed()
    }
    ws.send(JSON.stringify(obj));
}
function _getTimeZoneOffsetInMs() {
    return new Date().getTimezoneOffset() * -60;
}

// ============= CONFIGURACIÓN TARVOS =============
function openConfigModal() {
    document.getElementById('configModal').style.display = 'flex';
    document.getElementById('config_response').style.display = 'none';
    
    // Pedir configuración actual del Tarvos
    showConfigResponse('Cargando configuración actual...', true);
    const obj = { GET_RC: true };
    ws.send(JSON.stringify(obj));
}

function closeConfigModal() {
    document.getElementById('configModal').style.display = 'none';
}

function sendConfig() {
    const net = document.getElementById('config_net').value.trim().toUpperCase();
    const adress = document.getElementById('config_adress').value.trim().toUpperCase();
    const freq = parseFloat(document.getElementById('config_freq').value);

    // Validaciones
    if (!net || !adress || !freq) {
        showConfigResponse('Por favor, completa todos los campos', false);
        return;
    }

    // Validar formato hexadecimal
    if (!/^[0-9A-F]{1,2}$/i.test(net) || !/^[0-9A-F]{1,2}$/i.test(adress)) {
        showConfigResponse('Red y Nodo deben ser valores hexadecimales (00-FF)', false);
        return;
    }

    // Validar rango de frecuencia
    if (freq < 863.0 || freq > 870.0) {
        showConfigResponse('Frecuencia debe estar entre 863.0 y 870.0 MHz', false);
        return;
    }

    // Formatear valores con padding si es necesario
    const netFormatted = net.padStart(2, '0');
    const adressFormatted = adress.padStart(2, '0');

    // Crear JSON para enviar
    const obj = {
        SET_RC: {
            NET: netFormatted,
            ADRESS: adressFormatted,
            CHANNEL: freq
        }
    };

    console.log('Enviando configuración:', obj);
    ws.send(JSON.stringify(obj));
    showConfigResponse('Enviando configuración...', true);
}

function showConfigResponse(message, isSuccess) {
    const responseDiv = document.getElementById('config_response');
    responseDiv.textContent = message;
    responseDiv.style.display = 'block';
    responseDiv.style.background = isSuccess ? '#4CAF50' : '#f44336';
    responseDiv.style.color = 'white';

    // Auto-ocultar después de 3 segundos si es éxito
    if (isSuccess) {
        setTimeout(() => {
            responseDiv.style.display = 'none';
        }, 3000);
    }
}

// Validación en tiempo real para campos hexadecimales
document.addEventListener('DOMContentLoaded', function() {
    const hexInputs = ['config_net', 'config_adress'];
    hexInputs.forEach(id => {
        const input = document.getElementById(id);
        if (input) {
            input.addEventListener('input', function() {
                this.value = this.value.toUpperCase().replace(/[^0-9A-F]/g, '');
            });
        }
    });
});

