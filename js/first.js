console.log("---SCRIPT Firts")

async function cargaI(obj){
    var I = document.getElementById(obj[0]);
    let promise = new Promise((resolve, reject) => {
        console.log("cargo I " + obj[0])
        I.onload = function(){resolve("I Cargado: ")}
        I.src = obj[1];
      })
    MuestraCarga(obj[0]);
    let us = await promise;
    console.log(us + obj[0])
    if (It != 0){
        cargaI(cont[--It]);
    } else {
        if (Its != 0) cargaS("js/" + scripts[--Its])
    }
}
async function cargaS(scriptFile){
    var head = document.getElementsByTagName('head')[0];
    let script = document.createElement('script')
    script.type = 'text/javascript';
    head.appendChild(script);
    let promise = new Promise((resolve, reject) => {
        script.onload = function(){resolve("S Cargado: ")}
        script.src = scriptFile
      })
    MuestraCarga(scriptFile);
    console.log("Cargo S " + scriptFile)
    let us = await promise;    
    console.log(us + scriptFile)
    
    if (Its != 0){
        setTimeout(() => {
            cargaS("js/" + scripts[--Its]);
          }, 20);
    } else {
        TodoCargado();
    }

}
var cont = [
    ["img_minimiza", "img/minimiza.svg"],
    ["conn_OK", "img/CONN_OK.svg"],
    ["conn_KO", "img/CONN_KO.svg"],
    ["EstibaFrontWheels_svg", "img/EstibaFrontWheels.svg"],
    ["EstibaFrontTruck_svg", "img/EstibaFrontTruck.svg"],
    ["EstibaFixed_svg", "img/EstibaFixed.svg"],
    ["E_image","img/fondotest4.jpg"]
    // ["actualiza_svg", "img/actualiza.svg"]  // està al CSS :(
]
var It = cont.length;
if (It != 0){
    cargaI(cont[--It]);
}
function MuestraCarga(txt, progreso){
    var perCentProgres = (++progresIts/TotalIt  )*95;
    document.getElementById("loadText").innerHTML = txt;
    document.getElementById("loadBar").style = "width:"+perCentProgres+"%";
}
function TodoCargado(){
    document.getElementById("L_content").style.display = "none";
}
var scripts = [
    "scripts.js", "ws.js", "type_cent.js", "graf.js", "Chart.js",  "NoSleep.js"// <-- es carreguen en ordre invers <--
]
var Its = scripts.length;
var TotalIt = It + Its;
var progresIts = 0;
