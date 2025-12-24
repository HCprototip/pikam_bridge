
console.log("-EXE type_cent");
////Ojikjijijij
////////////////// PASSAT DE TYPE_CENT.js
// ESTO FUNCIONA EN SERVIDOR NO EN LOCAL
    
var TC;   // objecte que contindrà les dades del JSON typeCent
var xhttp = new XMLHttpRequest();


function TypeCent(n){
    str = "typeCent/type_cent_" + n + ".json"
    console.log("TypeCent " + str)  
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            TC = JSON.parse(xhttp.responseText);
            DibujaFrame(0);
        }
    }
    xhttp.open("GET", str, true);
    xhttp.send();
    type_cent = n;
    //mode_visu = n; ////////////////////////////////////// TODO: QUE FEM AQUI???
}
function Etype(type){
    DibujaFrame(type);
    eType = type;
}
function ActualizaFrame(frame_tipo){
    
}
function DibujaFrame(type){
    // Si tiene imagen, empezaremos cargandola, nos marcará el tamaño/relacion para la posicion de los elementos
    var I = document.getElementById("E_image");
    I.onload = function() {
        setTimeout(function() {
            console.log("PintaElementos");
            E_Image_OK(type);
        }, 1000);
    }
// #ifdef MEGATRAILER
//     if (mode == 0){
//     I.src = TC.type_visu[mode].img[VisuFons-1];
//     console.log("VisuFons:" + VisuFons);
//     } else {        
//         I.src = TC.type_visu[mode].img;
//     }
// #else
    I.src = TC.type_visu[type].img;
// #endif
    //////////////////////////////////////////////////// PARA CAMBIO DE IMAGEN DE FONDO EN MEGATRAILER SEGUN REMOLQUES ENGANCHADOS:::

    mode_visu = type;
}
function E_Image_OK(cent) {
    console.log("E_Image_OK");
    let C = TC.type_visu[cent];
    let it=0;
    document.getElementById("E_title").innerHTML = C.titol;
    let coords = document.getElementById("E_image").getBoundingClientRect();
    document.getElementById("items").innerHTML = "";
    for (let o in TC.type_visu[cent].items){
        let obj = TC.type_visu[cent].items[o];
        let tag = document.createElement('div');
        tag.innerHTML = "";
        tag.style.display = "block";
        // tag.id = obj.id != undefined? obj.id : "item" + it;
        var itemID = obj.id != undefined? obj.id : "item" + String(obj.data);
        switch (obj.tipo) {
            case "total":
                tag = document.getElementById("total_dummy").cloneNode(true);
                tag.firstElementChild.firstElementChild.id = itemID;
                tag.childNodes[3].childNodes[3].innerHTML = "TOTAL";
            break;
            case "neto":
                tag = document.getElementById("total_dummy").cloneNode(true);
                tag.firstElementChild.firstElementChild.id = itemID;
                tag.childNodes[3].childNodes[3].innerHTML = "NETO";     
            break;
            case "disp":
                tag = document.getElementById("I_dis_7seg_dummy").cloneNode(true);
                tag.style.display = "block";
                tag.childNodes[3].innerHTML = obj.texto;
                tag.childNodes[5].id = itemID;
                num = tag.children[0];
                if (obj.n != undefined) {
                    num.innerHTML = obj.n;
                    num.style.display = "block";
                } else {
                    num.style.display = "none";
                }
            break;
            case "Estiba": 
                tag = document.getElementById("EstibaFixed").cloneNode(true);
                //tag.childNodes[5].id= itemID;
            break;
            // case "F_Estiba": 
            //     tag = document.getElementById("Estiba_front").cloneNode(true);
            //     tag.childNodes[5].id= itemID;
            // break;
            // case "L_Estiba": 
            //     tag = document.getElementById("Estiba_left").cloneNode(true);
            //     tag.childNodes[3].id= itemID;
            // break;
            case "graf":
                tag = document.getElementById("Chart_dummy").cloneNode(true);
                tag.firstElementChild.id = itemID
            break
        }
        tag.style.color = obj.color;
        if (obj.medida != undefined) tag.style.transform  = "scale("+ obj.medida + ")";
        tag.style.position = "absolute";
        tag.style.left = coords.left + coords.width/100*obj.posX + "px";
        tag.style.top = coords.top - 80 + coords.height/100*obj.poxY + "px"; 
        document.getElementById("items").appendChild(tag);
        // HEM DE REGISTRAR els charts despres d'instanciar
        if (obj.tipo == "graf"){
            Graf(itemID, obj.dataSet, obj.color)
        }
        it++;
    }
    
}
// function _Image_OK(cent) {
//     cent = 0;
//     console.log("E_Image_OK");
//     let C = TC.type_visu[cent];
//     let it=0;
//     document.getElementById("E_title").innerHTML = C.titol;
//     let coords = document.getElementById("E_image").getBoundingClientRect();
//     document.getElementById("items").innerHTML = "";
//     for (let o in TC.type_visu[cent].items){
//         let obj = TC.type_visu[cent].items[o];
//         let tag = document.createElement('div');
//         tag.innerHTML = "";
//         tag.style.display = "block";
//         tag.id = obj.id != undefined? obj.id : "item" + it;
//         switch (obj.tipo) {
//             case "total":
//                 tag = document.getElementById("total_dummy").cloneNode(true);
//                 tag.firstElementChild.firstElementChild.id = obj.id;
//                 tag.childNodes[3].childNodes[3].innerHTML = "TOTAL";
//             break;
//             case "neto":
//                 tag = document.getElementById("total_dummy").cloneNode(true);
//                 tag.firstElementChild.firstElementChild.id = "item" + it;
//                 tag.childNodes[3].childNodes[3].innerHTML = "NETO";     
//             break;
//             case "totalX":
//                 tag = document.getElementById("total_dummy").cloneNode(true);
//                 tag.firstElementChild.firstElementChild.id = obj.id;
//                 tag.childNodes[3].childNodes[3].innerHTML = "TOTAL";
//                 tag.childNodes[1].childNodes[1].innerHTML = obj.valor;
//             break;
//             case "netoX":
//                 tag = document.getElementById("total_dummy").cloneNode(true);
//                 tag.firstElementChild.firstElementChild.id = "item" + it;
//                 tag.childNodes[3].childNodes[3].innerHTML = "NETO";
//                 tag.childNodes[1].childNodes[1].innerHTML = obj.valor;
//             break;
//             case "disp":
//                 tag = document.getElementById("I_dis_7seg_dummy").cloneNode(true);
//                 tag.style.display = "block";
//                 tag.childNodes[3].innerHTML = obj.texto;
//                 tag.childNodes[5].id = "item" + it;
//                 num = tag.children[0];
//                 if (obj.n != undefined) {
//                     num.innerHTML = obj.n;
//                     num.style.display = "block";
//                 } else {
//                     num.style.display = "none";
//                 }
//             break;
//             case "F_Estiba": 
//                 tag = document.getElementById("Estiba_front").cloneNode(true);
//                 tag.childNodes[5].id= "item" +it;
//             break;
//             case "L_Estiba": 
//                 tag = document.getElementById("Estiba_left").cloneNode(true);
//                 tag.childNodes[3].id= "item" +it;
//             break;
//         }
//         if (obj.medida != undefined) tag.style  = "transform: scale("+ obj.medida + ")"; 
//         tag.style.color = obj.color;
//         tag.style.position = "absolute";
//         tag.style.left = coords.left + coords.width/100*(obj.posX) + "px";
//         tag.style.top = (coords.top +100)+ (coords.height/100*obj.poxY) + "px";
//         console.log(coords.top);
//         document.getElementById("items").appendChild(tag);
//         it++;
//     }
// }      