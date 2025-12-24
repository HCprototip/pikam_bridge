
console.log("-EXE graf");
Chart.defaults.global.legend.display = false;



    
var last = 0;

var last2 = 50;
des = 0
var c = 0
let t_refresh_graf

function Graf(ID_container, DS, color){
    if (t_refresh_graf == undefined){
        t_refresh_graf = setInterval(Tic, 300);
    }
    const ctx = document.getElementById(ID_container);    
    arr = new Array(100)
    data_arr = new Array(100)
    data_arr2 = new Array(100)
    var a = new Chart(ctx, {
        type: 'line',
        data: {
            labels: arr, //Array[100],
            datasets: [
                {
                    data: data_arr,
                    fill: false,
                    borderColor: color[0],
                    tension: 0.2
                },
                {
                    data: data_arr2,
                    fill: false,
                    borderColor: color[1],
                    tension: 0.2
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            onResize: handleResize,
            datasets: {
                line: {
                    pointRadius: 0
                }
            },
            scales: {
                xAxes: [{
                    ticks: {
                        display: false,
                    },
                    gridLines: {
                        color: 'red',
                        lineWidth: 1,
                        display: false
                    }
                }],
                yAxes: [{
                    
                    ticks: { 
                        fontColor: '#ad5afa'
                    },
                    gridLines: {
                        color: "#ad5afa",
                        lineWidth: 1,
                        padding: 50
                    }
                }],
                y: {
                    grid: {
                        display: true,
                        drawOnChartArea: false,
                        drawTicks: false,
                    }
                }
            },

        }
    })
    a.origen = new Array()
    for (O of DS){
        a.origen.push("item" + O)
    }
}
function Tic(n) {
    for (let ins in Chart.instances){
        let A = Chart.instances[ins]
        for (O in A.origen){
            // console.log(Number(document.getElementById(A.origen[O]).innerHTML))
            A.data.datasets[O].data.push(Number(document.getElementById(A.origen[O]).innerHTML))
        }
        if (A.data.labels.length > 100) {
            A.data.datasets[0].data.shift()
            A.data.datasets[1].data.shift()
            A.data.labels.shift()
        }
        A.data.labels.push(c++)
        A.update();
    }
}
// DESTRUIMOS TODOS LOS GRAFS
function DeleteGrafs(){
    clearInterval(t_refresh_graf);
    t_refresh_graf = undefined;
    for (let ins in Chart.instances) {
        Chart.instances[ins].destroy();
    }
}





function ChartResice(){
    DeleteGrafs();
    for (let id in Chart.instances) {
        //Chart.instances[id].resize(200, 20)
        Chart.instances[id].resize();
    }
}

const handleResize = (chart) => {
    chart.resize();
    }

// function Tic(n) {
//     des += Math.random() * 0.5;
//     last = last + Math.sin(des) * (Math.random() * 0.5);

//     last2 = last2 + Math.sin(des) * (Math.random() * 0.5);
//     //last2 = Number(document.getElementById("valor").innerHTML);

//     data_arr.push(last2)
//     n = last2 + last * 100
//     data_arr2.push(n)
//     for (let ins in Chart.instances){
//         A = Chart.instances[ins]
//         console.log(A)
//     }
//     if (A.data.labels.length > 100) {
//         data_arr.shift()
//         data_arr2.shift()
//         A.data.labels.shift()
//     }
//     A.data.labels.push(c++)
//     A.update();

// }