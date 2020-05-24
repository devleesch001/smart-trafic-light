AWS.config.region = 'eu-west-2'; // Région
AWS.config.credentials = new AWS.CognitoIdentityCredentials({
    IdentityPoolId: 'eu-west-2:cb155f30-996a-4abb-b2a7-ef5bcb56446c',
});

let dynamodb = new AWS.DynamoDB({apiVersion: '2012-08-10'});


/* Create the context for applying the chart to the HTML canvas */
// var ctx = $("#graph").get(0).getContext("2d");

/* Set the options for our chart */
window.chartColors = {
    red: 'rgb(215,0,0)',
    orange: 'rgb(255, 159, 64)',
    yellow: 'rgb(255, 205, 86)',
    green: 'rgb(75,192,87)',
    blue: 'rgb(54, 162, 235)',
    purple: 'rgb(153, 102, 255)',
    grey: 'rgb(201, 203, 207)'
};

let chartData = {
    labels: ['00:00', '01:00', '02:00', '03:00', '04:00', '05:00', '06:00', '08:00', '09:00', '10:00', '11:00', '12:00', '13:00', '14:00', '15:00', '16:00', '17:00', '18:00', '19:00', '20:00', '21:00', '22:00', '23:00'],
    datasets: [{
        type: 'bar',
        label: 'Red',
        backgroundColor: window.chartColors.red,
        data: [0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 20, 25, 20, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    }, {
        type: 'bar',
        label: 'Green',
        backgroundColor: window.chartColors.green,
        data: [0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 20, 25, 20, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    }, {
        type: 'line',
        label: 'Total',
        borderColor: window.chartColors.yellow,
        borderWidth: 2,
        fill: false,
        data: [0, 0, 0, 0, 0, 0, 0, 5, 7, 7, 22, 27, 22, 7, 7, 5, 0, 0, 0, 0, 0, 0, 0, 0]
    }]
};

let ctx = document.getElementById('Chart').getContext('2d');

let chart = new Chart(ctx, {
    // The type of chart we want to create
    type: 'bar',
    // The data for our dataset
    data: chartData,
    ticks: {
        min: 0
    },
    // Configuration options go here
    options: {
        scales: {
            yAxes: [{
                ticks: {
                    min: 0,
                }
            }]
        },
        responsive: true,
        title: {
            display: true,
            text: 'Chart.js Combo Bar Line Chart'
        },
        tooltips: {
            mode: 'index',
            intersect: true
        }
    }
});

function hourFormatting(Time, sub) {
    let hours = new Date(Time).getHours();
    hours -= sub;
    if (hours < 0) {
        hours += 24;
    }

    if (hours < 10) {
        hours = "0" + hours;
    }

    return hours + ":00"

}

function hoursArray(Time) {
    let Form = []
    for (let i = 23; i >= 0; i--) {
        Form.push(hourFormatting(Time * 1000, i));
    }

    return Form;
}

function getData(TimeStart, TimeStop, callback) {

    let params = {
        TableName: 'db-smart-trafic-light',
        ExpressionAttributeValues: {
            ":ts": {
                N: TimeStop.toString()
            },
            ":ta": {
                N: TimeStart.toString()
            }
        },
        FilterExpression: "PosixTime <= :ts AND PosixTime >= :ta"
    };

    dynamodb.scan(params, function (err, data) {
        callback(err, data);
    });
}

initData();

function initData() {
    let date = new Date();
    date.setHours(date.getHours() + 1);
    date.setMilliseconds(0);
    date.setSeconds(0);
    date.setMinutes(0);

    let TimeStamp = date.getTime() / 1000 | 0;

    let TimeStart = (TimeStamp - (24 * 60 * 60));

    chart.data.labels = hoursArray(TimeStamp);
    chart.update();

    let colorValue = {"RED": [], "GREEN": [], "TOTAL": []};
    for (let i = 0; i < 24; i++) {
        colorValue["RED"].push(0);
        colorValue["GREEN"].push(0);
        colorValue["TOTAL"].push(0);
    }
    console.log(colorValue);


    getData(TimeStart, TimeStamp, function (err, data) {
        if (err) {
            console.error(err);
            return null;
        } else {
            console.log(data);
            for (let i in data['Items']) {
                let color = data['Items'][i]['Color']['S'];
                let PosixTime = data['Items'][i]['PosixTime']['N'];


                let TimeStart = (TimeStamp - (24 * 60 * 60));

                if (color === "RED") {
                    for (let j = 0; j < 24; j++) {
                        let TimeRule = TimeStart + (60 * 60);
                        if (TimeStart < PosixTime && PosixTime < TimeRule) {
                            colorValue["RED"][j]++;
                            colorValue["TOTAL"][j]++;
                        }

                        TimeStart = TimeRule;
                    }
                } else if (color === "GREEN") {
                    for (let j = 0; j < 24; j++) {
                        let TimeRule = TimeStart + (60 * 60);
                        if (TimeStart < PosixTime && PosixTime < TimeRule) {
                            colorValue["GREEN"][j]++;
                            colorValue["TOTAL"][j]++;

                        }
                        TimeStart = TimeRule;
                    }
                }
            }
            chart.data.datasets[0].data = colorValue["RED"];
            chart.data.datasets[1].data = colorValue["GREEN"];
            chart.data.datasets[2].data = colorValue["TOTAL"];
            chart.update();
        }
    });
}


/* Makes a scan of the DynamoDB table to set a data object for the chart */
function updateData() {
    let date = new Date();
    date.setHours(date.getHours() + 1);
    date.setMilliseconds(0);
    date.setSeconds(0);
    date.setMinutes(0);

    let TimeStamp = date.getTime() / 1000 | 0;
    let TimeStart = (TimeStamp - (60 * 60));

    if (chart.data.labels[23] !== hourFormatting(date, 0)) {
        initData();
    } else {
        getData(TimeStart, TimeStamp, function (err, data) {
            if (err) {
                console.error(err);
                return null;
            } else {
                console.log(data);

                let red = 0;
                let green = 0;

                for (let i in data['Items']) {
                    let color = data['Items'][i]['Color']['S'];

                    if (color === "RED") {
                        red++;
                    } else if (color === "GREEN") {
                        green++;
                    }
                    chart.data.datasets[0].data[23] = red;
                    chart.data.datasets[1].data[23] = green;
                    chart.data.datasets[2].data[23] = red + green;
                    chart.update();

                }
            }
        });
    }
}

$(function () {
    updateData();
    $.ajaxSetup({cache: false});
    setInterval(updateData, 3000);
});
